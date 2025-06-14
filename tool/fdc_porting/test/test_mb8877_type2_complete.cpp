/*
	MB8877 Type II Command Complete Tests - Phase 34
	
	Author : Claude AI Assistant
	Date   : 2025.01.14
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"
#include <chrono>
#include <vector>

// Helper function to wait for command completion
bool wait_command_complete(MB8877* fdc, MockEVENT* event, int max_cycles = 10000) {
	for(int i = 0; i < max_cycles; i++) {
		uint8_t status = fdc->read_io8(0);
		if(!(status & 0x01)) {  // BUSY cleared
			return true;
		}
		event->advance_clock(10);
	}
	return false;
}

// Helper function to read sector data
bool read_sector_data(MB8877* fdc, MockEVENT* event, std::vector<uint8_t>& data, int expected_size = 256) {
	data.clear();
	int timeout = 0;
	
	while(true) {
		uint8_t status = fdc->read_io8(0);
		
		if(!(status & 0x01)) {  // BUSY cleared
			break;
		}
		
		if(status & 0x02) {  // DRQ set
			uint8_t byte = fdc->read_io8(3);
			data.push_back(byte);
			timeout = 0;  // Reset timeout on successful read
		} else {
			timeout++;
			if(timeout > 1000) {
				return false;  // Timeout
			}
		}
		
		event->advance_clock(10);
	}
	
	return data.size() == expected_size;
}

// Helper function to write sector data
bool write_sector_data(MB8877* fdc, MockEVENT* event, const std::vector<uint8_t>& data) {
	size_t written = 0;
	int timeout = 0;
	
	while(written < data.size()) {
		uint8_t status = fdc->read_io8(0);
		
		if(!(status & 0x01)) {  // BUSY cleared prematurely
			return false;
		}
		
		if(status & 0x02) {  // DRQ set
			fdc->write_io8(3, data[written++]);
			timeout = 0;  // Reset timeout on successful write
		} else {
			timeout++;
			if(timeout > 1000) {
				return false;  // Timeout
			}
		}
		
		event->advance_clock(10);
	}
	
	// Wait for command completion
	return wait_command_complete(fdc, event);
}

void test_single_sector_read(TestFramework& test) {
	TEST_SECTION("Single Sector Read");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Position to track 0, sector 1
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 1);  // Sector register
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	// Read sector data
	std::vector<uint8_t> data;
	bool success = read_sector_data(&fdc, &event, data);
	
	test.assert_true(success, "Read sector completed successfully");
	test.assert_equal((int)data.size(), 256, "Read correct number of bytes");
	
	// Check status
	uint8_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared");
	test.assert_true((status & 0x04) == 0, "No lost data");
	test.assert_true((status & 0x08) == 0, "No CRC error");
}

void test_single_sector_write(TestFramework& test) {
	TEST_SECTION("Single Sector Write");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Generate test data
	std::vector<uint8_t> test_data(256);
	for(int i = 0; i < 256; i++) {
		test_data[i] = (i * 7 + 13) & 0xFF;  // Pseudo-random pattern
	}
	
	// Write to track 0, sector 2
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 2);  // Sector register
	fdc.write_io8(0, 0xA0);  // WRITE SECTOR command
	
	// Write sector data
	bool success = write_sector_data(&fdc, &event, test_data);
	
	test.assert_true(success, "Write sector completed successfully");
	
	// Check status
	uint8_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared");
	test.assert_true((status & 0x04) == 0, "No lost data");
	test.assert_true((status & 0x40) == 0, "Not write protected");
	
	// Read back and verify
	fdc.write_io8(2, 2);  // Sector register
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	std::vector<uint8_t> read_back;
	success = read_sector_data(&fdc, &event, read_back);
	
	test.assert_true(success, "Read back completed");
	bool data_matches = (read_back.size() == test_data.size());
	if(data_matches) {
		for(size_t i = 0; i < read_back.size(); i++) {
			if(read_back[i] != test_data[i]) {
				data_matches = false;
				break;
			}
		}
	}
	test.assert_true(data_matches, "Written data verified");
}

void test_multi_sector_read(TestFramework& test) {
	TEST_SECTION("Multi-Sector Read");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Read 3 consecutive sectors starting from sector 1
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 1);  // Sector register
	fdc.write_io8(0, 0x90);  // READ MULTIPLE command
	
	std::vector<uint8_t> all_data;
	int sectors_read = 0;
	int timeout = 0;
	
	while(sectors_read < 3) {
		uint8_t status = fdc.read_io8(0);
		
		if(!(status & 0x01)) {  // BUSY cleared
			break;
		}
		
		if(status & 0x02) {  // DRQ set
			uint8_t byte = fdc.read_io8(3);
			all_data.push_back(byte);
			
			if(all_data.size() % 256 == 0) {
				sectors_read++;
			}
			timeout = 0;
		} else {
			timeout++;
			if(timeout > 10000) {
				break;
			}
		}
		
		event.advance_clock(10);
		
		// Force interrupt after 3 sectors
		if(sectors_read >= 3) {
			fdc.write_io8(0, 0xD0);  // FORCE INTERRUPT
			event.advance_clock(100);
			break;
		}
	}
	
	test.assert_equal(sectors_read, 3, "Read 3 sectors");
	test.assert_equal((int)all_data.size(), 768, "Read 768 bytes total");
}

void test_multi_sector_write(TestFramework& test) {
	TEST_SECTION("Multi-Sector Write");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Prepare test data for 3 sectors
	std::vector<uint8_t> test_data(768);
	for(int i = 0; i < 768; i++) {
		test_data[i] = (i & 0xFF);
	}
	
	// Write 3 consecutive sectors starting from sector 5
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 5);  // Sector register
	fdc.write_io8(0, 0xB0);  // WRITE MULTIPLE command
	
	size_t written = 0;
	int timeout = 0;
	
	while(written < test_data.size()) {
		uint8_t status = fdc.read_io8(0);
		
		if(!(status & 0x01)) {  // BUSY cleared
			break;
		}
		
		if(status & 0x02) {  // DRQ set
			fdc.write_io8(3, test_data[written++]);
			timeout = 0;
		} else {
			timeout++;
			if(timeout > 10000) {
				break;
			}
		}
		
		event.advance_clock(10);
		
		// Force interrupt after 3 sectors
		if(written >= 768) {
			fdc.write_io8(0, 0xD0);  // FORCE INTERRUPT
			event.advance_clock(100);
			break;
		}
	}
	
	test.assert_equal((int)written, 768, "Wrote 768 bytes");
	
	// Verify by reading back
	fdc.write_io8(2, 5);  // Sector register
	fdc.write_io8(0, 0x90);  // READ MULTIPLE
	
	std::vector<uint8_t> read_back;
	read_sector_data(&fdc, &event, read_back, 256);
	
	test.assert_true(read_back.size() >= 256, "Read back at least first sector");
	bool match = true;
	for(size_t i = 0; i < 256 && i < read_back.size(); i++) {
		if(read_back[i] != test_data[i]) {
			match = false;
			break;
		}
	}
	test.assert_true(match, "Written data verified");
}

void test_sector_not_found(TestFramework& test) {
	TEST_SECTION("Sector Not Found");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Try to read non-existent sector 99
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 99);  // Invalid sector number
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	// Wait for command completion
	wait_command_complete(&fdc, &event, 20000);  // Longer timeout for RNF
	
	uint8_t status = fdc.read_io8(0);
	test.assert_true((status & 0x10) != 0, "Record Not Found flag set");
	test.assert_true((status & 0x01) == 0, "BUSY cleared");
}

void test_data_lost_condition(TestFramework& test) {
	TEST_SECTION("Data Lost Condition");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Issue read command
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 1);  // Sector register
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	// Wait for DRQ but don't read data immediately
	int drq_count = 0;
	for(int i = 0; i < 10000; i++) {
		uint8_t status = fdc.read_io8(0);
		if(status & 0x02) {  // DRQ set
			drq_count++;
			// Don't read data for first few DRQs to simulate slow CPU
			if(drq_count > 20) {
				// Now read, but it's too late
				fdc.read_io8(3);
			}
		}
		if(!(status & 0x01)) {  // BUSY cleared
			break;
		}
		event.advance_clock(50);  // Advance more to simulate delay
	}
	
	uint8_t status = fdc.read_io8(0);
	test.assert_true((status & 0x04) != 0, "Data Lost flag set");
}

void test_write_protect(TestFramework& test) {
	TEST_SECTION("Write Protect Detection");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Use SafeDISK and set write protect
	fdc.open_disk(0, nullptr, 0);
	fdc.is_disk_protected(0, true);  // Set write protect
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Try to write
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 1);  // Sector register
	fdc.write_io8(0, 0xA0);  // WRITE SECTOR command
	
	// Wait briefly
	event.advance_clock(1000);
	
	uint8_t status = fdc.read_io8(0);
	test.assert_true((status & 0x40) != 0, "Write Protect flag set");
	test.assert_true((status & 0x01) == 0, "BUSY cleared immediately");
}

void test_deleted_data_mark(TestFramework& test) {
	TEST_SECTION("Deleted Data Mark");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Write sector with deleted data mark (A1 command bit)
	std::vector<uint8_t> test_data(256, 0xDD);
	
	fdc.write_io8(1, 0);  // Track register
	fdc.write_io8(2, 8);  // Sector register
	fdc.write_io8(0, 0xA1);  // WRITE SECTOR with deleted data mark
	
	write_sector_data(&fdc, &event, test_data);
	
	// Read back and check for deleted data mark
	fdc.write_io8(2, 8);  // Sector register
	fdc.write_io8(0, 0x80);  // READ SECTOR command
	
	std::vector<uint8_t> read_data;
	read_sector_data(&fdc, &event, read_data);
	
	uint8_t status = fdc.read_io8(0);
	test.assert_true((status & 0x20) != 0, "Deleted data mark flag set");
}

void test_performance(TestFramework& test) {
	TEST_SECTION("Performance Test");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Don't open real disk in test - work with mock environment
	// Turn on motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	auto start = std::chrono::high_resolution_clock::now();
	
	// Read entire track (16 sectors)
	for(int sector = 1; sector <= 16; sector++) {
		fdc.write_io8(2, sector);
		fdc.write_io8(0, 0x80);  // READ SECTOR
		
		std::vector<uint8_t> data;
		read_sector_data(&fdc, &event, data);
	}
	
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	
	test.assert_true(duration.count() < 200, "Track read completed in reasonable time");
}

// Main test runner
bool run_type2_complete_tests() {
	TestFramework test;
	
	TEST_SUITE("MB8877 Type II Complete Tests - Phase 34");
	
	// Basic operations
	test_single_sector_read(test);
	test_single_sector_write(test);
	
	// Multi-sector operations
	test_multi_sector_read(test);
	test_multi_sector_write(test);
	
	// Error conditions
	test_sector_not_found(test);
	test_data_lost_condition(test);
	test_write_protect(test);
	test_deleted_data_mark(test);
	
	// Performance
	test_performance(test);
	
	test.print_summary();
	
	// Save results in JSON format
	FILE* fp = fopen("phase34_results.json", "w");
	if(fp) {
		// Calculate stats from test results
		int total = 9;  // Total number of tests
		int passed = test.all_tests_passed() ? total : 0;  // Simple approximation
		int failed = total - passed;
		float success_rate = total > 0 ? (float)passed / total * 100.0f : 0.0f;
		
		fprintf(fp, "{\n");
		fprintf(fp, "  \"phase\": 34,\n");
		fprintf(fp, "  \"task\": \"type2_command_completion\",\n");
		fprintf(fp, "  \"test_results\": {\n");
		fprintf(fp, "    \"total\": %d,\n", total);
		fprintf(fp, "    \"passed\": %d,\n", passed);
		fprintf(fp, "    \"failed\": %d,\n", failed);
		fprintf(fp, "    \"success_rate\": %.1f\n", success_rate);
		fprintf(fp, "  }\n");
		fprintf(fp, "}\n");
		fclose(fp);
	}
	
	return test.all_tests_passed();
}

int main() {
	return run_type2_complete_tests() ? 0 : 1;
}