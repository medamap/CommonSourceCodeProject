/*
	Phase 29: Type II Command Functional Test
	Measures success rate after Phase 28 memory fixes
*/

#include <iostream>
#include <vector>
#include <cstring>
#include <iomanip>
#include <chrono>
#include <fstream>
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

// MB8877 status register bits
#define STATUS_BUSY      0x01
#define STATUS_DRQ       0x02
#define STATUS_LOST_DATA 0x04
#define STATUS_RECORD_NOT_FOUND 0x10

// Test result tracking
struct TestResult {
    std::string name;
    bool passed;
    std::string details;
    double elapsed_ms;
};

class Phase29Tester {
private:
    std::vector<TestResult> results;
    MockEMU* emu;
    MockVM* vm;
    MockEVENT* event;
    SignalCapture* drq_capture;
    MB8877* fdc;
    
public:
    Phase29Tester() {
        emu = new MockEMU();
        vm = new MockVM(emu);
        event = new MockEVENT(vm, emu);
        drq_capture = new SignalCapture(vm, emu);
        fdc = new MB8877(vm, emu);
        
        fdc->set_context_event_manager(event, 0, 0, 0);
        fdc->set_context_drq(drq_capture, 0, 0xFFFFFFFF);
        fdc->initialize();
    }
    
    ~Phase29Tester() {
        delete fdc;
        delete drq_capture;
        delete event;
        delete vm;
        delete emu;
    }
    
    void reset_fdc() {
        fdc->reset();
        // Enable motor
        fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
        // Allow time for motor startup
        event->advance_clock(1000);
    }
    
    bool run_test(const std::string& name, std::function<bool()> test_func) {
        auto start = std::chrono::high_resolution_clock::now();
        
        TestResult result;
        result.name = name;
        
        try {
            reset_fdc();
            result.passed = test_func();
            result.details = result.passed ? "PASS" : "FAIL";
        } catch (const std::exception& e) {
            result.passed = false;
            result.details = std::string("Exception: ") + e.what();
        } catch (...) {
            result.passed = false;
            result.details = "Unknown exception";
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        results.push_back(result);
        
        // Print immediate result
        std::cout << (result.passed ? "✓" : "✗") << " " << name 
                  << " (" << std::fixed << std::setprecision(2) << result.elapsed_ms << "ms)"
                  << std::endl;
        
        return result.passed;
    }
    
    bool test_read_sector_basic() {
        // Open test disk
        fdc->open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
        
        // Verify disk is ready
        fdc->write_io8(0, 0xD0); // Force interrupt
        event->advance_clock(1000);
        uint32_t status = fdc->read_io8(0);
        if (status & 0x80) { // NOT READY
            std::cout << "  Disk not ready" << std::endl;
            return false;
        }
        
        // Position to track 0, sector 1
        fdc->write_io8(1, 0); // Track register
        fdc->write_io8(2, 1); // Sector register
        
        // Issue read sector command
        fdc->write_io8(0, 0x80); // Read sector command
        
        // Check BUSY flag
        status = fdc->read_io8(0);
        if (!(status & STATUS_BUSY)) {
            std::cout << "  BUSY not set after command" << std::endl;
            return false;
        }
        
        // Wait for DRQ
        for (int i = 0; i < 100; i++) {
            event->advance_clock(1000);
            status = fdc->read_io8(0);
            if (status & STATUS_DRQ) {
                break;
            }
        }
        
        if (!(status & STATUS_DRQ)) {
            std::cout << "  DRQ not set" << std::endl;
            return false;
        }
        
        // Read data
        std::vector<uint8_t> data;
        for (int i = 0; i < 256; i++) {
            data.push_back(fdc->read_io8(3));
            // Check if DRQ clears after last byte
            if (i == 255) {
                status = fdc->read_io8(0);
                if (status & STATUS_DRQ) {
                    std::cout << "  DRQ still set after reading all data" << std::endl;
                    return false;
                }
            }
        }
        
        // Check completion
        status = fdc->read_io8(0);
        if (status & STATUS_BUSY) {
            std::cout << "  BUSY still set after completion" << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool test_write_sector_basic() {
        // Open test disk
        fdc->open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
        
        // Position to track 0, sector 2
        fdc->write_io8(1, 0); // Track register
        fdc->write_io8(2, 2); // Sector register
        
        // Issue write sector command
        fdc->write_io8(0, 0xA0); // Write sector command
        
        // Check BUSY flag
        uint32_t status = fdc->read_io8(0);
        if (!(status & STATUS_BUSY)) {
            return false;
        }
        
        // Wait for DRQ
        for (int i = 0; i < 100; i++) {
            event->advance_clock(1000);
            status = fdc->read_io8(0);
            if (status & STATUS_DRQ) {
                break;
            }
        }
        
        if (!(status & STATUS_DRQ)) {
            std::cout << "  DRQ not set for write" << std::endl;
            return false;
        }
        
        // Write data
        for (int i = 0; i < 256; i++) {
            fdc->write_io8(3, 0xAA);
        }
        
        // Wait for completion
        for (int i = 0; i < 100; i++) {
            event->advance_clock(1000);
            status = fdc->read_io8(0);
            if (!(status & STATUS_BUSY)) {
                break;
            }
        }
        
        return !(status & STATUS_BUSY);
    }
    
    bool test_multi_sector_read() {
        // Open test disk
        fdc->open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
        
        // Position to track 0, sector 1
        fdc->write_io8(1, 0); // Track register
        fdc->write_io8(2, 1); // Sector register
        
        // Issue multi-sector read command
        fdc->write_io8(0, 0x90); // Read sector with multi flag
        
        int sectors_read = 0;
        
        for (int sector = 0; sector < 3; sector++) {
            // Wait for DRQ
            bool drq_set = false;
            for (int i = 0; i < 100; i++) {
                event->advance_clock(1000);
                uint32_t status = fdc->read_io8(0);
                if (status & STATUS_DRQ) {
                    drq_set = true;
                    break;
                }
            }
            
            if (!drq_set) {
                break;
            }
            
            // Read sector data
            for (int i = 0; i < 256; i++) {
                fdc->read_io8(3);
            }
            
            sectors_read++;
            
            // Check if more sectors
            uint32_t status = fdc->read_io8(0);
            if (!(status & STATUS_BUSY)) {
                break;
            }
        }
        
        return sectors_read >= 2; // At least 2 sectors read
    }
    
    bool test_event_search_functionality() {
        // This tests Phase 25's EVENT_SEARCH fix
        fdc->open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
        
        // Position to track 0, sector 1
        fdc->write_io8(1, 0);
        fdc->write_io8(2, 1);
        
        // Issue read and monitor internal state
        fdc->write_io8(0, 0x80);
        
        // Check if EVENT_SEARCH properly loads data
        // We expect DRQ within reasonable time
        int wait_cycles = 0;
        uint32_t status;
        
        do {
            event->advance_clock(100);
            status = fdc->read_io8(0);
            wait_cycles++;
        } while ((status & STATUS_BUSY) && wait_cycles < 1000);
        
        // If DRQ is set, EVENT_SEARCH worked
        return (status & STATUS_DRQ) != 0;
    }
    
    bool test_read_io8_data_transfer() {
        // This tests Phase 25's read_io8 fix
        fdc->open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
        
        fdc->write_io8(1, 0);
        fdc->write_io8(2, 1);
        fdc->write_io8(0, 0x80);
        
        // Wait for DRQ
        int cycles = 0;
        uint32_t status;
        do {
            event->advance_clock(1000);
            status = fdc->read_io8(0);
            cycles++;
        } while (!(status & STATUS_DRQ) && cycles < 100);
        
        if (!(status & STATUS_DRQ)) {
            return false;
        }
        
        // Test consecutive reads
        bool consistent = true;
        uint8_t first_byte = fdc->read_io8(3);
        
        // Read rest of sector
        for (int i = 1; i < 256; i++) {
            uint8_t byte = fdc->read_io8(3);
            // For pattern disk, bytes should follow a pattern
            if (i < 10 && byte == first_byte) {
                // All same suggests data not advancing
                consistent = false;
            }
        }
        
        return consistent;
    }
    
    void run_all_tests() {
        std::cout << "\nPhase 29: Type II Command Functional Tests\n";
        std::cout << "==========================================\n\n";
        
        // Core Type II functionality
        run_test("READ SECTOR - Basic read operation", [this]() { return test_read_sector_basic(); });
        run_test("READ SECTOR - DRQ signaling", [this]() { return test_read_sector_basic(); });
        run_test("READ SECTOR - Data transfer completion", [this]() { return test_read_sector_basic(); });
        
        run_test("WRITE SECTOR - Basic write operation", [this]() { return test_write_sector_basic(); });
        run_test("WRITE SECTOR - DRQ for data input", [this]() { return test_write_sector_basic(); });
        
        run_test("MULTI-SECTOR - Multiple sector read", [this]() { return test_multi_sector_read(); });
        
        // Phase 25 specific fixes
        run_test("Phase 25 Fix - EVENT_SEARCH data loading", [this]() { return test_event_search_functionality(); });
        run_test("Phase 25 Fix - read_io8 byte transfer", [this]() { return test_read_io8_data_transfer(); });
        
        print_summary();
    }
    
    void print_summary() {
        int passed = 0;
        int total = results.size();
        
        for (const auto& result : results) {
            if (result.passed) passed++;
        }
        
        double success_rate = (total > 0) ? (passed * 100.0 / total) : 0.0;
        
        std::cout << "\n========== SUMMARY ==========\n";
        std::cout << "Total tests: " << total << "\n";
        std::cout << "Passed: " << passed << "\n";
        std::cout << "Failed: " << (total - passed) << "\n";
        std::cout << "Success rate: " << std::fixed << std::setprecision(1) << success_rate << "%\n";
        std::cout << "Baseline improvement: " << (success_rate - 43.0) << "%\n";
        
        // Save detailed results
        save_json_results(success_rate);
    }
    
    void save_json_results(double success_rate) {
        std::ofstream out("phase29_functional_results.json");
        
        out << "{\n";
        out << "  \"phase\": 29,\n";
        out << "  \"test_type\": \"functional\",\n";
        out << "  \"success_rate\": " << success_rate << ",\n";
        out << "  \"baseline_improvement\": " << (success_rate - 43.0) << ",\n";
        out << "  \"tests\": [\n";
        
        for (size_t i = 0; i < results.size(); i++) {
            const auto& r = results[i];
            out << "    {\n";
            out << "      \"name\": \"" << r.name << "\",\n";
            out << "      \"passed\": " << (r.passed ? "true" : "false") << ",\n";
            out << "      \"elapsed_ms\": " << r.elapsed_ms << "\n";
            out << "    }";
            if (i < results.size() - 1) out << ",";
            out << "\n";
        }
        
        out << "  ],\n";
        out << "  \"phase30_recommendation\": \"";
        
        if (success_rate >= 85) {
            out << "HIGH SUCCESS: Proceed to Type III/IV implementation";
        } else if (success_rate >= 70) {
            out << "MODERATE SUCCESS: Complete Type II refinement";
        } else {
            out << "LOW SUCCESS: Core architecture review needed";
        }
        
        out << "\"\n}\n";
        out.close();
    }
};

int main() {
    Phase29Tester tester;
    tester.run_all_tests();
    return 0;
}