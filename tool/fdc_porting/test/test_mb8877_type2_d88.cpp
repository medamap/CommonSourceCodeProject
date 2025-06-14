/*
	MB8877 Type II Command Tests with D88 Support
	
	Author : Claude AI Assistant
	Date   : 2025.06.13
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "mock_disk_d88.h"
#include "mb8877_test_d88.h"

void test_read_sector_d88(TestFramework& test) {
	TEST_SECTION("Read Sector from D88 Disk");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877_Test fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	// Open D88 disk using FDC's disk manager
	fdc.open_disk(0, _T("test_disks/test_2d_hubasic_empty.d88"), 0);
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Position to track 0, sector 1
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	
	// Issue read sector command
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Check BUSY flag
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "BUSY set during read sector");
	
	// Process the command
	for (int i = 0; i < 100; i++) {
		event.advance_clock(1000);
		status = fdc.read_io8(0);
		
		// Check for DRQ
		if (status & 0x02) {
			// Read the data byte
			uint8_t data = fdc.read_io8(3);
			// Read data byte
		}
		
		// Check if command completed
		if (!(status & 0x01)) {
			break;
		}
	}
	
	// Verify final status
	test.assert_true(!(status & 0x01), "BUSY cleared after read");
	test.assert_true(!(status & 0x10), "No record not found error");
	
	fdc.close_disk(0);
}

void test_write_sector_d88(TestFramework& test) {
	TEST_SECTION("Write Sector to D88 Disk");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877_Test fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	// Open D88 disk using FDC's disk manager
	fdc.open_disk(0, _T("test_disks/test_2dd_hubasic_empty.d88"), 0);
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Position to track 0, sector 1
	fdc.write_io8(1, 0); // Track register
	fdc.write_io8(2, 1); // Sector register
	
	// Issue write sector command
	fdc.write_io8(0, 0xA0); // Write sector command
	
	// Check BUSY flag
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "BUSY set during write sector");
	
	// Write test pattern
	uint8_t test_data[256];
	for (int i = 0; i < 256; i++) {
		test_data[i] = (uint8_t)(i & 0xFF);
	}
	
	int bytes_written = 0;
	for (int i = 0; i < 100; i++) {
		event.advance_clock(100);
		status = fdc.read_io8(0);
		
		// Check for DRQ
		if ((status & 0x02) && bytes_written < 256) {
			// Write data byte
			fdc.write_io8(3, test_data[bytes_written++]);
		}
		
		// Check if command completed
		if (!(status & 0x01)) {
			break;
		}
	}
	
	// Verify final status
	test.assert_true(!(status & 0x01), "BUSY cleared after write");
	test.assert_true(!(status & 0x40), "No write protect error");
	test.assert_equal(bytes_written, 256, "Wrote full sector");
	
	// Read back and verify
	fdc.write_io8(0, 0x80); // Read sector command
	
	int bytes_read = 0;
	uint8_t read_data[256];
	for (int i = 0; i < 100; i++) {
		event.advance_clock(100);
		status = fdc.read_io8(0);
		
		if ((status & 0x02) && bytes_read < 256) {
			read_data[bytes_read++] = fdc.read_io8(3);
		}
		
		if (!(status & 0x01)) {
			break;
		}
	}
	
	// Verify data matches
	bool data_matches = true;
	for (int i = 0; i < 256; i++) {
		if (read_data[i] != test_data[i]) {
			data_matches = false;
			break;
		}
	}
	test.assert_true(data_matches, "Written data matches read data");
	
	fdc.close_disk(0);
}

void test_multi_sector_d88(TestFramework& test) {
	TEST_SECTION("Multi-Sector Operations with D88");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877_Test fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	// Open D88 disk with pattern files
	fdc.open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Read multiple sectors
	int sectors_read = 0;
	for (int sector = 1; sector <= 5; sector++) {
		fdc.write_io8(1, 0); // Track register
		fdc.write_io8(2, sector); // Sector register
		
		// Issue read sector command
		fdc.write_io8(0, 0x80);
		
		// Process command
		int bytes_read = 0;
		for (int i = 0; i < 200; i++) {
			event.advance_clock(100);
			uint32_t status = fdc.read_io8(0);
			
			if (status & 0x02) {
				uint8_t data = fdc.read_io8(3);
				bytes_read++;
			}
			
			if (!(status & 0x01)) {
				if (!(status & 0x10)) { // No RNF error
					sectors_read++;
				}
				break;
			}
		}
		
		// Sector read completed
	}
	
	test.assert_true(sectors_read >= 3, "Read at least 3 sectors successfully");
	
	fdc.close_disk(0);
}

void test_cross_track_d88(TestFramework& test) {
	TEST_SECTION("Cross-Track Operations with D88");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877_Test fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	// Open D88 disk with multiple files
	fdc.open_disk(0, _T("test_disks/test_2d_hubasic_multi.d88"), 0);
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Seek to track 5
	fdc.write_io8(3, 5); // Data register with track number
	fdc.write_io8(0, 0x10); // Seek command
	
	// Wait for seek completion
	for (int i = 0; i < 50; i++) {
		event.advance_clock(1000);
		uint32_t status = fdc.read_io8(0);
		if (!(status & 0x01)) break;
	}
	
	// Read sector from track 5
	fdc.write_io8(2, 1); // Sector register
	fdc.write_io8(0, 0x80); // Read sector command
	
	int bytes_read = 0;
	for (int i = 0; i < 200; i++) {
		event.advance_clock(100);
		uint32_t status = fdc.read_io8(0);
		
		if (status & 0x02) {
			fdc.read_io8(3);
			bytes_read++;
		}
		
		if (!(status & 0x01)) {
			break;
		}
	}
	
	// Verify track register was updated
	uint8_t track_reg = fdc.read_io8(1);
	test.assert_equal((int)track_reg, 5, "Track register updated after seek");
	test.assert_true(bytes_read > 0, "Read data from track 5");
	
	fdc.close_disk(0);
}

int main() {
	TestFramework test;
	
	printf("=== MB8877 Type II D88 Tests ===\n\n");
	
	// Run D88-specific tests
	test_read_sector_d88(test);
	test_write_sector_d88(test);
	test_multi_sector_d88(test);
	test_cross_track_d88(test);
	
	// Print summary
	int total = test.get_total_tests();
	int passed = test.get_passed_tests();
	int failed = test.get_failed_tests();
	
	printf("\n=== Test Summary ===\n");
	printf("Total tests: %d\n", total);
	printf("Passed: %d\n", passed);
	printf("Failed: %d\n", failed);
	
	if (failed == 0) {
		printf("\nAll tests passed!\n");
	} else {
		printf("\nSome tests failed.\n");
	}
	
	return failed > 0 ? 1 : 0;
}