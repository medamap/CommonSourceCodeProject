/**
 * @file test_mb8877_performance.cpp
 * @brief Performance tests for MB8877 FDC timing accuracy
 * 
 * This test suite measures and validates the timing precision of the MB8877
 * floppy disk controller emulation, ensuring microsecond-level accuracy.
 */

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>
#include <thread>
#include "../../../src/vm/mb8877_compat.h"
#include "../../../src/vm/disk.h"
#include "../../../src/vm/event.h"

class MB8877PerformanceTest : public ::testing::Test {
protected:
    MB8877* fdc;
    EVENT* event_manager;
    DISK* disk;
    
    void SetUp() override {
        // Create event manager and FDC
        event_manager = new EVENT(nullptr, nullptr, 0, 1, 1);
        fdc = new MB8877(nullptr, event_manager, 0);
        
        // Initialize FDC
        fdc->initialize();
        event_manager->initialize();
        
        // Create and setup disk
        disk = new DISK(nullptr);
        fdc->set_context_event_manager(event_manager);
        fdc->set_drive_type(0, DRIVE_TYPE_2DD);
        fdc->open_disk(0, _T("test.d88"), 0);
        
        // Ensure disk is ready
        disk->set_inserted(true);
    }
    
    void TearDown() override {
        fdc->close_disk(0);
        delete disk;
        delete fdc;
        delete event_manager;
    }
    
    // Helper to measure function execution time
    template<typename Func>
    double measureExecutionTime(Func func, int iterations = 1000) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            func();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        return static_cast<double>(duration.count()) / iterations;
    }
    
    // Helper to calculate timing accuracy percentage
    double calculateAccuracy(double measured, double expected) {
        return 100.0 * (1.0 - std::abs(measured - expected) / expected);
    }
};

// Test: get_cur_position performance
TEST_F(MB8877PerformanceTest, GetCurPositionPerformance) {
    // Measure execution time
    double avg_time = measureExecutionTime([this]() {
        fdc->get_cur_position();
    });
    
    // Should execute in less than 1 microsecond
    EXPECT_LT(avg_time, 1.0) << "get_cur_position() too slow: " << avg_time << " µs";
}

// Test: Timing accuracy for different RPM speeds
TEST_F(MB8877PerformanceTest, RotationTimingAccuracy) {
    struct RPMTest {
        int rpm;
        double expected_us_per_rotation;
        const char* name;
    } rpm_tests[] = {
        {300, 200000.0, "300 RPM (standard)"},
        {360, 166666.7, "360 RPM (1.2MB)"},
    };
    
    for (const auto& test : rpm_tests) {
        // Set disk RPM
        disk->set_rpm(test.rpm);
        
        // Measure time for one rotation
        double measured = fdc->get_usec_to_detect_index_hole(1, false);
        double accuracy = calculateAccuracy(measured, test.expected_us_per_rotation);
        
        EXPECT_GT(accuracy, 95.0) << test.name << " timing accuracy: " << accuracy << "%";
        EXPECT_NEAR(measured, test.expected_us_per_rotation, test.expected_us_per_rotation * 0.05)
            << test.name << " rotation time off by more than 5%";
    }
}

// Test: Transfer rate timing for FM and MFM
TEST_F(MB8877PerformanceTest, TransferRateAccuracy) {
    struct TransferTest {
        bool mfm_mode;
        double expected_us_per_byte;
        const char* name;
    } transfer_tests[] = {
        {false, 62.5, "FM mode (125 kbps)"},
        {true, 31.25, "MFM mode (250 kbps)"},
    };
    
    for (const auto& test : transfer_tests) {
        // Set density mode
        disk->set_mfm_mode(test.mfm_mode);
        
        // Calculate time for transferring 1KB
        int bytes = 1024;
        double expected_time = bytes * test.expected_us_per_byte;
        double measured_time = disk->get_usec_per_bytes(bytes);
        
        double accuracy = calculateAccuracy(measured_time, expected_time);
        
        EXPECT_GT(accuracy, 99.0) << test.name << " transfer rate accuracy: " << accuracy << "%";
        EXPECT_NEAR(measured_time, expected_time, expected_time * 0.01)
            << test.name << " transfer time off by more than 1%";
    }
}

// Test: Sector access latency
TEST_F(MB8877PerformanceTest, SectorAccessLatency) {
    // Format parameters
    const int sectors_per_track = 9;
    const int bytes_per_sector = 512;
    const int track_size = sectors_per_track * (bytes_per_sector + 62); // Including gaps
    
    // Test accessing different sectors
    for (int target_sector = 0; target_sector < sectors_per_track; target_sector++) {
        // Reset position to track start
        fdc->fdc[0].cur_position = 0;
        fdc->fdc[0].prev_clock = event_manager->get_current_clock();
        
        // Calculate expected position of target sector
        int expected_position = target_sector * (track_size / sectors_per_track);
        fdc->fdc[0].next_trans_position = expected_position;
        
        // Measure access time
        double access_time = fdc->get_usec_to_next_trans_pos(false);
        
        // Access time should be less than one rotation
        EXPECT_LT(access_time, 200000.0) << "Sector " << target_sector << " access time too high";
        
        // For sequential sectors, access should be quick
        if (target_sector == 0) {
            EXPECT_LT(access_time, 10000.0) << "First sector access should be fast";
        }
    }
}

// Test: Index hole detection timing
TEST_F(MB8877PerformanceTest, IndexHoleDetectionTiming) {
    // Test various positions on the track
    std::vector<int> test_positions = {0, 1000, 5000, 10000, 15000};
    
    for (int pos : test_positions) {
        // Set current position
        fdc->fdc[0].cur_position = pos;
        fdc->fdc[0].prev_clock = event_manager->get_current_clock();
        
        // Measure time to index hole
        double time_to_index = fdc->get_usec_to_detect_index_hole(1, false);
        
        // Time should be proportional to remaining track
        int track_size = disk->get_track_size();
        double expected_time = disk->get_usec_per_bytes(track_size - pos);
        
        EXPECT_NEAR(time_to_index, expected_time, expected_time * 0.05)
            << "Index detection from position " << pos << " off by more than 5%";
    }
}

// Test: Head load delay timing
TEST_F(MB8877PerformanceTest, HeadLoadDelayTiming) {
    // Test both 2DD and 2HD drives
    struct DriveTest {
        int drive_type;
        double expected_delay;
        const char* name;
    } drive_tests[] = {
        {DRIVE_TYPE_2DD, 30000.0, "2DD drive"},
        {DRIVE_TYPE_2HD, 15000.0, "2HD drive"},
    };
    
    for (const auto& test : drive_tests) {
        // Set drive type
        fdc->set_drive_type(0, test.drive_type);
        disk->drive_type = test.drive_type;
        
        // Measure timing with head load delay
        double time_with_delay = fdc->get_usec_to_next_trans_pos(true);
        double time_without_delay = fdc->get_usec_to_next_trans_pos(false);
        
        // The difference should be approximately the head load delay
        double measured_delay = time_with_delay - time_without_delay;
        
        EXPECT_NEAR(measured_delay, test.expected_delay, test.expected_delay * 0.1)
            << test.name << " head load delay off by more than 10%";
    }
}

// Test: Continuous operation timing
TEST_F(MB8877PerformanceTest, ContinuousOperationTiming) {
    // Simulate reading multiple sectors continuously
    const int num_sectors = 9;
    const int bytes_per_sector = 512;
    std::vector<double> sector_times;
    
    // Start from beginning of track
    fdc->fdc[0].cur_position = 0;
    uint32_t start_clock = event_manager->get_current_clock();
    fdc->fdc[0].prev_clock = start_clock;
    
    for (int i = 0; i < num_sectors; i++) {
        // Calculate sector position
        int sector_pos = i * (disk->get_track_size() / num_sectors);
        fdc->fdc[0].next_trans_position = sector_pos;
        
        // Get time to sector
        double time_to_sector = fdc->get_usec_to_next_trans_pos(false);
        sector_times.push_back(time_to_sector);
        
        // Simulate reading the sector
        event_manager->set_current_clock(start_clock + static_cast<uint32_t>(time_to_sector));
        fdc->fdc[0].cur_position = sector_pos + bytes_per_sector;
        fdc->fdc[0].prev_clock = event_manager->get_current_clock();
    }
    
    // Calculate total time
    double total_time = std::accumulate(sector_times.begin(), sector_times.end(), 0.0);
    
    // Should complete within one rotation plus some overhead
    EXPECT_LT(total_time, 250000.0) << "Reading all sectors took too long";
    
    // Verify consistency of sector access times
    double avg_time = total_time / num_sectors;
    for (size_t i = 1; i < sector_times.size(); i++) {
        // Sequential sectors should have similar access times
        EXPECT_NEAR(sector_times[i], avg_time, avg_time * 0.5)
            << "Sector " << i << " access time inconsistent";
    }
}

// Test: Timing precision under load
TEST_F(MB8877PerformanceTest, TimingPrecisionUnderLoad) {
    // Create background load
    std::atomic<bool> stop_load(false);
    std::thread load_thread([&stop_load]() {
        while (!stop_load) {
            // Simulate CPU load
            volatile int dummy = 0;
            for (int i = 0; i < 1000; i++) {
                dummy += i;
            }
        }
    });
    
    // Measure timing accuracy under load
    std::vector<double> measurements;
    for (int i = 0; i < 100; i++) {
        double time = fdc->get_usec_to_detect_index_hole(1, false);
        measurements.push_back(time);
    }
    
    // Stop background load
    stop_load = true;
    load_thread.join();
    
    // Calculate statistics
    double mean = std::accumulate(measurements.begin(), measurements.end(), 0.0) / measurements.size();
    double variance = 0.0;
    for (double m : measurements) {
        variance += (m - mean) * (m - mean);
    }
    variance /= measurements.size();
    double std_dev = std::sqrt(variance);
    
    // Timing should be consistent even under load
    double cv = (std_dev / mean) * 100.0; // Coefficient of variation
    EXPECT_LT(cv, 5.0) << "Timing variance too high under load: " << cv << "%";
}

// Test: Timing accuracy for partial rotations
TEST_F(MB8877PerformanceTest, PartialRotationTiming) {
    // Test various partial rotation counts
    struct PartialTest {
        int count;
        const char* description;
    } partial_tests[] = {
        {1, "1 rotation"},
        {2, "2 rotations"},
        {5, "5 rotations"},
        {10, "10 rotations"},
    };
    
    for (const auto& test : partial_tests) {
        double measured_time = fdc->get_usec_to_detect_index_hole(test.count, false);
        double expected_time = 200000.0 * test.count; // 200ms per rotation at 300 RPM
        
        double accuracy = calculateAccuracy(measured_time, expected_time);
        
        EXPECT_GT(accuracy, 95.0)
            << test.description << " timing accuracy: " << accuracy << "%";
    }
}