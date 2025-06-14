/*
	Phase 32: Safe DISK Implementation
	
	This provides a safe DISK implementation that prevents crashes
	while maintaining compatibility with MB8877.
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#ifndef _PHASE32_SAFE_DISK_H_
#define _PHASE32_SAFE_DISK_H_

#include "../../../src/common.h"
#include "../../../src/vm/disk.h"

// SafeDISK - A DISK implementation that safely handles all operations
class SafeDISK : public DISK {
private:
	bool initialized;
	uint8_t* safe_sector_buffer;
	uint8_t* safe_id_buffer;
	uint8_t* safe_track_buffer;
	static const int SAFE_BUFFER_SIZE = 8192;  // Larger than needed for safety
	
public:
	SafeDISK(EMU* emu) : DISK(emu), initialized(false) {
		// Allocate safe buffers
		safe_sector_buffer = new uint8_t[SAFE_BUFFER_SIZE];
		safe_id_buffer = new uint8_t[64];
		safe_track_buffer = new uint8_t[SAFE_BUFFER_SIZE];
		
		// Initialize buffers
		memset(safe_sector_buffer, 0, SAFE_BUFFER_SIZE);
		memset(safe_id_buffer, 0, 64);
		memset(safe_track_buffer, 0, SAFE_BUFFER_SIZE);
		
		// Override base class pointers with our safe ones
		if (sector) delete[] sector;
		if (id) delete[] id;
		if (track) delete[] track;
		
		sector = safe_sector_buffer;
		id = safe_id_buffer;
		track = safe_track_buffer;
		
		// Set safe defaults
		sector_size.sd = 256;
		sector_num.sd = 16;
		track_size = 6250;
		drive_type = DRIVE_TYPE_2HD;
		drive_rpm = 300;
		drive_mfm = true;
		
		initialized = true;
	}
	
	virtual ~SafeDISK() {
		// Don't delete the buffers here - base class will handle it
		// Just mark as uninitialized
		initialized = false;
	}
	
	// Override open to actually make disk available
	virtual void open(const _TCHAR* path, int bank) {
		if (!initialized) return;
		
		// For testing, just mark as inserted
		inserted = true;
		
		// Set up a simple disk format
		media_type = MEDIA_TYPE_2HD;
		
		// Initialize track with test pattern
		for (int i = 0; i < track_size; i++) {
			track[i] = 0x4E;  // Gap byte
		}
		
		// Create simple sector headers
		int pos = 0;
		for (int s = 1; s <= 16 && pos < track_size - 300; s++) {
			// Sync
			track[pos++] = 0x00;
			track[pos++] = 0x00;
			track[pos++] = 0x00;
			
			// IDAM
			track[pos++] = 0xFE;
			track[pos++] = 0;     // Track
			track[pos++] = 0;     // Side
			track[pos++] = s;     // Sector
			track[pos++] = 1;     // Size (256 bytes)
			track[pos++] = 0;     // CRC1
			track[pos++] = 0;     // CRC2
			
			// Gap
			for (int g = 0; g < 22; g++) track[pos++] = 0x4E;
			
			// Data sync
			track[pos++] = 0x00;
			track[pos++] = 0x00;
			track[pos++] = 0x00;
			
			// Data AM
			track[pos++] = 0xFB;
			
			// Data
			for (int d = 0; d < 256; d++) {
				track[pos++] = 0xE5;  // Format fill byte
			}
			
			// CRC
			track[pos++] = 0;
			track[pos++] = 0;
			
			// Gap
			for (int g = 0; g < 54; g++) track[pos++] = 0x4E;
		}
	}
	
	virtual void close() {
		inserted = false;
	}
	
	virtual bool is_disk_inserted() {
		return inserted;
	}
	
	virtual bool get_track(int trk, int side) {
		if (!inserted || !initialized) return false;
		
		// For testing, always succeed
		return true;
	}
	
	virtual bool get_sector(int trk, int side, int index) {
		if (!inserted || !initialized) return false;
		
		// Fill sector buffer with test data
		for (int i = 0; i < sector_size.sd; i++) {
			sector[i] = (trk * 16 + index) & 0xFF;
		}
		
		// Fill ID buffer
		id[0] = trk;
		id[1] = side;
		id[2] = index + 1;  // Sector number (1-based)
		id[3] = 1;          // Size code
		
		return true;
	}
	
	// Provide timing that matches 2HD disk
	virtual double get_usec_per_bytes(int bytes) {
		// 300rpm = 200ms per revolution, 6250 bytes per track
		return (double)bytes * 200000.0 / 6250.0;
	}
	
	virtual double get_usec_per_track() {
		return 200000.0;  // 200ms for 300rpm
	}
};

#endif // _PHASE32_SAFE_DISK_H_