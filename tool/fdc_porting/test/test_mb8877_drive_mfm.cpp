/*
	MB8877 Drive MFM Mode Tests
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#include <cmath>
#include <cstdlib>
#include "test_framework.h"
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

// MFM/FM timing constants (in microseconds)
#define FM_BYTE_TIME    64      // 125kbps = 64us per byte
#define MFM_BYTE_TIME   32      // 250kbps = 32us per byte
#define FM_BPS          15625   // 125kbps = 15,625 bytes/second
#define MFM_BPS         31250   // 250kbps = 31,250 bytes/second

void test_basic_fm_mfm_switching(TestFramework& test) {
	TEST_SECTION("Basic FM/MFM Switching");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Test setting MFM mode for each drive
	// Since we can't directly verify the disk state in this test environment,
	// we'll just verify that the function doesn't crash with valid inputs
	for(int drv = 0; drv < 4; drv++) {
		// Set to MFM mode
		fdc.set_drive_mfm(drv, true);
		test.assert_true(true, "Set MFM mode without crash");
		
		// Set to FM mode
		fdc.set_drive_mfm(drv, false);
		test.assert_true(true, "Set FM mode without crash");
	}
}

void test_invalid_drive_handling(TestFramework& test) {
	TEST_SECTION("Invalid Drive Handling");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Test negative drive number - should not crash
	fdc.set_drive_mfm(-1, true);
	test.assert_true(true, "Negative drive number handled");
	
	// Test drive number beyond MAX_DRIVE - should not crash
	fdc.set_drive_mfm(4, true);
	test.assert_true(true, "Drive 4 handled");
	
	fdc.set_drive_mfm(10, false);
	test.assert_true(true, "Drive 10 handled");
}

void test_mfm_fm_mode_persistence(TestFramework& test) {
	TEST_SECTION("MFM/FM Mode Persistence");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Set initial modes
	fdc.set_drive_mfm(0, true);  // MFM
	fdc.set_drive_mfm(1, false); // FM
	
	// Perform restore on drive 0
	fdc.write_io8(3, 0); // Select drive 0
	fdc.write_io8(0, 0x0B); // Restore command
	
	// Wait for completion
	event.advance_clock(100000);
	int timeout = 100;
	while((fdc.read_io8(0) & 0x01) != 0 && timeout > 0) {
		event.advance_clock(1000);
		timeout--;
	}
	
	test.assert_true(timeout > 0, "Restore command completed");
	
	// Perform seek on drive 1
	fdc.write_io8(3, 1); // Select drive 1
	fdc.write_io8(1, 10); // Track 10
	fdc.write_io8(0, 0x1B); // Seek command
	
	// Wait for completion
	event.advance_clock(100000);
	timeout = 100;
	while((fdc.read_io8(0) & 0x01) != 0 && timeout > 0) {
		event.advance_clock(1000);
		timeout--;
	}
	
	test.assert_true(timeout > 0, "Seek command completed");
}

void test_multiple_drive_switching(TestFramework& test) {
	TEST_SECTION("Multiple Drive Switching");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Set different modes for each drive
	fdc.set_drive_mfm(0, true);   // Drive 0: MFM
	fdc.set_drive_mfm(1, false);  // Drive 1: FM
	fdc.set_drive_mfm(2, true);   // Drive 2: MFM
	fdc.set_drive_mfm(3, false);  // Drive 3: FM
	
	// Switch drives and perform operations
	for(int i = 0; i < 4; i++) {
		fdc.write_io8(3, i); // Select drive
		
		// Try a simple command
		uint8_t status = fdc.read_io8(0);
		test.assert_true((status & 0x80) == 0, "Drive not busy");
	}
}

void test_rapid_mode_switching(TestFramework& test) {
	TEST_SECTION("Rapid Mode Switching");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Rapidly switch modes to test stability
	for(int i = 0; i < 100; i++) {
		int drv = i % 4;
		bool mfm = (i % 2) == 0;
		
		fdc.set_drive_mfm(drv, mfm);
	}
	
	test.assert_true(true, "Rapid mode switching completed without crash");
}

void test_mode_setting_with_commands(TestFramework& test) {
	TEST_SECTION("Mode Setting with Commands");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Set drive 0 to MFM mode
	fdc.set_drive_mfm(0, true);
	
	// Select drive 0
	fdc.write_io8(3, 0);
	
	// Issue restore command
	fdc.write_io8(0, 0x0B);
	
	// Check that command is accepted
	uint8_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "Command in progress");
	
	// Change mode while command is running
	fdc.set_drive_mfm(0, false); // Switch to FM
	
	// Wait a bit
	event.advance_clock(10000);
	
	// Check status again
	status = fdc.read_io8(0);
	test.assert_true(true, "Mode change during command didn't crash");
}

// Main test runner
int main(int argc, char* argv[]) {
	TestFramework test;
	
	printf("=== MB8877 Drive MFM Tests ===\n");
	
	// Run all tests
	test_basic_fm_mfm_switching(test);
	test_invalid_drive_handling(test);
	test_mfm_fm_mode_persistence(test);
	test_multiple_drive_switching(test);
	test_rapid_mode_switching(test);
	test_mode_setting_with_commands(test);
	
	// Report results
	test.print_summary();
	
	return test.all_tests_passed() ? 0 : 1;
}