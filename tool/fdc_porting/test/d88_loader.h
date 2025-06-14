/*
	D88 Disk Image Loader
	
	Author : Claude AI Assistant
	Date   : 2025.06.13
*/

#ifndef _D88_LOADER_H_
#define _D88_LOADER_H_

#include <cstdio>
#include <cstring>
#include <vector>

// D88 Format Constants
#define D88_HEADER_SIZE 0x2B0
#define D88_SECTOR_HEADER_SIZE 16
#define D88_MAX_TRACKS 164

// D88 Header Structure
struct D88Header {
	char name[17];
	uint8_t reserved1[9];
	uint8_t write_protect;
	uint8_t disk_type;
	uint32_t disk_size;
	uint32_t track_table[D88_MAX_TRACKS];
};

// D88 Sector Header
struct D88SectorHeader {
	uint8_t c;      // Cylinder
	uint8_t h;      // Head
	uint8_t r;      // Sector number
	uint8_t n;      // Sector size (128 * 2^n)
	uint16_t sectors_per_track;
	uint8_t density;
	uint8_t deleted;
	uint8_t status;
	uint8_t reserved[5];
	uint16_t size_of_data;
};

// D88 Disk Types
enum D88DiskType {
	D88_TYPE_2D  = 0x00,
	D88_TYPE_2DD = 0x10,
	D88_TYPE_2HD = 0x20
};

// D88 Loader Class
class D88Loader {
private:
	D88Header header;
	std::vector<uint8_t> disk_data;
	bool loaded;
	
public:
	D88Loader() : loaded(false) {
		memset(&header, 0, sizeof(header));
	}
	
	bool load(const char* filename) {
		FILE* fp = fopen(filename, "rb");
		if (!fp) {
			printf("Failed to open D88 file: %s\n", filename);
			return false;
		}
		
		// Read header
		if (fread(&header, 1, D88_HEADER_SIZE, fp) != D88_HEADER_SIZE) {
			printf("Failed to read D88 header\n");
			fclose(fp);
			return false;
		}
		
		// Get file size
		fseek(fp, 0, SEEK_END);
		long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		// Read entire file
		disk_data.resize(file_size);
		if (fread(disk_data.data(), 1, file_size, fp) != file_size) {
			printf("Failed to read D88 data\n");
			fclose(fp);
			return false;
		}
		
		fclose(fp);
		loaded = true;
		
		// Print disk info
		printf("D88 Disk loaded: %s\n", header.name);
		printf("  Type: ");
		switch (header.disk_type) {
			case D88_TYPE_2D:  printf("2D\n"); break;
			case D88_TYPE_2DD: printf("2DD\n"); break;
			case D88_TYPE_2HD: printf("2HD\n"); break;
			default: printf("Unknown (0x%02X)\n", header.disk_type); break;
		}
		printf("  Size: %u bytes\n", header.disk_size);
		printf("  Write Protected: %s\n", header.write_protect ? "Yes" : "No");
		
		return true;
	}
	
	bool is_loaded() const { return loaded; }
	
	bool is_write_protected() const { return header.write_protect != 0; }
	
	int get_disk_type() const { return header.disk_type; }
	
	bool get_track_data(int track, int side, std::vector<uint8_t>& track_data, 
	                   std::vector<int>& sector_ids, int& sector_count) {
		if (!loaded) return false;
		
		int track_num = track * 2 + side;
		if (track_num >= D88_MAX_TRACKS) return false;
		
		uint32_t track_offset = header.track_table[track_num];
		if (track_offset == 0 || track_offset >= disk_data.size()) {
			// No track data
			return false;
		}
		
		// Count sectors in this track
		sector_count = 0;
		uint32_t pos = track_offset;
		while (pos < disk_data.size()) {
			if (pos + sizeof(D88SectorHeader) > disk_data.size()) break;
			
			D88SectorHeader* sector_header = (D88SectorHeader*)&disk_data[pos];
			if (sector_header->c != track || sector_header->h != side) break;
			
			sector_ids.push_back(sector_header->r);
			sector_count++;
			pos += sizeof(D88SectorHeader) + sector_header->size_of_data;
			
			// Check for next track
			int next_track = track_num + 1;
			if (next_track < D88_MAX_TRACKS && header.track_table[next_track] != 0) {
				if (pos >= header.track_table[next_track]) break;
			}
		}
		
		// Build track data for MB8877 format (simplified)
		track_data.clear();
		track_data.resize(6250, 0xFF); // Fill with 0xFF
		
		// Add sectors with gaps
		int track_pos = 0;
		pos = track_offset;
		
		for (int i = 0; i < sector_count; i++) {
			D88SectorHeader* sector_header = (D88SectorHeader*)&disk_data[pos];
			uint8_t* sector_data = &disk_data[pos + sizeof(D88SectorHeader)];
			
			// Add IDAM (ID Address Mark)
			if (track_pos + 10 < track_data.size()) {
				track_data[track_pos++] = 0x00; // Sync
				track_data[track_pos++] = 0x00; // Sync
				track_data[track_pos++] = 0x00; // Sync
				track_data[track_pos++] = 0xFE; // IDAM
				track_data[track_pos++] = sector_header->c;
				track_data[track_pos++] = sector_header->h;
				track_data[track_pos++] = sector_header->r;
				track_data[track_pos++] = sector_header->n;
				track_data[track_pos++] = 0x00; // CRC1
				track_data[track_pos++] = 0x00; // CRC2
			}
			
			// Gap between ID and Data
			for (int j = 0; j < 22 && track_pos < track_data.size(); j++) {
				track_data[track_pos++] = 0x4E;
			}
			
			// Add DAM (Data Address Mark)
			if (track_pos + sector_header->size_of_data + 4 < track_data.size()) {
				track_data[track_pos++] = 0x00; // Sync
				track_data[track_pos++] = 0x00; // Sync
				track_data[track_pos++] = 0x00; // Sync
				track_data[track_pos++] = sector_header->deleted ? 0xF8 : 0xFB; // DAM
				
				// Copy sector data
				memcpy(&track_data[track_pos], sector_data, sector_header->size_of_data);
				track_pos += sector_header->size_of_data;
				
				// CRC
				track_data[track_pos++] = 0x00; // CRC1
				track_data[track_pos++] = 0x00; // CRC2
			}
			
			// Gap between sectors
			for (int j = 0; j < 54 && track_pos < track_data.size(); j++) {
				track_data[track_pos++] = 0x4E;
			}
			
			pos += sizeof(D88SectorHeader) + sector_header->size_of_data;
		}
		
		return sector_count > 0;
	}
	
	bool read_sector(int track, int side, int sector, uint8_t* buffer, int& size, 
	                bool& deleted, bool& crc_error) {
		if (!loaded) return false;
		
		int track_num = track * 2 + side;
		if (track_num >= D88_MAX_TRACKS) return false;
		
		uint32_t track_offset = header.track_table[track_num];
		if (track_offset == 0 || track_offset >= disk_data.size()) {
			return false;
		}
		
		// Find the sector
		uint32_t pos = track_offset;
		while (pos < disk_data.size()) {
			if (pos + sizeof(D88SectorHeader) > disk_data.size()) break;
			
			D88SectorHeader* sector_header = (D88SectorHeader*)&disk_data[pos];
			if (sector_header->c != track || sector_header->h != side) break;
			
			if (sector_header->r == sector) {
				// Found the sector
				size = sector_header->size_of_data;
				deleted = sector_header->deleted != 0;
				crc_error = (sector_header->status & 0xB0) != 0; // Status bits indicate errors
				
				// Copy sector data
				uint8_t* sector_data = &disk_data[pos + sizeof(D88SectorHeader)];
				memcpy(buffer, sector_data, size);
				
				return true;
			}
			
			pos += sizeof(D88SectorHeader) + sector_header->size_of_data;
		}
		
		return false;
	}
	
	bool write_sector(int track, int side, int sector, const uint8_t* buffer, int size, bool deleted) {
		if (!loaded || header.write_protect) return false;
		
		int track_num = track * 2 + side;
		if (track_num >= D88_MAX_TRACKS) return false;
		
		uint32_t track_offset = header.track_table[track_num];
		if (track_offset == 0 || track_offset >= disk_data.size()) {
			return false;
		}
		
		// Find the sector
		uint32_t pos = track_offset;
		while (pos < disk_data.size()) {
			if (pos + sizeof(D88SectorHeader) > disk_data.size()) break;
			
			D88SectorHeader* sector_header = (D88SectorHeader*)&disk_data[pos];
			if (sector_header->c != track || sector_header->h != side) break;
			
			if (sector_header->r == sector) {
				// Found the sector
				if (size != sector_header->size_of_data) {
					// Size mismatch
					return false;
				}
				
				// Update deleted flag
				sector_header->deleted = deleted ? 1 : 0;
				
				// Copy sector data
				uint8_t* sector_data = &disk_data[pos + sizeof(D88SectorHeader)];
				memcpy(sector_data, buffer, size);
				
				return true;
			}
			
			pos += sizeof(D88SectorHeader) + sector_header->size_of_data;
		}
		
		return false;
	}
	
	bool save(const char* filename) {
		if (!loaded) return false;
		
		FILE* fp = fopen(filename, "wb");
		if (!fp) {
			printf("Failed to create D88 file: %s\n", filename);
			return false;
		}
		
		// Update header from current data
		memcpy(disk_data.data(), &header, D88_HEADER_SIZE);
		
		// Write entire disk data
		if (fwrite(disk_data.data(), 1, disk_data.size(), fp) != disk_data.size()) {
			printf("Failed to write D88 data\n");
			fclose(fp);
			return false;
		}
		
		fclose(fp);
		return true;
	}
	
	// Get disk parameters based on type
	void get_disk_params(int& tracks, int& sides, int& sectors_per_track, int& sector_size) {
		switch (header.disk_type) {
			case D88_TYPE_2D:
				tracks = 40;
				sides = 2;
				sectors_per_track = 16;
				sector_size = 256;
				break;
			case D88_TYPE_2DD:
				tracks = 80;
				sides = 2;
				sectors_per_track = 16;
				sector_size = 256;
				break;
			case D88_TYPE_2HD:
				tracks = 77;
				sides = 2;
				sectors_per_track = 26;
				sector_size = 256;
				break;
			default:
				// Default to 2D
				tracks = 40;
				sides = 2;
				sectors_per_track = 16;
				sector_size = 256;
				break;
		}
	}
};

#endif // _D88_LOADER_H_