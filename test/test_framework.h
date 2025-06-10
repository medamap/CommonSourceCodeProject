/*
	Simple Test Framework for MB8877 Compatibility Testing
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#ifndef _TEST_FRAMEWORK_H_
#define _TEST_FRAMEWORK_H_

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

// Test result tracking
struct TestResult {
	std::string test_name;
	bool passed;
	std::string error_message;
	double execution_time;
};

// Test runner class
class TestFramework {
private:
	std::vector<TestResult> results;
	int total_tests;
	int passed_tests;
	int failed_tests;
	
public:
	TestFramework() : total_tests(0), passed_tests(0), failed_tests(0) {}
	
	// Test assertion macros
	void assert_true(bool condition, const char* test_name, const char* message = "") {
		TestResult result;
		result.test_name = test_name;
		result.passed = condition;
		result.error_message = condition ? "" : message;
		
		results.push_back(result);
		total_tests++;
		
		if (condition) {
			passed_tests++;
			printf("[PASS] %s\n", test_name);
		} else {
			failed_tests++;
			printf("[FAIL] %s: %s\n", test_name, message);
		}
	}
	
	void assert_equal(int expected, int actual, const char* test_name) {
		char message[256];
		sprintf(message, "Expected %d, got %d", expected, actual);
		assert_true(expected == actual, test_name, message);
	}
	
	void assert_equal_hex(uint8_t expected, uint8_t actual, const char* test_name) {
		char message[256];
		sprintf(message, "Expected 0x%02X, got 0x%02X", expected, actual);
		assert_true(expected == actual, test_name, message);
	}
	
	void assert_equal_hex16(uint16_t expected, uint16_t actual, const char* test_name) {
		char message[256];
		sprintf(message, "Expected 0x%04X, got 0x%04X", expected, actual);
		assert_true(expected == actual, test_name, message);
	}
	
	void assert_not_equal(int expected, int actual, const char* test_name) {
		char message[256];
		sprintf(message, "Expected not %d, but got %d", expected, actual);
		assert_true(expected != actual, test_name, message);
	}
	
	// Memory comparison
	void assert_memory_equal(const uint8_t* expected, const uint8_t* actual, size_t size, const char* test_name) {
		bool equal = (memcmp(expected, actual, size) == 0);
		
		if (!equal) {
			char message[1024];
			sprintf(message, "Memory mismatch at byte:");
			for (size_t i = 0; i < size && i < 16; i++) {
				if (expected[i] != actual[i]) {
					char temp[64];
					sprintf(temp, " [%zu] exp:0x%02X act:0x%02X", i, expected[i], actual[i]);
					strcat(message, temp);
					break;
				}
			}
			assert_true(false, test_name, message);
		} else {
			assert_true(true, test_name);
		}
	}
	
	// Report generation
	void print_summary() {
		printf("\n========== Test Summary ==========\n");
		printf("Total tests: %d\n", total_tests);
		printf("Passed: %d\n", passed_tests);
		printf("Failed: %d\n", failed_tests);
		printf("Success rate: %.1f%%\n", total_tests > 0 ? (passed_tests * 100.0 / total_tests) : 0.0);
		printf("==================================\n");
	}
	
	bool all_tests_passed() {
		return failed_tests == 0;
	}
	
	// Save detailed results
	void save_results(const char* filename) {
		FILE* fp = fopen(filename, "w");
		if (fp) {
			fprintf(fp, "MB8877 Compatibility Test Results\n");
			fprintf(fp, "=================================\n\n");
			
			for (const auto& result : results) {
				fprintf(fp, "[%s] %s\n", result.passed ? "PASS" : "FAIL", result.test_name.c_str());
				if (!result.passed && !result.error_message.empty()) {
					fprintf(fp, "  Error: %s\n", result.error_message.c_str());
				}
			}
			
			fprintf(fp, "\nSummary:\n");
			fprintf(fp, "Total: %d, Passed: %d, Failed: %d\n", total_tests, passed_tests, failed_tests);
			fclose(fp);
		}
	}
};

// Test section macros
#define TEST_SUITE(name) printf("\n=== Test Suite: %s ===\n", name);
#define TEST_SECTION(name) printf("\n--- %s ---\n", name);

#endif