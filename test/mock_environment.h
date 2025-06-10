/*
	Mock Environment for MB8877 Testing
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#ifndef _MOCK_ENVIRONMENT_H_
#define _MOCK_ENVIRONMENT_H_

#include "../src/common.h"
#include "../src/fileio.h"
#include <cstring>
#include <vector>

// Mock VM_TEMPLATE
class MockVM : public VM_TEMPLATE {
public:
	MockVM() {}
	virtual ~MockVM() {}
};

// Mock EMU class
class MockEMU : public EMU {
public:
	uint32_t current_cpu_clock;
	
	MockEMU() : current_cpu_clock(4000000) {} // 4MHz default
	
	uint32_t get_current_clock() { return 0; }
	uint32_t get_cpu_clock(int) { return current_cpu_clock; }
	bool is_frame_skippable() { return false; }
	
	// Add other required methods as stubs
	void out_debug_log(const _TCHAR* format, ...) {
		// Silent for tests unless debugging
	}
};

// Mock EVENT class
class MockEVENT : public DEVICE {
private:
	struct EventInfo {
		int device_id;
		int event_id;
		uint32_t clock;
		bool active;
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
	
	int register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id) {
		static int next_id = 1;
		EventInfo info;
		info.device_id = next_id;
		info.event_id = event_id;
		info.clock = current_clock + (uint32_t)(usec * 4); // Assuming 4MHz
		info.active = true;
		events.push_back(info);
		
		if (register_id) {
			*register_id = next_id;
		}
		return next_id++;
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
		// In real implementation, would trigger events
	}
	
	uint32_t get_current_clock() { return current_clock; }
	uint32_t get_cpu_clock(int) { return 4000000; }
};

// Mock DISK class
class MockDISK : public DISK {
private:
	bool inserted;
	bool write_protected;
	uint8_t* track_buffer;
	int track_size;
	int current_track;
	int current_sector;
	
public:
	MockDISK(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DISK(parent_vm, parent_emu) {
		inserted = false;
		write_protected = false;
		track_buffer = nullptr;
		track_size = 0;
		current_track = 0;
		current_sector = 0;
	}
	
	~MockDISK() {
		if (track_buffer) {
			delete[] track_buffer;
		}
	}
	
	void open(const _TCHAR* path, int bank) {
		inserted = true;
		// Create mock disk data
		track_size = 6250; // Standard track size
		if (track_buffer) delete[] track_buffer;
		track_buffer = new uint8_t[track_size];
		
		// Initialize with test pattern
		memset(track_buffer, 0x00, track_size);
	}
	
	void close() {
		inserted = false;
		if (track_buffer) {
			delete[] track_buffer;
			track_buffer = nullptr;
		}
	}
	
	bool is_disk_inserted() { return inserted; }
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
	
	// Create mock sector data
	void setup_mock_sector(int track, int sector, const uint8_t* data, int size) {
		// In real implementation, would store sector data
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

#endif