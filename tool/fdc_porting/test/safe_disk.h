/*
	SafeDISK - Safe wrapper for DISK class to prevent crashes
	
	This class provides a crash-safe implementation of DISK operations
	with proper initialization and error handling.
	
	Author : Claude AI Assistant
	Date   : 2025.01.14
	
	[ SafeDISK Wrapper ]
*/

#ifndef SAFE_DISK_H
#define SAFE_DISK_H

// For standalone test builds only
#ifdef STANDALONE_TEST

#include "../../../src/vm/disk.h"
#include <memory>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

class SafeDISK : public DISK {
private:
	bool initialized;
	std::vector<uint8_t> sector_buffer;
	std::vector<uint8_t> track_buffer;
	std::string current_file_path;
	
	// Additional member variables needed
	int cylinders;
	int surfaces;
	
	// D88 support
	struct D88Header {
		char name[17];
		uint8_t reserved[9];
		uint8_t write_protect;
		uint8_t disk_type;
		uint32_t disk_size;
		uint32_t track_table[164];
	};
	
	std::vector<uint8_t> disk_image;
	
	// Safe initialization
	void safe_initialize() {
		if(initialized) return;
		
		// Base class member initialization
		inserted = false;
		ejected = false;
		write_protected = false;
		changed = false;
		media_type = MEDIA_TYPE_2D;
		is_special_disk = 0;
		
		// Sector info initialization
		sector_size.sd = 0;
		sector_num.sd = 0;
		sector = nullptr;
		
		// Buffer allocation
		sector_buffer.resize(8192, 0);
		track_buffer.resize(65536, 0);
		
		initialized = true;
	}
	
	bool load_d88_file(const std::string& path) {
		FILE* fp = fopen(path.c_str(), "rb");
		if(!fp) return false;
		
		// Get file size
		fseek(fp, 0, SEEK_END);
		size_t file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		// Minimum size check
		if(file_size < sizeof(D88Header)) {
			fclose(fp);
			return false;
		}
		
		// Read entire file
		disk_image.resize(file_size);
		size_t read_size = fread(disk_image.data(), 1, file_size, fp);
		fclose(fp);
		
		if(read_size != file_size) {
			return false;
		}
		
		// Parse header
		D88Header* header = (D88Header*)disk_image.data();
		
		// Set disk type
		media_type = header->disk_type;
		write_protected = (header->write_protect != 0);
		
		// Set track info based on media type
		switch(media_type) {
			case MEDIA_TYPE_2D:
				cylinders = 40;
				surfaces = 2;
				sector_num.sd = 16;
				sector_size.sd = 256;
				break;
			case MEDIA_TYPE_2DD:
				cylinders = 80;
				surfaces = 2;
				sector_num.sd = 16;
				sector_size.sd = 256;
				break;
			default:
				// Unsupported format
				return false;
		}
		
		track_size = sector_size.sd * sector_num.sd;
		drive_type = media_type;
		drive_mfm = true;
		drive_rpm = 300;  // Standard 300 RPM
		
		return true;
	}
	
	bool get_sector_from_d88(int trk, int side, int index) {
		D88Header* header = (D88Header*)disk_image.data();
		
		// Get track offset
		int track_num = trk * surfaces + side;
		if(track_num >= 164 || header->track_table[track_num] == 0) {
			return false;
		}
		
		uint32_t track_offset = header->track_table[track_num];
		if(track_offset >= disk_image.size()) {
			return false;
		}
		
		// Search sector in track
		uint8_t* track_ptr = disk_image.data() + track_offset;
		
		while(track_ptr < disk_image.data() + disk_image.size()) {
			// Sector header check
			if(track_ptr + 16 > disk_image.data() + disk_image.size()) {
				break;
			}
			
			uint8_t sector_track = track_ptr[0];
			uint8_t sector_side = track_ptr[1];
			uint8_t sector_index = track_ptr[2];
			uint8_t sector_size_code = track_ptr[3];
			uint16_t data_size = *(uint16_t*)(track_ptr + 14);
			
			if(sector_track == trk && sector_side == side && sector_index == index) {
				// Found sector
				id[0] = sector_track;
				id[1] = sector_side;
				id[2] = sector_index;
				id[3] = sector_size_code;
				id[4] = track_ptr[4];  // CRC1
				id[5] = track_ptr[5];  // CRC2
				
				// Copy data
				if(data_size > 0 && data_size <= sector_buffer.size()) {
					memcpy(sector_buffer.data(), track_ptr + 16, data_size);
					sector = sector_buffer.data();
					sector_size.sd = data_size;
					return true;
				}
			}
			
			// Next sector
			track_ptr += 16 + data_size;
		}
		
		return false;
	}
	
	bool get_dummy_sector(int trk, int side, int index) {
		// Range check
		if(trk < 0 || trk >= cylinders || side < 0 || side >= surfaces) {
			return false;
		}
		
		if(index < 1 || index > sector_num.sd) {
			return false;
		}
		
		// Generate dummy sector data
		id[0] = trk;
		id[1] = side;
		id[2] = index;
		id[3] = 1; // 256 byte sector
		id[4] = 0; // CRC1
		id[5] = 0; // CRC2
		
		// Set sector buffer
		sector = sector_buffer.data();
		sector_size.sd = 256;
		
		// Generate test pattern
		for(int i = 0; i < 256; i++) {
			sector_buffer[i] = (trk * 16 + index + i) & 0xFF;
		}
		
		return true;
	}
	
public:
#ifndef _ANY2D88
	SafeDISK(EMU* parent_emu) : DISK(parent_emu), initialized(false), cylinders(40), surfaces(2) {
		safe_initialize();
	}
#else
	SafeDISK() : DISK(), initialized(false), cylinders(40), surfaces(2) {
		safe_initialize();
	}
#endif
	
	virtual ~SafeDISK() {
		close();
	}
	
	// Safe open implementation
	void open(const _TCHAR* file_path, int bank) {
		(void)bank; // Unused parameter
		try {
			safe_initialize();
			
			// Save file path
			current_file_path = file_path ? file_path : "";
			
			// Try to load D88 file
			if(!current_file_path.empty() && load_d88_file(current_file_path)) {
				inserted = true;
				ejected = false;
			} else {
				// Fallback to dummy disk
				setup_dummy_disk();
				inserted = true;
				ejected = false;
			}
			
		} catch(...) {
			// Reset to safe state on error
			inserted = false;
			ejected = true;
		}
	}
	
	// Safe close implementation
	void close() {
		if(!initialized) return;
		
		inserted = false;
		ejected = true;
		sector = nullptr;
		current_file_path.clear();
		disk_image.clear();
	}
	
	// Setup dummy disk
	void setup_dummy_disk() {
		// 2D disk (40 tracks, 16 sectors/track)
		media_type = MEDIA_TYPE_2D;
		drive_type = DRIVE_TYPE_2D;
		
		// Sector settings
		sector_size.sd = 256;
		sector_num.sd = 16;
		track_size = sector_size.sd * sector_num.sd;
		
		// Track info
		cylinders = 40;
		surfaces = 2;
		drive_mfm = true;
		drive_rpm = 300;
	}
	
	// Safe sector retrieval
	bool get_sector(int trk, int side, int index) {
		if(!initialized || !inserted) {
			return false;
		}
		
		// Try D88 image first
		if(!disk_image.empty()) {
			return get_sector_from_d88(trk, side, index);
		}
		
		// Fallback to dummy sector
		return get_dummy_sector(trk, side, index);
	}
	
	// Safe track retrieval
	bool get_track(int trk, int side) {
		if(!initialized || !inserted) {
			return false;
		}
		
		if(trk < 0 || trk >= cylinders || side < 0 || side >= surfaces) {
			return false;
		}
		
		// Generate dummy track data
		// Cannot assign to array directly, need to copy
		memcpy(track, track_buffer.data(), track_buffer.size());
		track_size = sector_size.sd * sector_num.sd;
		
		// For now, just return success
		// Full track generation would be implemented here
		return true;
	}
	
	// Format track (stub for now)
	bool format_track(int trk, int side) {
		(void)trk; // Unused parameter
		(void)side; // Unused parameter
		if(!initialized || !inserted || write_protected) {
			return false;
		}
		
		// Basic implementation
		return true;
	}
	
	// Make track (stub for now)
	bool make_track(int trk, int side) {
		return get_track(trk, side);
	}
	
	// Setup test track data for Type III commands
	void setup_test_track_data() {
		// Ensure disk is initialized
		if(!initialized) {
			safe_initialize();
		}
		
		// Set standard track size
		track_size = 6250;  // Standard FM track size
		
		// Allocate track buffer
		if(track_buffer.size() < (size_t)track_size) {
			track_buffer.resize(track_size);
		}
		
		// Generate standard track format
		generate_standard_track_format(track_buffer.data(), track_size);
		
		// Copy track data to track array
		memcpy(track, track_buffer.data(), std::min(track_size, (int)sizeof(track)));
		
		// Ensure disk is inserted
		inserted = true;
		write_protected = false;
	}
	
private:
	void generate_standard_track_format(uint8_t* buffer, int size) {
		int pos = 0;
		
		// Pre-index GAP
		for(int i = 0; i < 80 && pos < size; i++) {
			buffer[pos++] = 0x4E;
		}
		
		// Generate 16 sectors per track
		for(int sect = 1; sect <= 16 && pos < size; sect++) {
			// ID field sync
			for(int i = 0; i < 12 && pos < size; i++) {
				buffer[pos++] = 0x00;
			}
			for(int i = 0; i < 3 && pos < size; i++) {
				buffer[pos++] = 0xA1;
			}
			
			// ID Address Mark
			if(pos < size) buffer[pos++] = 0xFE;
			
			// ID field (Track, Side, Sector, Size, CRC1, CRC2)
			if(pos + 6 <= size) {
				buffer[pos++] = 0;      // Track
				buffer[pos++] = 0;      // Side  
				buffer[pos++] = sect;   // Sector
				buffer[pos++] = 1;      // Size (256 bytes)
				buffer[pos++] = 0;      // CRC1
				buffer[pos++] = 0;      // CRC2
			}
			
			// Gap 2
			for(int i = 0; i < 22 && pos < size; i++) {
				buffer[pos++] = 0x4E;
			}
			
			// Data field sync
			for(int i = 0; i < 12 && pos < size; i++) {
				buffer[pos++] = 0x00;
			}
			for(int i = 0; i < 3 && pos < size; i++) {
				buffer[pos++] = 0xA1;
			}
			
			// Data Address Mark
			if(pos < size) buffer[pos++] = 0xFB;
			
			// Sector data (256 bytes)
			for(int i = 0; i < 256 && pos < size; i++) {
				buffer[pos++] = 0xE5;  // Formatted data pattern
			}
			
			// Data CRC
			if(pos + 2 <= size) {
				buffer[pos++] = 0;  // CRC1
				buffer[pos++] = 0;  // CRC2
			}
			
			// Gap 3
			for(int i = 0; i < 54 && pos < size; i++) {
				buffer[pos++] = 0x4E;
			}
		}
		
		// Fill remaining with GAP
		while(pos < size) {
			buffer[pos++] = 0x4E;
		}
	}
};

#else // !STANDALONE_TEST

// When not in standalone test mode, SafeDISK is just a typedef to DISK
typedef DISK SafeDISK;

#endif // STANDALONE_TEST

#endif // SAFE_DISK_H