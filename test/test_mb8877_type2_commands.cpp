/*
	MB8877 Type II Command Tests (Read/Write Sector)
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "../src/vm/mb8877_compat.h"

void test_read_sector_basic(TestFramework& test) {
	TEST_SECTION("Basic Read Sector Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk with test data
	mock_disk.open(_T("test.dsk"), 0);
	uint8_t test_data[256];
	for (int i = 0; i < 256; i++) {
		test_data[i] = i & 0xFF;
	}
	mock_disk.setup_mock_sector(0, 1, test_data, 256);
	
	// Position to track 0, sector 1
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	
	// Issue read sector command
	fdc.write_io8(0, 0x80); // Read sector command (0x80-0x9F)
	
	// Check BUSY flag
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "BUSY set during read sector");
	
	// Wait for data ready
	event.advance_clock(30000);
	
	// Check DRQ flag and read data
	status = fdc.read_io8(0);
	if (status & 0x02) { // DRQ set
		uint8_t read_data[256];
		for (int i = 0; i < 256; i++) {
			read_data[i] = fdc.read_io8(3); // Data register
		}
		
		test.assert_memory_equal(test_data, read_data, 256, "Read sector data matches");
	} else {
		test.assert_true(false, "DRQ not set for read sector");
	}
	
	// Check command completed
	status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared after read sector");
}

void test_write_sector_basic(TestFramework& test) {
	TEST_SECTION("Basic Write Sector Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk
	mock_disk.open(_T("test.dsk"), 0);
	mock_disk.set_write_protect(false);
	
	// Position to track 0, sector 1
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	
	// Issue write sector command
	fdc.write_io8(0, 0xA0); // Write sector command (0xA0-0xBF)
	
	// Check BUSY flag
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "BUSY set during write sector");
	
	// Wait for DRQ
	event.advance_clock(10000);
	status = fdc.read_io8(0);
	
	if (status & 0x02) { // DRQ set
		// Write test data
		uint8_t test_data[256];
		for (int i = 0; i < 256; i++) {
			test_data[i] = 0xAA ^ (i & 0xFF);
			fdc.write_io8(3, test_data[i]); // Data register
		}
		
		// Wait for write completion
		event.advance_clock(20000);
		
		status = fdc.read_io8(0);
		test.assert_true((status & 0x01) == 0, "BUSY cleared after write sector");
		test.assert_true((status & 0x04) == 0, "No lost data error");
	} else {
		test.assert_true(false, "DRQ not set for write sector");
	}
}

void test_write_protect_detection(TestFramework& test) {
	TEST_SECTION("Write Protect Detection Tests");
	
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
	
	// Try to write to protected disk
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	fdc.write_io8(0, 0xA0); // Write sector command
	
	// Wait for command processing
	event.advance_clock(5000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x40) != 0, "Write protect flag set");
	test.assert_true((status & 0x01) == 0, "BUSY cleared quickly on write protect");
}

void test_sector_not_found(TestFramework& test) {
	TEST_SECTION("Sector Not Found Tests");
	
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
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 99); // Invalid sector number
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Wait for search timeout
	event.advance_clock(200000); // Long timeout for sector search
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x10) != 0, "Record Not Found flag set");
	test.assert_true((status & 0x01) == 0, "BUSY cleared after RNF");
}

void test_crc_error_detection(TestFramework& test) {
	TEST_SECTION("CRC Error Detection Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk with corrupted data
	mock_disk.open(_T("test_corrupt.dsk"), 0);
	uint8_t corrupted_data[256];
	memset(corrupted_data, 0xFF, 256); // Simulated corruption
	mock_disk.setup_mock_sector(0, 1, corrupted_data, 256);
	
	// Read sector with CRC error
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	fdc.write_io8(0, 0x80); // Read sector command
	
	event.advance_clock(50000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x08) != 0, "CRC error flag set");
	test.assert_true((status & 0x01) == 0, "BUSY cleared after CRC error");
}

void test_multiple_sector_read(TestFramework& test) {
	TEST_SECTION("Multiple Sector Read Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk with multiple sectors
	mock_disk.open(_T("test.dsk"), 0);
	
	// Setup test data for sectors 1-3
	for (int sector = 1; sector <= 3; sector++) {
		uint8_t test_data[256];
		for (int i = 0; i < 256; i++) {
			test_data[i] = (sector * 16) + (i & 0x0F);
		}
		mock_disk.setup_mock_sector(0, sector, test_data, 256);
	}
	
	// Issue multiple sector read command
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Starting sector
	fdc.write_io8(0, 0x90); // Read multiple sectors (M=1)
	
	// Read three sectors
	for (int sector = 1; sector <= 3; sector++) {
		// Wait for DRQ
		event.advance_clock(30000);
		uint32_t status = fdc.read_io8(0);
		
		if (status & 0x02) { // DRQ set
			uint8_t read_data[256];
			for (int i = 0; i < 256; i++) {
				read_data[i] = fdc.read_io8(3);
			}
			
			// Verify data pattern
			bool data_correct = true;
			for (int i = 0; i < 256; i++) {
				uint8_t expected = (sector * 16) + (i & 0x0F);
				if (read_data[i] != expected) {
					data_correct = false;
					break;
				}
			}
			
			char test_name[64];
			sprintf(test_name, "Multiple sector %d data correct", sector);
			test.assert_true(data_correct, test_name);
		}
	}
	
	// Check final status
	event.advance_clock(10000);
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared after multiple sector read");
}

void test_data_lost_condition(TestFramework& test) {
	TEST_SECTION("Data Lost Condition Tests");
	
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
	
	// Issue read sector command
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Wait for DRQ but don't read data (simulate CPU too slow)
	event.advance_clock(50000);
	
	// Let the data lost timeout occur
	event.advance_clock(50000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x04) != 0, "Data lost flag set when data not read in time");
}

void test_deleted_data_mark(TestFramework& test) {
	TEST_SECTION("Deleted Data Mark Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Setup mock disk with deleted data mark sector
	mock_disk.open(_T("test_deleted.dsk"), 0);
	
	// Read sector with deleted data mark
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	fdc.write_io8(0, 0x80); // Read sector command
	
	event.advance_clock(50000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x20) != 0, "Deleted data mark flag set");
}

// Main test runner for Type II command tests
bool run_type2_command_tests() {
	TestFramework test;
	
	TEST_SUITE("MB8877 Type II Command Tests");
	
	test_read_sector_basic(test);
	test_write_sector_basic(test);
	test_write_protect_detection(test);
	test_sector_not_found(test);
	test_crc_error_detection(test);
	test_multiple_sector_read(test);
	test_data_lost_condition(test);
	test_deleted_data_mark(test);
	
	test.print_summary();
	test.save_results("test/results/type2_command_test_results.txt");
	
	return test.all_tests_passed();
}

#ifdef STANDALONE_TEST
int main() {
	return run_type2_command_tests() ? 0 : 1;
}
#endif