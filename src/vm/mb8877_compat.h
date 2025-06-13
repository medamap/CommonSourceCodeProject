/*
	MB8877 Compatibility Layer for WD FDC
	
	This wrapper provides MB8877 interface compatibility using MAME's wd_fdc implementation
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
	
	[ MB8877 / MB8876 / MB8866 / MB89311 Compatibility Wrapper ]
*/

#ifndef _MB8877_COMPAT_H_
#define _MB8877_COMPAT_H_

#ifndef STANDALONE_TEST
#include "vm.h"
#include "../emu.h"  
#include "device.h"
#else
// For testing - rely on external definitions from mock environment
// All types should be defined before including this header
#endif

// Define MAX_DRIVE if not already defined
#ifndef MAX_DRIVE
#define MAX_DRIVE 4
#endif

// Inspired by MAME's wd_fdc implementation (BSD-3-Clause)
// Adapted for CommonSourceCodeProject compatibility

#define SIG_MB8877_ACCESS	0
#define SIG_MB8877_DRIVEREG	1
#define SIG_MB8877_SIDEREG	2
#define SIG_MB8877_MOTOR	3

class DISK;
class NOISE;

// MB8877 compatibility wrapper class
class MB8877 : public DEVICE
{
private:
	// FDC state machine states (from MAME wd_fdc)
	enum {
		// General "doing nothing" state
		IDLE,

		// Main states - the commands
		RESTORE,
		SEEK,
		STEP,
		READ_SECTOR,
		READ_TRACK,
		READ_ID,
		WRITE_TRACK,
		WRITE_SECTOR,

		// Sub states
		SPINUP,
		SPINUP_WAIT,
		SPINUP_DONE,

		SETTLE_WAIT,
		SETTLE_DONE,

		WRITE_PROTECT_WAIT,
		WRITE_PROTECT_DONE,

		DATA_LOAD_WAIT,
		DATA_LOAD_WAIT_DONE,

		SEEK_MOVE,
		SEEK_WAIT_STEP_TIME,
		SEEK_WAIT_STEP_TIME_DONE,
		SEEK_WAIT_STABILIZATION_TIME,
		SEEK_WAIT_STABILIZATION_TIME_DONE,
		SEEK_DONE,

		WAIT_INDEX,
		WAIT_INDEX_DONE,

		SCAN_ID,
		SCAN_ID_FAILED,

		SECTOR_READ,
		SECTOR_WRITE,
		TRACK_DONE,

		// Live states for sector/track operations
		SEARCH_ADDRESS_MARK_HEADER,
		READ_HEADER_BLOCK_HEADER,
		READ_DATA_BLOCK_HEADER,
		READ_ID_BLOCK_TO_LOCAL,
		READ_ID_BLOCK_TO_DMA,
		READ_ID_BLOCK_TO_DMA_BYTE,
		SEARCH_ADDRESS_MARK_DATA,
		SEARCH_ADDRESS_MARK_DATA_FAILED,
		READ_SECTOR_DATA,
		READ_SECTOR_DATA_BYTE,
		READ_TRACK_DATA,
		READ_TRACK_DATA_BYTE,
		WRITE_TRACK_DATA,
		WRITE_BYTE,
		WRITE_BYTE_DONE,
		WRITE_SECTOR_PRE,
		WRITE_SECTOR_PRE_BYTE
	};

	// Status register bits (MAME compatible)
	enum {
		S_BUSY = 0x01,
		S_DRQ  = 0x02,
		S_IP   = 0x02,  // Index pulse (Type I)
		S_TR00 = 0x04,
		S_LOST = 0x04,
		S_CRC  = 0x08,
		S_RNF  = 0x10,  // Record not found
		S_SEEKERR = 0x10,  // Seek error (Type I) - same bit as RNF
		S_HLD  = 0x20,  // Head loaded (Type I)
		S_DDM  = 0x20,  // Deleted data mark (Type II/III)
		S_WP   = 0x40,  // Write protect
		S_NRDY = 0x80   // Not ready
	};

	// Command types
	enum {
		TYPE_I   = 0,  // Restore, Seek, Step
		TYPE_II  = 1,  // Read/Write Sector
		TYPE_III = 2,  // Read Address, Read/Write Track
		TYPE_IV  = 3   // Force Interrupt
	};

	// Live operation info (for sector/track operations)
	struct live_info {
		enum { PT_NONE, PT_CRC_1, PT_CRC_2 };

		int state, next_state;
		uint32_t shift_reg;
		uint16_t crc;
		int bit_counter, byte_counter, previous_type;
		bool data_separator_phase, data_bit_context;
		uint8_t data_reg;
		uint8_t idbuf[6];
		int cur_track_position;
		int track_position_increment;
		bool byte_ready;
	};

	// State machine
	int main_state, sub_state;
	live_info cur_live;
	
	// Output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;
	outputs_t outputs_rdy;
	
	// Drive noise
	NOISE* d_noise_seek;
	NOISE* d_noise_head_down;
	NOISE* d_noise_head_up;
	
	// Drive info - maintains compatibility with original
	struct {
		int track;
		int index;
		bool access;
		bool head_load;
		// write track
		bool id_written;
		bool sector_found;
		bool data_mark_found;
		uint8_t track_buffer[4];  // Last 4 bytes for sector ID
		int sector_length;
		int sector_index;
		int side;
		bool side_changed;
		// timing
		int cur_position;
		int next_trans_position;
		int bytes_before_2nd_drq;
		int next_am1_position;
		uint32_t prev_clock;
	} fdc[MAX_DRIVE];
	
protected:
	// Original DISK array for compatibility - protected for test access
	DISK* disk[MAX_DRIVE];
	
private:
	// Registers - maintain original interface
	uint8_t status, status_tmp;
	uint8_t cmdreg, cmdreg_tmp;
	uint8_t trkreg;
	uint8_t secreg;
	uint8_t datareg;
	uint8_t drvreg;
	uint8_t sidereg;
	uint8_t cmdtype;
	
	// Event system
	int register_id[8];
	
	// Status flags
	bool now_search;
	bool now_seek;
	bool sector_changed;
	int no_command;
	int seektrk;
	bool seekvct;
	bool motor_on;
	bool drive_sel;
	int step_dir;  // Step direction: +1 for inward, -1 for outward
	
#ifdef HAS_MB89311
public:
	// MB89311 extended mode - made public for testing
	bool extended_mode;
	// MB89311 parameter storage
	uint8_t mb89311_params[8];
	// MB89311 format mode flags
	bool mb89311_format_mode;
	bool mb89311_use_params;
private:
#endif
	
	// Timing
	uint32_t prev_drq_clock;
	uint32_t seekend_clock;
	
	// IRQ state tracking
	bool irq_active;
	
	// Internal helper methods
	void update_fdc_status();
	void convert_command(uint8_t mb8877_cmd);
	uint8_t convert_status_to_mb8877();
	void setup_floppy_images();
	void sync_disk_to_floppy(int drv);
	void sync_floppy_to_disk(int drv);
	
	// Event handling
	void cancel_my_event(int event);
	void register_my_event(int event, double usec);
	void register_seek_event(bool first);
	void register_drq_event(int bytes);
	void register_lost_event(int bytes);
	
	// Status helpers
	int get_cur_position();
	double get_usec_to_start_trans(bool first_sector);
	double get_usec_to_next_trans_pos(bool delay);
	double get_usec_to_detect_index_hole(int count, bool delay);
	
	// Image handler helpers
	uint8_t search_track();
	uint8_t search_sector();
	uint8_t search_addr();
	
	// Command processing
	void process_cmd();
	void cmd_restore();
	void cmd_seek();
	void cmd_step();
	void cmd_stepin();
	void cmd_stepout();
	void cmd_step_common();
	void cmd_readdata(bool first_sector);
	void cmd_readdata_end();
	void cmd_writedata(bool first_sector);
	void cmd_writedata_end();
	void cmd_readaddr();
	void cmd_readtrack();
	void cmd_writetrack();
#ifdef HAS_MB89311
	void cmd_format();
#endif
	void cmd_forceint();
	void update_head_flag(int drv, bool head_load);
	void update_ready();
	
	// IRQ/DMA
	void set_irq(bool val);
	void set_drq(bool val);
	
public:
	MB8877(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		this->initialize_output_signals(&outputs_irq);
		this->initialize_output_signals(&outputs_drq);
		this->initialize_output_signals(&outputs_rdy);
		d_noise_seek = NULL;
		d_noise_head_down = NULL;
		d_noise_head_up = NULL;
		// these parameters may be modified before calling initialize()
		drvreg = sidereg = 0;
		motor_on = drive_sel = false;
		step_dir = 0;
#if defined(HAS_MB89311)
		set_device_name(_T("MB89311 FDC"));
#elif defined(HAS_MB8866)
		set_device_name(_T("MB8866 FDC"));
#elif defined(HAS_MB8876)
		set_device_name(_T("MB8876 FDC"));
#else
		set_device_name(_T("MB8877 FDC"));
#endif
	}
	~MB8877();
	
	// common functions - maintain exact original interface
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	void update_config();
#ifdef USE_DEBUGGER
	bool is_debugger_available()
	{
		return true;
	}
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions - maintain exact original interface
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		this->register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask)
	{
		this->register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_rdy(DEVICE* device, int id, uint32_t mask)
	{
		this->register_output_signal(&outputs_rdy, device, id, mask);
	}
	// Overloaded method to match test expectations (4 parameters)
	void set_context_event_manager(DEVICE* device, int id1, int id2, int id3)
	{
		event_manager = device;
		set_event_manager(device);  // Also set in base DEVICE class
	}
	void set_context_noise_seek(NOISE* device)
	{
		d_noise_seek = device;
	}
	NOISE* get_context_noise_seek()
	{
		return d_noise_seek;
	}
	void set_context_noise_head_down(NOISE* device)
	{
		d_noise_head_down = device;
	}
	NOISE* get_context_noise_head_down()
	{
		return d_noise_head_down;
	}
	void set_context_noise_head_up(NOISE* device)
	{
		d_noise_head_up = device;
	}
	NOISE* get_context_noise_head_up()
	{
		return d_noise_head_up;
	}
	DISK* get_disk_handler(int drv)
	{
		return disk[drv];
	}
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	bool is_disk_changed(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
	bool is_drive_ready();
	bool is_drive_ready(int drv);
	uint8_t get_media_type(int drv);
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	void set_drive_rpm(int drv, int rpm);
	void set_drive_mfm(int drv, bool mfm);
	void set_track_size(int drv, int size);
	uint8_t fdc_status();
	bool get_intr_ack();
};

#endif