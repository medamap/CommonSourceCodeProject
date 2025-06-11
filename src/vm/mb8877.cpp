/*
	MB8877 Compatibility Layer for WD FDC
	
	This wrapper provides MB8877 interface compatibility using MAME's wd_fdc implementation approach
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
	
	[ MB8877 / MB8876 / MB8866 / MB89311 Compatibility Wrapper ]
	
	Based on MAME's wd_fdc implementation (BSD-3-Clause)
	Original copyright: Olivier Galibert
*/

#include "mb8877.h"
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

MB8877::~MB8877()
{
	release();
}

void MB8877::initialize()
{
	// Initialize DISK handlers
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(vm, emu);
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
		disk[i]->initialize();
	}
	
	// Initialize noise
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
			disk[i]->release();
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
				// TODO: Implement write track data handling
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
			if(d_noise_seek != NULL) d_noise_seek->play();
		} else if(seektrk < fdc[drvreg].track) {
			fdc[drvreg].track--;
			if(d_noise_seek != NULL) d_noise_seek->play();
		}
		if((cmdreg & 0x10) || ((cmdreg & 0xf0) == 0)) {
			trkreg = fdc[drvreg].track;
		}
		if(seektrk != fdc[drvreg].track) {
			register_seek_event(false);
			break;
		}
		seekend_clock = get_current_clock();
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
		status_tmp = status;
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
		status = status_tmp;
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
			status = status_tmp | (S_BUSY | S_DRQ);
			if(main_state == WRITE_SECTOR) {
				register_lost_event(8);
			} else if(main_state == WRITE_TRACK) {
				register_lost_event(3);
			} else {
				register_lost_event(1);
			}
			fdc[drvreg].cur_position = fdc[drvreg].next_trans_position;
			fdc[drvreg].prev_clock = prev_drq_clock = get_current_clock();
			set_drq(true);
			drive_sel = false;
		}
		break;
		
	case EVENT_DRQ:
		if(status & S_BUSY) {
			status |= S_DRQ;
			register_lost_event(1);
			if((main_state == WRITE_SECTOR || main_state == WRITE_TRACK) && fdc[drvreg].index == 0) {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + fdc[drvreg].bytes_before_2nd_drq) % disk[drvreg]->get_track_size();
			} else {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + 1) % disk[drvreg]->get_track_size();
			}
			if(main_state == READ_SECTOR || main_state == WRITE_SECTOR ||
			   main_state == READ_TRACK || main_state == WRITE_TRACK ||
			   main_state == READ_ID) {
				fdc[drvreg].index++;
			}
			fdc[drvreg].prev_clock = prev_drq_clock = get_current_clock();
			set_drq(true);
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
			if(main_state == WRITE_SECTOR || main_state == WRITE_TRACK) {
				if(fdc[drvreg].index == 0) {
					status &= ~S_BUSY;
					main_state = IDLE;
					set_irq(true);
				} else {
					write_io8(3, 0x00);
				}
			} else {
				read_io8(3);
			}
			status |= S_LOST;
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
	set_irq(false);
	set_drq(false);
	
#ifdef HAS_MB89311
	// MB89311 mode commands
	if(cmdreg == 0xfc) {
		// Delay
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (DELAY   ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		cmdtype = status = 0;
		main_state = IDLE;
		return;
	} else if(cmdreg == 0xfd) {
		// Assign parameter
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (ASGN PAR) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		cmdtype = status = 0;
		main_state = IDLE;
		return;
	} else if(cmdreg == 0xfe) {
		// Assign mode
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (ASGN MOD) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		extended_mode = ((datareg & 1) != 0);
		cmdtype = status = 0;
		main_state = IDLE;
		return;
	} else if(cmdreg == 0xff) {
		// Reset (guess)
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (RESET   ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		cmdtype = 0;
		status = S_TR00;
		main_state = IDLE;
		return;
	}
#endif
	
	// Check drive ready
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
	register_id[event_id] = -1;
	
	// Handle events
	switch(event_id) {
	case EVENT_SEEK:
		// Continue seek operation
		if(seekvct) {
			if(seektrk > trkreg) {
				trkreg++;
			} else if(seektrk < trkreg) {
				trkreg--;
			}
			if(seektrk != trkreg) {
				register_seek_event(false);
				break;
			}
		}
		// Seek completed
		seekend_clock = current_clock();
		set_irq(true);
		break;
		
	case EVENT_SEEKEND:
		// Clear busy after seek
		now_seek = false;
		status &= ~FDC_ST_BUSY;
		break;
		
	case EVENT_SEARCH:
		// Search timeout
		now_search = false;
		if(cmdtype == FDC_CMD_TYPE_2 || cmdtype == FDC_CMD_TYPE_3) {
			status |= FDC_ST_RECNFND;
		}
		status &= ~FDC_ST_BUSY;
		set_irq(true);
		break;
		
	case EVENT_DRQ:
	case EVENT_MULTI1:
	case EVENT_MULTI2:
		// DRQ events
		if(!(status & FDC_ST_DRQ)) {
			status |= FDC_ST_DRQ;
			set_drq(true);
		}
		break;
		
	case EVENT_LOST:
		// Lost data
		status |= FDC_ST_LOSTDATA;
		status &= ~FDC_ST_BUSY;
		set_irq(true);
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
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
	
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
		return disk[drv]->is_inserted();
	}
	return false;
}

bool MB8877::is_disk_changed(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->is_changed();
	}
	return false;
}

void MB8877::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->set_write_protect(value);
	}
}

bool MB8877::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->is_write_protected();
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
		return disk[drv]->get_media_type();
	}
	return MEDIA_TYPE_UNK;
}

void MB8877::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->set_drive_type(type);
	}
}

uint8_t MB8877::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		return disk[drv]->get_drive_type();
	}
	return DRIVE_TYPE_UNK;
}

void MB8877::set_drive_rpm(int drv, int rpm)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->set_drive_rpm(rpm);
	}
}

void MB8877::set_drive_mfm(int drv, bool mfm)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->set_drive_mfm(mfm);
	}
}

void MB8877::set_track_size(int drv, int size)
{
	if(drv < MAX_DRIVE && disk[drv]) {
		disk[drv]->set_track_size(size);
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
void MB8877::process_cmd()
{
	// Process MB8877 command
	uint8_t cmd = cmdreg & 0xf0;
	
	// Clear status bits
	status &= ~(FDC_ST_SEEKERR | FDC_ST_CRCERR | FDC_ST_RECNFND | FDC_ST_LOSTDATA | FDC_ST_WRITEFAULT);
	
	// Determine command type
	if((cmd & 0x80) == 0) {
		// Type I commands
		cmdtype = FDC_CMD_TYPE_1;
		if((cmd & 0xf0) == 0x00) {
			cmd_restore();
		} else if((cmd & 0xf0) == 0x10) {
			cmd_seek();
		} else if((cmd & 0xe0) == 0x20) {
			cmd_step();
		} else if((cmd & 0xe0) == 0x40) {
			cmd_stepin();
		} else if((cmd & 0xe0) == 0x60) {
			cmd_stepout();
		}
	} else if((cmd & 0x40) == 0) {
		// Type II commands
		cmdtype = FDC_CMD_TYPE_2;
		if((cmd & 0x20) == 0) {
			cmd_readdata(true);
		} else {
			cmd_writedata(true);
		}
	} else if((cmd & 0x30) != 0x10) {
		// Type III commands
		cmdtype = FDC_CMD_TYPE_3;
		if(cmd == 0xc0) {
			cmd_readaddr();
		} else if((cmd & 0xf0) == 0xe0) {
			cmd_readtrack();
		} else if((cmd & 0xf0) == 0xf0) {
			cmd_writetrack();
		}
	} else {
		// Type IV command
		cmdtype = FDC_CMD_TYPE_4;
		cmd_forceint();
	}
}

// Command implementations (stubs for now)
void MB8877::cmd_restore()
{
	// Restore command - seek to track 0
	status |= FDC_ST_BUSY;
	now_seek = true;
	seektrk = 0;
	seekvct = true;
	
	// Set head load flag if requested
	if(cmdreg & 0x08) {
		update_head_flag(drvreg, true);
	}
	
	// Start seek
	register_seek_event(true);
}

void MB8877::cmd_seek()
{
	// Seek command
	status |= FDC_ST_BUSY;
	now_seek = true;
	seektrk = datareg;
	seekvct = true;
	
	// Set head load flag if requested
	if(cmdreg & 0x08) {
		update_head_flag(drvreg, true);
	}
	
	// Start seek
	register_seek_event(true);
}

void MB8877::cmd_step()
{
	// Step command - stub
	status |= FDC_ST_BUSY;
	// TODO: Implement step logic
	set_irq(true);
	status &= ~FDC_ST_BUSY;
}

void MB8877::cmd_stepin()
{
	// Step in command - stub
	status |= FDC_ST_BUSY;
	// TODO: Implement step in logic
	set_irq(true);
	status &= ~FDC_ST_BUSY;
}

void MB8877::cmd_stepout()
{
	// Step out command - stub
	status |= FDC_ST_BUSY;
	// TODO: Implement step out logic
	set_irq(true);
	status &= ~FDC_ST_BUSY;
}

void MB8877::cmd_readdata(bool first_sector)
{
	// Read sector command - stub
	status |= FDC_ST_BUSY;
	now_search = true;
	// TODO: Implement read sector logic
	register_drq_event(1);
}

void MB8877::cmd_writedata(bool first_sector)
{
	// Write sector command - stub
	status |= FDC_ST_BUSY;
	now_search = true;
	// TODO: Implement write sector logic
	register_drq_event(1);
}

void MB8877::cmd_readaddr()
{
	// Read address command - stub
	status |= FDC_ST_BUSY;
	// TODO: Implement read address logic
	set_irq(true);
	status &= ~FDC_ST_BUSY;
}

void MB8877::cmd_readtrack()
{
	// Read track command - stub
	status |= FDC_ST_BUSY;
	// TODO: Implement read track logic
	register_drq_event(1);
}

void MB8877::cmd_writetrack()
{
	// Write track command - stub
	status |= FDC_ST_BUSY;
	// TODO: Implement write track logic  
	register_drq_event(1);
}

#ifdef HAS_MB89311
void MB8877::cmd_format()
{
	// Format command - stub (MB89311 only)
	status |= FDC_ST_BUSY;
	// TODO: Implement format logic
	set_irq(true);
	status &= ~FDC_ST_BUSY;
}
#endif

void MB8877::cmd_forceint()
{
	// Force interrupt command
	// Cancel all pending operations
	for(int i = 0; i < 8; i++) {
		cancel_my_event(i);
	}
	
	now_search = now_seek = false;
	status &= ~FDC_ST_BUSY;
	
	// Trigger interrupt if requested
	if(cmdreg & 0x08) {
		set_irq(true);
	}
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
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
}

void MB8877::set_drq(bool val)
{
	// Set DRQ signal
	if(val) {
		prev_drq_clock = current_clock();
	}
	write_signals(&outputs_drq, val ? 0xffffffff : 0);
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
	register_event(this, event, usec, false, &register_id[event]);
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
	if(!disk[drvreg]->get_sector(-1, -1)) {
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
	if(!disk[drvreg]->get_sector_info(position, secreg, fdc[drvreg].track, sidereg, -1)) {
		// Sector not found
		return S_RNF;
	}
	
	// Save next position
	fdc[drvreg].next_trans_position = disk[drvreg]->data_position[0];
	fdc[drvreg].bytes_before_2nd_drq = 1;
	
	return 0;
}

uint8_t MB8877::search_addr()
{
	// Get current position
	int position = get_cur_position();
	
	// Search for next ID field
	if(!disk[drvreg]->get_sector_info(position, -1, -1, -1, 0)) {
		// No ID field found
		return S_RNF;
	}
	
	// Save position after ID field
	fdc[drvreg].next_trans_position = disk[drvreg]->id_position[0] + 6;
	
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
	
	// Track 00?
	if(fdc[drvreg].track == 0) {
		// Already at track 0
		trkreg = 0;
		status = S_HLD | S_TR00;
		set_irq(true);
	} else {
		// Start seeking to track 0
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
	
	// Already at target track?
	if(fdc[drvreg].track == seektrk) {
		// Verify if requested
		if(cmdreg & 4) {
			status_tmp = status;
			status_tmp |= search_track();
			double time;
			if(status_tmp & S_RNF) {
				time = get_usec_to_detect_index_hole(5, true);
			} else {
				time = get_usec_to_next_trans_pos(true);
			}
			register_my_event(EVENT_SEEKEND, time);
		} else {
			// Seek complete
			status = S_HLD;
			if(fdc[drvreg].track == 0) {
				status |= S_TR00;
			}
			set_irq(true);
		}
	} else {
		// Start seeking
		register_seek_event(true);
	}
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
	
	// Update track register if requested
	if(cmdreg & 0x10) {
		if(seekvct) {
			if(seektrk > fdc[drvreg].track) {
				trkreg++;
			} else if(seektrk < fdc[drvreg].track) {
				trkreg--;
			}
		}
	}
	
	// Start step
	main_state = STEP;
	register_seek_event(true);
}

// Type II Commands
void MB8877::cmd_readdata(bool first_sector)
{
	// Head load
	if(first_sector) {
		if((cmdreg & 0xf0) != 0x00) {
			// Set head load for first sector
			update_head_flag(drvreg, true);
		}
	}
	
	// Start read sector
	main_state = READ_SECTOR;
	status_tmp = status = S_BUSY;
	fdc[drvreg].index = 0;
	
	// Set side
	int side = (cmdreg >> 3) & 1;
	if(cmdreg & 2) {
		// Side compare
		side = (cmdreg >> 3) & 1;
	} else {
		// Use sidereg
		side = sidereg;
	}
	fdc[drvreg].side = side;
	
	// Get track and side
	int track = fdc[drvreg].track;
	if(!disk[drvreg]->get_track(track, side)) {
		// No disk or track
		cmd_readdata_end();
		return;
	}
	
	// Search sector
	sector_changed = false;
	now_search = true;
	
	double time;
	if(first_sector) {
		// Search from current position
		time = get_usec_to_start_trans(true);
	} else {
		// Continue from next sector
		time = get_usec_to_start_trans(false);
	}
	
	// Check if sector exists
	status_tmp |= search_sector();
	if(status_tmp & S_RNF) {
		// Sector not found
		time = get_usec_to_detect_index_hole(5, false);
	}
	register_my_event(EVENT_SEARCH, time);
}

void MB8877::cmd_readdata_end()
{
	// End of read
	now_search = false;
	status &= ~S_BUSY;
	main_state = IDLE;
	
	// Clear head load if not specified
	if(!(cmdreg & 8)) {
		update_head_flag(drvreg, false);
	}
	set_irq(true);
}

void MB8877::cmd_writedata(bool first_sector)
{
	// Check write protect
	if(disk[drvreg]->write_protected) {
		status = S_WP;
		main_state = IDLE;
		set_irq(true);
		return;
	}
	
	// Head load
	if(first_sector) {
		if((cmdreg & 0xf0) != 0x00) {
			// Set head load for first sector
			update_head_flag(drvreg, true);
		}
	}
	
	// Start write sector
	main_state = WRITE_SECTOR;
	status_tmp = status = S_BUSY;
	fdc[drvreg].index = 0;
	
	// Set side
	int side = (cmdreg >> 3) & 1;
	if(cmdreg & 2) {
		// Side compare
		side = (cmdreg >> 3) & 1;
	} else {
		// Use sidereg
		side = sidereg;
	}
	fdc[drvreg].side = side;
	
	// Get track and side
	int track = fdc[drvreg].track;
	if(!disk[drvreg]->get_track(track, side)) {
		// No disk or track
		cmd_writedata_end();
		return;
	}
	
	// Search sector
	sector_changed = false;
	now_search = true;
	
	double time;
	if(first_sector) {
		// Search from current position
		time = get_usec_to_start_trans(true);
	} else {
		// Continue from next sector
		time = get_usec_to_start_trans(false);
	}
	
	// Check if sector exists
	status_tmp |= search_sector();
	if(status_tmp & S_RNF) {
		// Sector not found
		time = get_usec_to_detect_index_hole(5, false);
	}
	register_my_event(EVENT_SEARCH, time);
}

void MB8877::cmd_writedata_end()
{
	// End of write
	now_search = false;
	status &= ~S_BUSY;
	main_state = IDLE;
	
	// Write buffered data
	if(sector_changed) {
		disk[drvreg]->set_data_crc_error(false);
		sector_changed = false;
	}
	
	// Clear head load if not specified
	if(!(cmdreg & 8)) {
		update_head_flag(drvreg, false);
	}
	set_irq(true);
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
	
	// Format track
	disk[drvreg]->format_track(track, side);
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
	// MB89311 format command
	// Similar to write track but with specific format
	cmd_writetrack();
}
#endif

// Type IV Commands
void MB8877::cmd_forceint()
{
	// Force interrupt
	bool now_busy = (status & S_BUSY) != 0;
	
	// Cancel all events
	for(int i = 0; i < 8; i++) {
		cancel_my_event(i);
	}
	
	// Clear busy
	status &= ~S_BUSY;
	main_state = IDLE;
	now_search = false;
	now_seek = false;
	
	// Set IRQ if busy was set or interrupt conditions met
	if(now_busy || (cmdreg & 0x0f)) {
		set_irq(true);
	}
	
	// Clear command type
	cmdtype = 0;
}

// Helper functions
void MB8877::update_head_flag(int drv, bool head_load)
{
	if(drv >= MAX_DRIVE) return;
	
	if(fdc[drv].head_load != head_load) {
		if(head_load) {
			if(d_noise_head_down != NULL) {
				d_noise_head_down->play();
			}
		} else {
			if(d_noise_head_up != NULL) {
				d_noise_head_up->play();
			}
		}
		fdc[drv].head_load = head_load;
	}
}

void MB8877::update_ready()
{
	// Update ready status
	bool ready = false;
	
	if(drvreg < MAX_DRIVE) {
		if(disk[drvreg]->inserted && motor_on) {
			ready = true;
		}
	}
	
	// Update ready signal
	write_signals(&outputs_rdy, ready ? 0xffffffff : 0);
}

void MB8877::set_irq(bool val)
{
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
}

void MB8877::set_drq(bool val)
{
	if(val != ((outputs_drq.value != 0) ? true : false)) {
		write_signals(&outputs_drq, val ? 0xffffffff : 0);
		
		// Set/Clear DRQ in status
		if(val) {
			if(main_state != IDLE) {
				status |= S_DRQ;
			}
		} else {
			status &= ~S_DRQ;
		}
	}
}

// Timing functions
int MB8877::get_cur_position()
{
	// Calculate current position on track
	if(drvreg >= MAX_DRIVE) return 0;
	
	double usec = get_passed_usec(fdc[drvreg].prev_clock);
	int bytes = (int)(usec / disk[drvreg]->get_usec_per_bytes(1));
	
	return (fdc[drvreg].cur_position + bytes) % disk[drvreg]->get_track_size();
}

double MB8877::get_usec_to_start_trans(bool first_sector)
{
	// Calculate time to start of sector transfer
	int position = get_cur_position();
	
	if(first_sector) {
		// Find next sector
		if(disk[drvreg]->get_sector_info(position, secreg, trkreg, sidereg, -1)) {
			int bytes = disk[drvreg]->data_position[0] - position;
			if(bytes < 0) {
				bytes += disk[drvreg]->get_track_size();
			}
			return disk[drvreg]->get_usec_per_bytes(bytes);
		}
	} else {
		// Multi-sector - immediate
		return 10.0;
	}
	
	// Default - wait for index
	return get_usec_to_detect_index_hole(1, false);
}

double MB8877::get_usec_to_next_trans_pos(bool delay)
{
	// Calculate time to next transfer position
	int bytes = 1;
	
	if(delay) {
		// Add settling time
		bytes += 8;
	}
	
	return disk[drvreg]->get_usec_per_bytes(bytes);
}

double MB8877::get_usec_to_detect_index_hole(int count, bool delay)
{
	// Calculate time to index hole
	int position = get_cur_position();
	int bytes = disk[drvreg]->get_track_size() - position;
	
	// Add full rotations
	if(count > 1) {
		bytes += (count - 1) * disk[drvreg]->get_track_size();
	}
	
	// Add delay if requested
	if(delay) {
		bytes += 100; // Small delay after index
	}
	
	return disk[drvreg]->get_usec_per_bytes(bytes);
}

// Disk interface functions
void MB8877::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
	}
}

void MB8877::close_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->close();
		update_ready();
	}
}

bool MB8877::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

bool MB8877::is_disk_changed(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->changed;
	}
	return false;
}

void MB8877::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool MB8877::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
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
		return disk[drv]->inserted && motor_on;
	}
	return false;
}

uint8_t MB8877::get_media_type(int drv)
{
	if(drv < MAX_DRIVE) {
		if(disk[drv]->inserted) {
			return disk[drv]->media_type;
		}
	}
	return MEDIA_TYPE_UNK;
}

void MB8877::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_type = type;
	}
}

uint8_t MB8877::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void MB8877::set_drive_rpm(int drv, int rpm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->set_drive_rpm(rpm);
	}
}

void MB8877::set_drive_mfm(int drv, bool mfm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->set_drive_mfm(mfm);
	}
}

void MB8877::set_track_size(int drv, int size)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->set_track_size(size);
	}
}

uint8_t MB8877::fdc_status()
{
	// Return FDC status
	if(!disk[drvreg]->inserted || !motor_on) {
		status |= S_NRDY;
	} else {
		status &= ~S_NRDY;
	}
	
	// Track 00
	if(fdc[drvreg].track == 0) {
		status |= S_TR00;
	} else {
		status &= ~S_TR00;
	}
	
	// Write protect
	if(disk[drvreg]->write_protected) {
		status |= S_WP;
	} else {
		status &= ~S_WP;
	}
	
	return status;
}

// Update config
void MB8877::update_config()
{
	if(d_noise_seek != NULL) {
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
}

// State save/load
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
	
	// Save/load state machine
	state_fio->StateValue(main_state);
	state_fio->StateValue(sub_state);
	
	// Save/load flags
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
#endif
	
	// Save/load timing
	state_fio->StateValue(prev_drq_clock);
	state_fio->StateValue(seekend_clock);
	
	// Save/load events
	for(int i = 0; i < 8; i++) {
		state_fio->StateValue(register_id[i]);
	}
	
	// Save/load FDC info
	state_fio->StateArray(fdc, sizeof(fdc), 1);
	
	// Save/load disk states
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
	
	return true;
}

#ifdef USE_DEBUGGER
bool MB8877::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR tmp[256];
	_sntprintf(buffer, buffer_len, _T("MB8877 FDC:\n"));
	
	_sntprintf(tmp, 256, _T("STATUS=%02X CMD=%02X TRK=%02X SEC=%02X DATA=%02X\n"),
		status, cmdreg, trkreg, secreg, datareg);
	_tcscat(buffer, tmp);
	
	_sntprintf(tmp, 256, _T("DRV=%d SIDE=%d MOTOR=%s TYPE=%d STATE=%d\n"),
		drvreg, sidereg, motor_on ? _T("ON") : _T("OFF"), cmdtype, main_state);
	_tcscat(buffer, tmp);
	
	_sntprintf(tmp, 256, _T("CUR TRK=%d SEEK TRK=%d SEEK=%s SEARCH=%s\n"),
		fdc[drvreg].track, seektrk, now_seek ? _T("YES") : _T("NO"), 
		now_search ? _T("YES") : _T("NO"));
	_tcscat(buffer, tmp);
	
	return true;
}
#endif