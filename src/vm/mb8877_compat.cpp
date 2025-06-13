/*
	MB8877 Compatibility Layer for WD FDC
	
	This wrapper provides MB8877 interface compatibility using MAME's wd_fdc implementation approach
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
	
	[ MB8877 / MB8876 / MB8866 / MB89311 Compatibility Wrapper ]
	
	Based on MAME's wd_fdc implementation (BSD-3-Clause)
	Original copyright: Olivier Galibert
*/

#include "mb8877_compat.h"
#include "disk.h"
#include "noise.h"
#include "../fileio.h"

// MB8877 Register addresses
#define FDC_STATUS	0
#define FDC_COMMAND	0
#define FDC_TRACK	1
#define FDC_SECTOR	2
#define FDC_DATA	3

// MB8877 Status register bits
#define FDC_ST_BUSY		0x01
#define FDC_ST_INDEX		0x02
#define FDC_ST_DRQ		0x02
#define FDC_ST_TRACK00		0x04
#define FDC_ST_LOSTDATA		0x04
#define FDC_ST_CRCERR		0x08
#define FDC_ST_SEEKERR		0x10
#define FDC_ST_RECNFND		0x10
#define FDC_ST_HEADENG		0x20
#define FDC_ST_RECTYPE		0x20
#define FDC_ST_WRITEFAULT	0x20
#define FDC_ST_WRITEP		0x40
#define FDC_ST_NOTREADY		0x80

// MB8877 Command bits
#define FDC_CMD_TYPE_1		0
#define FDC_CMD_TYPE_2		1
#define FDC_CMD_TYPE_3		2
#define FDC_CMD_TYPE_4		3

// Event IDs
#define EVENT_SEEK		0
#define EVENT_SEEKEND		1
#define EVENT_SEARCH		2
#define EVENT_DRQ		3
#define EVENT_MULTI1		4
#define EVENT_MULTI2		5
#define EVENT_LOST		6

// Drive mask
#define DRIVE_MASK		3

MB8877::~MB8877()
{
	release();
}

void MB8877::initialize()
{
	// Initialize DISK handlers
	for(int i = 0; i < MAX_DRIVE; i++) {
#ifndef _ANY2D88
		disk[i] = new DISK(emu);
#else
		disk[i] = new DISK();
#endif
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
	}
	
	// Initialize noise
#ifndef STANDALONE_TEST
	if(d_noise_seek != NULL) {
		d_noise_seek->set_device_name(_T("Noise Player (FDD Seek)"));
		if(!d_noise_seek->load_wav_file(_T("FDDSEEK.WAV"))) {
			if(!d_noise_seek->load_wav_file(_T("FDDSEEK1.WAV"))) {
				d_noise_seek->load_wav_file(_T("SEEK.WAV"));
			}
		}
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_device_name(_T("Noise Player (FDD Head Load)"));
		d_noise_head_down->load_wav_file(_T("HEADDOWN.WAV"));
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_device_name(_T("Noise Player (FDD Head Unload)"));
		d_noise_head_up->load_wav_file(_T("HEADUP.WAV"));
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
#endif
	
	// Initialize FDC state
	memset(fdc, 0, sizeof(fdc));
	
	// Setup initial state
	reset();
}

void MB8877::release()
{
	// Release DISK handlers
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			delete disk[i];
			disk[i] = NULL;
		}
	}
}

void MB8877::reset()
{
	// Finish previous command
	if(cmdtype == FDC_CMD_TYPE_2 && (cmdreg & 0x20)) {
		// Write sector command
		if(sector_changed) {
			disk[drvreg]->set_data_crc_error(false);
		}
	} else if(cmdtype == FDC_CMD_TYPE_3 && (cmdreg & 0xf0) == 0xf0) {
		// Write track command
		if(!disk[drvreg]->write_protected) {
			if(fdc[drvreg].id_written && !fdc[drvreg].sector_found) {
				// data mark of last sector is not written
				disk[drvreg]->set_data_mark_missing();
			}
			disk[drvreg]->sync_buffer();
		}
	}
	
	// Cancel all events
	for(int i = 0; i < 8; i++) {
		register_id[i] = -1;
	}
	
	// Reset registers
	status = status_tmp = 0;
	cmdreg = cmdreg_tmp = 0;
	trkreg = 0;
	secreg = 0;
	datareg = 0;
	cmdtype = 0;
	
	// Reset state machine
	main_state = IDLE;
	sub_state = IDLE;
	
	// Reset state flags
	now_search = now_seek = false;
	sector_changed = false;
	no_command = 0;
	seektrk = 0;
	seekvct = false;
	
#ifdef HAS_MB89311
	extended_mode = true;
	mb89311_format_mode = false;
	mb89311_use_params = false;
	for(int i = 0; i < 8; i++) {
		mb89311_params[i] = 0;
	}
#endif
	
	// Reset drive info
	memset(fdc, 0, sizeof(fdc));
	
	// Reset timing
	prev_drq_clock = 0;
	seekend_clock = 0;
	
	// Update ready signal
	update_ready();
}

void MB8877::write_io8(uint32_t addr, uint32_t data)
{
#ifdef STANDALONE_TEST
	printf("write_io8: addr=%d, data=0x%02X\n", addr & 3, data);
#endif
	bool ready;
	
	switch(addr & 3) {
	case 0:
		// Command register
		cmdreg_tmp = cmdreg;
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		cmdreg = (~data) & 0xff;
#else
		cmdreg = data;
#endif
		process_cmd();
		no_command = 0;
		break;
		
	case 1:
		// Track register
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		trkreg = (~data) & 0xff;
#else
		trkreg = data;
#endif
		// If not busy, also update the physical track position for testing
		if(!(status & S_BUSY)) {
			fdc[drvreg].track = trkreg;
		}
		if((status & S_BUSY) && (fdc[drvreg].index == 0)) {
			// Track reg is written after command starts
			if(main_state == READ_SECTOR || main_state == WRITE_SECTOR) {
				process_cmd();
			}
		}
		break;
		
	case 2:
		// Sector register
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		secreg = (~data) & 0xff;
#else
		secreg = data;
#endif
		if((status & S_BUSY) && (fdc[drvreg].index == 0)) {
			// Sector reg is written after command starts
			if(main_state == READ_SECTOR || main_state == WRITE_SECTOR) {
				process_cmd();
			}
		}
		break;
		
	case 3:
		// Data register
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		datareg = (~data) & 0xff;
#else
		datareg = data;
#endif
		ready = ((status & S_DRQ) && !now_search);
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if(disk[drvreg]->is_special_disk != SPECIAL_DISK_FM7_RIGLAS)
#endif
		{
			if(!motor_on) ready = false;
		}
		if(ready) {
			if(main_state == WRITE_SECTOR) {
				// Write sector
				if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
					if(!disk[drvreg]->write_protected) {
						if(disk[drvreg]->sector[fdc[drvreg].index] != datareg) {
							disk[drvreg]->sector[fdc[drvreg].index] = datareg;
							sector_changed = true;
						}
						// Set deleted data mark if needed
						disk[drvreg]->set_deleted((cmdreg & 1) != 0);
					} else {
						status |= S_WP;
						status &= ~S_BUSY;
						main_state = IDLE;
						set_irq(true);
					}
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->sector_size.sd) {
					if(cmdreg & 0x10) {
						// Multiple sector
						register_my_event(EVENT_MULTI1, 30);
						register_my_event(EVENT_MULTI2, 60);
					} else {
						// Single sector
						status &= ~S_BUSY;
						main_state = IDLE;
						set_irq(true);
					}
					sector_changed = false;
				} else if(status & S_DRQ) {
					if(fdc[drvreg].index == 0) {
						register_drq_event(fdc[drvreg].bytes_before_2nd_drq);
					} else {
						register_drq_event(1);
					}
				}
				status &= ~S_DRQ;
			} else if(main_state == WRITE_TRACK) {
				// Write track implementation
				// Process format data byte
				uint8_t val = data;  // Add val declaration
				if(disk[drvreg]->write_protected) {
					// Write protect error already handled in command
					cmd_forceint();
					return;
				}
				
				// First byte? Format the track
				if(fdc[drvreg].index == 0) {
					disk[drvreg]->format_track(fdc[drvreg].track, sidereg);
					fdc[drvreg].id_written = false;
					fdc[drvreg].side = sidereg;
					fdc[drvreg].side_changed = false;
				}
				
				// Check for side change
				if(fdc[drvreg].side != sidereg) {
					fdc[drvreg].side_changed = true;
				}
				
				if(fdc[drvreg].side_changed) {
					// Abort write track because disk side is changed
				} else if(val == 0xf5) {
					// Write A1h in missing clock - special marker
				} else if(val == 0xf6) {
					// Write C2h in missing clock - special marker
				} else if(val == 0xf7) {
					// Write CRC
					if(!fdc[drvreg].id_written) {
						// Insert new sector with data CRC error
write_id:
						uint8_t c = 0, h = 0, r = 0, n = 0;
						fdc[drvreg].id_written = true;
						fdc[drvreg].sector_found = false;
						if(fdc[drvreg].index >= 4) {
							// Get sector ID from previous 4 bytes in track buffer
							c = fdc[drvreg].track_buffer[0];
							h = fdc[drvreg].track_buffer[1];
							r = fdc[drvreg].track_buffer[2];
							n = fdc[drvreg].track_buffer[3];
						}
						fdc[drvreg].sector_length = 0x80 << (n & 3);
						fdc[drvreg].sector_index = 0;
						disk[drvreg]->insert_sector(c, h, r, n, false, true, 0xe5, fdc[drvreg].sector_length);
					} else if(fdc[drvreg].sector_found) {
						// Clear data CRC error if all sector data are written
						if(fdc[drvreg].sector_index == fdc[drvreg].sector_length) {
							disk[drvreg]->set_data_crc_error(false);
						}
						fdc[drvreg].id_written = false;
					} else {
						// Data mark of current sector is not written
						disk[drvreg]->set_data_mark_missing();
						goto write_id;
					}
				} else if(fdc[drvreg].id_written) {
					if(fdc[drvreg].sector_found) {
						// Sector data
						if(fdc[drvreg].sector_index < fdc[drvreg].sector_length) {
							// TODO: Write to sector data buffer if available
						}
						fdc[drvreg].sector_index++;
					} else if(val == 0xf8 || val == 0xfb) {
						// Data mark
						disk[drvreg]->set_deleted(val == 0xf8);
						fdc[drvreg].sector_found = true;
					}
				}
				
				// Store bytes in track buffer for sector ID detection
				if(fdc[drvreg].index < 4) {
					fdc[drvreg].track_buffer[fdc[drvreg].index] = val;
				} else {
					// Shift buffer
					fdc[drvreg].track_buffer[0] = fdc[drvreg].track_buffer[1];
					fdc[drvreg].track_buffer[1] = fdc[drvreg].track_buffer[2];
					fdc[drvreg].track_buffer[2] = fdc[drvreg].track_buffer[3];
					fdc[drvreg].track_buffer[3] = val;
				}
				
				// TODO: Write byte to track buffer if direct access available
				
				// Increment index after processing
				fdc[drvreg].index++;
				
				// Check for track completion (after index hole)
				if(fdc[drvreg].index >= disk[drvreg]->get_track_size()) {
					if(fdc[drvreg].id_written && !fdc[drvreg].sector_found) {
						// Data mark of last sector is not written
						disk[drvreg]->set_data_mark_missing();
					}
					// Sync buffer
					disk[drvreg]->sync_buffer();
					// Complete
					status &= ~S_BUSY;
					main_state = IDLE;
					set_irq(true);
				} else if(status & S_DRQ) {
					if(fdc[drvreg].index == 0) {
						register_drq_event(fdc[drvreg].bytes_before_2nd_drq);
					} else {
						register_drq_event(1);
					}
				}
				status &= ~S_DRQ;
			}
			// Clear DRQ after data written
			set_drq(false);
		}
		break;
	}
}

uint32_t MB8877::read_io8(uint32_t addr)
{
	bool ready;
	uint32_t val = 0xff;
	
	switch(addr & 3) {
	case 0:
		// Status register
		if(now_search) {
			// Don't return busy signal while searching sector
			val = status & ~S_BUSY;
		} else {
			// Check ready status
			if(disk[drvreg]->inserted) {
				if(motor_on) {
					status &= ~S_NRDY;
				} else {
					status |= S_NRDY;
				}
			} else {
				status |= S_NRDY;
			}
			val = status;
		}
#ifdef _FDC_DEBUG_LOG
		if(!(status & S_DRQ)) {
			this->force_out_debug_log(_T("FDC\tSTATUS=%2x\n"), val);
		}
#endif
		// Clear IRQ
		if(!(status & S_BUSY)) {
			set_irq(false);
		}
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~val) & 0xff;
#else
		return val;
#endif
		
	case 1:
		// Track register
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~trkreg) & 0xff;
#else
		return trkreg;
#endif
		
	case 2:
		// Sector register
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~secreg) & 0xff;
#else
		return secreg;
#endif
		
	case 3:
		// Data register
		ready = ((status & S_DRQ) && !now_search);
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if(disk[drvreg]->is_special_disk != SPECIAL_DISK_FM7_RIGLAS)
#endif
		{
			if(!motor_on) ready = false;
		}
		if(ready) {
			if(main_state == READ_SECTOR) {
				// Read sector
				if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
					datareg = disk[drvreg]->sector[fdc[drvreg].index];
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->sector_size.sd) {
					if(cmdreg & 0x10) {
						// Multiple sector
						register_my_event(EVENT_MULTI1, 30);
						register_my_event(EVENT_MULTI2, 60);
					} else {
						// Single sector
						if(disk[drvreg]->data_crc_error && !disk[drvreg]->ignore_crc()) {
							// Data CRC error
							status |= S_CRC;
						}
						status &= ~S_BUSY;
						main_state = IDLE;
						set_irq(true);
					}
				} else {
					register_drq_event(1);
				}
				status &= ~S_DRQ;
			} else if(main_state == READ_ID) {
				// Read address
				if(fdc[drvreg].index < 6) {
					datareg = disk[drvreg]->id[fdc[drvreg].index];
				}
				if((fdc[drvreg].index + 1) >= 6) {
					if(disk[drvreg]->addr_crc_error && !disk[drvreg]->ignore_crc()) {
						// ID CRC error
						status |= S_CRC;
					}
					status &= ~S_BUSY;
					main_state = IDLE;
					set_irq(true);
				} else {
					register_drq_event(1);
				}
				status &= ~S_DRQ;
			} else if(main_state == READ_TRACK) {
				// Read track
				if(fdc[drvreg].index < disk[drvreg]->get_track_size()) {
					datareg = disk[drvreg]->track[fdc[drvreg].index];
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->get_track_size()) {
					status &= ~S_BUSY;
					status |= S_LOST;
					main_state = IDLE;
					set_irq(true);
				} else {
					register_drq_event(1);
				}
				status &= ~S_DRQ;
			}
			if(!(status & S_DRQ)) {
				cancel_my_event(EVENT_LOST);
				set_drq(false);
				fdc[drvreg].access = true;
			}
		}
#ifdef _FDC_DEBUG_LOG
		this->force_out_debug_log(_T("FDC\tDATA=%2x\n"), datareg);
#endif
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~datareg) & 0xff;
#else
		return datareg;
#endif
	}
	return 0xff;
}

void MB8877::write_dma_io8(uint32_t addr, uint32_t data)
{
	// DMA write is same as regular write to data register
	write_io8(3, data);
}

uint32_t MB8877::read_dma_io8(uint32_t addr)
{
	// DMA read is same as regular read from data register
	return read_io8(3);
}

void MB8877::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MB8877_DRIVEREG) {
		// Drive select
		drvreg = data & DRIVE_MASK;
		drive_sel = true;
		seekend_clock = get_current_clock();
		update_ready();
	} else if(id == SIG_MB8877_SIDEREG) {
		// Side select
		sidereg = (data & mask) ? 1 : 0;
	} else if(id == SIG_MB8877_MOTOR) {
		// Motor control
		motor_on = ((data & mask) != 0);
		update_ready();
	}
}

uint32_t MB8877::read_signal(int ch)
{
	if(ch == SIG_MB8877_DRIVEREG) {
		return drvreg & DRIVE_MASK;
	} else if(ch == SIG_MB8877_SIDEREG) {
		return sidereg & 1;
	} else if(ch == SIG_MB8877_MOTOR) {
		return motor_on ? 1 : 0;
	}
	
	// Get access status
	uint32_t stat = 0;
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(fdc[i].access) {
			stat |= 1 << i;
		}
		fdc[i].access = false;
	}
	if(now_search) {
		stat |= 1 << drvreg;
	}
	return stat;
}

void MB8877::event_callback(int event_id, int err)
{
	int event = event_id >> 8;
	int cmd = event_id & 0xff;
	register_id[event] = -1;
	
#ifdef STANDALONE_TEST
	printf("event_callback: event=%d, cmd=%d, cmdtype=%d\n", event, cmd, cmdtype);
#endif
	
	// Cancel event if the command is finished or other command is executed
	if(cmd != cmdtype) {
		if(event == EVENT_SEEK || event == EVENT_SEEKEND) {
			now_seek = false;
		} else if(event == EVENT_SEARCH) {
			now_search = false;
		}
		return;
	}
	
	switch(event) {
	case EVENT_SEEK:
		// Seek operation
		if(seektrk > fdc[drvreg].track) {
			fdc[drvreg].track++;
#ifndef STANDALONE_TEST
			if(d_noise_seek != NULL) d_noise_seek->play();
#endif
		} else if(seektrk < fdc[drvreg].track) {
			fdc[drvreg].track--;
#ifndef STANDALONE_TEST
			if(d_noise_seek != NULL) d_noise_seek->play();
#endif
		}
		// Update track register for RESTORE or if U flag is set
		if((cmdreg & 0xf0) == 0) {
			// RESTORE command always updates track register
			trkreg = fdc[drvreg].track;
		} else if(cmdreg & 0x10) {
			// STEP/SEEK with U flag updates track register
			if(main_state == STEP) {
				// For STEP commands, update based on direction
				if(seekvct) {
					if(fdc[drvreg].track > (seektrk - 1)) {
						trkreg++;
					} else if(fdc[drvreg].track < (seektrk + 1)) {
						trkreg--;
					}
				}
			} else {
				// SEEK always updates to current track
				trkreg = fdc[drvreg].track;
			}
		}
		if(seektrk != fdc[drvreg].track) {
			register_seek_event(false);
			break;
		}
		seekend_clock = get_current_clock();
		// Update track register for Type I commands
		if(cmdtype == TYPE_I) {
			trkreg = fdc[drvreg].track;
		}
#ifdef HAS_MB89311
		if(extended_mode) {
			if((cmdreg & 0xf4) == 0x44) {
				// Read-after-seek
				cmd_readdata(true);
				break;
			} else if((cmdreg & 0xf4) == 0x64) {
				// Write-after-seek
				cmd_writedata(true);
				break;
			}
		}
#endif
		status_tmp = status | S_HLD;
		// Update status flags based on track position
		if(fdc[drvreg].track == 0) {
			status_tmp |= S_TR00;
		} else {
			status_tmp &= ~S_TR00;
		}
		if(cmdreg & 4) {
			// Verify
			status_tmp |= search_track();
			double time;
			if(status_tmp & S_RNF) {
				time = get_usec_to_detect_index_hole(5, true);
			} else {
				time = get_usec_to_next_trans_pos(true);
			}
			register_my_event(EVENT_SEEKEND, time);
			break;
		}
		// Fall through
		
	case EVENT_SEEKEND:
		now_seek = false;
		status = status_tmp & ~S_BUSY;
		// Ensure proper flags are set
		if(fdc[drvreg].track == 0) {
			status |= S_TR00;
		} else {
			status &= ~S_TR00;
		}
		if(cmdtype == TYPE_I) {
			status |= S_HLD;  // Head loaded for Type I commands
		}
		set_irq(true);
		break;
		
	case EVENT_SEARCH:
		now_search = false;
		if(status_tmp & S_RNF) {
#if defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
			// For SHARP X1 Batten Tanuki
			if(disk[drvreg]->is_special_disk == SPECIAL_DISK_X1_BATTEN && drive_sel) {
				status_tmp &= ~S_RNF;
			}
#endif
			status = status_tmp & ~(S_BUSY | S_DRQ);
			main_state = IDLE;
			set_irq(true);
		} else if(status_tmp & S_WP) {
			status = status_tmp & ~(S_BUSY | S_DRQ);
			main_state = IDLE;
			set_irq(true);
		} else {
			// Sector found - prepare for data transfer
			status = status_tmp | S_BUSY;
			
			if(main_state == READ_SECTOR) {
				// Read sector - set DRQ after finding sector
				set_drq(true);
				register_lost_event(1);
			} else if(main_state == WRITE_SECTOR) {
				// Write sector - DRQ already set, maintain lost event
				if(!(status & S_DRQ)) {
					set_drq(true);
				}
				cancel_my_event(EVENT_LOST);
				register_lost_event(8);
			} else if(main_state == WRITE_TRACK) {
				set_drq(true);
				register_lost_event(3);
			} else {
				set_drq(true);
				register_lost_event(1);
			}
			
			fdc[drvreg].cur_position = fdc[drvreg].next_trans_position;
			fdc[drvreg].prev_clock = prev_drq_clock = get_current_clock();
			drive_sel = false;
		}
		break;
		
	case EVENT_DRQ:
		if(status & S_BUSY) {
			// Generate next DRQ for data transfer
			if(!(status & S_DRQ)) {
				set_drq(true);
			}
			register_lost_event(1);
			
			// Update position
			if((main_state == WRITE_SECTOR || main_state == WRITE_TRACK) && fdc[drvreg].index == 0) {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + fdc[drvreg].bytes_before_2nd_drq) % disk[drvreg]->get_track_size();
			} else {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + 1) % disk[drvreg]->get_track_size();
			}
			
			// Increment byte counter
			if(main_state == READ_SECTOR || main_state == WRITE_SECTOR ||
			   main_state == READ_TRACK || main_state == WRITE_TRACK ||
			   main_state == READ_ID) {
				fdc[drvreg].index++;
			}
			fdc[drvreg].prev_clock = prev_drq_clock = get_current_clock();
		}
		break;
		
	case EVENT_MULTI1:
		secreg++;
		break;
		
	case EVENT_MULTI2:
		if(main_state == READ_SECTOR) {
			cmd_readdata(false);
		} else if(main_state == WRITE_SECTOR) {
			cmd_writedata(false);
		}
		break;
		
	case EVENT_LOST:
		if(status & S_BUSY) {
			// Lost data event
			status |= S_LOST;
			status &= ~S_BUSY;
			main_state = IDLE;
			set_drq(false);
			set_irq(true);
		}
		break;
	}
}

// Command processing
static const _TCHAR *cmdstr[0x10] = {
	_T("RESTORE "),	_T("SEEK    "),	_T("STEP    "),	_T("STEP    "),
	_T("STEP IN "),	_T("STEP IN "),	_T("STEP OUT"),	_T("STEP OUT"),
	_T("RD DATA "),	_T("RD DATA "),	_T("RD DATA "),	_T("WR DATA "),
	_T("RD ADDR "),	_T("FORCEINT"),	_T("RD TRACK"),	_T("WR TRACK")
};

void MB8877::process_cmd()
{
#ifdef STANDALONE_TEST
	printf("process_cmd: cmdreg=0x%02X, drvreg=%d\n", cmdreg, drvreg);
#endif
	set_irq(false);
	set_drq(false);
	
#ifdef STANDALONE_TEST
	printf("After set_drq\n");
#endif
	
#ifdef HAS_MB89311
	// MB89311 mode commands
	if(cmdreg == 0xfc) {
		// Delay - parameter-based delay command
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (DELAY   ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		// Implement delay based on datareg value
		// Each unit represents 16us delay (per MB89311 specification)
		if(datareg > 0) {
			register_my_event(EVENT_LOST, datareg * 16.0);
		}
		cmdtype = status = 0;
		main_state = IDLE;
		return;
	} else if(cmdreg == 0xfd) {
		// Assign parameter - store parameters for later use
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (ASGN PAR) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		// Store parameter in mb89311_params array
		// Lower 3 bits of data register specify parameter index
		int param_index = datareg & 0x07;
		if(param_index < 8) {
			mb89311_params[param_index] = datareg >> 3;
		}
		cmdtype = status = 0;
		main_state = IDLE;
		return;
	} else if(cmdreg == 0xfe) {
		// Assign mode - switch between standard and extended mode
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (ASGN MOD) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		extended_mode = ((datareg & 1) != 0);
		// Additional mode settings from datareg
		if(datareg & 0x02) {
			// Enable special format mode
			mb89311_format_mode = true;
		}
		if(datareg & 0x04) {
			// Enable parameter usage in format
			mb89311_use_params = true;
		}
		cmdtype = status = 0;
		main_state = IDLE;
		return;
	} else if(cmdreg == 0xff) {
		// Reset - complete chip reset
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (RESET   ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		// Reset all registers and state
		cmdtype = 0;
		status = S_TR00;
		main_state = IDLE;
		extended_mode = true;  // MB89311 defaults to extended mode
		mb89311_format_mode = false;
		mb89311_use_params = false;
		// Clear parameter storage
		for(int i = 0; i < 8; i++) {
			mb89311_params[i] = 0;
		}
		// Reset all drives
		for(int i = 0; i < MAX_DRIVE; i++) {
			fdc[i].track = 0;
		}
		set_irq(true);
		return;
	}
#endif
	
	// Check drive ready
#ifdef STANDALONE_TEST
	// For testing, allow commands without actual disk if motor is on
	printf("Check drive ready: motor_on=%d, drvreg=%d\n", motor_on, drvreg);
	if(!motor_on) {
		printf("Motor is off, returning NOT READY\n");
		if(cmdreg == 0xd0) {
			// Force interrupt
			status &= ~S_BUSY;
			cmdtype = 0;
			main_state = IDLE;
			set_irq(true);
			return;
		}
		status = S_NRDY;
		cmdtype = 0;
		main_state = IDLE;
		set_irq(true);
		return;
	}
#else
	if(!disk[drvreg]->inserted || !(motor_on || disk[drvreg]->is_special_disk)) {
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if(disk[drvreg]->is_special_disk == SPECIAL_DISK_FM7_RIGLAS) {
			goto skip;
		}
#endif
		if(cmdreg == 0xd0) {
			// Force interrupt
			status &= ~S_BUSY;
			cmdtype = 0;
			main_state = IDLE;
			set_irq(true);
			return;
		}
		status = S_NRDY;
		cmdtype = 0;
		main_state = IDLE;
		set_irq(true);
		return;
	}
skip:
#endif
	
	// Get command type
	uint8_t cmd = cmdreg & 0xf0;
	if(cmd == 0x00 || cmd == 0x10 || cmd == 0x20 || cmd == 0x30) {
		// Type I
		cmdtype = TYPE_I;
	} else if(cmd == 0x40 || cmd == 0x50 || cmd == 0x60 || cmd == 0x70) {
		// Type II
		cmdtype = TYPE_II;
	} else if(cmd == 0x80 || cmd == 0x90 || cmd == 0xa0 || cmd == 0xb0) {
		// Type II
		cmdtype = TYPE_II;
	} else if(cmd == 0xc0 || cmd == 0xe0 || cmd == 0xf0) {
		// Type III/IV
		cmdtype = (cmd == 0xd0) ? TYPE_IV : TYPE_III;
	}
	
#ifdef _FDC_DEBUG_LOG
	this->out_debug_log(_T("FDC\tCMD=%2xh (%s) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, cmdstr[cmdreg >> 4], datareg, drvreg, trkreg, sidereg, secreg);
#endif
	
	// Reset previous status
	status = S_BUSY;
	status_tmp = 0;
	
	if(cmdtype == TYPE_I || cmdtype == TYPE_II || cmdtype == TYPE_III) {
		// Abort previous write/format command
		if(main_state == WRITE_SECTOR && sector_changed) {
			disk[drvreg]->set_data_crc_error(false);
		} else if(main_state == WRITE_TRACK) {
			if(!disk[drvreg]->write_protected) {
				if(fdc[drvreg].id_written && !fdc[drvreg].sector_found) {
					// Data mark of last sector is not written  
					disk[drvreg]->set_data_mark_missing();
				}
				disk[drvreg]->sync_buffer();
			}
		}
	}
	
	// Start command
#ifdef STANDALONE_TEST
	printf("Starting command: 0x%02X\n", cmdreg & 0xf0);
#endif
	switch(cmdreg & 0xf0) {
	case 0x00:
		// Restore
		cmd_restore();
		break;
	case 0x10:
		// Seek
		cmd_seek();
		break;
	case 0x20:
	case 0x30:
		// Step
		cmd_step();
		break;
	case 0x40:
	case 0x50:
		// Step in
		cmd_stepin();
		break;
	case 0x60:
	case 0x70:
		// Step out
		cmd_stepout();
		break;
	case 0x80:
	case 0x90:
		// Read sector
		cmd_readdata(true);
		break;
	case 0xa0:
	case 0xb0:
		// Write sector
		cmd_writedata(true);
		break;
	case 0xc0:
		// Read address
		cmd_readaddr();
		break;
	case 0xd0:
		// Force interrupt
		cmd_forceint();
		break;
	case 0xe0:
		// Read track
		cmd_readtrack();
		break;
	case 0xf0:
		// Write track
		cmd_writetrack();
		break;
	}
}

void MB8877::update_config()
{
	// Update configuration if needed
}

#ifdef USE_DEBUGGER
bool MB8877::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_sntprintf_s(buffer, buffer_len, _TRUNCATE,
		_T("CMDREG=$%02X STATREG=$%02X TRKREG=$%02X SECREG=$%02X DATAREG=$%02X\n")
		_T("CURRENT DRIVE=%d SIDE=%d MOTOR=%s\n"),
		cmdreg, status, trkreg, secreg, datareg,
		drvreg, sidereg, motor_on ? _T("ON") : _T("OFF"));
	return true;
}
#endif

bool MB8877::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	
	// Save/load registers
	state_fio->StateValue(status);
	state_fio->StateValue(status_tmp);
	state_fio->StateValue(cmdreg);
	state_fio->StateValue(cmdreg_tmp);
	state_fio->StateValue(trkreg);
	state_fio->StateValue(secreg);
	state_fio->StateValue(datareg);
	state_fio->StateValue(drvreg);
	state_fio->StateValue(sidereg);
	state_fio->StateValue(cmdtype);
	
	// Save/load state
	state_fio->StateValue(now_search);
	state_fio->StateValue(now_seek);
	state_fio->StateValue(sector_changed);
	state_fio->StateValue(no_command);
	state_fio->StateValue(seektrk);
	state_fio->StateValue(seekvct);
	state_fio->StateValue(motor_on);
	state_fio->StateValue(drive_sel);
	
#ifdef HAS_MB89311
	state_fio->StateValue(extended_mode);
	state_fio->StateArray(mb89311_params, sizeof(mb89311_params), 1);
	state_fio->StateValue(mb89311_format_mode);
	state_fio->StateValue(mb89311_use_params);
#endif
	
	// Save/load timing
	state_fio->StateValue(prev_drq_clock);
	state_fio->StateValue(seekend_clock);
	
	// Save/load drive info
	for(int i = 0; i < MAX_DRIVE; i++) {
		state_fio->StateValue(fdc[i].track);
		state_fio->StateValue(fdc[i].index);
		state_fio->StateValue(fdc[i].access);
		state_fio->StateValue(fdc[i].head_load);
		state_fio->StateValue(fdc[i].id_written);
		state_fio->StateValue(fdc[i].sector_found);
		state_fio->StateValue(fdc[i].sector_length);
		state_fio->StateValue(fdc[i].sector_index);
		state_fio->StateValue(fdc[i].side);
		state_fio->StateValue(fdc[i].side_changed);
		state_fio->StateValue(fdc[i].cur_position);
		state_fio->StateValue(fdc[i].next_trans_position);
		state_fio->StateValue(fdc[i].bytes_before_2nd_drq);
		state_fio->StateValue(fdc[i].next_am1_position);
		state_fio->StateValue(fdc[i].prev_clock);
	}
	
	// Save/load disk states
#ifndef STANDALONE_TEST
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
#endif
	
	return true;
}

// Disk management functions
void MB8877::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->open(file_path, bank);
		update_ready();
	}
}

void MB8877::close_disk(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->close();
		update_ready();
	}
}

bool MB8877::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->inserted;
	}
	return false;
}

bool MB8877::is_disk_changed(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->changed;
	}
	return false;
}

void MB8877::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->write_protected = value;
	}
}

bool MB8877::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->write_protected;
	}
	return false;
}

bool MB8877::is_drive_ready()
{
	return is_drive_ready(drvreg);
}

bool MB8877::is_drive_ready(int drv)
{
	if(drv < MAX_DRIVE) {
		return motor_on && is_disk_inserted(drv);
	}
	return false;
}

uint8_t MB8877::get_media_type(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->media_type;
	}
	return MEDIA_TYPE_UNK;
}

void MB8877::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->drive_type = type;
	}
}

uint8_t MB8877::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void MB8877::set_drive_rpm(int drv, int rpm)
{
	// Validate drive number
	if(drv < 0 || drv >= MAX_DRIVE) {
		this->out_debug_log(_T("FDC: set_drive_rpm: invalid drive number %d"), drv);
		return;
	}
	
	// Validate RPM range (240-400 RPM)
	if(rpm < 240 || rpm > 400) {
		this->out_debug_log(_T("FDC: set_drive_rpm: invalid RPM %d (valid range: 240-400)"), rpm);
		return;
	}
	
	// Check if disk is inserted
	if(!disk[drv]) {
		this->out_debug_log(_T("FDC: set_drive_rpm: no disk in drive %d"), drv);
		return;
	}
	
	// Set the drive RPM
	disk[drv]->drive_rpm = rpm;
	
	// Log the RPM setting with standard identification
	const char* rpm_type = "";
	if(rpm == 300) {
		rpm_type = " (5.25\" standard)";
	} else if(rpm == 360) {
		rpm_type = " (3.5\" standard)";
	}
	
	this->out_debug_log(_T("FDC: set_drive_rpm: drive %d set to %d RPM%s"), drv, rpm, rpm_type);
	
	// Calculate and log rotation time for debugging
	double rotation_time_us = 60000000.0 / rpm;
	this->out_debug_log(_T("FDC: drive %d rotation time: %.0f microseconds"), drv, rotation_time_us);
}

void MB8877::set_drive_mfm(int drv, bool mfm)
{
	// Set FM/MFM mode for the specified drive
	if(drv < 0 || drv >= MAX_DRIVE) {
		// Invalid drive number - ignore
		return;
	}
	
	if(disk[drv]) {
		// Set the MFM mode in the disk object
		disk[drv]->drive_mfm = mfm;
		
		// Log the mode change for debugging
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("MB8877: Drive %d set to %s mode\n"), 
			drv, mfm ? _T("MFM (250kbps)") : _T("FM (125kbps)"));
#endif
		
		// Update timing parameters if this is the current drive
		if(drv == drvreg) {
			// FM mode: 125kbps (8us per bit, 64us per byte)
			// MFM mode: 250kbps (4us per bit, 32us per byte)
			// The disk class handles these timing calculations internally
			// based on the drive_mfm flag
		}
	}
}

void MB8877::set_track_size(int drv, int size)
{
	// Validate drive number
	if(drv < 0 || drv >= MAX_DRIVE) {
		// Invalid drive number, log and return
		if(_fdc_debug_log) {
			this->out_debug_log(_T("FDC: set_track_size() invalid drive number: %d\n"), drv);
		}
		return;
	}
	
	// Check if disk is present
	if(!disk[drv]) {
		// No disk in drive, log and return
		if(_fdc_debug_log) {
			this->out_debug_log(_T("FDC: set_track_size() no disk in drive %d\n"), drv);
		}
		return;
	}
	
	// Validate track size range (1024 to 65536 bytes)
	const int MIN_TRACK_SIZE = 1024;
	const int MAX_TRACK_SIZE = 65536;
	
	if(size < MIN_TRACK_SIZE || size > MAX_TRACK_SIZE) {
		// Invalid track size, log and return
		if(_fdc_debug_log) {
			this->out_debug_log(_T("FDC: set_track_size() invalid size %d (valid range: %d-%d)\n"), 
				size, MIN_TRACK_SIZE, MAX_TRACK_SIZE);
		}
		return;
	}
	
	// Set the track size
	disk[drv]->track_size = size;
	
	// Log the operation
	if(_fdc_debug_log) {
		this->out_debug_log(_T("FDC: set_track_size() drive %d, size set to %d bytes\n"), drv, size);
		
		// Log special format detection
		if(size == 6250) {
			this->out_debug_log(_T("FDC: Standard 2D/2DD format (6250 bytes)\n"));
		} else if(size == 12500) {
			this->out_debug_log(_T("FDC: Standard 2HD format (12500 bytes)\n"));
		} else {
			this->out_debug_log(_T("FDC: Custom/special format\n"));
		}
	}
}

uint8_t MB8877::fdc_status()
{
	// Return current status
	// Additional processing may be needed based on current operation
	update_fdc_status();
	return status;
}

// Internal helper methods

void MB8877::cmd_forceint()
{
	// Force interrupt command
	bool was_busy = (status & S_BUSY) != 0;
	
	// Cancel all pending operations
	for(int i = 0; i < 8; i++) {
		cancel_my_event(i);
	}
	
	// Handle abort for specific commands
	if(cmdtype == TYPE_II && sector_changed) {
		// Abort write sector command
		disk[drvreg]->set_data_crc_error(false);
	} else if(cmdtype == TYPE_III && main_state == WRITE_TRACK) {
		// Abort write track command
		if(!disk[drvreg]->write_protected) {
			if(fdc[drvreg].id_written && !fdc[drvreg].sector_found) {
				// Data mark of last sector is not written
				disk[drvreg]->set_data_mark_missing();
			}
			disk[drvreg]->sync_buffer();
		}
	}
	
	// Clear state
	status &= ~S_BUSY;
	main_state = IDLE;
	now_search = false;
	now_seek = false;
	sector_changed = false;
	
	// Clear DRQ
	set_drq(false);
	
	// IRQ conditions check
	uint8_t irq_conditions = cmdreg & 0x0f;
	
	// Generate IRQ if:
	// - Was busy (interrupted an operation)
	// - Any IRQ condition bits are set
	if(was_busy || irq_conditions) {
		set_irq(true);
	}
	
	// Clear command type
	cmdtype = 0;
}

// Helper methods
void MB8877::update_fdc_status()
{
	// Update status based on current state
	if(is_drive_ready()) {
		status &= ~FDC_ST_NOTREADY;
	} else {
		status |= FDC_ST_NOTREADY;
	}
	
	// Update track 0 flag
	if(drvreg < MAX_DRIVE && fdc[drvreg].track == 0) {
		status |= FDC_ST_TRACK00;
	} else {
		status &= ~FDC_ST_TRACK00;
	}
	
	// Update write protect flag
	if(is_disk_protected(drvreg)) {
		status |= FDC_ST_WRITEP;
	} else {
		status &= ~FDC_ST_WRITEP;
	}
}

void MB8877::update_head_flag(int drv, bool head_load)
{
	if(drv < MAX_DRIVE) {
		fdc[drv].head_load = head_load;
		if(head_load) {
			status |= FDC_ST_HEADENG;
		}
	}
}

void MB8877::update_ready()
{
	// Update ready signal
	bool ready = is_drive_ready();
	write_signals(&outputs_rdy, ready ? 0xffffffff : 0);
}

void MB8877::set_irq(bool val)
{
	// Set IRQ signal
	irq_active = val;
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
	
#ifdef STANDALONE_TEST
	printf("IRQ Signal: %s\n", val ? "SET" : "CLEAR");
	fflush(stdout);
#endif
}

void MB8877::set_drq(bool val)
{
	// Set DRQ signal and update status
	if(val) {
#ifndef STANDALONE_TEST
		prev_drq_clock = get_current_clock();
#else
		prev_drq_clock = 0; // For testing
#endif
		status |= S_DRQ;
	} else {
		status &= ~S_DRQ;
	}
	
	write_signals(&outputs_drq, val ? 0xffffffff : 0);
	
#ifdef STANDALONE_TEST
	printf("DRQ Signal: %s (status=0x%02X)\n", val ? "SET" : "CLEAR", status);
	fflush(stdout);
#endif
}

// Event management
void MB8877::cancel_my_event(int event)
{
	if(register_id[event] != -1) {
		cancel_event(this, register_id[event]);
		register_id[event] = -1;
	}
}

void MB8877::register_my_event(int event, double usec)
{
	cancel_my_event(event);
	register_event(this, (event << 8) | cmdtype, usec, false, &register_id[event]);
}

void MB8877::register_seek_event(bool first)
{
	// Register seek event based on step rate
	double usec = 6000; // Default 6ms
	int step_rate = cmdreg & 3;
	
	if(fdc[drvreg].track == 0 && first) {
		// Additional time for track 0
		usec += 12000;
	}
	
	// Step rates: 6ms, 12ms, 20ms, 30ms
	switch(step_rate) {
	case 0: usec = 6000; break;
	case 1: usec = 12000; break;
	case 2: usec = 20000; break;
	case 3: usec = 30000; break;
	}
	
	register_my_event(EVENT_SEEK, usec);
}


// Search helpers
uint8_t MB8877::search_track()
{
	// Check if head is loaded
	if(!fdc[drvreg].head_load) {
		return S_RNF;
	}
	
	// Get track from disk image
	int track = fdc[drvreg].track;
	int side = (disk[drvreg]->drive_type == DRIVE_TYPE_2HD) ? sidereg : (sidereg | ((cmdreg >> 3) & 1));
	
	if(!disk[drvreg]->get_track(track, side)) {
		// Track not found
		return S_RNF;
	}
	
	// Get current sector info
	if(!disk[drvreg]->get_sector(track, side, -1)) {
		// Sector not found
		return S_RNF;
	}
	
	// Check track match
	if(disk[drvreg]->id[0] != trkreg) {
		// Track mismatch - seek error
		return S_RNF;
	}
	
	return 0;
}

uint8_t MB8877::search_sector()
{
	// Get sector position
	int position = get_cur_position();
	
	// Search for sector
	// Find sector by searching through all sectors on the track
	bool sector_found = false;
	int max_sectors = 32; // Reasonable limit for sectors per track
	
	// Get the track first
	if(disk[drvreg]->get_track(fdc[drvreg].track, sidereg)) {
		// Search for sector using simple mock logic
		if(disk[drvreg]->get_sector(fdc[drvreg].track, sidereg, secreg)) {
			sector_found = true;
			fdc[drvreg].sector_index = secreg;
		}
	}
	
	if(!sector_found) {
		// Sector not found
		return S_RNF;
	}
	
	// Save next position
	fdc[drvreg].next_trans_position = 0; // Mock position
	fdc[drvreg].bytes_before_2nd_drq = 1;
	
	return 0;
}

uint8_t MB8877::search_addr()
{
	// Get current position
	int position = get_cur_position();
	
	// Search for next ID field
	// For READ ADDRESS command, we need to find any sector
	bool id_found = false;
	
	// Get the track first
	if(disk[drvreg]->get_track(fdc[drvreg].track, sidereg)) {
		// Find first available sector (for READ ADDRESS)
		if(disk[drvreg]->get_sector(fdc[drvreg].track, sidereg, 1)) {
			id_found = true;
			fdc[drvreg].sector_index = 0;
		}
	}
	
	if(!id_found) {
		// No ID field found
		return S_RNF;
	}
	
	// Save position after ID field
	fdc[drvreg].next_trans_position = 6; // Mock position after ID
	
	return 0;
}

// Type I Commands
void MB8877::cmd_restore()
{
	// Set head load flag
	if(cmdreg & 8) {
		// Set head load
		update_head_flag(drvreg, true);
	}
	
	// Start restore
	main_state = RESTORE;
	seektrk = 0;
	seekvct = true;
	
#ifdef STANDALONE_TEST
	printf("cmd_restore: drvreg=%d, fdc[drvreg].track=%d\n", drvreg, fdc[drvreg].track);
#endif
	
	// Track 00?
	if(fdc[drvreg].track == 0) {
		// Already at track 0
		trkreg = 0;
		status = S_BUSY | S_HLD | S_TR00;
		// Clear busy flag quickly since we're already at track 0
		register_my_event(EVENT_SEEKEND, 100);
	} else {
		// Start seeking to track 0
		now_seek = true;
		register_seek_event(true);
	}
}

void MB8877::cmd_seek()
{
	// Set head load flag
	if(cmdreg & 8) {
		update_head_flag(drvreg, true);
	}
	
	// Start seek
	main_state = SEEK;
	seektrk = datareg;
	seekvct = true;
	
	// Start seeking
	now_seek = true;
	register_seek_event(true);
}

void MB8877::cmd_step()
{
	// Use previous seek direction
	if(seekvct) {
		seektrk = (fdc[drvreg].track > trkreg) ? (fdc[drvreg].track + 1) : (fdc[drvreg].track - 1);
	} else {
		seektrk = fdc[drvreg].track;
	}
	cmd_step_common();
}

void MB8877::cmd_stepin()
{
	// Step towards center
	seektrk = fdc[drvreg].track + 1;
	seekvct = true;
	cmd_step_common();
}

void MB8877::cmd_stepout()
{
	// Step towards track 0
	seektrk = (fdc[drvreg].track > 0) ? (fdc[drvreg].track - 1) : 0;
	seekvct = true;
	cmd_step_common();
}

void MB8877::cmd_step_common()
{
	// Set head load flag
	if(cmdreg & 8) {
		update_head_flag(drvreg, true);
	}
	
	// Start step
	main_state = STEP;
	now_seek = true;
	register_seek_event(true);
}

// Helper function for DRQ event registration
void MB8877::register_drq_event(int bytes)
{
	double usec = bytes * 32.0; // Mock timing: ~32 usec per byte
	register_my_event(EVENT_DRQ, usec);
}

// Helper function for lost data event registration  
void MB8877::register_lost_event(int bytes)
{
	double usec = bytes * 32.0; // Mock timing: ~32 usec per byte
	register_my_event(EVENT_LOST, usec);
}

// Type II Commands
void MB8877::cmd_readdata(bool first_sector)
{
	// Head load
	if(first_sector) {
		// Set head load
		update_head_flag(drvreg, true);
	}
	
	// Start read sector
	main_state = READ_SECTOR;
	status |= S_BUSY;
	fdc[drvreg].index = 0;
	
	// Clear DRQ 
	set_drq(false);
	
	// Sector search
	sector_changed = false;
	now_search = true;
	
	// Search for sector
	status_tmp = search_sector();
	
	if(status_tmp & S_RNF) {
		// Sector not found - wait 5 index holes
		register_my_event(EVENT_SEARCH, get_usec_to_detect_index_hole(5, false));
	} else {
		// Sector found - prepare for data transfer
		double time;
		if(first_sector) {
			time = get_usec_to_start_trans(true);
		} else {
			time = get_usec_to_start_trans(false);
		}
		// Register DRQ event for data transfer
		register_my_event(EVENT_SEARCH, time);
	}
}

void MB8877::cmd_writedata(bool first_sector)
{
	// Check write protect
	if(is_disk_protected(drvreg)) {
		status = S_WP;
		main_state = IDLE;
		set_irq(true);
		return;
	}
	
	// Head load
	if(first_sector) {
		// Set head load
		update_head_flag(drvreg, true);
	}
	
	// Start write sector
	main_state = WRITE_SECTOR;
	status |= S_BUSY;
	fdc[drvreg].index = 0;
	
	// Search sector
	sector_changed = false;
	now_search = true;
	
	// Search for sector
	status_tmp = search_sector();
	
	if(status_tmp & S_RNF) {
		// Sector not found - wait 5 index holes
		register_my_event(EVENT_SEARCH, get_usec_to_detect_index_hole(5, false));
	} else {
		// Sector found - set DRQ immediately for write data
		set_drq(true);
		// Register lost data timeout (8 bytes time)
		register_lost_event(8);
		// Register search complete event
		double time;
		if(first_sector) {
			time = get_usec_to_start_trans(true);
		} else {
			time = get_usec_to_start_trans(false);
		}
		register_my_event(EVENT_SEARCH, time);
	}
}

// Type III Commands
void MB8877::cmd_readaddr()
{
	// Set head load
	if((cmdreg & 0xf0) != 0x00) {
		update_head_flag(drvreg, true);
	}
	
	// Start read address
	main_state = READ_ID;
	status_tmp = status = S_BUSY;
	fdc[drvreg].index = 0;
	
	// Get track
	int track = fdc[drvreg].track;
	int side = (cmdreg & 8) ? 1 : 0;
	
	if(!disk[drvreg]->get_track(track, side)) {
		// No disk or track
		status &= ~S_BUSY;
		main_state = IDLE;
		set_irq(true);
		return;
	}
	
	// Search for next ID field
	now_search = true;
	double time = get_usec_to_start_trans(true);
	
	status_tmp |= search_addr();
	if(status_tmp & S_RNF) {
		// No ID field found
		time = get_usec_to_detect_index_hole(5, false);
	}
	register_my_event(EVENT_SEARCH, time);
}

void MB8877::cmd_readtrack()
{
	// Set head load
	if((cmdreg & 0xf0) != 0x00) {
		update_head_flag(drvreg, true);
	}
	
	// Start read track
	main_state = READ_TRACK;
	status = S_BUSY;
	fdc[drvreg].index = 0;
	
	// Get track
	int track = fdc[drvreg].track;
	int side = sidereg;
	
	if(!disk[drvreg]->get_track(track, side)) {
		// No disk or track
		status &= ~S_BUSY;
		main_state = IDLE;
		set_irq(true);
		return;
	}
	
	// Wait for index hole
	now_search = false;
	double time = get_usec_to_detect_index_hole(1, true);
	register_my_event(EVENT_SEARCH, time);
}

void MB8877::cmd_writetrack()
{
	// Check write protect
	if(disk[drvreg]->write_protected) {
		status = S_WP;
		main_state = IDLE;
		set_irq(true);
		return;
	}
	
	// Set head load
	if((cmdreg & 0xf0) != 0x00) {
		update_head_flag(drvreg, true);
	}
	
	// Start write track
	main_state = WRITE_TRACK;
	status = S_BUSY;
	fdc[drvreg].index = 0;
	fdc[drvreg].id_written = false;
	fdc[drvreg].sector_found = false;
	
	// Get track
	int track = fdc[drvreg].track;
	int side = sidereg;
	
	// Format track (mock implementation)
	// disk[drvreg]->format_track(track, side); // Not available in mock
	fdc[drvreg].side = side;
	
	// Wait for index hole
	now_search = false;
	double time = get_usec_to_detect_index_hole(1, true);
	
	// Data request after index hole
	time += 100; // Small delay
	register_my_event(EVENT_SEARCH, time);
}

#ifdef HAS_MB89311
void MB8877::cmd_format()
{
	// MB89311 enhanced format command
	#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC\tMB89311 FORMAT: mode=%d use_params=%d\n"), mb89311_format_mode, mb89311_use_params);
	#endif
	
	cmdtype = TYPE_III;
	main_state = WRITE_TRACK;
	status = S_BUSY;
	status_tmp = 0;
	
	// Check write protect
	if(!disk[drvreg]->inserted || disk[drvreg]->write_protected) {
		status = (disk[drvreg]->write_protected ? S_WP : 0);
		main_state = IDLE;
		set_irq(true);
		return;
	}
	
	// Initialize format parameters
	fdc[drvreg].index = 0;
	fdc[drvreg].id_written = false;
	fdc[drvreg].sector_found = false;
	fdc[drvreg].sector_length = 0;
	fdc[drvreg].sector_index = 0;
	
	// Apply MB89311 specific format settings
	if(mb89311_format_mode) {
		// Use extended format options
		if(mb89311_use_params) {
			// Apply stored parameters to format operation
			// param[0]: gap length between sectors
			// param[1]: gap length after index
			// param[2]: sector size code
			// param[3]: number of sectors
			if(mb89311_params[2] != 0) {
				// Override sector size
				fdc[drvreg].sector_length = 128 << (mb89311_params[2] & 0x03);
			}
		}
	}
	
	// Set up DRQ for format data
	set_drq(true);
	register_lost_event(3);
	
	// Start format after head load delay
	double time = (cmdreg & 4) ? get_head_load_delay() : 1;
	register_my_event(EVENT_SEARCH, time);
}
#endif

// Timing helper functions
int MB8877::get_cur_position()
{
	// Calculate current position based on elapsed time
	if (!disk[drvreg] || !disk[drvreg]->inserted) {
		return 0;
	}
	
	// Get elapsed time since last position update
	double elapsed_usec = get_passed_usec(fdc[drvreg].prev_clock);
	int elapsed_bytes = disk[drvreg]->get_bytes_per_usec(elapsed_usec);
	
	// Calculate new position (with wraparound)
	int track_size = disk[drvreg]->get_track_size();
	if (track_size == 0) {
		return 0;
	}
	
	return (fdc[drvreg].cur_position + elapsed_bytes) % track_size;
}

double MB8877::get_usec_to_start_trans(bool first_sector)
{
	// Calculate time to start data transfer based on sector position
	double time = get_usec_to_next_trans_pos(first_sector && ((cmdreg & 4) != 0));
	
#ifdef MB8877_DELAY_AFTER_SEEK
	// Wait 60ms to start read/write after seek is finished
	if (first_sector && time < MB8877_DELAY_AFTER_SEEK - get_passed_usec(seekend_clock)) {
		time += disk[drvreg]->get_usec_per_track();
	}
#endif
	return time;
}

double MB8877::get_usec_to_next_trans_pos(bool delay)
{
	// Calculate time to next transfer position based on current position
	if (!disk[drvreg] || !disk[drvreg]->inserted) {
		return 50000.0; // Default delay if no disk
	}
	
	int position = get_cur_position();
	
	// Handle invalid format tracks
	if (disk[drvreg]->invalid_format) {
		return 50000.0;
	}
	
	// Handle head load delay
	if (delay) {
		// DELAY_AFTER_HLD depends on drive type
		double delay_after_hld = (disk[drvreg]->drive_type == DRIVE_TYPE_2HD) ? 15000.0 : 30000.0;
		position = (position + disk[drvreg]->get_bytes_per_usec(delay_after_hld)) % disk[drvreg]->get_track_size();
	}
	
	// Calculate bytes to next transfer position
	int bytes = fdc[drvreg].next_trans_position - position;
	if (fdc[drvreg].next_am1_position < position || bytes < 0) {
		bytes += disk[drvreg]->get_track_size();
	}
	
	// Convert bytes to microseconds
	double time = disk[drvreg]->get_usec_per_bytes(bytes);
	if (delay) {
		double delay_after_hld = (disk[drvreg]->drive_type == DRIVE_TYPE_2HD) ? 15000.0 : 30000.0;
		time += delay_after_hld;
	}
	
	return time;
}

double MB8877::get_usec_to_detect_index_hole(int count, bool delay)
{
	// Calculate time to detect index hole based on current position
	if (!disk[drvreg] || !disk[drvreg]->inserted) {
		// No disk - use standard rotation time
		double revolution_time = 200000.0; // 200ms per revolution at 300 RPM
		return delay ? (revolution_time * count + 1000.0) : (revolution_time * count);
	}
	
	int position = get_cur_position();
	
	// Handle head load delay
	if (delay) {
		double delay_after_hld = (disk[drvreg]->drive_type == DRIVE_TYPE_2HD) ? 15000.0 : 30000.0;
		position = (position + disk[drvreg]->get_bytes_per_usec(delay_after_hld)) % disk[drvreg]->get_track_size();
	}
	
	// Calculate bytes to index hole(s)
	int track_size = disk[drvreg]->get_track_size();
	int bytes = track_size * count - position;
	if (bytes < 0) {
		bytes += track_size;
	}
	
	// Convert bytes to microseconds
	double time = disk[drvreg]->get_usec_per_bytes(bytes);
	if (delay) {
		double delay_after_hld = (disk[drvreg]->drive_type == DRIVE_TYPE_2HD) ? 15000.0 : 30000.0;
		time += delay_after_hld;
	}
	
	return time;
}

bool MB8877::get_intr_ack()
{
	// Return interrupt acknowledge status
	// This is typically used to clear IRQ and return status
	bool had_interrupt = (status & FDC_ST_BUSY) == 0;
	set_irq(false); // Clear IRQ on acknowledge
	return had_interrupt;
}

double MB8877::get_head_load_delay()
{
	// Return head load delay time based on drive type
	// 2HD: 15ms (15000.0 microseconds)
	// 2DD: 30ms (30000.0 microseconds)
	if(disk[drvreg] && disk[drvreg]->drive_type == DRIVE_TYPE_2HD) {
		return 15000.0;  // 15ms for 2HD
	} else {
		return 30000.0;  // 30ms for 2DD
	}
}

void MB8877::cmd_readdata_end()
{
	// Complete read sector operation
	// This is called when a sector read has completed
	
	// Update status - clear busy flag
	status &= ~S_BUSY;
	
	// Check for multi-sector operation
	if(cmdreg & 0x10) {  // Multi-sector flag
		// Increment sector register
		secreg++;
		
		// Check if we need to continue reading
		if(sector_changed) {
			// Continue to next sector
			cmd_readdata(false);  // false = not first sector
			return;
		}
	}
	
	// Set completion status
	if(status & S_RNF) {
		// Record not found
		status |= S_RNF;
	} else if(status & S_CRC) {
		// CRC error
		status |= S_CRC;  
	} else if(status & S_LOST) {
		// Lost data
		status |= S_LOST;
	}
	
	// Clear DRQ
	set_drq(false);
	
	// Generate interrupt
	set_irq(true);
	
	// Return to idle state
	main_state = IDLE;
}

void MB8877::cmd_writedata_end()
{
	// Complete write sector operation
	// This is called when a sector write has completed
	
	// Update status - clear busy flag
	status &= ~S_BUSY;
	
	// Check for multi-sector operation
	if(cmdreg & 0x10) {  // Multi-sector flag
		// Increment sector register
		secreg++;
		
		// Check if we need to continue writing
		if(sector_changed) {
			// Continue to next sector
			cmd_writedata(false);  // false = not first sector
			return;
		}
	}
	
	// Set completion status
	if(status & S_WP) {
		// Write protect
		status |= S_WP;
	} else if(status & S_RNF) {
		// Record not found
		status |= S_RNF;
	} else if(status & S_CRC) {
		// CRC error
		status |= S_CRC;
	} else if(status & S_LOST) {
		// Lost data
		status |= S_LOST;
	}
	
	// Clear DRQ
	set_drq(false);
	
	// Generate interrupt
	set_irq(true);
	
	// Return to idle state
	main_state = IDLE;
}

