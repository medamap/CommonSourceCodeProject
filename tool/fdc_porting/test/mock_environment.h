/*
	Mock Environment for MB8877 Testing
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#ifndef _MOCK_ENVIRONMENT_H_
#define _MOCK_ENVIRONMENT_H_

#include "../../../src/common.h"

// Ensure standard integer types are available
#include <stdint.h>
#include <cstdint>
#include <functional>

// Define missing constants for disk operations
#define MEDIA_TYPE_UNK 0
#define DRIVE_TYPE_UNK 0
#define DRIVE_TYPE_2HD 2

// Define MAX_DRIVE if not already defined
#ifndef MAX_DRIVE
#define MAX_DRIVE 4
#endif

// Define missing constants for standalone testing
#ifndef FRAMES_PER_SEC
#define FRAMES_PER_SEC 60
#endif

#ifndef LINES_PER_FRAME  
#define LINES_PER_FRAME 262
#endif

#ifndef CPU_CLOCKS
#define CPU_CLOCKS 4000000
#endif

// Forward declarations
class FILEIO;

// Mock FILEIO class for state saving
class FILEIO {
public:
	virtual ~FILEIO() {}
	virtual int fgetc() { return 0; }
	virtual int fputc(int c) { return c; }
	virtual size_t fread(void* ptr, size_t size, size_t nmemb) { return 0; }
	virtual size_t fwrite(const void* ptr, size_t size, size_t nmemb) { return size * nmemb; }
	virtual bool StateCheckUint32(uint32_t x) { return true; }
	virtual bool StateCheckInt32(int32_t x) { return true; }
	virtual void StateValue(uint32_t& x) {}
	virtual void StateValue(int32_t& x) {}
	virtual void StateValue(uint16_t& x) {}
	virtual void StateValue(uint8_t& x) {}
	virtual void StateValue(bool& x) {}
	virtual void StateArray(void* x, size_t size, size_t count) {}
};

// Prevent including the real EMU class
#define _EMU_H_

// Define missing constants
#ifndef STATE_VERSION
#define STATE_VERSION 1
#endif

// Basic EMU class for testing
class EMU {
public:
	virtual ~EMU() {}
	virtual uint32_t get_current_clock() { return 0; }
	virtual uint32_t get_cpu_clock(int) { return 4000000; }
	virtual bool is_frame_skippable() { return false; }
	virtual void out_debug_log(const _TCHAR* format, ...) {}
	virtual void force_out_debug_log(const _TCHAR* format, ...) {}
};

// Forward declaration of DEVICE for linked list
class DEVICE;

// Define output signal types first (from device.h)
#ifndef MAX_OUTPUT
#define MAX_OUTPUT 16
#endif

// device to device output structure (from device.h)
typedef struct {
	DEVICE *device;
	int id;
	uint32_t mask;
	int shift;
} output_t;

typedef struct {
	int count;
	output_t item[MAX_OUTPUT];
} outputs_t;

// Define VM_TEMPLATE before any includes that might need it
class VM_TEMPLATE {
protected:
	EMU* emu;
public:
	DEVICE* first_device;
	DEVICE* last_device;
	
	VM_TEMPLATE(EMU* parent_emu) : emu(parent_emu), first_device(NULL), last_device(NULL) {}
	virtual ~VM_TEMPLATE() {}
};

// Basic DEVICE class for testing
class DEVICE {
protected:
	VM_TEMPLATE* vm;
	EMU* emu;
	DEVICE* event_manager;  // Event manager pointer
	
public:
	DEVICE* next_device;
	DEVICE* prev_device;
	int this_device_id;
	_TCHAR this_device_name[64];
	
	DEVICE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : vm(parent_vm), emu(parent_emu), event_manager(NULL), next_device(NULL), prev_device(NULL), this_device_id(0) {
		memset(this_device_name, 0, sizeof(this_device_name));
		strcpy(this_device_name, _T("MockDevice"));
		// Simple device linking
		if (vm) {
			if (vm->last_device) {
				vm->last_device->next_device = this;
				prev_device = vm->last_device;
			} else {
				vm->first_device = this;
			}
			vm->last_device = this;
		}
	}
	
	virtual ~DEVICE() {}
	
	// Essential methods that MB8877 needs
	virtual void initialize() {}
	virtual void reset() {}
	virtual void release() {}
	virtual uint32_t get_current_clock() { return 0; }
	virtual void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id) {
		if (event_manager) {
			event_manager->register_event(device, event_id, usec, loop, register_id);
		} else {
			if (register_id) *register_id = -1;
		}
	}
	virtual void cancel_event(DEVICE* device, int register_id) {
		if (event_manager) {
			event_manager->cancel_event(device, register_id);
		}
	}
	virtual void write_signals(void* outputs, uint32_t data) {}
	virtual void write_signal(int id, uint32_t data, uint32_t mask) {}
	virtual void event_callback(int event_id, int err) {}  // Add event callback method
	virtual void set_device_name(const _TCHAR* format, ...) {}
	
	// Method to set event manager (called by set_context_event_manager)
	virtual void set_event_manager(DEVICE* mgr) {
		event_manager = mgr;
	}
	
	// Output signal methods (from device.h)
	virtual void initialize_output_signals(outputs_t *items) {
		items->count = 0;
	}
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, int shift) {
		int c = items->count++;
		if (c < MAX_OUTPUT) {
			items->item[c].device = device;
			items->item[c].id = id;
			items->item[c].mask = mask;
			items->item[c].shift = shift;
		}
	}
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask) {
		register_output_signal(items, device, id, mask, 0);
	}
	virtual void write_signals(outputs_t *items, uint32_t data) {
		for (int i = 0; i < items->count; i++) {
			if (items->item[i].device) {
				items->item[i].device->write_signal(items->item[i].id, (data & items->item[i].mask) >> items->item[i].shift, items->item[i].mask);
			}
		}
	}
	
	// For testing, provide minimal event manager functionality
	// (event_manager is inherited from DEVICE base class)
};

// output types already defined above

// Prevent including the real classes - these guards are set by test files
#ifndef _VM_TEMPLATE_H_
#define _VM_TEMPLATE_H_
#endif
#ifndef _DEVICE_H_
#define _DEVICE_H_
#endif

// We provide our own mocks instead of including real headers
// #include "../../../src/fileio.h" 
// #include "../../../src/vm/event.h"
// #include "../../../src/vm/disk.h"
// #include "../../../src/vm/noise.h"
#include <cstring>
#include <vector>

// Forward declaration
class MockEVENT;

// Base DISK class (enhanced for MB8877 compatibility)
class DISK {
public:
	// Basic disk state
	bool write_protected;
	bool inserted;
	bool _ignore_crc;
	bool data_crc_error;
	bool addr_crc_error;  // Keep as property for direct access
	bool changed;
	bool is_special_disk;
	int media_type;
	int drive_type;
	int drive_rpm;
	bool drive_mfm;
	int track_size;
	
	// Additional properties used by original MB8877
	bool invalid_format;
	bool no_skew;
	bool deleted;
	uint8_t* unstable;  // Unstable data mask for read operations
	
	// Disk data structures (mock complex types with simple ones)
	struct {
		int sd;  // single density size  
	} sector_size;
	
	uint8_t* sector;  // Current sector data
	uint8_t* id;      // ID field data
	uint8_t* track;   // Track data
	
	// Position arrays and sector info used by original MB8877
	static const int MAX_SECTORS = 32;
	int id_position[MAX_SECTORS];
	int data_position[MAX_SECTORS];
	int am1_position[MAX_SECTORS];
	bool sector_mfm;
	struct {
		int sd;  // sector number
	} sector_num;
	
	virtual ~DISK() { 
		if (sector) delete[] sector;
		if (id) delete[] id;
		if (track) delete[] track;
		if (unstable) delete[] unstable;
	}
	
	DISK() : write_protected(false), inserted(false), _ignore_crc(false), data_crc_error(false), addr_crc_error(false), changed(false), is_special_disk(false), media_type(0), drive_type(0), drive_rpm(300), drive_mfm(true), track_size(6250), invalid_format(false), no_skew(false), deleted(false), sector(nullptr), id(nullptr), track(nullptr), unstable(nullptr), sector_mfm(false) {
		sector_size.sd = 256;
		sector_num.sd = 16;  // Default 16 sectors per track
		sector = new uint8_t[512];  // Larger buffer for safety
		id = new uint8_t[16];
		track = new uint8_t[6250];  // Standard track size
		unstable = new uint8_t[6250];  // Unstable data mask array
		memset(sector, 0, 512);
		memset(id, 0, 16);
		memset(track, 0, 6250);
		memset(unstable, 0, 6250);  // Initialize unstable mask to stable (0)
		
		// Initialize position arrays
		for (int i = 0; i < MAX_SECTORS; i++) {
			id_position[i] = i * 390;      // Approximate sector spacing
			data_position[i] = i * 390 + 30;  // Data follows ID
			am1_position[i] = i * 390 - 10;   // AM1 before ID
		}
	}
	
	virtual void open(const _TCHAR* path, int bank) {}
	virtual void close() {}
	virtual bool is_disk_inserted() { return false; }
	virtual bool is_inserted() { return false; }
	virtual void set_write_protect(bool val) { write_protected = val; }
	virtual bool is_disk_protected() { return write_protected; }
	virtual bool get_track(int trk, int side) { return false; }
	virtual bool get_sector(int trk, int side, int sec) { 
		// Mock implementation supports both 3-param and index-based access
		if (sec >= 0 && sec < sector_num.sd) {
			// Set up mock sector data
			id[0] = trk;    // cylinder/track
			id[1] = side;   // head/side  
			id[2] = sec;    // sector number
			id[3] = 1;      // sector size code (256 bytes)
			return true;
		}
		return false;
	}
	
	// Additional methods expected by MB8877
	virtual void set_device_name(const _TCHAR* format, ...) {}
	virtual void set_data_crc_error(bool error) { data_crc_error = error; }
	virtual void set_data_mark_missing(bool missing = true) {}  // Default parameter
	virtual void sync_buffer() {}  // Method not member variable
	virtual void set_deleted(bool deleted) {}  // Set deleted data mark
	virtual bool ignore_crc() { return _ignore_crc; }
	virtual int get_track_size() { return 6250; }  // Standard track size
	virtual bool process_state(FILEIO* state_fio, bool loading) { return true; }  // State save/load
	
	// Timing methods required by original MB8877
	virtual double get_usec_per_bytes(int bytes) { 
		// Simulate 2HD disk timing: 360rpm, ~16.67ms per revolution, 6250 bytes per track
		return (double)bytes * 16670.0 / 6250.0;  // microseconds
	}
	virtual double get_usec_per_track() { return 16670.0; }  // 360rpm = 16.67ms per revolution
	virtual int get_bytes_per_usec(double usec) { 
		return (int)(usec * 6250.0 / 16670.0); 
	}
	virtual int get_max_tracks() { return 84; }  // 2HD disk has 84 tracks
	virtual bool correct_timing() { return false; }  // Use standard timing
	
	// Track/sector management
	virtual void make_track(int track, int side) {}
	virtual void format_track(int track, int side) {}
	virtual void insert_sector(uint8_t c, uint8_t h, uint8_t r, uint8_t n, bool deleted, bool crc_error, uint8_t fill_data, int length) {}
	
	// Constructor that takes EMU parameter (required by original)
	DISK(EMU* emu) : write_protected(false), inserted(false), _ignore_crc(false), data_crc_error(false), addr_crc_error(false), changed(false), is_special_disk(false), media_type(0), drive_type(0), drive_rpm(300), drive_mfm(true), track_size(6250), invalid_format(false), no_skew(false), deleted(false), sector(nullptr), id(nullptr), track(nullptr), unstable(nullptr), sector_mfm(false) {
		sector_size.sd = 256;
		sector_num.sd = 16;  // Default 16 sectors per track
		sector = new uint8_t[512];  // Larger buffer for safety
		id = new uint8_t[16];
		track = new uint8_t[6250];  // Standard track size
		unstable = new uint8_t[6250];  // Unstable data mask array
		memset(sector, 0, 512);
		memset(id, 0, 16);
		memset(track, 0, 6250);
		memset(unstable, 0, 6250);  // Initialize unstable mask to stable (0)
		
		// Initialize position arrays
		for (int i = 0; i < MAX_SECTORS; i++) {
			id_position[i] = i * 390;      // Approximate sector spacing
			data_position[i] = i * 390 + 30;  // Data follows ID
			am1_position[i] = i * 390 - 10;   // AM1 before ID
		}
	}
};

// Base NOISE class  
class NOISE {
public:
	virtual ~NOISE() {}
	virtual void play() {}
	virtual void stop() {}
	
	// Methods required by original MB8877
	virtual void set_device_name(const _TCHAR* format, ...) {}
	virtual bool load_wav_file(const _TCHAR* filename) { return true; }
	virtual void set_mute(bool mute) {}
};

// Mock EVENT class
class MockEVENT : public DEVICE {
private:
	struct EventInfo {
		int device_id;
		int event_id;
		uint32_t clock;
		bool active;
		DEVICE* device;
	};
	
	std::vector<EventInfo> events;
	uint32_t current_clock;
	
public:
	MockEVENT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu), current_clock(0) {}
	
	void initialize() {}
	void reset() { 
		events.clear(); 
		current_clock = 0;
	}
	
	void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id) {
		static int next_id = 1;
		EventInfo info;
		info.device_id = next_id;
		info.event_id = event_id;
		info.clock = current_clock + (uint32_t)(usec * 4); // Assuming 4MHz
		info.active = true;
		info.device = device;  // Store device pointer
		events.push_back(info);
		
		if (register_id) {
			*register_id = next_id;
		}
		next_id++;
	}
	
	void cancel_event(DEVICE* device, int register_id) {
		for (auto& evt : events) {
			if (evt.device_id == register_id) {
				evt.active = false;
			}
		}
	}
	
	// Advance time and trigger events
	void advance_clock(uint32_t cycles) {
		current_clock += cycles;
		// Trigger events that should occur
		for (auto& evt : events) {
			if (evt.active && current_clock >= evt.clock) {
				evt.active = false; // Mark as processed
				// Call the device's event_callback method directly
				if (evt.device) {
					evt.device->event_callback(evt.event_id, 0);
				}
			}
		}
	}
	
	uint32_t get_current_clock() { return current_clock; }
	uint32_t get_cpu_clock(int) { return 4000000; }
};

// Mock VM_TEMPLATE with proper event manager setup
class MockVM : public VM_TEMPLATE {
public:
	MockEVENT* event_device;
	
	MockVM(EMU* parent_emu) : VM_TEMPLATE(parent_emu) {
		event_device = new MockEVENT(this, parent_emu);
		first_device = event_device;
		last_device = event_device;
		event_device->next_device = event_device; // Point to itself for event manager
	}
	virtual ~MockVM() {
		if (event_device) delete event_device;
	}
};

// Mock EMU class  
class MockEMU : public EMU {
public:
	uint32_t current_cpu_clock;
	
	MockEMU() : current_cpu_clock(4000000) {} // 4MHz default
	
	uint32_t get_current_clock() { return 0; }
	uint32_t get_cpu_clock(int) { return current_cpu_clock; }
	bool is_frame_skippable() { return false; }
};

// Mock DISK class
class MockDISK : public DISK {
private:
	bool crc_error_state;
	uint8_t* track_buffer;
	int current_track;
	int current_sector;
	
public:
	MockDISK(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DISK() {
		crc_error_state = false;
		track_buffer = nullptr;
		current_track = 0;
		current_sector = 0;
		
		// Initialize position arrays
		memset(id_position, 0, sizeof(id_position));
		memset(data_position, 0, sizeof(data_position));
	}
	
	~MockDISK() {
		if (track_buffer) {
			delete[] track_buffer;
		}
	}
	
	void open(const _TCHAR* path, int bank) {
		DISK::inserted = true;  // Use base class member
		// Create mock disk data
		DISK::track_size = 6250; // Standard track size
		if (track_buffer) delete[] track_buffer;
		track_buffer = new uint8_t[DISK::track_size];
		
		// Initialize with test pattern
		memset(track_buffer, 0x00, DISK::track_size);
	}
	
	void close() {
		DISK::inserted = false;  // Use base class member
		if (track_buffer) {
			delete[] track_buffer;
			track_buffer = nullptr;
		}
	}
	
	bool is_disk_inserted() { return inserted; }
	bool is_inserted() { return inserted; }  // Alternative method name
	void set_write_protect(bool val) { write_protected = val; }
	bool is_disk_protected() { return write_protected; }
	
	// Mock disk operations
	bool get_track(int trk, int side) {
		if (!inserted) return false;
		current_track = trk;
		// Generate test track data
		return true;
	}
	
	bool get_sector(int trk, int side, int sector) {
		if (!inserted) return false;
		current_track = trk;
		current_sector = sector;
		return true;
	}
	
	// MB8877 expects these methods for Type III commands
	bool get_sector_info(int position, int sec, int trk, int side, int compare) {
		if (!inserted) return false;
		
		// Mock sector ID field
		id[0] = trk;      // Track
		id[1] = side;     // Side
		id[2] = 1;        // Sector (starting at 1)
		id[3] = 1;        // Size code (256 bytes)
		id[4] = 0;        // CRC1
		id[5] = 0;        // CRC2
		
		// Set position info for MockDISK
		id_position[0] = position;
		data_position[0] = position + 100;  // Data follows ID field
		
		return true;
	}
	
	// Position arrays needed by MB8877
	int id_position[8];
	int data_position[8];
	
	// Create mock sector data
	void setup_mock_sector(int track, int sector, const uint8_t* data, int size) {
		// In real implementation, would store sector data
	}
	
	// Set CRC error state for testing
	void set_crc_error(bool error) {
		crc_error_state = error;
	}
};

// Mock NOISE class
class MockNOISE : public DEVICE {
public:
	bool playing;
	
	MockNOISE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu), playing(false) {}
	
	void initialize() {}
	void reset() { playing = false; }
	void play() { playing = true; }
	void stop() { playing = false; }
};

// Test Device for signal capture
class SignalCapture : public DEVICE {
public:
	struct Signal {
		int id;
		uint32_t data;
		uint32_t mask;
	};
	
	std::vector<Signal> captured_signals;
	
	SignalCapture(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	
	void initialize() {}
	void reset() { captured_signals.clear(); }
	
	void write_signal(int id, uint32_t data, uint32_t mask) {
		Signal sig = {id, data, mask};
		captured_signals.push_back(sig);
	}
	
	bool has_signal(int id, uint32_t data) {
		for (const auto& sig : captured_signals) {
			if (sig.id == id && (sig.data & sig.mask) == (data & sig.mask)) {
				return true;
			}
		}
		return false;
	}
	
	void clear_signals() {
		captured_signals.clear();
	}
};

// Helper to create test disk image data
class TestDiskBuilder {
public:
	// IBM MFM format constants
	static const int SECTOR_SIZE = 256;
	static const int SECTORS_PER_TRACK = 16;
	
	// Create a formatted track
	static void create_formatted_track(uint8_t* buffer, int track, int side) {
		// Simplified - in real implementation would create proper gaps and sync marks
		memset(buffer, 0x4E, 6250); // Fill with gap byte
		
		// Add sector headers and data
		int pos = 0;
		for (int sector = 1; sector <= SECTORS_PER_TRACK; sector++) {
			// Add IDAM (ID Address Mark)
			buffer[pos++] = 0xFE;
			buffer[pos++] = track;
			buffer[pos++] = side;
			buffer[pos++] = sector;
			buffer[pos++] = 0x01; // 256 bytes
			// CRC would go here
			pos += 2;
			
			// Gap
			pos += 22;
			
			// Data Address Mark
			buffer[pos++] = 0xFB;
			
			// Sector data
			for (int i = 0; i < SECTOR_SIZE; i++) {
				buffer[pos++] = (uint8_t)(track * 16 + sector);
			}
			
			// CRC would go here
			pos += 2;
			
			// Gap
			pos += 54;
		}
	}
};

// Global functions required by original MB8877
inline double get_passed_usec(uint32_t prev_clock) {
	// Mock implementation - return small value to simulate time passage
	return 100.0;  // 100 microseconds
}

inline uint32_t get_current_clock() {
	// Mock implementation - return incrementing clock
	static uint32_t mock_clock = 0;
	return ++mock_clock;
}

// Global config variable required by original MB8877
struct {
	bool sound_noise_fdd = false;  // Disable sound for tests
} config;

// Utility macros used by original MB8877
#define array_length(array) (sizeof(array) / sizeof(array[0]))

#ifndef _TCHAR
#define _TCHAR char
#endif

#ifndef _T
#define _T(x) x
#endif

#endif