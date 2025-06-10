/*
	MB8877 Compatibility Layer for WD FDC
	
	This wrapper provides MB8877 interface compatibility using MAME's wd_fdc implementation
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
	
	[ MB8877 / MB8876 / MB8866 / MB89311 Compatibility Wrapper ]
*/

#include "mb8877_compat.h"
#include "disk.h"
#include "noise.h"
#include "../fileio.h"

// Include MAME headers
// Note: These paths may need adjustment based on actual MAME integration
#include "../../temp/mame-wd_fdc/wd_fdc.h"

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
	// Create MAME mb8877 device instance
	// Note: This is a simplified initialization - actual MAME integration may differ
	m_fdc = new mb8877_device(nullptr, "mb8877", nullptr, 1000000); // 1MHz clock
	
	// Initialize DISK handlers
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(vm, emu);
		disk[i]->set_device_name(_T("Floppy Disk #%d"), i + 1);
		disk[i]->initialize();
	}
	
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
	
	// Release MAME device
	if(m_fdc) {
		delete m_fdc;
		m_fdc = NULL;
	}
}

void MB8877::reset()
{
	// Reset registers
	status = FDC_ST_NOTREADY;
	status_tmp = 0;
	cmdreg = cmdreg_tmp = 0;
	trkreg = 0;
	secreg = 0;
	datareg = 0;
	cmdtype = FDC_CMD_TYPE_1;
	
	// Reset state
	now_search = now_seek = false;
	sector_changed = false;
	no_command = 0;
	seektrk = 0;
	seekvct = false;
	
#ifdef HAS_MB89311
	extended_mode = false;
#endif
	
	// Reset drive info
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc[i].track = 0;
		fdc[i].index = 0;
		fdc[i].access = false;
		fdc[i].head_load = false;
		fdc[i].id_written = false;
		fdc[i].sector_found = false;
		fdc[i].sector_length = 0;
		fdc[i].sector_index = 0;
		fdc[i].side = 0;
		fdc[i].side_changed = false;
		fdc[i].cur_position = 0;
		fdc[i].next_trans_position = 0;
		fdc[i].bytes_before_2nd_drq = 0;
		fdc[i].next_am1_position = 0;
		fdc[i].prev_clock = 0;
	}
	
	// Reset timing
	prev_drq_clock = 0;
	seekend_clock = 0;
	
	// Cancel all events
	for(int i = 0; i < 8; i++) {
		cancel_my_event(i);
	}
	
	// Reset MAME FDC
	if(m_fdc) {
		m_fdc->reset();
	}
	
	// Update ready signal
	update_ready();
}

void MB8877::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case FDC_COMMAND:
		// Command register
		cmdreg_tmp = cmdreg = data;
		process_cmd();
		break;
		
	case FDC_TRACK:
		// Track register
		trkreg = data;
		if(m_fdc) {
			m_fdc->track_w(data);
		}
		break;
		
	case FDC_SECTOR:
		// Sector register
		secreg = data;
		if(m_fdc) {
			m_fdc->sector_w(data);
		}
		break;
		
	case FDC_DATA:
		// Data register
		datareg = data;
		if(m_fdc) {
			m_fdc->data_w(data);
		}
		// Clear DRQ on write
		if(status & FDC_ST_DRQ) {
			status &= ~FDC_ST_DRQ;
			set_drq(false);
		}
		break;
	}
}

uint32_t MB8877::read_io8(uint32_t addr)
{
	switch(addr & 3) {
	case FDC_STATUS:
		// Status register
		if(now_search) {
			// Update status during search operation
			status &= ~FDC_ST_BUSY;
		}
		return fdc_status();
		
	case FDC_TRACK:
		// Track register
		return trkreg;
		
	case FDC_SECTOR:
		// Sector register  
		return secreg;
		
	case FDC_DATA:
		// Data register
		if(m_fdc) {
			datareg = m_fdc->data_r();
		}
		// Clear DRQ on read
		if(status & FDC_ST_DRQ) {
			status &= ~FDC_ST_DRQ;
			set_drq(false);
		}
		return datareg;
	}
	return 0xff;
}

void MB8877::write_dma_io8(uint32_t addr, uint32_t data)
{
	// DMA write is same as regular write to data register
	write_io8(FDC_DATA, data);
}

uint32_t MB8877::read_dma_io8(uint32_t addr)
{
	// DMA read is same as regular read from data register
	return read_io8(FDC_DATA);
}

void MB8877::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MB8877_DRIVEREG:
		// Drive select
		drvreg = data & mask;
		break;
		
	case SIG_MB8877_SIDEREG:
		// Side select
		sidereg = data & mask;
		if(drvreg < MAX_DRIVE) {
			fdc[drvreg].side = sidereg & 1;
		}
		break;
		
	case SIG_MB8877_MOTOR:
		// Motor control
		motor_on = ((data & mask) != 0);
		update_ready();
		break;
		
	case SIG_MB8877_ACCESS:
		// Drive access LED
		if(drvreg < MAX_DRIVE) {
			fdc[drvreg].access = ((data & mask) != 0);
		}
		break;
	}
}

uint32_t MB8877::read_signal(int ch)
{
	// Return IRQ/DRQ status
	switch(ch) {
	case SIG_MB8877_ACCESS:
		return (drvreg < MAX_DRIVE) ? (fdc[drvreg].access ? 1 : 0) : 0;
	}
	return 0;
}

void MB8877::event_callback(int event_id, int err)
{
	// Clear event
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

void MB8877::register_drq_event(int bytes)
{
	// Register DRQ event
	double usec = 30; // Default delay
	register_my_event(EVENT_DRQ, usec);
}

void MB8877::register_lost_event(int bytes)
{
	// Register lost data event
	double usec = 16 * bytes; // Estimate based on byte count
	register_my_event(EVENT_LOST, usec);
}

// Timing helpers (stubs for now)
int MB8877::get_cur_position()
{
	// TODO: Implement based on disk rotation
	return 0;
}

double MB8877::get_usec_to_start_trans(bool first_sector)
{
	// TODO: Calculate time to start of sector
	return 100.0;
}

double MB8877::get_usec_to_next_trans_pos(bool delay)
{
	// TODO: Calculate time to next byte position
	return 16.0;
}

double MB8877::get_usec_to_detect_index_hole(int count, bool delay)
{
	// TODO: Calculate time to index hole
	return 200000.0; // 200ms for one rotation
}

// Search helpers (stubs for now)
uint8_t MB8877::search_track()
{
	// TODO: Search for track
	return 0;
}

uint8_t MB8877::search_sector()
{
	// TODO: Search for sector
	return 0;
}

uint8_t MB8877::search_addr()
{
	// TODO: Search for address mark
	return 0;
}