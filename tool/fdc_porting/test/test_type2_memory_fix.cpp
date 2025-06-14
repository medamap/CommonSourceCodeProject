/*
	Type II Command Memory Fix Test
	
	Purpose: Test Type II commands with fixed MockDISK_D88 memory management
	Author : Claude AI Assistant
	Date   : 2025.06.13
*/

#include <iostream>
#include <cstring>
#include "mock_environment.h"
#include "mock_disk_d88.h"
#include "mb8877_test_d88.h"
// MB8877 signal definitions
#define SIG_MB8877_MOTOR       0
#define SIG_MB8877_DRIVEREG    1
#define SIG_MB8877_SIDEREG     2

// Minimal test for Type II command with D88 disk
void test_type2_with_d88() {
	std::cout << "=== Testing Type II Commands with D88 Disk ===" << std::endl;
	
	try {
		// Create environment
		EMU emu;
		VM_TEMPLATE vm(&emu);
		MockEVENT event(&vm, &emu);
		
		// Create FDC with D88 support
		MB8877_Test fdc(&vm, &emu);
		fdc.set_context_event_manager(&event, 0, 0, 0);
		fdc.initialize();
		fdc.reset();
		
		// Enable motor
		fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
		
		// Load D88 disk
		std::cout << "Loading D88 disk..." << std::endl;
		fdc.open_disk(0, "test_disks/type2_test.d88", 0);
		
		// Position to track 0
		fdc.write_io8(1, 0); // Track register = 0
		fdc.write_io8(2, 1); // Sector register = 1
		
		// Issue read sector command
		std::cout << "Issuing read sector command..." << std::endl;
		fdc.write_io8(0, 0x80); // Read sector command
		
		// Check status
		uint32_t status = fdc.read_io8(0);
		std::cout << "Initial status: 0x" << std::hex << status << std::dec << std::endl;
		
		// Wait for completion
		for (int i = 0; i < 100; i++) {
			event.advance_clock(1000);
			status = fdc.read_io8(0);
			if (!(status & 0x01)) { // BUSY cleared
				break;
			}
		}
		
		std::cout << "Final status: 0x" << std::hex << status << std::dec << std::endl;
		
		if (status & 0x01) {
			std::cout << "ERROR: Command still busy after timeout" << std::endl;
		} else {
			std::cout << "SUCCESS: Read sector completed without crash" << std::endl;
		}
		
	} catch (const std::exception& e) {
		std::cout << "ERROR: Exception caught: " << e.what() << std::endl;
	} catch (...) {
		std::cout << "ERROR: Unknown exception caught" << std::endl;
	}
}

int main() {
	std::cout << "Type II Command Memory Fix Test\n" << std::endl;
	
	// First ensure we have test disks
	std::cout << "Generating test disks..." << std::endl;
	system("./generate_test_disks.sh");
	
	// Run the test
	test_type2_with_d88();
	
	std::cout << "\nTest completed." << std::endl;
	return 0;
}