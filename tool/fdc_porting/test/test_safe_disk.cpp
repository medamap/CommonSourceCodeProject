/*
	Test program for SafeDISK implementation
	
	Author : Claude AI Assistant
	Date   : 2025.01.14
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "safe_disk.h"

// Simple test framework
class TestFramework {
public:
	int tests_run;
	int tests_passed;
	int tests_failed;
	
	TestFramework() : tests_run(0), tests_passed(0), tests_failed(0) {}
	
	void assert_true(bool condition, const char* message) {
		tests_run++;
		if(condition) {
			tests_passed++;
			printf("  ✓ %s\n", message);
		} else {
			tests_failed++;
			printf("  ✗ %s - FAILED\n", message);
		}
	}
	
	void assert_false(bool condition, const char* message) {
		assert_true(!condition, message);
	}
	
	void assert_equal(int expected, int actual, const char* message) {
		tests_run++;
		if(expected == actual) {
			tests_passed++;
			printf("  ✓ %s\n", message);
		} else {
			tests_failed++;
			printf("  ✗ %s - Expected %d, got %d\n", message, expected, actual);
		}
	}
	
	void assert_not_null(void* ptr, const char* message) {
		assert_true(ptr != nullptr, message);
	}
	
	void summary() {
		printf("\nTest Summary:\n");
		printf("  Total tests: %d\n", tests_run);
		printf("  Passed: %d\n", tests_passed);
		printf("  Failed: %d\n", tests_failed);
		printf("  Success rate: %.1f%%\n", 
			tests_run > 0 ? (tests_passed * 100.0 / tests_run) : 0.0);
	}
};

#define TEST_SECTION(name) printf("\n=== %s ===\n", name)

// Test SafeDISK basic operations
void test_safe_disk_basic(TestFramework& test) {
	TEST_SECTION("SafeDISK Basic Operations");
	
	SafeDISK disk;
	
	// Initial state
	test.assert_false(disk.inserted, "Not inserted initially");
	test.assert_false(disk.write_protected, "Not write protected initially");
	
	// Open dummy disk
	disk.open(nullptr, 0);
	test.assert_true(disk.inserted, "Inserted after open");
	test.assert_false(disk.ejected, "Not ejected after open");
	
	// Get sector
	bool got = disk.get_sector(0, 0, 1);
	test.assert_true(got, "Get sector successful");
	test.assert_not_null(disk.sector, "Sector buffer allocated");
	test.assert_equal(256, disk.sector_size.sd, "Sector size is 256");
	
	// Verify sector ID
	test.assert_equal(0, disk.id[0], "Track ID = 0");
	test.assert_equal(0, disk.id[1], "Side ID = 0");
	test.assert_equal(1, disk.id[2], "Sector ID = 1");
	test.assert_equal(1, disk.id[3], "Size code = 1 (256 bytes)");
	
	// Close disk
	disk.close();
	test.assert_false(disk.inserted, "Not inserted after close");
	test.assert_true(disk.ejected, "Ejected after close");
}

// Test SafeDISK error handling
void test_safe_disk_error_handling(TestFramework& test) {
	TEST_SECTION("SafeDISK Error Handling");
	
	SafeDISK disk;
	
	// Try to get sector without opening
	bool got = disk.get_sector(0, 0, 1);
	test.assert_false(got, "Cannot get sector when not inserted");
	
	// Open disk
	disk.open(nullptr, 0);
	
	// Invalid track
	got = disk.get_sector(99, 0, 1);
	test.assert_false(got, "Invalid track rejected");
	
	// Invalid side
	got = disk.get_sector(0, 3, 1);
	test.assert_false(got, "Invalid side rejected");
	
	// Invalid sector
	got = disk.get_sector(0, 0, 99);
	test.assert_false(got, "Invalid sector rejected");
	
	// Negative values
	got = disk.get_sector(-1, 0, 1);
	test.assert_false(got, "Negative track rejected");
	
	got = disk.get_sector(0, -1, 1);
	test.assert_false(got, "Negative side rejected");
	
	got = disk.get_sector(0, 0, 0);
	test.assert_false(got, "Sector 0 rejected (1-based)");
}

// Test SafeDISK multiple sectors
void test_safe_disk_multiple_sectors(TestFramework& test) {
	TEST_SECTION("SafeDISK Multiple Sectors");
	
	SafeDISK disk;
	disk.open(nullptr, 0);
	
	// Read multiple sectors
	for(int sector = 1; sector <= 16; sector++) {
		bool got = disk.get_sector(0, 0, sector);
		test.assert_true(got, "Get sector successful");
		test.assert_equal(sector, disk.id[2], "Correct sector ID");
		
		// Verify test pattern
		if(disk.sector) {
			uint8_t expected = (0 * 16 + sector) & 0xFF;
			test.assert_equal(expected, disk.sector[0], "First byte correct");
		}
	}
}

// Test SafeDISK track operations
void test_safe_disk_track_operations(TestFramework& test) {
	TEST_SECTION("SafeDISK Track Operations");
	
	SafeDISK disk;
	disk.open(nullptr, 0);
	
	// Get track
	bool got = disk.get_track(0, 0);
	test.assert_true(got, "Get track successful");
	test.assert_not_null(disk.track, "Track buffer allocated");
	test.assert_equal(4096, disk.track_size, "Track size = 16 * 256");
	
	// Invalid track
	got = disk.get_track(99, 0);
	test.assert_false(got, "Invalid track rejected");
}

// Test SafeDISK D88 support (with dummy file)
void test_safe_disk_d88_support(TestFramework& test) {
	TEST_SECTION("SafeDISK D88 Support");
	
	SafeDISK disk;
	
	// Try to open non-existent D88 file - should fall back to dummy
	disk.open(_T("nonexistent.d88"), 0);
	test.assert_true(disk.inserted, "Disk inserted even with missing file");
	
	// Should still work as dummy disk
	bool got = disk.get_sector(0, 0, 1);
	test.assert_true(got, "Can get sector from dummy disk");
}

// Test SafeDISK crash prevention
void test_safe_disk_crash_prevention(TestFramework& test) {
	TEST_SECTION("SafeDISK Crash Prevention");
	
	// Multiple open/close cycles
	SafeDISK disk;
	for(int i = 0; i < 10; i++) {
		disk.open(nullptr, 0);
		test.assert_true(disk.inserted, "Disk inserted");
		disk.close();
		test.assert_false(disk.inserted, "Disk not inserted");
	}
	
	// Open with various parameters
	disk.open(_T("test.d88"), 0);
	test.assert_true(disk.inserted, "Opened with file path");
	
	disk.close();
	disk.open(nullptr, 1);
	test.assert_true(disk.inserted, "Opened with bank 1");
	
	// Double close (should not crash)
	disk.close();
	disk.close();
	test.assert_false(disk.inserted, "Still not inserted after double close");
}

// Main test runner
int main(int /*argc*/, char* /*argv*/[]) {
	printf("SafeDISK Test Suite\n");
	printf("==================\n");
	
	TestFramework test;
	
	// Run all tests
	test_safe_disk_basic(test);
	test_safe_disk_error_handling(test);
	test_safe_disk_multiple_sectors(test);
	test_safe_disk_track_operations(test);
	test_safe_disk_d88_support(test);
	test_safe_disk_crash_prevention(test);
	
	// Print summary
	test.summary();
	
	// Return status
	return test.tests_failed > 0 ? 1 : 0;
}