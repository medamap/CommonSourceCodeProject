#include <iostream>
#include <cassert>
#include <cstring>
#include <memory>
#include "../../../src/vm/mb8877_compat.h"
#include "../../../src/vm/disk.h"
#include "../../../src/vm/diskimg.h"

// Mock EMU class
class EMU {
public:
    void out_debug_log(const _TCHAR* format, ...) {}
};

// Test helper function to create disk
DISK* create_test_disk() {
    DISK* disk = new DISK(nullptr);
    return disk;
}

// Test 1: Basic track size setting
void test_basic_track_size_setting() {
    std::cout << "Test 1: Basic track size setting... ";
    
    EMU emu;
    MB8877 fdc(&emu);
    
    // Create and insert test disk
    DISK* disk = create_test_disk();
    fdc.open_disk(0, disk, false);
    
    // Test standard 2D/2DD size
    fdc.set_track_size(0, 6250);
    assert(disk->track_size == 6250);
    
    // Test standard 2HD size
    fdc.set_track_size(0, 12500);
    assert(disk->track_size == 12500);
    
    // Test custom size
    fdc.set_track_size(0, 8192);
    assert(disk->track_size == 8192);
    
    fdc.close_disk(0);
    delete disk;
    
    std::cout << "PASSED" << std::endl;
}

// Test 2: Size range validation
void test_size_range_validation() {
    std::cout << "Test 2: Size range validation... ";
    
    EMU emu;
    MB8877 fdc(&emu);
    
    DISK* disk = create_test_disk();
    fdc.open_disk(0, disk, false);
    
    // Set initial valid size
    fdc.set_track_size(0, 6250);
    assert(disk->track_size == 6250);
    
    // Test minimum boundary (1024)
    fdc.set_track_size(0, 1024);
    assert(disk->track_size == 1024);
    
    // Test below minimum (should not change)
    fdc.set_track_size(0, 1023);
    assert(disk->track_size == 1024);  // Should remain unchanged
    
    // Test below minimum extreme
    fdc.set_track_size(0, 0);
    assert(disk->track_size == 1024);  // Should remain unchanged
    
    // Test maximum boundary (65536)
    fdc.set_track_size(0, 65536);
    assert(disk->track_size == 65536);
    
    // Test above maximum (should not change)
    fdc.set_track_size(0, 65537);
    assert(disk->track_size == 65536);  // Should remain unchanged
    
    // Test extreme above maximum
    fdc.set_track_size(0, 100000);
    assert(disk->track_size == 65536);  // Should remain unchanged
    
    fdc.close_disk(0);
    delete disk;
    
    std::cout << "PASSED" << std::endl;
}

// Test 3: Individual drive setting
void test_individual_drive_setting() {
    std::cout << "Test 3: Individual drive setting... ";
    
    EMU emu;
    MB8877 fdc(&emu);
    
    // Create and insert disks in multiple drives
    DISK* disk0 = create_test_disk();
    DISK* disk1 = create_test_disk();
    DISK* disk2 = create_test_disk();
    DISK* disk3 = create_test_disk();
    
    fdc.open_disk(0, disk0, false);
    fdc.open_disk(1, disk1, false);
    fdc.open_disk(2, disk2, false);
    fdc.open_disk(3, disk3, false);
    
    // Set different sizes for each drive
    fdc.set_track_size(0, 6250);   // 2D/2DD
    fdc.set_track_size(1, 12500);  // 2HD
    fdc.set_track_size(2, 8192);   // Custom
    fdc.set_track_size(3, 10240);  // Custom
    
    // Verify each drive has its own size
    assert(disk0->track_size == 6250);
    assert(disk1->track_size == 12500);
    assert(disk2->track_size == 8192);
    assert(disk3->track_size == 10240);
    
    // Change one drive's size and verify others remain unchanged
    fdc.set_track_size(1, 16384);
    assert(disk0->track_size == 6250);   // Unchanged
    assert(disk1->track_size == 16384);  // Changed
    assert(disk2->track_size == 8192);   // Unchanged
    assert(disk3->track_size == 10240);  // Unchanged
    
    fdc.close_disk(0);
    fdc.close_disk(1);
    fdc.close_disk(2);
    fdc.close_disk(3);
    
    delete disk0;
    delete disk1;
    delete disk2;
    delete disk3;
    
    std::cout << "PASSED" << std::endl;
}

// Test 4: Invalid drive handling
void test_invalid_drive_handling() {
    std::cout << "Test 4: Invalid drive handling... ";
    
    EMU emu;
    MB8877 fdc(&emu);
    
    // Test with no disk inserted
    fdc.set_track_size(0, 6250);  // Should not crash
    
    // Test with invalid drive numbers
    fdc.set_track_size(-1, 6250);  // Negative drive number
    fdc.set_track_size(4, 6250);   // Beyond MAX_DRIVE
    fdc.set_track_size(10, 6250);  // Way beyond MAX_DRIVE
    
    // Insert disk in drive 0 only
    DISK* disk = create_test_disk();
    fdc.open_disk(0, disk, false);
    
    // Set valid size on drive 0
    fdc.set_track_size(0, 6250);
    assert(disk->track_size == 6250);
    
    // Try to set size on drive without disk
    fdc.set_track_size(1, 8192);  // Should not crash
    
    // Verify drive 0 is unchanged
    assert(disk->track_size == 6250);
    
    fdc.close_disk(0);
    delete disk;
    
    std::cout << "PASSED" << std::endl;
}

// Test 5: Special size verification
void test_special_size_verification() {
    std::cout << "Test 5: Special size verification... ";
    
    EMU emu;
    MB8877 fdc(&emu);
    
    DISK* disk = create_test_disk();
    fdc.open_disk(0, disk, false);
    
    // Test various special format sizes
    struct {
        int size;
        const char* description;
    } special_sizes[] = {
        {1024, "Minimum size"},
        {3100, "FM standard"},
        {6144, "Special format 1"},
        {6250, "2D/2DD standard"},
        {6400, "Special format 2"},
        {8192, "Special format 3"},
        {10240, "Special format 4"},
        {10410, "2HD alternate"},
        {12500, "2HD standard"},
        {16384, "Special format 5"},
        {32768, "Large format"},
        {65536, "Maximum size"}
    };
    
    for(const auto& test : special_sizes) {
        fdc.set_track_size(0, test.size);
        assert(disk->track_size == test.size);
        std::cout << "\n  - " << test.description << " (" << test.size << " bytes): OK";
    }
    
    fdc.close_disk(0);
    delete disk;
    
    std::cout << "\n  PASSED" << std::endl;
}

// Test 6: Integration with MFM mode
void test_mfm_track_size_combination() {
    std::cout << "Test 6: Integration with MFM mode... ";
    
    EMU emu;
    MB8877 fdc(&emu);
    
    DISK* disk = create_test_disk();
    fdc.open_disk(0, disk, false);
    
    // Test FM mode with appropriate track size
    fdc.set_drive_mfm(0, false);  // FM mode
    fdc.set_track_size(0, 3100);  // FM standard size
    assert(disk->track_size == 3100);
    
    // Test MFM mode with appropriate track size
    fdc.set_drive_mfm(0, true);   // MFM mode
    fdc.set_track_size(0, 6250);  // MFM standard size
    assert(disk->track_size == 6250);
    
    // Test custom combinations
    fdc.set_drive_mfm(0, false);  // FM mode
    fdc.set_track_size(0, 4096);  // Custom FM size
    assert(disk->track_size == 4096);
    
    fdc.set_drive_mfm(0, true);   // MFM mode
    fdc.set_track_size(0, 8192);  // Custom MFM size
    assert(disk->track_size == 8192);
    
    fdc.close_disk(0);
    delete disk;
    
    std::cout << "PASSED" << std::endl;
}

// Test 7: Track size persistence
void test_track_size_persistence() {
    std::cout << "Test 7: Track size persistence... ";
    
    EMU emu;
    MB8877 fdc(&emu);
    
    DISK* disk = create_test_disk();
    fdc.open_disk(0, disk, false);
    
    // Set custom track size
    fdc.set_track_size(0, 7777);
    assert(disk->track_size == 7777);
    
    // Verify get_track_size returns our custom size
    assert(disk->get_track_size() == 7777);
    
    // Test with another size
    fdc.set_track_size(0, 9999);
    assert(disk->track_size == 9999);
    assert(disk->get_track_size() == 9999);
    
    fdc.close_disk(0);
    delete disk;
    
    std::cout << "PASSED" << std::endl;
}

// Main test runner
int main() {
    std::cout << "=== MB8877 Track Size Tests ===" << std::endl;
    std::cout << "Testing track size functionality implementation" << std::endl;
    std::cout << std::endl;
    
    try {
        test_basic_track_size_setting();
        test_size_range_validation();
        test_individual_drive_setting();
        test_invalid_drive_handling();
        test_special_size_verification();
        test_mfm_track_size_combination();
        test_track_size_persistence();
        
        std::cout << std::endl;
        std::cout << "=== All tests PASSED ===" << std::endl;
        std::cout << std::endl;
        std::cout << "Summary:" << std::endl;
        std::cout << "- Basic track size setting works correctly" << std::endl;
        std::cout << "- Size range validation enforces 1024-65536 byte limits" << std::endl;
        std::cout << "- Each drive can have individual track size settings" << std::endl;
        std::cout << "- Invalid drive numbers and missing disks handled gracefully" << std::endl;
        std::cout << "- Special format sizes are supported" << std::endl;
        std::cout << "- MFM/FM mode and track size work together" << std::endl;
        std::cout << "- Track size persists and is returned by get_track_size()" << std::endl;
        
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}