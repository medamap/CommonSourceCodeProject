/*
	Quick Type II Command Test
	
	Isolates Type II issues without disk operations
*/

#include <iostream>
#include <cstring>
#include "mock_environment.h"
#include "mb8877_test_wrapper.h"
#include "mb8877_test_helper.h"

int main() {
	std::cout << "=== Quick Type II Command Test ===" << std::endl;
	
	// Initialize test environment
	MockEMU* emu = new MockEMU();
	VM_TEMPLATE* vm = new VM_TEMPLATE(emu);
	
	// Create FDC without disk operations
	MB8877* fdc = new MB8877(vm, emu);
	
	// Initialize FDC
	fdc->initialize();
	fdc->reset();
	
	std::cout << "FDC initialized successfully" << std::endl;
	
	// Test basic Type II command
	std::cout << "\n--- Testing READ SECTOR command ---" << std::endl;
	
	// Set track 0, sector 1
	fdc->write_io8(1, 0);  // Track register = 0
	fdc->write_io8(2, 1);  // Sector register = 1
	
	// Issue READ SECTOR command (0x80)
	std::cout << "Issuing READ SECTOR command..." << std::endl;
	fdc->write_io8(0, 0x80);
	
	// Check status
	uint32_t status = fdc->read_io8(0);
	std::cout << "Status after command: 0x" << std::hex << status << std::dec << std::endl;
	
	// Check if NOT READY (expected without disk)
	if (status & 0x80) {
		std::cout << "[PASS] NOT READY flag set as expected" << std::endl;
	} else {
		std::cout << "[FAIL] NOT READY flag not set" << std::endl;
	}
	
	// Test WRITE SECTOR command
	std::cout << "\n--- Testing WRITE SECTOR command ---" << std::endl;
	fdc->write_io8(0, 0xA0);
	
	status = fdc->read_io8(0);
	std::cout << "Status after command: 0x" << std::hex << status << std::dec << std::endl;
	
	// Clean up
	delete fdc;
	delete vm;
	delete emu;
	
	std::cout << "\nTest completed without crash!" << std::endl;
	
	return 0;
}