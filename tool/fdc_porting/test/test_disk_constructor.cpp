/*
	MockDISK_D88 Constructor Memory Debug Test
	
	Purpose: Isolate memory corruption in MockDISK_D88 constructor
	Author : Claude AI Assistant
	Date   : 2025.06.13
*/

#include <iostream>
#include <cstring>
#include "mock_environment.h"
#include "mock_disk_d88.h"

void test_basic_disk_construction() {
	std::cout << "=== Testing Basic DISK Construction ===" << std::endl;
	try {
		EMU emu;
		DISK* disk = new DISK(&emu);
		std::cout << "Basic DISK created successfully" << std::endl;
		std::cout << "  sector buffer: " << (void*)disk->sector << std::endl;
		std::cout << "  id buffer: " << (void*)disk->id << std::endl;
		std::cout << "  track buffer: " << (void*)disk->track << std::endl;
		std::cout << "  unstable buffer: " << (void*)disk->unstable << std::endl;
		delete disk;
		std::cout << "Basic DISK destroyed successfully" << std::endl;
	} catch (...) {
		std::cout << "ERROR: Exception in basic DISK construction!" << std::endl;
	}
}

void test_mock_disk_d88_construction() {
	std::cout << "\n=== Testing MockDISK_D88 Construction ===" << std::endl;
	try {
		EMU emu;
		VM_TEMPLATE vm(&emu);
		
		std::cout << "Creating MockDISK_D88..." << std::endl;
		MockDISK_D88* disk = new MockDISK_D88(&vm, &emu);
		std::cout << "MockDISK_D88 created successfully" << std::endl;
		
		// Check inherited pointers
		std::cout << "  sector buffer: " << (void*)disk->sector << std::endl;
		std::cout << "  id buffer: " << (void*)disk->id << std::endl;
		std::cout << "  track buffer: " << (void*)disk->track << std::endl;
		std::cout << "  unstable buffer: " << (void*)disk->unstable << std::endl;
		
		// Test basic operations
		std::cout << "Testing basic operations..." << std::endl;
		bool inserted = disk->is_disk_inserted();
		std::cout << "  is_disk_inserted(): " << inserted << std::endl;
		
		delete disk;
		std::cout << "MockDISK_D88 destroyed successfully" << std::endl;
	} catch (...) {
		std::cout << "ERROR: Exception in MockDISK_D88 construction!" << std::endl;
	}
}

void test_disk_open_operation() {
	std::cout << "\n=== Testing Disk Open Operation ===" << std::endl;
	try {
		EMU emu;
		VM_TEMPLATE vm(&emu);
		
		MockDISK_D88* disk = new MockDISK_D88(&vm, &emu);
		std::cout << "MockDISK_D88 created" << std::endl;
		
		// Test opening a non-D88 file
		std::cout << "Opening mock disk..." << std::endl;
		disk->open("test.dsk", 0);
		std::cout << "Mock disk opened successfully" << std::endl;
		
		// Test opening a D88 file
		std::cout << "Opening D88 disk..." << std::endl;
		disk->open("test.d88", 0);
		std::cout << "D88 disk open attempted" << std::endl;
		
		delete disk;
		std::cout << "MockDISK_D88 destroyed successfully" << std::endl;
	} catch (...) {
		std::cout << "ERROR: Exception in disk open operation!" << std::endl;
	}
}

int main() {
	std::cout << "MockDISK_D88 Constructor Memory Debug Test\n" << std::endl;
	
	// Test 1: Basic DISK construction
	test_basic_disk_construction();
	
	// Test 2: MockDISK_D88 construction
	test_mock_disk_d88_construction();
	
	// Test 3: Disk open operations
	test_disk_open_operation();
	
	std::cout << "\nAll tests completed." << std::endl;
	return 0;
}