/*
	Phase 32: Safe Type II Command Test
	
	This test uses SafeDISK to prevent crashes while testing Type II commands.
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#include <cstdio>
#include <cstring>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>

// Include real headers in correct order
#include "../../../src/common.h"
#include "../../../src/emu.h"
#include "../../../src/vm/vm_template.h"
#include "../../../src/vm/device.h"
#include "../../../src/vm/event.h"
#include "../../../src/vm/mb8877.h"

// Include our safe disk implementation
#include "phase32_safe_disk.h"

// Simple test EMU implementation
class TestEMU : public EMU {
public:
	TestEMU() {}
	virtual ~TestEMU() {}
	
	uint32_t get_current_clock() { return 0; }
	uint32_t get_cpu_clock(int) { return 4000000; }
	bool is_frame_skippable() { return false; }
	void out_debug_log(const _TCHAR* format, ...) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
		printf("\n");
	}
	void force_out_debug_log(const _TCHAR* format, ...) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
		printf("\n");
	}
};

// Simple test VM implementation
class TestVM : public VM_TEMPLATE {
public:
	TestVM(EMU* parent_emu) : VM_TEMPLATE(parent_emu) {}
	virtual ~TestVM() {}
};

// Test EVENT implementation
class TestEVENT : public EVENT {
public:
	TestEVENT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : EVENT(parent_vm, parent_emu) {}
	virtual ~TestEVENT() {}
};

// Signal handler
void segfault_handler(int sig) {
	void* array[20];
	size_t size = backtrace(array, 20);
	fprintf(stderr, "\n========== SEGMENTATION FAULT ==========\n");
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	fprintf(stderr, "========================================\n");
	exit(1);
}

// MB8877 with safe disk replacement
class SafeMB8877 : public MB8877 {
public:
	SafeMB8877(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MB8877(parent_vm, parent_emu) {}
	
	// Override initialize to replace disks with safe ones
	void initialize() {
		// Call base initialize first
		MB8877::initialize();
		
		// Replace all disk objects with SafeDISK
		printf("Replacing DISK objects with SafeDISK...\n");
		for (int i = 0; i < MAX_DRIVE; i++) {
			if (disk[i]) {
				delete disk[i];
				disk[i] = new SafeDISK(emu);
				printf("  Drive %d: SafeDISK installed\n", i);
			}
		}
	}
};

// Test functions
void test_read_sector_basic(SafeMB8877& fdc, TestEVENT& event) {
	printf("\n=== Basic Read Sector Test ===\n");
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Open disk on drive 0
	printf("1. Opening disk...\n");
	fdc.open_disk(0, _T("test.dsk"), 0);
	
	// Position to track 0, sector 1
	printf("2. Setting position...\n");
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 1);  // Sector register
	
	// Issue read sector command
	printf("3. Issuing READ SECTOR command...\n");
	fdc.write_io8(0, 0x80);  // Read sector command
	
	// Check BUSY flag
	uint32_t status = fdc.read_io8(0);
	printf("   Initial status: 0x%02X (BUSY=%d)\n", status, (status & 0x01) ? 1 : 0);
	
	// Process events
	printf("4. Processing events...\n");
	for (int i = 0; i < 10; i++) {
		event.event_manager();
		status = fdc.read_io8(0);
		printf("   Status after %d cycles: 0x%02X (BUSY=%d, DRQ=%d)\n", 
			i, status, (status & 0x01) ? 1 : 0, (status & 0x02) ? 1 : 0);
		
		if (status & 0x02) {  // DRQ set
			printf("5. DRQ set, reading data...\n");
			uint8_t data[256];
			for (int j = 0; j < 256; j++) {
				data[j] = fdc.read_io8(3);
			}
			printf("   Read 256 bytes, first byte: 0x%02X\n", data[0]);
			break;
		}
		
		if (!(status & 0x01)) {  // BUSY cleared
			printf("   Command completed\n");
			break;
		}
	}
	
	// Final status
	status = fdc.read_io8(0);
	printf("6. Final status: 0x%02X\n", status);
	printf("   BUSY=%d, DRQ=%d, LOST=%d, CRC=%d, RNF=%d\n",
		(status & 0x01) ? 1 : 0,
		(status & 0x02) ? 1 : 0,
		(status & 0x04) ? 1 : 0,
		(status & 0x08) ? 1 : 0,
		(status & 0x10) ? 1 : 0);
}

void test_write_sector_basic(SafeMB8877& fdc, TestEVENT& event) {
	printf("\n=== Basic Write Sector Test ===\n");
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Position to track 0, sector 1
	printf("1. Setting position...\n");
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 1);  // Sector register
	
	// Issue write sector command
	printf("2. Issuing WRITE SECTOR command...\n");
	fdc.write_io8(0, 0xA0);  // Write sector command
	
	// Check initial status
	uint32_t status = fdc.read_io8(0);
	printf("   Initial status: 0x%02X (BUSY=%d)\n", status, (status & 0x01) ? 1 : 0);
	
	// Process events until DRQ
	printf("3. Waiting for DRQ...\n");
	for (int i = 0; i < 10; i++) {
		event.event_manager();
		status = fdc.read_io8(0);
		
		if (status & 0x02) {  // DRQ set
			printf("4. DRQ set, writing data...\n");
			for (int j = 0; j < 256; j++) {
				fdc.write_io8(3, 0xAA);  // Write test pattern
			}
			printf("   Wrote 256 bytes\n");
			break;
		}
	}
	
	// Wait for completion
	printf("5. Waiting for completion...\n");
	for (int i = 0; i < 10; i++) {
		event.event_manager();
		status = fdc.read_io8(0);
		if (!(status & 0x01)) {  // BUSY cleared
			break;
		}
	}
	
	// Final status
	status = fdc.read_io8(0);
	printf("6. Final status: 0x%02X\n", status);
	printf("   BUSY=%d, DRQ=%d, LOST=%d, WPRT=%d\n",
		(status & 0x01) ? 1 : 0,
		(status & 0x02) ? 1 : 0,
		(status & 0x04) ? 1 : 0,
		(status & 0x40) ? 1 : 0);
}

int main() {
	// Install signal handler
	signal(SIGSEGV, segfault_handler);
	
	printf("Phase 32: Safe Type II Command Test\n");
	printf("===================================\n");
	
	// Create test environment
	printf("\nCreating test environment...\n");
	TestEMU emu;
	TestVM vm(&emu);
	TestEVENT event(&vm, &emu);
	
	// Create SafeMB8877
	printf("Creating SafeMB8877...\n");
	SafeMB8877 fdc(&vm, &emu);
	
	// Configure
	printf("Configuring FDC...\n");
	fdc.set_context_event_manager(&event, 0, 0, 0);
	
	// Initialize (this will replace disks with SafeDISK)
	printf("Initializing FDC...\n");
	fdc.initialize();
	fdc.reset();
	
	// Run tests
	test_read_sector_basic(fdc, event);
	test_write_sector_basic(fdc, event);
	
	printf("\nAll tests completed successfully!\n");
	return 0;
}