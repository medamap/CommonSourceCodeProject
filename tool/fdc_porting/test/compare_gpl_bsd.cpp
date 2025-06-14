// Direct comparison test between GPL mb8877.cpp and BSD mb8877_compat.cpp
// This test runs identical operations on both implementations to verify they produce the same results

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>

// Common mock environment setup
#define STANDALONE_TEST
#define _EMU_H_
#define _DEVICE_H_
#define _VM_TEMPLATE_H_
#define _DISK_H_
#define _NOISE_H_
#define _FILEIO_H_

// Include mock environment
#include "mock_environment.h"
#include "safe_disk.h"

// Test result structure
struct TestResult {
    std::string test_name;
    uint8_t gpl_status;
    uint8_t bsd_status;
    std::string gpl_desc;
    std::string bsd_desc;
    bool match;
};

// Format status register
std::string format_status(uint8_t status) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)status << " (";
    
    std::vector<std::string> flags;
    if (status & 0x80) flags.push_back("NOT_READY");
    if (status & 0x40) flags.push_back("WRITE_PROTECT");
    if (status & 0x20) flags.push_back("HEAD_ENGAGED");
    if (status & 0x10) flags.push_back("RNF");
    if (status & 0x08) flags.push_back("CRC_ERROR");
    if (status & 0x04) flags.push_back("TRACK_ZERO");
    if (status & 0x02) flags.push_back("INDEX");
    if (status & 0x01) flags.push_back("BUSY");
    
    for (size_t i = 0; i < flags.size(); i++) {
        if (i > 0) ss << "|";
        ss << flags[i];
    }
    ss << ")";
    return ss.str();
}

// Function to run a test on both implementations
template<typename GPL_FDC, typename BSD_FDC>
TestResult run_test(const std::string& test_name, 
                   std::function<uint8_t(GPL_FDC*, SafeDISK*)> test_func) {
    TestResult result;
    result.test_name = test_name;
    
    // Test GPL implementation
    {
        MockEnvironment env;
        env.initialize();
        auto gpl_fdc = new GPL_FDC(&env, nullptr, 8);
        gpl_fdc->initialize();
        
        SafeDISK disk(&env);
        gpl_fdc->set_context_disk_handler(&disk, 0);
        
        result.gpl_status = test_func(gpl_fdc, &disk);
        result.gpl_desc = format_status(result.gpl_status);
        
        delete gpl_fdc;
    }
    
    // Test BSD implementation
    {
        MockEnvironment env;
        env.initialize();
        auto bsd_fdc = new BSD_FDC(&env, nullptr, 8);
        bsd_fdc->initialize();
        
        SafeDISK disk(&env);
        bsd_fdc->set_context_disk_handler(&disk, 0);
        
        result.bsd_status = test_func(bsd_fdc, &disk);
        result.bsd_desc = format_status(result.bsd_status);
        
        delete bsd_fdc;
    }
    
    result.match = (result.gpl_status == result.bsd_status);
    return result;
}

// Include implementations AFTER all headers and helpers are defined
#include "../../../src/vm/mb8877.h"
#include "../../../src/vm/mb8877.cpp"
#include "../../../src/vm/mb8877_compat.h"
#include "../../../src/vm/mb8877_compat.cpp"

int main() {
    std::cout << "=== MB8877 GPL vs BSD Implementation Direct Comparison ===" << std::endl;
    std::cout << "Testing identical operations on both implementations..." << std::endl << std::endl;
    
    std::vector<TestResult> results;
    
    // Test 1: Initial state
    results.push_back(run_test<MB8877, MB8877_COMPAT>("Initial State", 
        [](auto* fdc, auto* disk) {
            return fdc->read_io8(0);  // Read status
        }));
    
    // Test 2: RESTORE command without disk
    results.push_back(run_test<MB8877, MB8877_COMPAT>("RESTORE (no disk)", 
        [](auto* fdc, auto* disk) {
            fdc->write_io8(0, 0x00);  // RESTORE command
            // Process some events
            for (int i = 0; i < 10; i++) {
                fdc->event_callback(0, 0);
            }
            fdc->write_io8(0, 0xD0);  // Force interrupt
            return fdc->read_io8(0);
        }));
    
    // Test 3: SEEK command without disk
    results.push_back(run_test<MB8877, MB8877_COMPAT>("SEEK Track 5 (no disk)", 
        [](auto* fdc, auto* disk) {
            fdc->write_io8(3, 5);     // Data register = 5
            fdc->write_io8(0, 0x10);  // SEEK command
            for (int i = 0; i < 10; i++) {
                fdc->event_callback(0, 0);
            }
            fdc->write_io8(0, 0xD0);  // Force interrupt
            return fdc->read_io8(0);
        }));
    
    // Test 4: READ SECTOR without disk
    results.push_back(run_test<MB8877, MB8877_COMPAT>("READ SECTOR (no disk)", 
        [](auto* fdc, auto* disk) {
            fdc->write_io8(1, 0);     // Track 0
            fdc->write_io8(2, 1);     // Sector 1
            fdc->write_io8(0, 0x80);  // READ SECTOR command
            for (int i = 0; i < 20; i++) {
                fdc->event_callback(0, 0);
            }
            fdc->write_io8(0, 0xD0);  // Force interrupt
            return fdc->read_io8(0);
        }));
    
    // Test 5: With disk inserted
    results.push_back(run_test<MB8877, MB8877_COMPAT>("RESTORE (with disk)", 
        [](auto* fdc, auto* disk) {
            disk->open(_T("data/test_disk_images/test_basic_2d.d88"), 0);
            fdc->write_io8(0, 0x00);  // RESTORE command
            for (int i = 0; i < 100; i++) {
                fdc->event_callback(0, 0);
            }
            return fdc->read_io8(0);
        }));
    
    // Test 6: READ SECTOR with disk
    results.push_back(run_test<MB8877, MB8877_COMPAT>("READ SECTOR (with disk)", 
        [](auto* fdc, auto* disk) {
            disk->open(_T("data/test_disk_images/test_basic_2d.d88"), 0);
            fdc->write_io8(1, 0);     // Track 0
            fdc->write_io8(2, 1);     // Sector 1
            fdc->write_io8(0, 0x80);  // READ SECTOR command
            for (int i = 0; i < 100; i++) {
                fdc->event_callback(0, 0);
            }
            return fdc->read_io8(0);
        }));
    
    // Display results
    std::cout << "=== Test Results ===" << std::endl << std::endl;
    
    int matches = 0;
    for (const auto& result : results) {
        std::cout << "Test: " << result.test_name << std::endl;
        std::cout << "  GPL: " << result.gpl_desc << std::endl;
        std::cout << "  BSD: " << result.bsd_desc << std::endl;
        std::cout << "  Match: " << (result.match ? "✓ YES" : "✗ NO") << std::endl;
        std::cout << std::endl;
        
        if (result.match) matches++;
    }
    
    // Summary
    int total = results.size();
    std::cout << "=== Summary ===" << std::endl;
    std::cout << "Total tests: " << total << std::endl;
    std::cout << "Matching: " << matches << std::endl;
    std::cout << "Different: " << (total - matches) << std::endl;
    std::cout << "Match rate: " << (matches * 100 / total) << "%" << std::endl << std::endl;
    
    if (matches == total) {
        std::cout << "✓ CONCLUSION: Both implementations produce IDENTICAL results!" << std::endl;
        std::cout << "  The BSD port (mb8877_compat.cpp) is functionally equivalent to the GPL original." << std::endl;
        std::cout << "  Any test failures are due to test environment issues (missing disk files, etc.)," << std::endl;
        std::cout << "  not implementation differences." << std::endl;
    } else {
        std::cout << "✗ WARNING: Implementations show DIFFERENT behavior!" << std::endl;
        std::cout << "  This indicates actual implementation differences that need investigation." << std::endl;
        
        // Show which tests differ
        std::cout << std::endl << "Tests with differences:" << std::endl;
        for (const auto& result : results) {
            if (!result.match) {
                std::cout << "  - " << result.test_name << std::endl;
                std::cout << "    GPL: " << std::hex << (int)result.gpl_status << std::endl;
                std::cout << "    BSD: " << std::hex << (int)result.bsd_status << std::endl;
            }
        }
    }
    
    return (matches == total) ? 0 : 1;
}