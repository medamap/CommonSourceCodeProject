/*
	MB8877 Timing Verification Tests
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "../src/vm/mb8877_compat.h"

// MB8877 timing constants (in microseconds)
#define STEP_TIME_6MS    6000
#define STEP_TIME_12MS   12000
#define STEP_TIME_20MS   20000
#define STEP_TIME_30MS   30000
#define HEAD_LOAD_TIME   15000  // Typical head load time
#define DRQ_TIMING_BYTE  32     // Typical byte timing for 250kbps
#define INDEX_PULSE_TIME 200000 // 200ms for 300rpm disk

void test_seek_timing(TestFramework& test) {
	TEST_SECTION("Seek Command Timing");
	
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
	
	// Test different step rates
	uint8_t step_rates[] = {0x00, 0x01, 0x02, 0x03}; // 6ms, 12ms, 20ms, 30ms
	uint32_t expected_times[] = {STEP_TIME_6MS, STEP_TIME_12MS, STEP_TIME_20MS, STEP_TIME_30MS};
	
	for(int i = 0; i < 4; i++) {
		// Reset to track 0
		fdc.write_io8(0, 0x00); // Restore
		event.advance_clock(100000);
		
		// Seek to track 10 with specific step rate
		fdc.write_io8(3, 10); // Target track
		fdc.write_io8(0, 0x10 | (step_rates[i] << 0)); // Seek command with step rate
		
		uint32_t start_time = event.get_current_clock();
		
		// Wait for seek completion
		while((fdc.read_io8(0) & 0x01) != 0 && event.get_current_clock() - start_time < 500000) {
			event.advance_clock(1000);
		}
		
		uint32_t elapsed = event.get_current_clock() - start_time;
		uint32_t expected = expected_times[i] * 10; // 10 tracks
		
		// Allow 20% tolerance
		test.assert_true(elapsed >= expected * 0.8 && elapsed <= expected * 1.2,
			"Seek timing within tolerance");
	}
}

void test_drq_timing(TestFramework& test) {
	TEST_SECTION("DRQ Timing");
	
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
	
	// Read sector
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0x80); // Read sector command
	
	// Wait for first DRQ
	uint32_t drq_count = 0;
	uint32_t first_drq_time = 0;
	uint32_t last_drq_time = 0;
	
	for(int i = 0; i < 100000; i++) {
		uint32_t status = fdc.read_io8(0);
		if(status & 0x02) { // DRQ set
			if(drq_count == 0) {
				first_drq_time = event.get_current_clock();
			}
			last_drq_time = event.get_current_clock();
			
			fdc.read_io8(3); // Read data
			drq_count++;
			
			if(drq_count >= 256) break; // Sector complete
		}
		event.advance_clock(1);
	}
	
	test.assert_true(drq_count > 0, "DRQ generated during read");
	
	if(drq_count > 1) {
		uint32_t total_time = last_drq_time - first_drq_time;
		uint32_t avg_byte_time = total_time / (drq_count - 1);
		
		// Check average byte timing (allow wide tolerance for emulation)
		test.assert_true(avg_byte_time >= DRQ_TIMING_BYTE * 0.5 && 
						avg_byte_time <= DRQ_TIMING_BYTE * 2.0,
						"DRQ byte timing reasonable");
	}
}

void test_sector_timing(TestFramework& test) {
	TEST_SECTION("Sector-to-Sector Timing");
	
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
	
	// Read multiple sectors
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0x90); // Read multiple sectors
	
	uint32_t sector_times[4] = {0};
	int sector_count = 0;
	uint32_t byte_count = 0;
	
	for(int i = 0; i < 500000 && sector_count < 4; i++) {
		uint32_t status = fdc.read_io8(0);
		
		if(status & 0x02) { // DRQ
			if(byte_count == 0) {
				sector_times[sector_count] = event.get_current_clock();
			}
			
			fdc.read_io8(3);
			byte_count++;
			
			if(byte_count >= 256) {
				byte_count = 0;
				sector_count++;
			}
		}
		
		if((status & 0x01) == 0) break; // Not busy
		
		event.advance_clock(1);
	}
	
	test.assert_true(sector_count >= 2, "Multiple sectors read");
	
	// Check inter-sector gap timing
	if(sector_count >= 2) {
		for(int i = 1; i < sector_count; i++) {
			uint32_t gap_time = sector_times[i] - sector_times[i-1];
			// Gap should be reasonable (not instantaneous, not too long)
			test.assert_true(gap_time > 256 * DRQ_TIMING_BYTE && 
							gap_time < INDEX_PULSE_TIME,
							"Inter-sector gap timing reasonable");
		}
	}
}

void test_index_timing(TestFramework& test) {
	TEST_SECTION("Index Pulse Timing");
	
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
	
	// Issue Type I command to see index pulses
	fdc.write_io8(0, 0x00); // Restore command
	
	// Wait and count index pulses
	uint32_t index_count = 0;
	uint32_t first_index_time = 0;
	uint32_t last_index_time = 0;
	bool last_index_state = false;
	
	for(int i = 0; i < 1000000; i++) { // 1 second
		uint32_t status = fdc.read_io8(0);
		bool index_state = (status & 0x02) != 0; // IP bit in Type I
		
		if(index_state && !last_index_state) { // Rising edge
			if(index_count == 0) {
				first_index_time = event.get_current_clock();
			}
			last_index_time = event.get_current_clock();
			index_count++;
		}
		
		last_index_state = index_state;
		
		if(index_count >= 5) break; // Got enough samples
		
		event.advance_clock(100);
	}
	
	if(index_count >= 2) {
		uint32_t rotation_time = (last_index_time - first_index_time) / (index_count - 1);
		// 300 RPM = 200ms per rotation, allow tolerance
		test.assert_true(rotation_time >= INDEX_PULSE_TIME * 0.8 &&
						rotation_time <= INDEX_PULSE_TIME * 1.2,
						"Index pulse timing indicates ~300 RPM");
	}
}

void test_head_load_timing(TestFramework& test) {
	TEST_SECTION("Head Load Timing");
	
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
	
	// Seek with head load bit set
	fdc.write_io8(3, 5);    // Target track
	fdc.write_io8(0, 0x18); // Seek with H bit set
	
	uint32_t start_time = event.get_current_clock();
	
	// Wait for head load flag
	bool head_loaded = false;
	uint32_t head_load_time = 0;
	
	for(int i = 0; i < 100000; i++) {
		uint32_t status = fdc.read_io8(0);
		if((status & 0x20) != 0 && !head_loaded) { // HLD bit
			head_loaded = true;
			head_load_time = event.get_current_clock() - start_time;
		}
		
		if((status & 0x01) == 0) break; // Not busy
		
		event.advance_clock(100);
	}
	
	test.assert_true(head_loaded, "Head load flag set");
	
	// Head should load after seek but with some delay
	if(head_loaded) {
		test.assert_true(head_load_time >= STEP_TIME_6MS * 5, // After seek
						"Head loads after seek completion");
	}
}

void test_write_timing(TestFramework& test) {
	TEST_SECTION("Write Sector Timing");
	
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
	mock_disk.set_write_protect(false);
	
	// Write sector
	fdc.write_io8(1, 0);    // Track register = 0
	fdc.write_io8(2, 1);    // Sector register = 1
	fdc.write_io8(0, 0xA0); // Write sector command
	
	// Measure write timing
	uint32_t write_start = event.get_current_clock();
	uint32_t drq_count = 0;
	
	for(int i = 0; i < 100000; i++) {
		uint32_t status = fdc.read_io8(0);
		
		if(status & 0x02) { // DRQ
			fdc.write_io8(3, 0xE5); // Write data
			drq_count++;
			
			if(drq_count >= 256) break;
		}
		
		if((status & 0x01) == 0) break; // Not busy
		
		event.advance_clock(1);
	}
	
	uint32_t write_time = event.get_current_clock() - write_start;
	
	test.assert_equal(256, drq_count, "256 bytes written");
	
	// Write should take similar time to read
	uint32_t expected_time = 256 * DRQ_TIMING_BYTE;
	test.assert_true(write_time >= expected_time * 0.5 &&
					write_time <= expected_time * 3.0,
					"Write timing reasonable");
}

void test_command_abort_timing(TestFramework& test) {
	TEST_SECTION("Command Abort Timing");
	
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
	
	// Start long operation (seek to far track)
	fdc.write_io8(3, 79);   // Target track
	fdc.write_io8(0, 0x13); // Seek with slow step rate
	
	// Let it run briefly
	event.advance_clock(10000);
	
	// Abort with force interrupt
	uint32_t abort_time = event.get_current_clock();
	fdc.write_io8(0, 0xD8); // Force interrupt
	
	// Measure how quickly it aborts
	uint32_t abort_complete_time = 0;
	for(int i = 0; i < 10000; i++) {
		if((fdc.read_io8(0) & 0x01) == 0) { // Not busy
			abort_complete_time = event.get_current_clock();
			break;
		}
		event.advance_clock(1);
	}
	
	uint32_t abort_duration = abort_complete_time - abort_time;
	
	// Abort should be nearly instantaneous
	test.assert_true(abort_duration < 1000, "Force interrupt aborts quickly");
}

// Main test runner for timing verification
bool run_timing_tests() {
	TestFramework test;
	
	TEST_SUITE("MB8877 Timing Verification Tests");
	
	test_seek_timing(test);
	test_drq_timing(test);
	test_sector_timing(test);
	test_index_timing(test);
	test_head_load_timing(test);
	test_write_timing(test);
	test_command_abort_timing(test);
	
	test.print_summary();
	test.save_results("test/results/timing_test_results.txt");
	
	return test.all_tests_passed();
}

#ifdef STANDALONE_TEST
int main() {
	return run_timing_tests() ? 0 : 1;
}
#endif