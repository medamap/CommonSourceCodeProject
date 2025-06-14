/*
	Phase 32: Simple DISK::open crash debug
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#include <cstdio>
#include <cstring>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>

// Include test framework first
#include "test_framework.h"

// Define needed constants before including mock_environment
#define _MB8877_H_  // Prevent mb8877.h inclusion

// Include mock environment
#include "mock_environment.h"

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

void test_base_disk_class() {
	printf("\n=== Testing Base DISK Class ===\n");
	
	// Test 1: Create base DISK
	printf("1. Creating base DISK...\n");
	MockEMU emu;
	DISK* disk = new DISK(&emu);
	printf("   Base DISK created at %p\n", disk);
	
	// Test 2: Call open
	printf("2. Calling open on base DISK...\n");
	disk->open(_T("test.dsk"), 0);
	printf("   open() called (no-op)\n");
	printf("   inserted: %s\n", disk->is_disk_inserted() ? "true" : "false");
	
	delete disk;
	printf("   Base DISK deleted\n");
}

void test_mock_disk_class() {
	printf("\n=== Testing MockDISK Class ===\n");
	
	// Test 1: Create MockDISK
	printf("1. Creating MockDISK...\n");
	MockEMU emu;
	MockVM vm(&emu);
	MockDISK* disk = new MockDISK(&vm, &emu);
	printf("   MockDISK created at %p\n", disk);
	
	// Test 2: Call open
	printf("2. Calling open on MockDISK...\n");
	disk->open(_T("test.dsk"), 0);
	printf("   open() called\n");
	printf("   inserted: %s\n", disk->is_disk_inserted() ? "true" : "false");
	
	// Test 3: Access disk members
	printf("3. Testing disk members...\n");
	printf("   sector buffer: %p\n", disk->sector);
	printf("   id buffer: %p\n", disk->id);
	printf("   track buffer: %p\n", disk->track);
	
	delete disk;
	printf("   MockDISK deleted\n");
}

void test_polymorphic_usage() {
	printf("\n=== Testing Polymorphic Usage ===\n");
	
	MockEMU emu;
	MockVM vm(&emu);
	
	// Create as base pointer
	printf("1. Creating MockDISK through base pointer...\n");
	DISK* disk = new MockDISK(&vm, &emu);
	printf("   Created at %p\n", disk);
	
	// Call virtual methods
	printf("2. Calling virtual methods...\n");
	disk->open(_T("test.dsk"), 0);
	printf("   open() called\n");
	
	bool inserted = disk->is_disk_inserted();
	printf("   is_disk_inserted(): %s\n", inserted ? "true" : "false");
	
	delete disk;
	printf("   Deleted through base pointer\n");
}

int main() {
	// Install signal handler
	signal(SIGSEGV, segfault_handler);
	
	printf("Phase 32: Simple DISK::open Debug\n");
	printf("==================================\n");
	
	// Run tests
	test_base_disk_class();
	test_mock_disk_class(); 
	test_polymorphic_usage();
	
	printf("\nAll tests completed successfully!\n");
	return 0;
}