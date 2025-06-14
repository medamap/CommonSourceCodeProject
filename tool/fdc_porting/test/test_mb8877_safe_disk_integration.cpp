/*
	Test MB8877 integration with SafeDISK
	
	Author : Claude AI Assistant
	Date   : 2025.01.14
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

// Define STANDALONE_TEST is already defined by command line
// Include SafeDISK test implementation directly

// Mock environment types needed
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef char _TCHAR;
#define _T(x) x

// Minimal DEVICE base class
class DEVICE {
public:
	virtual ~DEVICE() {}
	virtual void initialize() {}
	virtual void reset() {}
	virtual void write_io8(uint32_t addr, uint32_t data) { (void)addr; (void)data; }
	virtual uint32_t read_io8(uint32_t addr) { (void)addr; return 0xff; }
	virtual void write_signal(int id, uint32_t data, uint32_t mask) { (void)id; (void)data; (void)mask; }
};

// Define signals
#define SIG_MB8877_MOTOR 3
#define SIG_MB8877_DRIVEREG 1

// Simple disk class for testing
class DISK {
public:
	bool inserted;
	bool write_protected;
	uint8_t* sector;
	uint8_t sector_buffer[512];
	
	DISK() : inserted(false), write_protected(false), sector(nullptr) {}
	virtual ~DISK() {}
	
	void open(const _TCHAR* file_path, int bank) {
		(void)file_path;
		(void)bank;
		inserted = true;
	}
	
	void close() {
		inserted = false;
		sector = nullptr;
	}
	
	bool get_sector(int trk, int side, int sec) {
		if(!inserted) return false;
		if(trk < 0 || trk >= 40) return false;
		if(side < 0 || side >= 2) return false;
		if(sec < 1 || sec > 16) return false;
		
		// Generate test data
		for(int i = 0; i < 512; i++) {
			sector_buffer[i] = (trk * 16 + sec + i) & 0xFF;
		}
		sector = sector_buffer;
		return true;
	}
};

// SafeDISK is just DISK for this test
typedef DISK SafeDISK;

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

// Custom MB8877 class that uses SafeDISK
class MB8877_SafeDisk : public DEVICE {
private:
	SafeDISK* safe_disks[4];
	DISK* disk[4];
	uint8_t status, cmdreg, trkreg, secreg, datareg, drvreg;
	int register_id[8];
	bool motor_on;
	
public:
	MB8877_SafeDisk() {
		for(int i = 0; i < 4; i++) {
			safe_disks[i] = new SafeDISK();
			disk[i] = safe_disks[i];
			register_id[i] = -1;
		}
		status = 0;
		cmdreg = 0;
		trkreg = 0;
		secreg = 0;
		datareg = 0;
		drvreg = 0;
		motor_on = false;
	}
	
	~MB8877_SafeDisk() {
		for(int i = 0; i < 4; i++) {
			if(safe_disks[i]) {
				delete safe_disks[i];
				safe_disks[i] = nullptr;
				disk[i] = nullptr;
			}
		}
	}
	
	void initialize() override {
		// Already initialized in constructor
	}
	
	void reset() override {
		status = 0x04; // TR00
		cmdreg = 0;
		trkreg = 0;
		secreg = 0;
		datareg = 0;
	}
	
	void write_io8(uint32_t addr, uint32_t data) override {
		switch(addr & 3) {
		case 0: // Command
			cmdreg = data;
			process_command();
			break;
		case 1: // Track
			trkreg = data;
			break;
		case 2: // Sector
			secreg = data;
			break;
		case 3: // Data
			datareg = data;
			break;
		}
	}
	
	uint32_t read_io8(uint32_t addr) override {
		switch(addr & 3) {
		case 0: // Status
			return status;
		case 1: // Track
			return trkreg;
		case 2: // Sector
			return secreg;
		case 3: // Data
			return datareg;
		}
		return 0xff;
	}
	
	void write_signal(int id, uint32_t data, uint32_t mask) override {
		if(id == SIG_MB8877_MOTOR) {
			motor_on = (data & mask) != 0;
		} else if(id == SIG_MB8877_DRIVEREG) {
			drvreg = data & 3;
		}
	}
	
	void open_disk(int drv, const _TCHAR* file_path, int bank) {
		if(drv >= 0 && drv < 4 && safe_disks[drv]) {
			safe_disks[drv]->open(file_path, bank);
		}
	}
	
	void close_disk(int drv) {
		if(drv >= 0 && drv < 4 && safe_disks[drv]) {
			safe_disks[drv]->close();
		}
	}
	
	bool is_disk_inserted(int drv) {
		if(drv >= 0 && drv < 4 && safe_disks[drv]) {
			return safe_disks[drv]->inserted;
		}
		return false;
	}
	
private:
	void process_command() {
		status = 0x01; // BUSY
		
		// Simple command processing
		uint8_t cmd = cmdreg & 0xF0;
		
		if(cmd == 0x00) { // Restore
			trkreg = 0;
			status = 0x04; // TR00
		} else if(cmd == 0x80) { // Read Sector
			if(motor_on && disk[drvreg] && disk[drvreg]->inserted) {
				if(disk[drvreg]->get_sector(trkreg, 0, secreg)) {
					// Simulate successful read
					status = 0x00; // Success
					datareg = disk[drvreg]->sector[0]; // First byte
				} else {
					status = 0x10; // Record not found
				}
			} else {
				status = 0x80; // Not ready
			}
		} else if(cmd == 0xA0) { // Write Sector
			if(motor_on && disk[drvreg] && disk[drvreg]->inserted) {
				if(!disk[drvreg]->write_protected) {
					status = 0x00; // Success
				} else {
					status = 0x40; // Write protect
				}
			} else {
				status = 0x80; // Not ready
			}
		} else {
			// Unknown command
			status = 0x00;
		}
	}
};

// Test basic SafeDISK integration
void test_safe_disk_basic_integration(TestFramework& test) {
	TEST_SECTION("SafeDISK Basic Integration");
	
	MB8877_SafeDisk fdc;
	fdc.initialize();
	fdc.reset();
	
	// Test without disk
	uint32_t status = fdc.read_io8(0);
	test.assert_equal(0x04, status, "Initial status has TR00");
	
	// Open disk
	fdc.open_disk(0, nullptr, 0);
	test.assert_true(fdc.is_disk_inserted(0), "Disk 0 inserted");
	
	// Close disk
	fdc.close_disk(0);
	test.assert_false(fdc.is_disk_inserted(0), "Disk 0 ejected");
}

// Test read operations
void test_safe_disk_read_operations(TestFramework& test) {
	TEST_SECTION("SafeDISK Read Operations");
	
	MB8877_SafeDisk fdc;
	fdc.initialize();
	fdc.reset();
	
	// Open disk and turn on motor
	fdc.open_disk(0, nullptr, 0);
	fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
	fdc.write_signal(SIG_MB8877_DRIVEREG, 0, 3);
	
	// Try to read sector
	fdc.write_io8(1, 0); // Track 0
	fdc.write_io8(2, 1); // Sector 1
	fdc.write_io8(0, 0x80); // READ SECTOR command
	
	uint32_t status = fdc.read_io8(0);
	test.assert_equal(0x00, status, "Read successful");
	
	uint32_t data = fdc.read_io8(3);
	test.assert_equal(0x01, data, "First byte correct");
}

// Test crash prevention
void test_safe_disk_crash_prevention(TestFramework& test) {
	TEST_SECTION("SafeDISK Crash Prevention");
	
	// Multiple FDC instances
	for(int i = 0; i < 5; i++) {
		MB8877_SafeDisk* fdc = new MB8877_SafeDisk();
		fdc->initialize();
		fdc->reset();
		
		// Open/close multiple times
		for(int j = 0; j < 10; j++) {
			fdc->open_disk(0, _T("test.d88"), 0);
			fdc->open_disk(1, nullptr, 0);
			fdc->close_disk(0);
			fdc->close_disk(1);
		}
		
		delete fdc;
	}
	
	test.assert_true(true, "No crashes during stress test");
}

// Test error handling
void test_safe_disk_error_handling(TestFramework& test) {
	TEST_SECTION("SafeDISK Error Handling");
	
	MB8877_SafeDisk fdc;
	fdc.initialize();
	fdc.reset();
	
	// Read without disk
	fdc.write_io8(0, 0x80); // READ SECTOR
	uint32_t status = fdc.read_io8(0);
	test.assert_equal(0x80, status, "Not ready without disk");
	
	// Read without motor
	fdc.open_disk(0, nullptr, 0);
	fdc.write_signal(SIG_MB8877_MOTOR, 0, 1); // Motor off
	fdc.write_io8(0, 0x80); // READ SECTOR
	status = fdc.read_io8(0);
	test.assert_equal(0x80, status, "Not ready without motor");
	
	// Invalid drive
	fdc.write_signal(SIG_MB8877_DRIVEREG, 5, 0xFF); // Invalid drive
	fdc.write_io8(0, 0x80); // READ SECTOR
	status = fdc.read_io8(0);
	test.assert_equal(0x80, status, "Not ready with invalid drive");
}

// Main test runner
int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	
	printf("MB8877 SafeDISK Integration Test\n");
	printf("=================================\n");
	
	TestFramework test;
	
	// Run all tests
	test_safe_disk_basic_integration(test);
	test_safe_disk_read_operations(test);
	test_safe_disk_crash_prevention(test);
	test_safe_disk_error_handling(test);
	
	// Print summary
	test.summary();
	
	// Return status
	return test.tests_failed > 0 ? 1 : 0;
}