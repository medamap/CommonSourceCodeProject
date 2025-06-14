/*
	Phase 32: Measure Type II Success Rate
	
	This tool measures the actual success rate of Type II commands
	using the Phase 31 MB8877 implementation with READ fixes.
	
	Author : Claude AI Assistant
	Date   : 2025.01.13
*/

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>

// Forward declarations to avoid conflicts
#define _MB8877_H_
#define _MB8877_COMPAT_H_
#define _DEVICE_H_
#define _EMU_H_
#define _EVENT_H_
#define _DISK_H_

// Define needed constants
#define MAX_DRIVE 4
#define FRAMES_PER_SEC 60
#define LINES_PER_FRAME 262
#define CPU_CLOCKS 4000000
#define STATE_VERSION 1
#define SIG_MB8877_MOTOR 3
#define MEDIA_TYPE_2HD 2
#define DRIVE_TYPE_2HD 2

// Include test framework
#include "test_framework.h"

// Now include our environment that has its own DEVICE/DISK definitions
#include "mock_environment.h"

// Include the Phase 31 MB8877 implementation through compat layer
#include "../../../src/vm/mb8877_compat.h"

// Test result tracking
struct CommandResult {
	int track;
	int sector;
	bool success;
	uint32_t final_status;
	const char* error;
};

// Run Type II command tests and measure success rate
void measure_type2_success_rate() {
	printf("\n=== Measuring Type II Command Success Rate ===\n");
	printf("Using Phase 31 MB8877 implementation with READ fixes\n\n");
	
	std::vector<CommandResult> read_results;
	std::vector<CommandResult> write_results;
	
	// Test configuration
	const int test_tracks[] = {0, 1, 10, 40, 79};
	const int test_sectors[] = {1, 2, 8, 15, 16};
	const int num_tracks = sizeof(test_tracks) / sizeof(test_tracks[0]);
	const int num_sectors = sizeof(test_sectors) / sizeof(test_sectors[0]);
	
	// Create test environment
	MockEMU emu;
	MockVM vm(&emu);
	MockEVENT event(&vm, &emu);
	
	// Create MB8877 (Phase 31 version)
	MB8877 fdc(&vm, &emu);
	fdc.set_context_event_manager(&event, 0, 0, 0);
	fdc.initialize();
	fdc.reset();
	
	// Enable motor
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	
	// Test READ SECTOR commands
	printf("Testing READ SECTOR commands...\n");
	for (int t = 0; t < num_tracks; t++) {
		for (int s = 0; s < num_sectors; s++) {
			int track = test_tracks[t];
			int sector = test_sectors[s];
			
			// Position to track/sector
			fdc.write_io8(1, track);   // Track register
			fdc.write_io8(2, sector);  // Sector register
			
			// Issue READ SECTOR command
			fdc.write_io8(0, 0x80);
			
			// Process command
			bool drq_seen = false;
			bool completed = false;
			uint32_t final_status = 0;
			
			for (int i = 0; i < 100; i++) {
				event.advance_clock(1000);
				uint32_t status = fdc.read_io8(0);
				
				if (status & 0x02) {  // DRQ set
					drq_seen = true;
					// Read data
					for (int j = 0; j < 256; j++) {
						fdc.read_io8(3);
					}
				}
				
				if (!(status & 0x01)) {  // BUSY cleared
					completed = true;
					final_status = status;
					break;
				}
			}
			
			// Record result
			CommandResult result;
			result.track = track;
			result.sector = sector;
			result.final_status = final_status;
			
			if (!completed) {
				result.success = false;
				result.error = "Timeout";
			} else if (final_status & 0x10) {  // RNF
				result.success = false;
				result.error = "Record Not Found";
			} else if (final_status & 0x08) {  // CRC
				result.success = false;
				result.error = "CRC Error";
			} else if (!drq_seen) {
				result.success = false;
				result.error = "No DRQ";
			} else {
				result.success = true;
				result.error = nullptr;
			}
			
			read_results.push_back(result);
			
			if (result.success) {
				printf("  Track %d, Sector %d: SUCCESS\n", track, sector);
			} else {
				printf("  Track %d, Sector %d: FAIL (%s, status=0x%02X)\n", 
					track, sector, result.error, final_status);
			}
		}
	}
	
	// Test WRITE SECTOR commands
	printf("\nTesting WRITE SECTOR commands...\n");
	for (int t = 0; t < num_tracks; t++) {
		for (int s = 0; s < num_sectors; s++) {
			int track = test_tracks[t];
			int sector = test_sectors[s];
			
			// Position to track/sector
			fdc.write_io8(1, track);   // Track register
			fdc.write_io8(2, sector);  // Sector register
			
			// Issue WRITE SECTOR command
			fdc.write_io8(0, 0xA0);
			
			// Process command
			bool drq_seen = false;
			bool completed = false;
			uint32_t final_status = 0;
			
			for (int i = 0; i < 100; i++) {
				event.advance_clock(1000);
				uint32_t status = fdc.read_io8(0);
				
				if (status & 0x02) {  // DRQ set
					drq_seen = true;
					// Write data
					for (int j = 0; j < 256; j++) {
						fdc.write_io8(3, 0xAA);
					}
				}
				
				if (!(status & 0x01)) {  // BUSY cleared
					completed = true;
					final_status = status;
					break;
				}
			}
			
			// Record result
			CommandResult result;
			result.track = track;
			result.sector = sector;
			result.final_status = final_status;
			
			if (!completed) {
				result.success = false;
				result.error = "Timeout";
			} else if (final_status & 0x40) {  // Write Protect
				result.success = false;
				result.error = "Write Protected";
			} else if (final_status & 0x04) {  // Lost Data
				result.success = false;
				result.error = "Lost Data";
			} else if (!drq_seen) {
				result.success = false;
				result.error = "No DRQ";
			} else {
				result.success = true;
				result.error = nullptr;
			}
			
			write_results.push_back(result);
			
			if (result.success) {
				printf("  Track %d, Sector %d: SUCCESS\n", track, sector);
			} else {
				printf("  Track %d, Sector %d: FAIL (%s, status=0x%02X)\n", 
					track, sector, result.error, final_status);
			}
		}
	}
	
	// Calculate success rates
	int read_success = 0;
	int write_success = 0;
	
	for (const auto& r : read_results) {
		if (r.success) read_success++;
	}
	
	for (const auto& r : write_results) {
		if (r.success) write_success++;
	}
	
	double read_rate = read_results.size() > 0 ? (read_success * 100.0 / read_results.size()) : 0.0;
	double write_rate = write_results.size() > 0 ? (write_success * 100.0 / write_results.size()) : 0.0;
	double overall_rate = (read_results.size() + write_results.size()) > 0 ? 
		((read_success + write_success) * 100.0 / (read_results.size() + write_results.size())) : 0.0;
	
	// Print summary
	printf("\n=== Type II Command Success Rate Summary ===\n");
	printf("READ SECTOR:  %d/%zu = %.1f%%\n", read_success, read_results.size(), read_rate);
	printf("WRITE SECTOR: %d/%zu = %.1f%%\n", write_success, write_results.size(), write_rate);
	printf("OVERALL:      %d/%zu = %.1f%%\n", 
		read_success + write_success, read_results.size() + write_results.size(), overall_rate);
	
	// Save detailed results
	FILE* fp = fopen("phase32_measurement.json", "w");
	if (fp) {
		fprintf(fp, "{\n");
		fprintf(fp, "  \"phase\": 32,\n");
		fprintf(fp, "  \"measurement_type\": \"type2_success_rate\",\n");
		fprintf(fp, "  \"mb8877_version\": \"Phase 31 with READ fixes\",\n");
		fprintf(fp, "  \"summary\": {\n");
		fprintf(fp, "    \"read_success_rate\": %.1f,\n", read_rate);
		fprintf(fp, "    \"write_success_rate\": %.1f,\n", write_rate);
		fprintf(fp, "    \"overall_success_rate\": %.1f,\n", overall_rate);
		fprintf(fp, "    \"total_tests\": %zu,\n", read_results.size() + write_results.size());
		fprintf(fp, "    \"passed\": %d,\n", read_success + write_success);
		fprintf(fp, "    \"failed\": %zu\n", read_results.size() + write_results.size() - read_success - write_success);
		fprintf(fp, "  },\n");
		fprintf(fp, "  \"phase31_validation\": {\n");
		fprintf(fp, "    \"expected_improvement\": \"80%% success rate\",\n");
		fprintf(fp, "    \"actual_rate\": \"%.1f%%\",\n", overall_rate);
		fprintf(fp, "    \"meets_expectation\": %s\n", overall_rate >= 80.0 ? "true" : "false");
		fprintf(fp, "  }\n");
		fprintf(fp, "}\n");
		fclose(fp);
	}
	
	// Comparison with Phase 31 estimates
	printf("\n=== Phase 31 Validation ===\n");
	printf("Phase 31 estimated: 80%% success rate after READ fixes\n");
	printf("Phase 32 measured:  %.1f%% actual success rate\n", overall_rate);
	if (overall_rate >= 80.0) {
		printf("✓ Phase 31 fixes are validated as effective!\n");
	} else {
		printf("✗ Success rate below Phase 31 estimate\n");
	}
}

int main() {
	printf("Phase 32: Type II Command Success Rate Measurement\n");
	printf("=================================================\n");
	
	// Run measurement
	measure_type2_success_rate();
	
	printf("\nMeasurement complete. Results saved to phase32_measurement.json\n");
	
	return 0;
}