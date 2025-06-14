/*
	MockDISK_D88 Isolated Memory Test
	
	Purpose: Test MockDISK_D88 operations that were causing crashes
	Author : Claude AI Assistant
	Date   : 2025.06.13
*/

#include <iostream>
#include <cstring>
#include "mock_environment.h"
#include "mock_disk_d88.h"

void test_disk_track_operations() {
	std::cout << "=== Testing Disk Track Operations ===" << std::endl;
	
	try {
		EMU emu;
		VM_TEMPLATE vm(&emu);
		
		// Create disk
		MockDISK_D88* disk = new MockDISK_D88(&vm, &emu);
		std::cout << "Disk created successfully" << std::endl;
		
		// Open a D88 file
		disk->open("test_disks/type2_test.d88", 0);
		std::cout << "D88 file opened" << std::endl;
		
		// Test track reading
		for (int track = 0; track < 5; track++) {
			for (int side = 0; side < 2; side++) {
				std::cout << "Reading track " << track << " side " << side << "... ";
				if (disk->get_track(track, side)) {
					std::cout << "OK (size=" << disk->track_size << ")" << std::endl;
				} else {
					std::cout << "FAILED" << std::endl;
				}
			}
		}
		
		// Test sector reading
		std::cout << "\nTesting sector reads..." << std::endl;
		for (int sector = 1; sector <= 3; sector++) {
			std::cout << "Reading sector " << sector << "... ";
			if (disk->get_sector(0, 0, sector)) {
				std::cout << "OK (ID: " << (int)disk->id[0] << "/" 
				          << (int)disk->id[1] << "/" << (int)disk->id[2] << ")" << std::endl;
			} else {
				std::cout << "FAILED" << std::endl;
			}
		}
		
		// Clean up
		delete disk;
		std::cout << "\nDisk destroyed successfully" << std::endl;
		
	} catch (const std::exception& e) {
		std::cout << "ERROR: Exception caught: " << e.what() << std::endl;
	} catch (...) {
		std::cout << "ERROR: Unknown exception caught" << std::endl;
	}
}

void test_multiple_disk_instances() {
	std::cout << "\n=== Testing Multiple Disk Instances ===" << std::endl;
	
	try {
		EMU emu;
		VM_TEMPLATE vm(&emu);
		
		// Create multiple disks
		MockDISK_D88* disks[4];
		for (int i = 0; i < 4; i++) {
			disks[i] = new MockDISK_D88(&vm, &emu);
			std::cout << "Disk " << i << " created" << std::endl;
		}
		
		// Open files on each
		disks[0]->open("test_disks/type2_test.d88", 0);
		disks[1]->open("test.dsk", 0);
		// Leave disks[2] and disks[3] unopened
		
		// Perform operations
		for (int i = 0; i < 4; i++) {
			if (disks[i]->is_disk_inserted()) {
				std::cout << "Disk " << i << " is inserted" << std::endl;
				disks[i]->get_track(0, 0);
			}
		}
		
		// Clean up in reverse order
		for (int i = 3; i >= 0; i--) {
			delete disks[i];
			std::cout << "Disk " << i << " destroyed" << std::endl;
		}
		
	} catch (const std::exception& e) {
		std::cout << "ERROR: Exception caught: " << e.what() << std::endl;
	} catch (...) {
		std::cout << "ERROR: Unknown exception caught" << std::endl;
	}
}

int main() {
	std::cout << "MockDISK_D88 Isolated Memory Test\n" << std::endl;
	
	// First ensure we have test disks
	std::cout << "Ensuring test disk exists..." << std::endl;
	system("mkdir -p test_disks");
	system("./generate_minimal_d88.py test_disks/type2_test.d88");
	
	// Run tests
	test_disk_track_operations();
	test_multiple_disk_instances();
	
	std::cout << "\nAll tests completed." << std::endl;
	return 0;
}