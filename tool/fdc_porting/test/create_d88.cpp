#include <cstdio>
#include <cstring>
#include <cstdint>

// Create a minimal D88 disk image for testing
int main() {
    FILE* fp = fopen("test.d88", "wb");
    if (!fp) {
        printf("Failed to create test.d88\n");
        return 1;
    }
    
    // D88 header
    uint8_t header[0x2b0] = {0};
    
    // D88 signature
    const char* name = "TEST DISK";
    memcpy(header, name, strlen(name));
    
    // Disk type (0x00 = 2D)
    header[0x1b] = 0x00;
    
    // Disk size (header + 1 track with 1 sector)
    uint32_t disk_size = 0x2b0 + 0x110; // Header + 1 sector
    header[0x1c] = disk_size & 0xff;
    header[0x1d] = (disk_size >> 8) & 0xff;
    header[0x1e] = (disk_size >> 16) & 0xff;
    header[0x1f] = (disk_size >> 24) & 0xff;
    
    // Track offset table - track 0 at offset 0x2b0
    uint32_t track_offset = 0x2b0;
    header[0x20] = track_offset & 0xff;
    header[0x21] = (track_offset >> 8) & 0xff;
    header[0x22] = (track_offset >> 16) & 0xff;
    header[0x23] = (track_offset >> 24) & 0xff;
    
    // Other tracks are not present (0x00000000)
    
    fwrite(header, 0x2b0, 1, fp);
    
    // Track 0 header
    uint8_t track_header[0x10] = {0};
    track_header[0] = 0;    // Cylinder
    track_header[1] = 0;    // Head
    track_header[2] = 1;    // Sector
    track_header[3] = 1;    // Size (128 << 1 = 256 bytes)
    track_header[4] = 1;    // Number of sectors (low)
    track_header[5] = 0;    // Number of sectors (high)
    track_header[8] = 0;    // Density (0 = double)
    track_header[9] = 0;    // Deleted mark
    track_header[0xa] = 0;  // Status
    
    // Size of this sector (including header)
    uint16_t sector_size = 0x110; // 16 + 256 bytes
    track_header[0xe] = sector_size & 0xff;
    track_header[0xf] = (sector_size >> 8) & 0xff;
    
    fwrite(track_header, 0x10, 1, fp);
    
    // Sector data (256 bytes of 0xE5)
    uint8_t sector_data[256];
    memset(sector_data, 0xE5, 256);
    fwrite(sector_data, 256, 1, fp);
    
    fclose(fp);
    printf("Created test.d88\n");
    return 0;
}