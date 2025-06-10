/*
	MB8877 Register Access Tests
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "../src/vm/mb8877_compat.h"

// Test register access patterns
void test_register_basic_access(TestFramework& test) {
	TEST_SECTION("Basic Register Access");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	
	// Create MB8877 instance
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Test initial status register
	uint32_t status = fdc.read_io8(0); // Status register
	test.assert_equal_hex(0x00, status & 0x01, "Initial BUSY bit should be 0");
	
	// Test track register write/read
	fdc.write_io8(1, 0x55); // Track register
	uint32_t track = fdc.read_io8(1);
	test.assert_equal_hex(0x55, track, "Track register write/read");
	
	// Test sector register write/read
	fdc.write_io8(2, 0xAA); // Sector register
	uint32_t sector = fdc.read_io8(2);
	test.assert_equal_hex(0xAA, sector, "Sector register write/read");
	
	// Test data register write/read
	fdc.write_io8(3, 0x33); // Data register
	uint32_t data = fdc.read_io8(3);
	test.assert_equal_hex(0x33, data, "Data register write/read");
}

void test_status_register_bits(TestFramework& test) {
	TEST_SECTION("Status Register Bit Operations");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Test TRACK00 bit when on track 0
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x04) != 0, "TRACK00 bit set on track 0");
	
	// Move to track 1 and test TRACK00 bit clears
	fdc.write_io8(1, 1); // Set track register to 1
	fdc.write_io8(0, 0x18); // Step In command
	// Wait for command completion
	event.advance_clock(10000);
	
	status = fdc.read_io8(0);
	test.assert_true((status & 0x04) == 0, "TRACK00 bit clear on track 1");
}

void test_mb8866_inverted_bus(TestFramework& test) {
	TEST_SECTION("MB8866/MB8876 Inverted Bus Test");
	
	#ifdef HAS_MB8866
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Test inverted data bus behavior
	fdc.write_io8(3, 0xAA); // Write 0xAA to data register
	uint32_t data = fdc.read_io8(3);
	test.assert_equal_hex(0x55, data, "MB8866 inverted bus: 0xAA -> 0x55");
	
	fdc.write_io8(3, 0x55); // Write 0x55 to data register
	data = fdc.read_io8(3);
	test.assert_equal_hex(0xAA, data, "MB8866 inverted bus: 0x55 -> 0xAA");
	#else
	test.assert_true(true, "MB8866 inverted bus test skipped (not MB8866)");
	#endif
}

void test_command_register_types(TestFramework& test) {
	TEST_SECTION("Command Register Type Detection");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Test Type I commands (Restore)
	fdc.write_io8(0, 0x00); // Restore command
	event.advance_clock(1000);
	uint32_t status = fdc.read_io8(0);
	test.assert_true((status & 0x01) != 0, "Type I command sets BUSY");
	
	// Wait for command completion
	event.advance_clock(50000);
	status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "Type I command clears BUSY when done");
	
	// Test Type II commands (Read Sector) - needs disk
	// This would require mock disk setup
	
	// Test Type IV commands (Force Interrupt)
	fdc.write_io8(0, 0x00); // Start restore first
	event.advance_clock(1000);
	fdc.write_io8(0, 0xD0); // Force interrupt
	event.advance_clock(100);
	status = fdc.read_io8(0);
	test.assert_true((status & 0x01) == 0, "Type IV Force Interrupt clears BUSY");
}

void test_drq_irq_signals(TestFramework& test) {
	TEST_SECTION("DRQ and IRQ Signal Tests");
	
	MockVM vm;
	MockEMU emu;
	MockEVENT event(&vm, &emu);
	SignalCapture irq_capture(&vm, &emu);
	SignalCapture drq_capture(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.set_context_irq(&irq_capture, 0, 0xFFFFFFFF);
	fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
	fdc.initialize();
	fdc.reset();
	
	irq_capture.clear_signals();
	drq_capture.clear_signals();
	
	// Issue a command that should generate IRQ
	fdc.write_io8(0, 0x00); // Restore command
	event.advance_clock(60000); // Wait for completion
	
	test.assert_true(irq_capture.has_signal(0, 1), "IRQ signal generated on command completion");
}

// Main test runner for register tests
bool run_register_tests() {
	TestFramework test;
	
	TEST_SUITE("MB8877 Register Access Tests");
	
	test_register_basic_access(test);
	test_status_register_bits(test);
	test_mb8866_inverted_bus(test);
	test_command_register_types(test);
	test_drq_irq_signals(test);
	
	test.print_summary();
	test.save_results("test/results/register_test_results.txt");
	
	return test.all_tests_passed();
}

#ifdef STANDALONE_TEST
int main() {
	return run_register_tests() ? 0 : 1;
}
#endif