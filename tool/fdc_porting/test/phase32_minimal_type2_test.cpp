/*
	Phase 32: Minimal Type II Test with Safe Infrastructure
	
	This test creates a minimal environment to test Type II commands
	without crashes, using a custom approach to avoid header conflicts.
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>

// Define all needed constants first
#define MAX_DRIVE 4
#define FRAMES_PER_SEC 60
#define LINES_PER_FRAME 262
#define CPU_CLOCKS 4000000
#define STATE_VERSION 1

// MB8877 signal definitions
#define SIG_MB8877_DRVREG	0
#define SIG_MB8877_DRIVEREG	0
#define SIG_MB8877_SIDEREG	1
#define SIG_MB8877_MOTOR	2
#define SIG_MB8877_HEADLOAD	3

// Media types
#define MEDIA_TYPE_2HD 2
#define DRIVE_TYPE_2HD 2

// Now include test framework
#include "test_framework.h"

// Forward declare to avoid conflicts
class DEVICE;
class DISK;
class EVENT;
class FILEIO;
class MB8877;

// Include our mock environment (which has its own DEVICE definition)
#include "mock_environment.h"

// Now include MB8877 compat layer
#include "../../../src/vm/mb8877_compat.h"

// Test results tracking
struct TestResult {
	const char* test_name;
	bool success;
	const char* error_msg;
};

std::vector<TestResult> results;

void record_result(const char* test_name, bool success, const char* error_msg = nullptr) {
	TestResult result;
	result.test_name = test_name;
	result.success = success;
	result.error_msg = error_msg;
	results.push_back(result);
	
	printf("[%s] %s", success ? "PASS" : "FAIL", test_name);
	if (!success && error_msg) {
		printf(" - %s", error_msg);
	}
	printf("\n");
}

// Type II command test functions
void test_read_sector_success() {
	printf("\n=== Test: Read Sector Success ===\n");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	// Create MB8877 and MockDISK separately
	MB8877 fdc(&vm, &emu);
	MockDISK* mock_disk = new MockDISK(&vm, &emu);
	
	// Initialize FDC
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Open disk on MockDISK (not through FDC)
	mock_disk->open(_T("test.dsk"), 0);
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Set up for sector read
	fdc.write_io8(1, 0);  // Track 0
	fdc.write_io8(2, 1);  // Sector 1
	
	// Issue READ SECTOR command
	fdc.write_io8(0, 0x80);
	
	// Check initial status
	uint32_t status = fdc.read_io8(0);
	bool busy_set = (status & 0x01) != 0;
	
	// Simulate some event processing
	for (int i = 0; i < 50; i++) {
		event.advance_clock(1000);
		status = fdc.read_io8(0);
		
		// Check for DRQ
		if (status & 0x02) {
			// Read data
			uint8_t data[256];
			for (int j = 0; j < 256; j++) {
				data[j] = fdc.read_io8(3);
			}
			
			// Check final status
			status = fdc.read_io8(0);
			bool success = !(status & 0x01) && !(status & 0x10);  // Not busy, no RNF
			record_result("Read Sector Success", success);
			
			delete mock_disk;
			return;
		}
		
		// Check if command completed
		if (!(status & 0x01)) {
			break;
		}
	}
	
	// If we get here, something went wrong
	record_result("Read Sector Success", false, "DRQ never set or timeout");
	delete mock_disk;
}

void test_write_sector_success() {
	printf("\n=== Test: Write Sector Success ===\n");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	MockDISK* mock_disk = new MockDISK(&vm, &emu);
	
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk->open(_T("test.dsk"), 0);
	mock_disk->set_write_protect(false);
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Set up for sector write
	fdc.write_io8(1, 0);  // Track 0
	fdc.write_io8(2, 1);  // Sector 1
	
	// Issue WRITE SECTOR command
	fdc.write_io8(0, 0xA0);
	
	// Wait for DRQ
	bool drq_found = false;
	for (int i = 0; i < 50; i++) {
		event.advance_clock(1000);
		uint32_t status = fdc.read_io8(0);
		
		if (status & 0x02) {  // DRQ set
			drq_found = true;
			
			// Write data
			for (int j = 0; j < 256; j++) {
				fdc.write_io8(3, 0xAA);
			}
			
			// Wait for completion
			for (int k = 0; k < 50; k++) {
				event.advance_clock(1000);
				status = fdc.read_io8(0);
				if (!(status & 0x01)) {  // Not busy
					bool success = !(status & 0x04) && !(status & 0x40);  // No lost data, not write protected
					record_result("Write Sector Success", success);
					delete mock_disk;
					return;
				}
			}
			break;
		}
	}
	
	record_result("Write Sector Success", false, drq_found ? "Command timeout" : "DRQ never set");
	delete mock_disk;
}

void test_write_protect() {
	printf("\n=== Test: Write Protect Detection ===\n");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	MockDISK* mock_disk = new MockDISK(&vm, &emu);
	
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk->open(_T("test.dsk"), 0);
	mock_disk->set_write_protect(true);  // Set write protect
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Try to write
	fdc.write_io8(1, 0);  // Track 0
	fdc.write_io8(2, 1);  // Sector 1
	fdc.write_io8(0, 0xA0);  // Write sector
	
	// Wait a bit
	event.advance_clock(5000);
	
	uint32_t status = fdc.read_io8(0);
	bool wp_detected = (status & 0x40) != 0;
	bool not_busy = (status & 0x01) == 0;
	
	record_result("Write Protect Detection", wp_detected && not_busy);
	delete mock_disk;
}

void test_sector_not_found() {
	printf("\n=== Test: Sector Not Found ===\n");
	
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	MB8877 fdc(&vm, &emu);
	MockDISK* mock_disk = new MockDISK(&vm, &emu);
	
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	mock_disk->open(_T("test.dsk"), 0);
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Try to read non-existent sector
	fdc.write_io8(1, 0);   // Track 0
	fdc.write_io8(2, 99);  // Invalid sector
	fdc.write_io8(0, 0x80);  // Read sector
	
	// Wait for timeout
	for (int i = 0; i < 200; i++) {
		event.advance_clock(1000);
		uint32_t status = fdc.read_io8(0);
		if (!(status & 0x01)) {  // Not busy
			bool rnf_set = (status & 0x10) != 0;
			record_result("Sector Not Found", rnf_set);
			delete mock_disk;
			return;
		}
	}
	
	record_result("Sector Not Found", false, "Command did not complete");
	delete mock_disk;
}

// Calculate success rate
void print_summary() {
	printf("\n=== Test Summary ===\n");
	
	int total = 0;
	int passed = 0;
	
	for (const auto& result : results) {
		total++;
		if (result.success) passed++;
	}
	
	printf("Total tests: %d\n", total);
	printf("Passed: %d\n", passed);
	printf("Failed: %d\n", total - passed);
	printf("Success rate: %.1f%%\n", total > 0 ? (passed * 100.0 / total) : 0.0);
	
	// Save results to JSON
	FILE* fp = fopen("phase32_results.json", "w");
	if (fp) {
		fprintf(fp, "{\n");
		fprintf(fp, "  \"phase\": 32,\n");
		fprintf(fp, "  \"total_tests\": %d,\n", total);
		fprintf(fp, "  \"passed\": %d,\n", passed);
		fprintf(fp, "  \"failed\": %d,\n", total - passed);
		fprintf(fp, "  \"success_rate\": %.1f,\n", total > 0 ? (passed * 100.0 / total) : 0.0);
		fprintf(fp, "  \"tests\": [\n");
		
		for (size_t i = 0; i < results.size(); i++) {
			const auto& result = results[i];
			fprintf(fp, "    {\n");
			fprintf(fp, "      \"name\": \"%s\",\n", result.test_name);
			fprintf(fp, "      \"success\": %s", result.success ? "true" : "false");
			if (result.error_msg) {
				fprintf(fp, ",\n      \"error\": \"%s\"", result.error_msg);
			}
			fprintf(fp, "\n    }");
			if (i < results.size() - 1) fprintf(fp, ",");
			fprintf(fp, "\n");
		}
		
		fprintf(fp, "  ]\n");
		fprintf(fp, "}\n");
		fclose(fp);
	}
}

int main() {
	printf("Phase 32: Minimal Type II Test\n");
	printf("==============================\n");
	printf("Testing Type II commands with safe infrastructure\n");
	
	// Run tests
	test_read_sector_success();
	test_write_sector_success();
	test_write_protect();
	test_sector_not_found();
	
	// Print summary
	print_summary();
	
	return 0;
}