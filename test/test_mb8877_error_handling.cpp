/*
	MB8877 Error Handling Tests
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "../src/vm/mb8877_compat.h"

void test_record_not_found_error(TestFramework& test) {
	TEST_SECTION("Record Not Found (RNF) Error");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk
	mock_disk.open(_T("test.dsk"), 0);
	
	// Try to read non-existent sector
	fdc.write_io8(1, 0);     // Track register = 0
	fdc.write_io8(2, 99);    // Sector register = 99 (non-existent)
	fdc.write_io8(0, 0x80);  // Read sector command
	
	// Wait for command to complete
	event.advance_clock(200000); // Long timeout for search
	
	// Check status
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY flag cleared");
	test.assert_true((status & 0x10) != 0, "RNF (Record Not Found) flag set");
	test.assert_true(fdc.get_intr_ack(), "IRQ generated on error");
	
	// Clear IRQ and check RNF persists
	status = fdc.read_io8(0);
	test.assert_true((status & 0x10) != 0, "RNF flag persists after status read");
	
	// Issue new command to clear RNF
	fdc.write_io8(0, 0x00); // Restore command
	event.advance_clock(100);
	status = fdc.read_io8(0);
	test.assert_true((status & 0x10) == 0, "RNF flag cleared by new command");
}

void test_crc_error(TestFramework& test) {
	TEST_SECTION("CRC Error");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk with CRC error
	mock_disk.open(_T("test.dsk"), 0);
	mock_disk.set_crc_error(true);
	
	// Read sector
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Process some data
	event.advance_clock(10000);
	
	// Read data bytes to trigger CRC check
	for(int i = 0; i < 256; i++) {
		if(fdc.read_io8(0) & 0x02) { // DRQ set
			fdc.read_io8(3); // Read data
			event.advance_clock(100);
		}
	}
	
	// Wait for completion
	event.advance_clock(10000);
	
	// Check CRC error flag
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x08) != 0, "CRC error flag set");
	test.assert_true(fdc.get_intr_ack(), "IRQ generated on CRC error");
}

void test_data_lost_error(TestFramework& test) {
	TEST_SECTION("Data Lost Error");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk
	mock_disk.open(_T("test.dsk"), 0);
	
	// Start read sector
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Wait for DRQ
	event.advance_clock(10000);
	uint32_t status = fdc.read_io8(0);
	
	if(status & 0x02) { // DRQ set
		// Don't read data, let it timeout to cause data lost
		event.advance_clock(50000); // Wait for overrun
		
		// Check for data lost
		status = fdc.read_io8(0);
		test.assert_true((status & 0x04) != 0, "Data Lost flag set");
	}
}

void test_write_protect_error(TestFramework& test) {
	TEST_SECTION("Write Protect Error");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup write-protected disk
	mock_disk.open(_T("test.dsk"), 0);
	mock_disk.set_write_protect(true);
	
	// Attempt write sector
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0xA0); // Write sector command
	
	// Check immediate write protect error
	event.advance_clock(1000);
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x40) != 0, "Write Protect flag set");
	test.assert_true((status & 0x01) == 0, "BUSY flag cleared on WP error");
	test.assert_true(fdc.get_intr_ack(), "IRQ generated on WP error");
	
	// Try write track command
	fdc.write_io8(0, 0xF0); // Write track command
	event.advance_clock(1000);
	status = fdc.read_io8(0);
	test.assert_true((status & 0x40) != 0, "Write Protect flag set for write track");
}

void test_not_ready_error(TestFramework& test) {
	TEST_SECTION("Not Ready Error");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open disk - drive not ready
	
	// Try read sector on not ready drive
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Check not ready error
	event.advance_clock(1000);
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x80) != 0, "Not Ready flag set");
	test.assert_true((status & 0x01) == 0, "BUSY flag cleared on not ready");
	test.assert_true(fdc.get_intr_ack(), "IRQ generated on not ready");
	
	// Type I command should also show not ready
	fdc.write_io8(0, 0x00); // Restore command
	event.advance_clock(1000);
	status = fdc.read_io8(0);
	test.assert_true((status & 0x80) != 0, "Not Ready flag set for Type I command");
}

void test_multiple_errors(TestFramework& test) {
	TEST_SECTION("Multiple Simultaneous Errors");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup write-protected disk with CRC error
	mock_disk.open(_T("test.dsk"), 0);
	mock_disk.set_write_protect(true);
	mock_disk.set_crc_error(true);
	
	// Try to write - should get write protect error first
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0xA0); // Write sector command
	
	event.advance_clock(1000);
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x40) != 0, "Write Protect flag has priority");
	
	// Clear write protect and try read - should get CRC error
	mock_disk.set_write_protect(false);
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Process sector
	event.advance_clock(10000);
	for(int i = 0; i < 256; i++) {
		if(fdc.read_io8(0) & 0x02) {
			fdc.read_io8(3);
			event.advance_clock(100);
		}
	}
	
	event.advance_clock(10000);
	status = fdc.read_io8(0);
	test.assert_true((status & 0x08) != 0, "CRC error flag set after WP cleared");
}

void test_error_recovery(TestFramework& test) {
	TEST_SECTION("Error Recovery");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup disk with temporary error
	mock_disk.open(_T("test.dsk"), 0);
	mock_disk.set_crc_error(true);
	
	// First read fails with CRC error
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0x80); // Read sector command
	
	event.advance_clock(50000);
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x08) != 0, "First read has CRC error");
	
	// Clear error condition
	mock_disk.set_crc_error(false);
	
	// Retry read - should succeed
	fdc.write_io8(0, 0x80); // Read sector command again
	
	event.advance_clock(10000);
	bool data_read = false;
	for(int i = 0; i < 300; i++) {
		status = fdc.read_io8(0);
		if(status & 0x02) { // DRQ
			fdc.read_io8(3);
			data_read = true;
		}
		event.advance_clock(100);
	}
	
	status = fdc.read_io8(0);
	test.assert_true((status & 0x08) == 0, "CRC error cleared on retry");
	test.assert_true(data_read, "Data successfully read on retry");
}

void test_error_during_seek(TestFramework& test) {
	TEST_SECTION("Error During Seek");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Start with ready drive
	mock_disk.open(_T("test.dsk"), 0);
	
	// Start seek
	fdc.write_io8(3, 10);   // Data register = target track
	fdc.write_io8(0, 0x10); // Seek command
	
	// Simulate drive becoming not ready during seek
	event.advance_clock(5000);
	mock_disk.close();
	
	// Wait for seek to complete/fail
	event.advance_clock(55000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x80) != 0, "Not Ready flag set when drive removed during seek");
	test.assert_true((status & 0x01) == 0, "BUSY flag cleared");
	test.assert_true(fdc.get_intr_ack(), "IRQ generated on seek error");
}

// Main test runner for error handling
bool run_error_handling_tests() {
	TestFramework test;
	
	TEST_SUITE("MB8877 Error Handling Tests");
	
	test_record_not_found_error(test);
	test_crc_error(test);
	test_data_lost_error(test);
	test_write_protect_error(test);
	test_not_ready_error(test);
	test_multiple_errors(test);
	test_error_recovery(test);
	test_error_during_seek(test);
	
	test.print_summary();
	test.save_results("test/results/error_handling_test_results.txt");
	
	return test.all_tests_passed();
}

#ifdef STANDALONE_TEST
int main() {
	return run_error_handling_tests() ? 0 : 1;
}
#endif