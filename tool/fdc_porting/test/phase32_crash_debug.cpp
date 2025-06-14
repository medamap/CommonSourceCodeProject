/*
	Phase 32: Debug DISK::open crash
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#include <cstdio>
#include <cstring>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>

#include "test_framework.h"
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

// Signal handler for SIGSEGV
void segfault_handler(int sig) {
	void* array[20];
	size_t size;
	
	// Get void*'s for all entries on the stack
	size = backtrace(array, 20);
	
	// Print out all the frames to stderr
	fprintf(stderr, "\n========== SEGMENTATION FAULT ==========\n");
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	fprintf(stderr, "========================================\n");
	
	exit(1);
}

void test_disk_initialization() {
	printf("\n=== Testing DISK Initialization ===\n");
	
	// Test 1: Create MockDISK directly
	printf("1. Creating MockDISK directly...\n");
	MockEMU emu;
	MockVM vm(&emu);
	MockDISK* mock_disk = new MockDISK(&vm, &emu);
	printf("   MockDISK created successfully at %p\n", mock_disk);
	
	// Test 2: Call open on MockDISK
	printf("2. Calling open on MockDISK...\n");
	mock_disk->open(_T("test.dsk"), 0);
	printf("   open() called successfully\n");
	
	// Test 3: Check disk state
	printf("3. Checking disk state...\n");
	printf("   inserted: %s\n", mock_disk->is_disk_inserted() ? "true" : "false");
	printf("   write_protected: %s\n", mock_disk->is_disk_protected() ? "true" : "false");
	
	delete mock_disk;
	printf("   MockDISK deleted successfully\n");
}

void test_mb8877_disk_array() {
	printf("\n=== Testing MB8877 Disk Array ===\n");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	// Create MB8877
	printf("1. Creating MB8877...\n");
	MB8877 fdc(&vm, &emu);
	printf("   MB8877 created at %p\n", &fdc);
	
	// Initialize and configure
	printf("2. Initializing MB8877...\n");
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	printf("   MB8877 initialized\n");
	
	// Check disk array
	printf("3. Checking disk array...\n");
	// We can't directly access private disk array, but we can try open_disk
	
	// Try to open disk
	printf("4. Calling open_disk(0, \"test.dsk\", 0)...\n");
	fdc.open_disk(0, _T("test.dsk"), 0);
	printf("   open_disk() returned (if we get here, no crash)\n");
}

void test_with_replacement_disk() {
	printf("\n=== Testing with Disk Replacement ===\n");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	// Create MB8877
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	
	// Now we need to replace the disk[0] with a MockDISK
	// But disk array is private...
	// Let's see if we can work around this
	
	printf("1. Initializing MB8877...\n");
	fdc.initialize();
	
	// The initialize() method creates new DISK objects
	// We need a different approach
	printf("   Need to implement SafeMockDISK wrapper\n");
}

int main() {
	// Install signal handler
	signal(SIGSEGV, segfault_handler);
	
	printf("Phase 32: DISK::open Crash Debug\n");
	printf("=================================\n");
	
	// Run tests
	test_disk_initialization();
	test_mb8877_disk_array();
	test_with_replacement_disk();
	
	printf("\nAll tests completed without crash!\n");
	return 0;
}