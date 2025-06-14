/*
	Enhanced MockDISK with D88 File Support
	
	Author : Claude AI Assistant
	Date   : 2025.06.13
*/

#ifndef _MOCK_DISK_D88_H_
#define _MOCK_DISK_D88_H_

#include "mock_environment.h"
#include "d88_loader.h"
#include <string>

// Enhanced MockDISK class with D88 support
class MockDISK_D88 : public DISK {
private:
	D88Loader d88_loader;
	std::string disk_path;
	bool d88_loaded;
	int current_track;
	int current_side;
	int current_sector;
	std::vector<uint8_t> current_track_data;
	std::vector<int> current_sector_ids;
	int current_sector_count;
	
	// Disk parameters
	int num_tracks;
	int num_sides;
	int sectors_per_track;
	int bytes_per_sector;
	
public:
	MockDISK_D88(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DISK(parent_emu) {
		// Initialize all member variables BEFORE accessing any base class members
		d88_loaded = false;
		current_track = 0;
		current_side = 0;
		current_sector = 0;
		current_sector_count = 0;
		disk_path.clear();
		current_track_data.clear();
		current_sector_ids.clear();
		
		// Default to 2D disk parameters
		num_tracks = 40;
		num_sides = 2;
		sectors_per_track = 16;
		bytes_per_sector = 256;
		
		// Now safely initialize base class members
		// Ensure these are valid after DISK constructor has run
		if (this->sector_size.sd == 0) {
			this->sector_size.sd = 256;
		}
		if (this->sector_num.sd == 0) {
			this->sector_num.sd = 16;
		}
		
		drive_type = DRIVE_TYPE_2HD;
		drive_rpm = 300;
		drive_mfm = false;  // Default to FM for 2D disks
		track_size = 6250;
		
		// Reserve space to avoid reallocations
		current_track_data.reserve(10240);
		current_sector_ids.reserve(32);
	}
	
	~MockDISK_D88() {
		// Base class handles cleanup
	}
	
	void open(const _TCHAR* path, int bank) {
		if (!path) {
			printf("ERROR: NULL path passed to open()\n");
			return;
		}
		
		disk_path = path;
		
		// Check if it's a D88 file
		if (disk_path.find(".d88") != std::string::npos || 
		    disk_path.find(".D88") != std::string::npos) {
			// Load D88 file
			if (d88_loader.load(path)) {
				d88_loaded = true;
				inserted = true;
				write_protected = d88_loader.is_write_protected();
				
				// Get disk parameters with validation
				d88_loader.get_disk_params(num_tracks, num_sides, sectors_per_track, bytes_per_sector);
				
				// Validate parameters
				if (num_tracks <= 0 || num_tracks > 84) num_tracks = 40;
				if (num_sides <= 0 || num_sides > 2) num_sides = 2;
				if (sectors_per_track <= 0 || sectors_per_track > 26) sectors_per_track = 16;
				if (bytes_per_sector < 128 || bytes_per_sector > 1024) bytes_per_sector = 256;
				
				// Update base class parameters
				sector_size.sd = bytes_per_sector;
				sector_num.sd = sectors_per_track;
				
				// Set disk type based on D88 type
				switch (d88_loader.get_disk_type()) {
					case D88_TYPE_2D:
						media_type = MEDIA_TYPE_UNK; // Use as 2D indicator
						drive_type = DRIVE_TYPE_2HD;
						drive_mfm = false;  // 2D uses FM
						track_size = 6250;
						break;
					case D88_TYPE_2DD:
						media_type = MEDIA_TYPE_UNK;
						drive_type = DRIVE_TYPE_2HD;
						drive_mfm = true;   // 2DD uses MFM
						track_size = 10416;
						break;
					case D88_TYPE_2HD:
						media_type = MEDIA_TYPE_UNK;
						drive_type = DRIVE_TYPE_2HD;
						drive_mfm = true;   // 2HD uses MFM
						track_size = 10416;
						break;
				}
				
				printf("D88 disk opened: %s\n", path);
				printf("  Tracks: %d, Sides: %d, Sectors: %d, Sector size: %d\n",
				       num_tracks, num_sides, sectors_per_track, bytes_per_sector);
			} else {
				printf("Failed to load D88 file: %s\n", path);
				d88_loaded = false;
				inserted = false;
			}
		} else {
			// Non-D88 file, use mock data
			d88_loaded = false;
			inserted = true;
			write_protected = false;
			printf("Mock disk opened: %s\n", path);
		}
	}
	
	void close() {
		if (d88_loaded && !write_protected) {
			// Save changes back to D88 file
			d88_loader.save(disk_path.c_str());
		}
		
		d88_loaded = false;
		inserted = false;
		disk_path.clear();
	}
	
	bool is_disk_inserted() { return inserted; }
	bool is_inserted() { return inserted; }
	void set_write_protect(bool val) { write_protected = val; }
	bool is_disk_protected() { return write_protected; }
	
	// Get track data
	bool get_track(int trk, int side) {
		if (!inserted) return false;
		
		current_track = trk;
		current_side = side;
		
		if (d88_loaded) {
			// Load track data from D88
			current_sector_ids.clear();
			if (d88_loader.get_track_data(trk, side, current_track_data, 
			                             current_sector_ids, current_sector_count)) {
				// Ensure track buffer is valid before copying
				if (!track) {
					printf("ERROR: track buffer is NULL!\n");
					return false;
				}
				
				// Copy track data to base class buffer with size limits
				track_size = current_track_data.size();
				if (track_size > 6250) track_size = 6250; // Limit to buffer size
				if (track_size > 0) {
					memcpy(track, current_track_data.data(), track_size);
				}
				
				// Set up sector positions with bounds checking
				int pos = 0;
				for (int i = 0; i < current_sector_count && i < MAX_SECTORS; i++) {
					// Initialize positions to safe defaults
					id_position[i] = 0;
					data_position[i] = 0;
					
					// Find IDAM positions in track data
					for (int j = pos; j < track_size - 4 && j >= 0; j++) {
						if (track[j] == 0xFE) { // IDAM marker
							id_position[i] = j;
							// Find corresponding data position
							for (int k = j + 10; k < track_size - 4 && k >= 0; k++) {
								if (track[k] == 0xFB || track[k] == 0xF8) { // DAM marker
									data_position[i] = k;
									pos = k + bytes_per_sector + 2; // Move past this sector
									if (pos > track_size) pos = track_size; // Prevent overflow
									break;
								}
							}
							break;
						}
					}
				}
				
				return true;
			}
		} else {
			// Generate mock track data
			track_size = 6250;
			memset(track, 0xFF, track_size);
			
			// Add mock sectors
			int pos = 0;
			for (int i = 0; i < sectors_per_track && pos < track_size - 300; i++) {
				// IDAM
				track[pos++] = 0x00; // Sync
				track[pos++] = 0x00;
				track[pos++] = 0x00;
				track[pos++] = 0xFE; // IDAM
				track[pos++] = trk;  // C
				track[pos++] = side; // H
				track[pos++] = i + 1; // R (1-based)
				track[pos++] = 1;    // N (256 bytes)
				track[pos++] = 0x00; // CRC1
				track[pos++] = 0x00; // CRC2
				
				id_position[i] = pos - 6;
				
				// Gap
				for (int j = 0; j < 22; j++) track[pos++] = 0x4E;
				
				// DAM
				track[pos++] = 0x00; // Sync
				track[pos++] = 0x00;
				track[pos++] = 0x00;
				track[pos++] = 0xFB; // DAM
				
				data_position[i] = pos - 1;
				
				// Data
				for (int j = 0; j < 256; j++) {
					track[pos++] = 0xE5; // Format fill byte
				}
				
				// CRC
				track[pos++] = 0x00;
				track[pos++] = 0x00;
				
				// Gap
				for (int j = 0; j < 54; j++) track[pos++] = 0x4E;
			}
			
			current_sector_count = sectors_per_track;
		}
		
		return true;
	}
	
	// Get sector data
	bool get_sector(int trk, int side, int sec) {
		if (!inserted) return false;
		
		// Validate parameters
		if (trk < 0 || trk >= num_tracks || side < 0 || side >= num_sides) {
			printf("ERROR: Invalid track/side: %d/%d\n", trk, side);
			return false;
		}
		
		current_track = trk;
		current_side = side;
		current_sector = sec;
		
		// Make sure we have the track loaded
		if (current_track != trk || current_side != side) {
			if (!get_track(trk, side)) {
				return false;
			}
		}
		
		if (d88_loaded) {
			// Read sector from D88
			int size;
			bool deleted, crc_error;
			
			// Ensure sector buffer is valid
			if (!sector || !id) {
				printf("ERROR: sector or id buffer is NULL!\n");
				return false;
			}
			
			if (d88_loader.read_sector(trk, side, sec, sector, size, deleted, crc_error)) {
				// Set up ID field
				id[0] = trk;
				id[1] = side;
				id[2] = sec;
				id[3] = (size == 128) ? 0 : (size == 256) ? 1 : (size == 512) ? 2 : 3;
				
				// Set flags
				this->deleted = deleted;
				data_crc_error = crc_error;
				addr_crc_error = false; // D88 doesn't track address CRC errors
				
				return true;
			}
		} else {
			// Generate mock sector
			if (sec >= 1 && sec <= sectors_per_track) {
				// Set up ID field
				id[0] = trk;
				id[1] = side;
				id[2] = sec;
				id[3] = 1; // 256 bytes
				
				// Generate test pattern
				for (int i = 0; i < 256; i++) {
					sector[i] = (uint8_t)((trk * 16 + sec) & 0xFF);
				}
				
				// No errors in mock data
				deleted = false;
				data_crc_error = false;
				addr_crc_error = false;
				
				return true;
			}
		}
		
		return false;
	}
	
	// Write sector data
	bool write_sector(int trk, int side, int sec, const uint8_t* buffer, int size) {
		if (!inserted || write_protected) return false;
		
		if (d88_loaded) {
			// Write to D88
			return d88_loader.write_sector(trk, side, sec, buffer, size, deleted);
		} else {
			// Mock write - just return success
			return true;
		}
	}
	
	// Format track
	void format_track(int track, int side) {
		if (!inserted || write_protected) return;
		
		// For D88, formatting would require rebuilding the track
		// For now, just clear the track in mock mode
		if (!d88_loaded) {
			get_track(track, side);
			// Track is already formatted with 0xE5 pattern
		}
	}
	
	// Insert sector (for formatting)
	void insert_sector(uint8_t c, uint8_t h, uint8_t r, uint8_t n, bool deleted, bool crc_error, uint8_t fill_data, int length) {
		// This would be used during format operations
		// For D88, we'd need to rebuild the track structure
	}
	
	// Get timing information based on disk type
	double get_usec_per_bytes(int bytes) {
		double rpm = (d88_loader.get_disk_type() == D88_TYPE_2HD) ? 360.0 : 300.0;
		double usec_per_rotation = 60000000.0 / rpm;
		return (double)bytes * usec_per_rotation / (double)track_size;
	}
	
	double get_usec_per_track() {
		double rpm = (d88_loader.get_disk_type() == D88_TYPE_2HD) ? 360.0 : 300.0;
		return 60000000.0 / rpm;
	}
	
	int get_bytes_per_usec(double usec) {
		double rpm = (d88_loader.get_disk_type() == D88_TYPE_2HD) ? 360.0 : 300.0;
		double usec_per_rotation = 60000000.0 / rpm;
		return (int)(usec * (double)track_size / usec_per_rotation);
	}
};

#endif // _MOCK_DISK_D88_H_