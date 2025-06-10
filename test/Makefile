# MB8877 Compatibility Test Makefile
# Author: Claude AI Assistant
# Date: 2025.01.11

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I../src -I. -DSTANDALONE_TEST
LDFLAGS = 

# Source directories
SRC_DIR = ../src
VM_DIR = $(SRC_DIR)/vm
TEST_DIR = .

# Common source files needed for testing
COMMON_SRCS = \
	$(SRC_DIR)/common.cpp \
	$(SRC_DIR)/config.cpp \
	$(SRC_DIR)/fileio.cpp \
	$(SRC_DIR)/fifo.cpp

# VM source files
VM_SRCS = \
	$(VM_DIR)/device.cpp \
	$(VM_DIR)/event.cpp \
	$(VM_DIR)/disk.cpp \
	$(VM_DIR)/noise.cpp

# MB8877 implementation
MB8877_SRCS = \
	$(VM_DIR)/mb8877_compat.cpp

# Test source files
TEST_SRCS = \
	test_mb8877_registers.cpp \
	test_mb8877_type1_commands.cpp \
	test_mb8877_type2_commands.cpp

# Object files
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)
VM_OBJS = $(VM_SRCS:.cpp=.o)
MB8877_OBJS = $(MB8877_SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Test executables
TESTS = \
	test_registers \
	test_type1_commands \
	test_type2_commands

# Main test runner
TEST_RUNNER = run_all_tests

.PHONY: all clean test

all: $(TESTS) $(TEST_RUNNER)

# Individual test executables
test_registers: test_mb8877_registers.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_type1_commands: test_mb8877_type1_commands.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_type2_commands: test_mb8877_type2_commands.o $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Main test runner
$(TEST_RUNNER): run_all_tests.o $(TEST_OBJS) $(COMMON_OBJS) $(VM_OBJS) $(MB8877_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run tests
test: $(TEST_RUNNER)
	@echo "Creating test results directory..."
	@mkdir -p results
	@echo "Running MB8877 compatibility tests..."
	./$(TEST_RUNNER)

# Run individual tests
test-registers: test_registers
	@mkdir -p results
	./test_registers

test-type1: test_type1_commands
	@mkdir -p results
	./test_type1_commands

test-type2: test_type2_commands
	@mkdir -p results
	./test_type2_commands

# Clean up
clean:
	rm -f *.o $(SRC_DIR)/*.o $(VM_DIR)/*.o
	rm -f $(TESTS) $(TEST_RUNNER)
	rm -rf results

# Help
help:
	@echo "Available targets:"
	@echo "  all               - Build all tests"
	@echo "  test              - Run all tests"
	@echo "  test-registers    - Run register tests only"
	@echo "  test-type1        - Run Type I command tests only"
	@echo "  test-type2        - Run Type II command tests only"
	@echo "  clean             - Clean build files"
	@echo "  help              - Show this help"

# Note: This Makefile assumes the MB8877 compatibility layer exists.
# If the layer is not yet fully implemented, some tests may fail to compile.
# In that case, comment out the problematic tests and build incrementally.

# Dependencies (simplified - in real project would use auto-dependency generation)
test_mb8877_registers.o: test_mb8877_registers.cpp test_framework.h mock_environment.h
test_mb8877_type1_commands.o: test_mb8877_type1_commands.cpp test_framework.h mock_environment.h
test_mb8877_type2_commands.o: test_mb8877_type2_commands.cpp test_framework.h mock_environment.h
run_all_tests.o: run_all_tests.cpp test_framework.h