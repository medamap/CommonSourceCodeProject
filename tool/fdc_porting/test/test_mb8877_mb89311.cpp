/*
	MB89311 Extended Functionality Tests
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#include "test_framework.h"

// Prevent including real headers that would conflict with mock environment
#define _EMU_H_
#define _DEVICE_H_
#define _VM_TEMPLATE_H_

#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

// Helper class for MB89311 testing
class MB89311TestHelper {
public:
	MockEMU emu;
	MockVM vm;
	MockEVENT event;
	MB8877 fdc;
	
	MB89311TestHelper() : vm(&emu), event(&vm, &emu), fdc(&vm, &emu) {
		fdc.set_context_event_manager(&event, 0, 0, 0);
		fdc.initialize();
		fdc.reset();
	}
	
	void InsertFormattedDisk(int drive) {
		// Create a minimal disk image for testing
		DISK* disk = new DISK(&emu);
		disk->set_device_name("TEST DISK");
		disk->inserted = true;
		disk->write_protected = false;
		disk->drive_rpm = 300;
		disk->drive_mfm = true;
		
		// Create a simple track for testing
		// Initialize track data
		
		// Set id and data positions
		for(int i = 0; i < 16; i++) {
			disk->id_position[i] = i * 600;
			disk->data_position[i] = i * 600 + 100;
		}
		
		fdc.open_disk(drive, "test.d88", 0);
	}
	
	void WaitForCompletion(int max_cycles = 1000000) {
		int cycles = 0;
		while((fdc.read_io8(0) & 0x01) && cycles < max_cycles) { // Check BUSY bit
			event.event_callback(0, 0);
			cycles++;
		}
	}
	
#ifdef HAS_MB89311
	bool IsExtendedMode() {
		return fdc.extended_mode;
	}
	
	uint8_t GetMB89311Param(int index) {
		if(index >= 0 && index < 8) {
			return fdc.mb89311_params[index];
		}
		return 0;
	}
	
	bool IsFormatMode() {
		return fdc.mb89311_format_mode;
	}
	
	bool IsUsingParams() {
		return fdc.mb89311_use_params;
	}
#endif
};

// Test extended commands (0xFC-0xFF)
void test_extended_command_delay(TestFramework& test) {
	TEST_SECTION("Extended Command - Delay (0xFC)");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	
	// Test delay command with various parameters
	helper.fdc.write_io8(0, 0xFC);  // Delay command
	helper.fdc.write_io8(3, 0x10);  // 16 * 16us = 256us delay
	
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_equal_hex(0x00, status, "Status should be idle after delay command");
	
	// Test with zero delay
	helper.fdc.write_io8(0, 0xFC);
	helper.fdc.write_io8(3, 0x00);  // No delay
	
	status = helper.fdc.read_io8(0);
	test.assert_equal_hex(0x00, status, "Status should be idle with zero delay");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_extended_command_assign_parameter(TestFramework& test) {
	TEST_SECTION("Extended Command - Assign Parameter (0xFD)");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	
	// Test parameter assignment
	for(int i = 0; i < 8; i++) {
		uint8_t test_value = (i + 1) * 10;
		uint8_t param_data = (test_value << 3) | i;  // Value in upper 5 bits, index in lower 3
		
		helper.fdc.write_io8(0, 0xFD);  // Assign parameter command
		helper.fdc.write_io8(3, param_data);
		
		char msg[100];
		sprintf(msg, "Parameter %d not stored correctly", i);
		test.assert_equal(test_value, helper.GetMB89311Param(i), msg);
	}
	
	// Test parameter persistence
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_equal_hex(0x00, status, "Status should be idle after parameter assignment");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_extended_command_assign_mode(TestFramework& test) {
	TEST_SECTION("Extended Command - Assign Mode (0xFE)");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	
	// Test mode switching
	helper.fdc.write_io8(0, 0xFE);  // Assign mode command
	helper.fdc.write_io8(3, 0x00);  // Standard mode
	
	test.assert_true(!helper.IsExtendedMode(), "Should be in standard mode");
	test.assert_true(!helper.IsFormatMode(), "Format mode should be disabled");
	test.assert_true(!helper.IsUsingParams(), "Parameter usage should be disabled");
	
	// Switch to extended mode with format mode
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x03);  // Extended mode + format mode
	
	test.assert_true(helper.IsExtendedMode(), "Should be in extended mode");
	test.assert_true(helper.IsFormatMode(), "Format mode should be enabled");
	test.assert_true(!helper.IsUsingParams(), "Parameter usage should still be disabled");
	
	// Enable parameter usage
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x07);  // All modes enabled
	
	test.assert_true(helper.IsExtendedMode(), "Should be in extended mode");
	test.assert_true(helper.IsFormatMode(), "Format mode should be enabled");
	test.assert_true(helper.IsUsingParams(), "Parameter usage should be enabled");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_extended_command_reset(TestFramework& test) {
	TEST_SECTION("Extended Command - Reset (0xFF)");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	
	// Set up some state
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x00);  // Switch to standard mode
	
	// Store some parameters
	for(int i = 0; i < 4; i++) {
		helper.fdc.write_io8(0, 0xFD);
		helper.fdc.write_io8(3, (0x20 << 3) | i);
	}
	
	// Issue reset command
	helper.fdc.write_io8(0, 0xFF);  // Reset command
	
	// Verify reset state
	test.assert_true(helper.IsExtendedMode(), "Should default to extended mode after reset");
	test.assert_true(!helper.IsFormatMode(), "Format mode should be disabled after reset");
	test.assert_true(!helper.IsUsingParams(), "Parameter usage should be disabled after reset");
	
	// Check parameters cleared
	for(int i = 0; i < 8; i++) {
		char msg[100];
		sprintf(msg, "Parameter %d not cleared", i);
		test.assert_equal(0, helper.GetMB89311Param(i), msg);
	}
	
	// Check status indicates track 0
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x04) != 0, "TR00 bit should be set after reset");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_read_after_seek(TestFramework& test) {
	TEST_SECTION("Read After Seek (0x44)");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	helper.InsertFormattedDisk(0);
	
	// Set extended mode
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x01);  // Extended mode
	
	// Set target track and sector
	helper.fdc.write_io8(1, 10);  // Track 10
	helper.fdc.write_io8(2, 1);   // Sector 1
	
	// Issue read-after-seek command
	helper.fdc.write_io8(0, 0x44);  // Read after seek (extended mode)
	
	// Wait for seek completion
	helper.WaitForCompletion();
	
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY bit should be clear after completion");
	
	// Should have performed both seek and read
	test.assert_equal(10, helper.fdc.read_io8(1), "Track register should be updated to 10");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_write_after_seek(TestFramework& test) {
	TEST_SECTION("Write After Seek (0x64)");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	helper.InsertFormattedDisk(0);
	
	// Set extended mode
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x01);  // Extended mode
	
	// Set target track and sector
	helper.fdc.write_io8(1, 5);   // Track 5
	helper.fdc.write_io8(2, 3);   // Sector 3
	
	// Issue write-after-seek command
	helper.fdc.write_io8(0, 0x64);  // Write after seek (extended mode)
	
	// Should set DRQ for data
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x02) != 0, "DRQ should be set for write data");
	
	// Write test data
	for(int i = 0; i < 256; i++) {
		helper.fdc.write_io8(3, i & 0xFF);
	}
	
	helper.WaitForCompletion();
	
	status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY bit should be clear after completion");
	test.assert_equal(5, helper.fdc.read_io8(1), "Track register should be updated to 5");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_enhanced_format_command(TestFramework& test) {
	TEST_SECTION("Enhanced Format Command");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	helper.InsertFormattedDisk(0);
	
	// Enable extended mode with format mode and parameters
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x07);  // All modes enabled
	
	// Set format parameters
	helper.fdc.write_io8(0, 0xFD);
	helper.fdc.write_io8(3, (22 << 3) | 0);  // Gap length between sectors
	
	helper.fdc.write_io8(0, 0xFD);
	helper.fdc.write_io8(3, (50 << 3) | 1);  // Gap length after index
	
	helper.fdc.write_io8(0, 0xFD);
	helper.fdc.write_io8(3, (1 << 3) | 2);   // Sector size code (256 bytes)
	
	helper.fdc.write_io8(0, 0xFD);
	helper.fdc.write_io8(3, (16 << 3) | 3);  // Number of sectors
	
	// Issue format command
	helper.fdc.write_io8(0, 0xF1);  // Format command
	
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "BUSY bit should be set during format");
	test.assert_true((status & 0x02) != 0, "DRQ should be set for format data");
	
	// Provide format data
	for(int sector = 1; sector <= 16; sector++) {
		helper.fdc.write_io8(3, 0);       // Track
		helper.fdc.write_io8(3, 0);       // Side
		helper.fdc.write_io8(3, sector);  // Sector
		helper.fdc.write_io8(3, 1);       // Size
	}
	
	helper.WaitForCompletion();
	
	status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY bit should be clear after format");
	test.assert_true((status & 0x20) == 0, "Write fault should not be set");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_mode_compatibility(TestFramework& test) {
	TEST_SECTION("Mode Compatibility");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	helper.InsertFormattedDisk(0);
	
	// Standard mode
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x00);  // Standard mode
	
	// Try restore command
	helper.fdc.write_io8(0, 0x03);  // Restore with verify
	helper.WaitForCompletion();
	
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x04) != 0, "TR00 should be set after restore");
	
	// Extended mode
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x01);  // Extended mode
	
	// Same restore command should work
	helper.fdc.write_io8(1, 10);    // Set track to non-zero
	helper.fdc.write_io8(0, 0x03);  // Restore with verify
	helper.WaitForCompletion();
	
	status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x04) != 0, "TR00 should be set after restore in extended mode");
	test.assert_equal(0, helper.fdc.read_io8(1), "Track register should be 0");
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_parameter_persistence(TestFramework& test) {
	TEST_SECTION("Parameter Persistence");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	
	// Store parameters
	for(int i = 0; i < 8; i++) {
		helper.fdc.write_io8(0, 0xFD);
		helper.fdc.write_io8(3, ((i * 5 + 10) << 3) | i);
	}
	
	// Verify parameters persist across mode changes
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x00);  // Standard mode
	
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x01);  // Back to extended mode
	
	// Check parameters still there
	for(int i = 0; i < 8; i++) {
		char msg[100];
		sprintf(msg, "Parameter %d should persist", i);
		test.assert_equal(i * 5 + 10, helper.GetMB89311Param(i), msg);
	}
#else
	test.skip("MB89311 support not enabled");
#endif
}

void test_complete_workflow(TestFramework& test) {
	TEST_SECTION("Complete MB89311 Workflow");
	
#ifdef HAS_MB89311
	MB89311TestHelper helper;
	helper.InsertFormattedDisk(0);
	
	// 1. Reset to known state
	helper.fdc.write_io8(0, 0xFF);
	test.assert_true(helper.IsExtendedMode(), "Should be in extended mode after reset");
	
	// 2. Set up parameters for custom format
	helper.fdc.write_io8(0, 0xFD);
	helper.fdc.write_io8(3, (30 << 3) | 0);  // Gap parameter
	test.assert_equal(30, helper.GetMB89311Param(0), "Parameter 0 should be set");
	
	// 3. Enable all extended features
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x07);
	test.assert_true(helper.IsExtendedMode() && helper.IsFormatMode() && helper.IsUsingParams(),
		"All extended features should be enabled");
	
	// 4. Perform read-after-seek
	helper.fdc.write_io8(1, 2);
	helper.fdc.write_io8(2, 1);
	helper.fdc.write_io8(0, 0x44);
	
	helper.WaitForCompletion();
	
	uint8_t status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY should be clear after read-after-seek");
	
	// 5. Add delay
	helper.fdc.write_io8(0, 0xFC);
	helper.fdc.write_io8(3, 10);
	
	// 6. Switch to standard mode and verify basic operation
	helper.fdc.write_io8(0, 0xFE);
	helper.fdc.write_io8(3, 0x00);
	
	// Standard seek should still work
	helper.fdc.write_io8(1, 0);
	helper.fdc.write_io8(0, 0x13);  // Seek with verify
	
	helper.WaitForCompletion();
	
	status = helper.fdc.read_io8(0);
	test.assert_true((status & 0x04) != 0, "TR00 should be set after seek to track 0");
#else
	test.skip("MB89311 support not enabled");
#endif
}

int main() {
	TestFramework test;
	printf("MB8877 MB89311 Extended Functionality Tests\n");
	printf("============================================\n");
	
	// Extended command tests
	test_extended_command_delay(test);
	test_extended_command_assign_parameter(test);
	test_extended_command_assign_mode(test);
	test_extended_command_reset(test);
	
	// Read/Write after seek tests
	test_read_after_seek(test);
	test_write_after_seek(test);
	
	// Enhanced format test
	test_enhanced_format_command(test);
	
	// Compatibility tests
	test_mode_compatibility(test);
	test_parameter_persistence(test);
	
	// Integration test
	test_complete_workflow(test);
	
	test.print_summary();
	return test.all_tests_passed() ? 0 : 1;
}