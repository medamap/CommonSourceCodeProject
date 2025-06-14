/*
	Phase 34: Simple Type II Command Test
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

int main() {
	printf("Phase 34: Simple Type II Command Test\n");
	printf("=====================================\n");
	
	// Create test environment
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	// Create FDC
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Open disk - use a test disk file
	fdc.open_disk(0, _T("test_disks/test_2d_hubasic_empty.d88"), 0);
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	printf("\n1. Testing READ SECTOR command:\n");
	
	// Set up for read
	fdc.write_io8(1, 0);  // Track 0
	fdc.write_io8(2, 1);  // Sector 1
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	// Wait for completion
	int cycles = 0;
	uint8_t status;
	int drq_count = 0;
	int bytes_read = 0;
	uint8_t read_data[256];
	
	while(cycles < 10000) {
		status = fdc.read_io8(0);
		
		if(status & 0x02) {  // DRQ
			read_data[bytes_read++] = fdc.read_io8(3);
			drq_count++;
			if(bytes_read >= 256) break;
		}
		
		if(!(status & 0x01)) {  // Not BUSY
			break;
		}
		
		event.advance_clock(10);
		cycles++;
	}
	
	printf("   - Status after READ: 0x%02X\n", status);
	printf("   - DRQ count: %d\n", drq_count);
	printf("   - Bytes read: %d\n", bytes_read);
	printf("   - Result: %s\n", (bytes_read == 256 && !(status & 0x01)) ? "PASS" : "FAIL");
	
	printf("\n2. Testing WRITE SECTOR command:\n");
	
	// Generate test data
	uint8_t write_data[256];
	for(int i = 0; i < 256; i++) {
		write_data[i] = (i * 3 + 7) & 0xFF;
	}
	
	// Set up for write
	fdc.write_io8(1, 0);  // Track 0
	fdc.write_io8(2, 2);  // Sector 2
	fdc.write_io8(0, 0xA0);  // WRITE SECTOR command
	
	// Write data
	cycles = 0;
	drq_count = 0;
	int bytes_written = 0;
	
	while(cycles < 10000) {
		status = fdc.read_io8(0);
		
		if(status & 0x02) {  // DRQ
			fdc.write_io8(3, write_data[bytes_written++]);
			drq_count++;
			if(bytes_written >= 256) break;
		}
		
		if(!(status & 0x01)) {  // Not BUSY
			break;
		}
		
		event.advance_clock(10);
		cycles++;
	}
	
	printf("   - Status after WRITE: 0x%02X\n", status);
	printf("   - DRQ count: %d\n", drq_count);
	printf("   - Bytes written: %d\n", bytes_written);
	printf("   - Result: %s\n", (bytes_written == 256 && !(status & 0x01)) ? "PASS" : "FAIL");
	
	printf("\n3. Testing WRITE then READ verification:\n");
	
	// Read back the written sector
	fdc.write_io8(2, 2);  // Sector 2
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	cycles = 0;
	bytes_read = 0;
	memset(read_data, 0, 256);
	
	while(cycles < 10000) {
		status = fdc.read_io8(0);
		
		if(status & 0x02) {  // DRQ
			read_data[bytes_read++] = fdc.read_io8(3);
			if(bytes_read >= 256) break;
		}
		
		if(!(status & 0x01)) {  // Not BUSY
			break;
		}
		
		event.advance_clock(10);
		cycles++;
	}
	
	// Verify data
	bool match = true;
	for(int i = 0; i < 256; i++) {
		if(read_data[i] != write_data[i]) {
			match = false;
			break;
		}
	}
	
	printf("   - Bytes read back: %d\n", bytes_read);
	printf("   - Data verification: %s\n", match ? "MATCH" : "MISMATCH");
	printf("   - Result: %s\n", (match && bytes_read == 256) ? "PASS" : "FAIL");
	
	printf("\n4. Testing error conditions:\n");
	
	// Test invalid sector
	fdc.write_io8(2, 99);  // Invalid sector
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	cycles = 0;
	while(cycles < 20000) {
		status = fdc.read_io8(0);
		if(!(status & 0x01)) break;
		event.advance_clock(10);
		cycles++;
	}
	
	printf("   - Invalid sector status: 0x%02X\n", status);
	printf("   - RNF flag: %s\n", (status & 0x10) ? "SET" : "NOT SET");
	printf("   - Result: %s\n", (status & 0x10) ? "PASS" : "FAIL");
	
	// Summary
	printf("\n=====================================\n");
	printf("Phase 34 Simple Test Complete\n");
	
	return 0;
}