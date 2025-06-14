#!/usr/bin/env python3
"""
Minimal D88 disk image generator for MB8877 testing
Generates test D88 files with various data patterns
"""

import struct
import sys

class D88Generator:
    def __init__(self):
        # D88 header structure
        self.HEADER_SIZE = 0x2B0  # 688 bytes
        self.SECTOR_HEADER_SIZE = 16
        
        # Standard disk parameters
        self.TRACKS = 40
        self.HEADS = 2
        self.SECTORS = 8
        self.SECTOR_SIZE = 256
        
    def create_header(self, disk_name="TEST", write_protect=False):
        """Create D88 header"""
        header = bytearray(self.HEADER_SIZE)
        
        # Disk name (0x00-0x10)
        name_bytes = disk_name.encode('ascii')[:17]
        header[0:len(name_bytes)] = name_bytes
        
        # Write protect (0x1A)
        header[0x1A] = 0x10 if write_protect else 0x00
        
        # Disk type (0x1B) - 2D (double density)
        header[0x1B] = 0x00
        
        # Disk size will be updated later
        
        # Track offset table starts at 0x20
        # Will be filled as we add tracks
        
        return header
        
    def create_sector(self, track, head, sector_num, data_pattern=None, sector_size=256):
        """Create a sector with header and data"""
        # Sector header (16 bytes)
        sector_header = bytearray(self.SECTOR_HEADER_SIZE)
        
        # C,H,R,N values
        sector_header[0] = track & 0xFF
        sector_header[1] = head & 0xFF
        sector_header[2] = sector_num & 0xFF
        
        # N (sector size) - 1 for 256 bytes
        sector_header[3] = 1
        
        # Number of sectors in this track
        struct.pack_into('<H', sector_header, 4, self.SECTORS)
        
        # Density (0=double density)
        sector_header[6] = 0x00
        
        # Deleted mark
        sector_header[7] = 0x00
        
        # Status
        sector_header[8] = 0x00
        
        # Reserved (5 bytes)
        # sector_header[9:14] = 0
        
        # Actual data size
        struct.pack_into('<H', sector_header, 14, sector_size)
        
        # Create sector data
        if data_pattern is None:
            # Default pattern: sequential values
            data = bytearray(range(sector_size)) if sector_size <= 256 else bytearray([i % 256 for i in range(sector_size)])
        elif callable(data_pattern):
            data = bytearray(data_pattern(track, head, sector_num, sector_size))
        else:
            # Fixed pattern
            data = bytearray([data_pattern] * sector_size)
            
        return sector_header + data
        
    def generate_disk(self, output_file, pattern_type='sequential'):
        """Generate complete D88 disk image"""
        
        # Define data patterns
        patterns = {
            'sequential': lambda t,h,s,sz: [(t * 100 + h * 10 + s + i) % 256 for i in range(sz)],
            'sector_id': lambda t,h,s,sz: [s] * sz,
            'alternating': lambda t,h,s,sz: [0x55 if i % 2 == 0 else 0xAA for i in range(sz)],
            'track_based': lambda t,h,s,sz: [t] * sz,
            'checkered': lambda t,h,s,sz: [(0xFF if (t+s) % 2 == 0 else 0x00) for i in range(sz)]
        }
        
        pattern_func = patterns.get(pattern_type, patterns['sequential'])
        
        # Create header
        header = self.create_header(f"TEST_{pattern_type.upper()}")
        
        # Generate all tracks
        disk_data = bytearray()
        track_offsets = []
        
        for track in range(self.TRACKS):
            for head in range(self.HEADS):
                track_start = self.HEADER_SIZE + len(disk_data)
                track_offsets.append(track_start)
                
                # Add all sectors in track
                for sector in range(1, self.SECTORS + 1):  # Sectors numbered 1-8
                    sector_data = self.create_sector(track, head, sector, pattern_func, self.SECTOR_SIZE)
                    disk_data.extend(sector_data)
        
        # Update header with track offsets
        for i, offset in enumerate(track_offsets):
            struct.pack_into('<I', header, 0x20 + i * 4, offset)
            
        # Update disk size in header
        total_size = self.HEADER_SIZE + len(disk_data)
        struct.pack_into('<I', header, 0x1C, total_size)
        
        # Write to file
        with open(output_file, 'wb') as f:
            f.write(header)
            f.write(disk_data)
            
        print(f"Generated {output_file} ({total_size} bytes) with {pattern_type} pattern")
        print(f"  Tracks: {self.TRACKS}, Heads: {self.HEADS}, Sectors: {self.SECTORS}")
        print(f"  Sector size: {self.SECTOR_SIZE} bytes")
        
    def generate_all_test_disks(self):
        """Generate all test disk patterns"""
        patterns = ['sequential', 'sector_id', 'alternating', 'track_based', 'checkered']
        
        for pattern in patterns:
            self.generate_disk(f"test_disks/test_{pattern}.d88", pattern)

def verify_d88(filename):
    """Verify D88 file structure"""
    with open(filename, 'rb') as f:
        # Read header
        header = f.read(0x2B0)
        
        print(f"\nVerifying {filename}:")
        disk_name = header[0:17].decode('ascii', errors='ignore').rstrip('\x00')
        print(f"  Disk name: {disk_name}")
        print(f"  Write protect: {'Yes' if header[0x1A] == 0x10 else 'No'}")
        print(f"  Disk type: {header[0x1B]:02X}")
        print(f"  Disk size: {struct.unpack('<I', header[0x1C:0x20])[0]} bytes")
        
        # Check first few track offsets
        print("  First 5 track offsets:")
        for i in range(5):
            offset = struct.unpack('<I', header[0x20 + i*4:0x24 + i*4])[0]
            print(f"    Track {i}: 0x{offset:08X}")
            
        # Read first sector
        f.seek(struct.unpack('<I', header[0x20:0x24])[0])
        sector_header = f.read(16)
        c, h, r, n = sector_header[0:4]
        print(f"\n  First sector: C={c}, H={h}, R={r}, N={n}")
        
def main():
    import os
    
    # Create test_disks directory if it doesn't exist
    os.makedirs('test_disks', exist_ok=True)
    
    generator = D88Generator()
    
    if len(sys.argv) > 1:
        # Generate specific pattern
        pattern = sys.argv[1]
        generator.generate_disk(f"test_disks/test_{pattern}.d88", pattern)
        verify_d88(f"test_disks/test_{pattern}.d88")
    else:
        # Generate all test patterns
        generator.generate_all_test_disks()
        
        # Verify all generated files
        for pattern in ['sequential', 'sector_id', 'alternating', 'track_based', 'checkered']:
            verify_d88(f"test_disks/test_{pattern}.d88")

if __name__ == '__main__':
    main()