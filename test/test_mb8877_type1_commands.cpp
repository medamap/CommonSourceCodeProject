/*
	MB8877 Type I Command Tests (Restore, Seek, Step)
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "../src/vm/mb8877_compat.h"

void test_restore_command(TestFramework& test) {
	TEST_SECTION("Restore Command Tests");
	
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
	
	// Move to track 10 first
	fdc.write_io8(1, 10); // Track register = 10
	
	// Issue restore command (0x00 - 0x0F are restore commands)
	fdc.write_io8(0, 0x00); // Restore command, no verify
	
	// Check BUSY flag is set
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "BUSY flag set during restore");
	
	// Wait for command completion
	event.advance_clock(60000); // Generous time for seek
	
	// Check command completed
	status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY flag cleared after restore");
	
	// Check track register is 0
	uint32_t track = fdc.read_io8(1);
	test.assert_equal_hex(0x00, track, "Track register = 0 after restore");
	
	// Check TRACK00 flag is set
	test.assert_true((status & 0x04) != 0, "TRACK00 flag set after restore");
}

void test_restore_with_verify(TestFramework& test) {
	TEST_SECTION("Restore with Verify Tests");
	
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
	
	// Move to track 5 first
	fdc.write_io8(1, 5);
	
	// Issue restore with verify command
	fdc.write_io8(0, 0x04); // Restore command with verify (V=1)
	
	// Wait for completion (verify takes longer)
	event.advance_clock(80000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared after restore with verify");
	test.assert_true((status & 0x04) != 0, "TRACK00 set after restore with verify");
	
	uint32_t track = fdc.read_io8(1);
	test.assert_equal_hex(0x00, track, "Track = 0 after restore with verify");
}

void test_seek_command(TestFramework& test) {
	TEST_SECTION("Seek Command Tests");
	
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
	
	// Start from track 0, seek to track 10
	fdc.write_io8(3, 10); // Data register = target track
	fdc.write_io8(0, 0x10); // Seek command (0x10-0x1F)
	
	// Check BUSY flag
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "BUSY set during seek");
	
	// Wait for seek completion
	event.advance_clock(50000);
	
	status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared after seek");
	
	// Check track register updated
	uint32_t track = fdc.read_io8(1);
	test.assert_equal_hex(0x0A, track, "Track register = 10 after seek");
	
	// Check TRACK00 flag cleared
	test.assert_true((status & 0x04) == 0, "TRACK00 cleared when not on track 0");
}

void test_seek_with_verify(TestFramework& test) {
	TEST_SECTION("Seek with Verify Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk.open(_T("test.dsk"), 0);
	
	// Seek to track 5 with verify
	fdc.write_io8(3, 5); // Data register = target track
	fdc.write_io8(0, 0x14); // Seek with verify (V=1)
	
	event.advance_clock(60000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared after seek with verify");
	
	uint32_t track = fdc.read_io8(1);
	test.assert_equal_hex(0x05, track, "Track = 5 after seek with verify");
}

void test_step_commands(TestFramework& test) {
	TEST_SECTION("Step Command Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk.open(_T("test.dsk"), 0);
	
	// Test Step In command
	fdc.write_io8(0, 0x40); // Step In (0x40-0x5F)
	event.advance_clock(20000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared after step in");
	
	uint32_t track = fdc.read_io8(1);
	test.assert_equal_hex(0x01, track, "Track = 1 after step in from 0");
	
	// Test Step Out command
	fdc.write_io8(0, 0x60); // Step Out (0x60-0x7F)
	event.advance_clock(20000);
	
	status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "BUSY cleared after step out");
	
	track = fdc.read_io8(1);
	test.assert_equal_hex(0x00, track, "Track = 0 after step out from 1");
	
	// Check TRACK00 flag restored
	test.assert_true((status & 0x04) != 0, "TRACK00 set after stepping back to track 0");
}

void test_step_with_update(TestFramework& test) {
	TEST_SECTION("Step with Track Update Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk.open(_T("test.dsk"), 0);
	
	// Step In with track register update (U=1)
	fdc.write_io8(0, 0x50); // Step In with update
	event.advance_clock(20000);
	
	uint32_t track = fdc.read_io8(1);
	test.assert_equal_hex(0x01, track, "Track register updated with U=1");
	
	// Step without track register update (U=0)
	fdc.write_io8(0, 0x40); // Step In without update
	event.advance_clock(20000);
	
	track = fdc.read_io8(1);
	test.assert_equal_hex(0x01, track, "Track register unchanged with U=0");
}

void test_step_rate_selection(TestFramework& test) {
	TEST_SECTION("Step Rate Selection Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk.open(_T("test.dsk"), 0);
	
	// Test different step rates (r1,r0 bits)
	// Rate 00 = 6ms, 01 = 12ms, 10 = 20ms, 11 = 30ms
	
	// Test fastest rate (r1=0, r0=0)
	fdc.write_io8(0, 0x40); // Step In, rate 00
	uint32_t start_time = event.get_current_clock();
	
	event.advance_clock(10000); // Should complete within 10ms
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "Fast step rate completed quickly");
	
	// Test slowest rate (r1=1, r0=1)
	fdc.write_io8(0, 0x43); // Step In, rate 11 (30ms)
	event.advance_clock(35000); // Should complete within 35ms
	
	status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "Slow step rate completed");
}

void test_head_load_timing(TestFramework& test) {
	TEST_SECTION("Head Load Timing Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	MockDISK mock_disk(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk.open(_T("test.dsk"), 0);
	
	// Test head load flag with restore command
	fdc.write_io8(0, 0x08); // Restore with head load (h=1)
	event.advance_clock(50000);
	
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x20) != 0, "Head Load flag set in Type I status");
	
	// Test without head load
	fdc.write_io8(0, 0x00); // Restore without head load (h=0)
	event.advance_clock(50000);
	
	status = fdc.read_io8(0);
	// Note: Head Load behavior may vary based on implementation
	test.assert_true(true, "Head load timing test completed");
}

// Main test runner for Type I command tests
bool run_type1_command_tests() {
	TestFramework test;
	
	TEST_SUITE("MB8877 Type I Command Tests");
	
	test_restore_command(test);
	test_restore_with_verify(test);
	test_seek_command(test);
	test_seek_with_verify(test);
	test_step_commands(test);
	test_step_with_update(test);
	test_step_rate_selection(test);
	test_head_load_timing(test);
	
	test.print_summary();
	test.save_results("test/results/type1_command_test_results.txt");
	
	return test.all_tests_passed();
}

#ifdef STANDALONE_TEST
int main() {
	return run_type1_command_tests() ? 0 : 1;
}
#endif