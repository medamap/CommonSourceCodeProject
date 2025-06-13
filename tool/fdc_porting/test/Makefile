# MB8877 Compatibility Test Makefile
# Author: Claude AI Assistant
# Date: 2025.01.11

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I../../../src -I. -DSTANDALONE_TEST -D_ANY2D88
LDFLAGS = 

# Source directories
SRC_DIR = ../../../src
VM_DIR = $(SRC_DIR)/vm
TEST_DIR = .
OBJ_DIR = obj

# Common source files needed for testing
COMMON_SRCS = \
	$(SRC_DIR)/common.cpp \
	$(SRC_DIR)/config.cpp \
	$(SRC_DIR)/fileio.cpp \
	$(SRC_DIR)/fifo.cpp

# VM source files
VM_SRCS = \
	$(VM_DIR)/disk.cpp

# MB8877 implementation (use test wrapper for proper include order)
MB8877_SRCS = \
	mb8877_test_wrapper.cpp

# Object files - output to obj directory
COMMON_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(COMMON_SRCS))
VM_OBJS = $(patsubst $(VM_DIR)/%.cpp,$(OBJ_DIR)/vm/%.o,$(VM_SRCS))
MB8877_OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(MB8877_SRCS))

# Test executables - each test gets its own binary
TEST_EXECUTABLES = \
	test_mb8877_registers \
	test_mb8877_type1_commands \
	test_mb8877_type2_commands \
	test_mb8877_type3_commands \
	test_mb8877_type4_commands \
	test_mb8877_error_handling \
	test_mb8877_timing \
	test_mb8877_write_track \
	test_mb8877_drive_mfm \
	test_mb8877_drive_rpm

.PHONY: all clean test run_all_tests

all: $(TEST_EXECUTABLES) run_all_tests.sh

# Individual test executables - each links independently
test_mb8877_registers: $(OBJ_DIR)/test_mb8877_registers.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_type1_commands: $(OBJ_DIR)/test_mb8877_type1_commands.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_type2_commands: $(OBJ_DIR)/test_mb8877_type2_commands.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_type3_commands: $(OBJ_DIR)/test_mb8877_type3_commands.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_type4_commands: $(OBJ_DIR)/test_mb8877_type4_commands.o $(COMMON_OBJS) $(VM_OBJS) $(OBJ_DIR)/mb8877_compat_wrapper.o
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_error_handling: $(OBJ_DIR)/test_mb8877_error_handling.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_timing: $(OBJ_DIR)/test_mb8877_timing.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_write_track: $(OBJ_DIR)/test_mb8877_write_track.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_drive_mfm: $(OBJ_DIR)/test_mb8877_drive_mfm.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_mb8877_drive_rpm: $(OBJ_DIR)/test_mb8877_drive_rpm.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Create shell script to run all tests
run_all_tests.sh:
	@echo '#!/bin/bash' > $@
	@echo '# Auto-generated script to run all MB8877 tests' >> $@
	@echo '' >> $@
	@echo 'echo "Running MB8877 Compatibility Test Suite"' >> $@
	@echo 'echo "========================================"' >> $@
	@echo '' >> $@
	@echo 'TESTS_PASSED=0' >> $@
	@echo 'TESTS_FAILED=0' >> $@
	@echo '' >> $@
	@for test in $(TEST_EXECUTABLES); do \
		echo "echo \"Running $$test...\"" >> $@; \
		echo "if ./$$test; then" >> $@; \
		echo "    echo \"  ✓ $$test PASSED\"" >> $@; \
		echo "    ((TESTS_PASSED++))" >> $@; \
		echo "else" >> $@; \
		echo "    echo \"  ✗ $$test FAILED\"" >> $@; \
		echo "    ((TESTS_FAILED++))" >> $@; \
		echo "fi" >> $@; \
		echo "" >> $@; \
	done
	@echo 'echo "========================================"' >> $@
	@echo 'echo "Total: $$((TESTS_PASSED + TESTS_FAILED)) tests"' >> $@
	@echo 'echo "Passed: $$TESTS_PASSED"' >> $@
	@echo 'echo "Failed: $$TESTS_FAILED"' >> $@
	@echo '' >> $@
	@echo 'if [ $$TESTS_FAILED -eq 0 ]; then' >> $@
	@echo '    echo "All tests PASSED!"' >> $@
	@echo '    exit 0' >> $@
	@echo 'else' >> $@
	@echo '    echo "Some tests FAILED!"' >> $@
	@echo '    exit 1' >> $@
	@echo 'fi' >> $@
	@chmod +x $@

# Create obj directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/vm

# Compile rules - output to obj directory
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/vm/%.o: $(VM_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run all tests using the shell script
test: all
	@echo "Running all tests..."
	@./run_all_tests.sh

# Run individual tests
test-registers: test_mb8877_registers
	./test_mb8877_registers

test-type1: test_mb8877_type1_commands
	./test_mb8877_type1_commands

test-type2: test_mb8877_type2_commands
	./test_mb8877_type2_commands

test-type3: test_mb8877_type3_commands
	./test_mb8877_type3_commands

test-type4: test_mb8877_type4_commands
	./test_mb8877_type4_commands

test-error: test_mb8877_error_handling
	./test_mb8877_error_handling

test-timing: test_mb8877_timing
	./test_mb8877_timing

# Clean up
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TEST_EXECUTABLES)
	rm -f run_all_tests.sh
	rm -rf test_results

# Help
help:
	@echo "Available targets:"
	@echo "  all               - Build all test executables"
	@echo "  test              - Run all tests"
	@echo "  test-registers    - Run register tests only"
	@echo "  test-type1        - Run Type I command tests only"
	@echo "  test-type2        - Run Type II command tests only"
	@echo "  test-type3        - Run Type III command tests only"
	@echo "  test-type4        - Run Type IV command tests only"
	@echo "  test-error        - Run error handling tests only"
	@echo "  test-timing       - Run timing verification tests only"
	@echo "  clean             - Clean build files"
	@echo "  help              - Show this help"

# Dependencies
test_mb8877_registers.o: test_mb8877_registers.cpp test_framework.h mock_environment.h
test_mb8877_type1_commands.o: test_mb8877_type1_commands.cpp test_framework.h mock_environment.h
test_mb8877_type2_commands.o: test_mb8877_type2_commands.cpp test_framework.h mock_environment.h
test_mb8877_type3_commands.o: test_mb8877_type3_commands.cpp test_framework.h mock_environment.h
test_mb8877_type4_commands.o: test_mb8877_type4_commands.cpp test_framework.h mock_environment.h
test_mb8877_error_handling.o: test_mb8877_error_handling.cpp test_framework.h mock_environment.h
test_mb8877_timing.o: test_mb8877_timing.cpp test_framework.h mock_environment.h