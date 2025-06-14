#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>

// D88 disk format structures
struct D88Header {
    char name[17];           // Disk name
    uint8_t reserved[9];     // Reserved area
    uint8_t write_protect;   // Write protection flag
    uint8_t disk_type;       // Media type (0x00=2D, 0x10=2DD, 0x20=2HD)
    uint32_t disk_size;      // Total disk size
    uint32_t track_table[164]; // Track offset table
};

struct D88Sector {
    uint8_t track;           // Track number
    uint8_t side;            // Side number
    uint8_t sector;          // Sector number
    uint8_t size;            // Size code (0=128, 1=256, 2=512, 3=1024)
    uint16_t nsec;           // Number of sectors in track
    uint8_t dens;            // Density (0=MFM, 0x40=FM)
    uint8_t del;             // Delete mark (0=normal, 0x10=deleted)
    uint8_t stat;            // Status (0=normal)
    uint8_t reserved[5];     // Reserved
    uint16_t size_of_data;   // Actual data size
};

void create_dummy_2d_disk(const std::string& filename, bool write_protected = false) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return;
    }
    
    // Create D88 header
    D88Header header = {};
    strncpy(header.name, "TEST DISK", 16);
    header.write_protect = write_protected ? 0x10 : 0x00;
    header.disk_type = 0x00;  // 2D
    
    // Calculate track offsets (40 tracks × 2 sides)
    uint32_t offset = sizeof(D88Header);
    for(int track = 0; track < 40; track++) {
        for(int side = 0; side < 2; side++) {
            int track_index = track * 2 + side;
            header.track_table[track_index] = offset;
            
            // Each track: 16 sectors × (sector header + 256 bytes data)
            offset += 16 * (sizeof(D88Sector) + 256);
        }
    }
    
    // Set unused track entries to 0
    for(int i = 80; i < 164; i++) {
        header.track_table[i] = 0;
    }
    
    header.disk_size = offset;
    
    // Write header
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Create track data
    for(int track = 0; track < 40; track++) {
        for(int side = 0; side < 2; side++) {
            // 16 sectors per track
            for(int sector = 1; sector <= 16; sector++) {
                // Sector header
                D88Sector sec_header = {};
                sec_header.track = track;
                sec_header.side = side;
                sec_header.sector = sector;
                sec_header.size = 1;      // 256 bytes
                sec_header.nsec = 16;     // 16 sectors per track
                sec_header.dens = 0;      // MFM
                sec_header.del = 0;       // Normal data
                sec_header.stat = 0;      // Normal status
                sec_header.size_of_data = 256;
                
                file.write(reinterpret_cast<const char*>(&sec_header), sizeof(sec_header));
                
                // Sector data (test pattern)
                std::vector<uint8_t> sector_data(256);
                
                // Fill with recognizable pattern
                if (track == 0 && side == 0 && sector <= 9) {
                    // Boot sector area - fill with simple pattern
                    for(int i = 0; i < 256; i++) {
                        sector_data[i] = 0xEB + i;  // Simple boot-like pattern
                    }
                } else {
                    // Data area - fill with test pattern
                    for(int i = 0; i < 256; i++) {
                        sector_data[i] = (track * 16 + sector + i) & 0xFF;
                    }
                }
                
                file.write(reinterpret_cast<const char*>(sector_data.data()), 256);
            }
        }
    }
    
    file.close();
    std::cout << "Created dummy disk: " << filename << std::endl;
}

void create_dummy_2dd_disk(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return;
    }
    
    // Create D88 header for 2DD
    D88Header header = {};
    strncpy(header.name, "MSX TEST", 16);
    header.write_protect = 0x00;
    header.disk_type = 0x10;  // 2DD
    
    // Calculate track offsets (80 tracks × 2 sides)
    uint32_t offset = sizeof(D88Header);
    for(int track = 0; track < 80; track++) {
        for(int side = 0; side < 2; side++) {
            int track_index = track * 2 + side;
            if (track_index < 164) {
                header.track_table[track_index] = offset;
                // Each track: 9 sectors × (sector header + 512 bytes data)
                offset += 9 * (sizeof(D88Sector) + 512);
            }
        }
    }
    
    header.disk_size = offset;
    
    // Write header
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Create track data
    for(int track = 0; track < 80; track++) {
        for(int side = 0; side < 2; side++) {
            // 9 sectors per track (standard 2DD format)
            for(int sector = 1; sector <= 9; sector++) {
                // Sector header
                D88Sector sec_header = {};
                sec_header.track = track;
                sec_header.side = side;
                sec_header.sector = sector;
                sec_header.size = 2;      // 512 bytes
                sec_header.nsec = 9;      // 9 sectors per track
                sec_header.dens = 0;      // MFM
                sec_header.del = 0;       // Normal data
                sec_header.stat = 0;      // Normal status
                sec_header.size_of_data = 512;
                
                file.write(reinterpret_cast<const char*>(&sec_header), sizeof(sec_header));
                
                // Sector data
                std::vector<uint8_t> sector_data(512, 0xF6);  // Format fill byte
                
                file.write(reinterpret_cast<const char*>(sector_data.data()), 512);
            }
        }
    }
    
    file.close();
    std::cout << "Created dummy 2DD disk: " << filename << std::endl;
}

int main() {
    std::cout << "Creating dummy test disks..." << std::endl;
    
    // Create directory if it doesn't exist
    system("mkdir -p data/test_disk_images");
    
    // Create various test disks
    create_dummy_2d_disk("data/test_disk_images/test_basic_2d.d88");
    create_dummy_2d_disk("data/test_disk_images/test_data_2d.d88");
    create_dummy_2d_disk("data/test_disk_images/test_protected_2d.d88", true);
    create_dummy_2d_disk("data/test_disk_images/test_empty_2d.d88");
    create_dummy_2dd_disk("data/test_disk_images/test_msx_2dd.d88");
    
    std::cout << "Dummy disk creation completed!" << std::endl;
    std::cout << "Created files:" << std::endl;
    system("ls -la data/test_disk_images/*.d88");
    
    return 0;
}