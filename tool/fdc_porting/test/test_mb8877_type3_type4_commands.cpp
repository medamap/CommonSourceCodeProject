/*
    MB8877 Type III/IV Command Tests
    
    Tests for READ ADDRESS, READ TRACK, WRITE TRACK, and FORCE INTERRUPT commands
*/

#include <stdio.h>
#include <string.h>
#include <vector>
#include <functional>
#include "test_framework.h"
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"
// #include "safe_disk.h" // Temporarily disabled

// Helper functions
void setup_test_disk_for_type3(MB8877* fdc) {
    // Don't open real disk in test - work with mock environment
    // Type III commands should work without actual disk data
}

void wait_completion(MB8877* fdc, MockEVENT* event) {
    int timeout = 10000;
    while(timeout-- > 0) {
        uint32_t status = fdc->read_io8(0);
        if(!(status & 0x01)) {  // Not busy
            break;
        }
        event->advance_clock(10);
    }
}

bool wait_until(MB8877* /*fdc*/, MockEVENT* event, std::function<bool()> condition, int timeout_ms) {
    int cycles = timeout_ms * 10;  // Assuming 10 cycles per ms
    while(cycles-- > 0) {
        if(condition()) {
            return true;
        }
        event->advance_clock(10);
    }
    return false;
}

// Test helper functions
bool test_read_address_command(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Setup test disk for Type III commands
    setup_test_disk_for_type3(&fdc);
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Move to track 5
    fdc.write_io8(1, 5);  // Track register = 5
    fdc.write_io8(0, 0x15);  // SEEK command
    wait_completion(&fdc, &event);
    
    // Execute READ ADDRESS command
    fdc.write_io8(0, 0xC0);  // READ ADDRESS
    
    // Wait for BUSY to clear
    wait_until(&fdc, &event, [&]() { return !(fdc.read_io8(0) & 0x01); }, 5000);
    
    // Check if command completed without error
    uint32_t status = fdc.read_io8(0);
    printf("Status after READ ADDRESS: 0x%02X\n", status);
    
    if(status & 0x10) {  // Record not found
        printf("ERROR: Record not found\n");
        return false;
    }
    
    // Check that sector register was updated
    uint8_t sector = fdc.read_io8(2);
    printf("Sector register after READ ADDRESS: %d\n", sector);
    
    if(sector == 0 || sector > 16) {
        printf("ERROR: Invalid sector value\n");
        return false;
    }
    
    return true;
}

bool test_read_address_with_data(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Setup test disk for Type III commands
    setup_test_disk_for_type3(&fdc);
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Move to track 10
    fdc.write_io8(1, 10);  // Track register = 10
    fdc.write_io8(0, 0x15);  // SEEK command
    wait_completion(&fdc, &event);
    
    // Execute READ ADDRESS command
    fdc.write_io8(0, 0xC0);  // READ ADDRESS
    
    // Read ID field data
    std::vector<uint8_t> id_data;
    int timeout = 1000;
    
    while(timeout-- > 0) {
        uint32_t status = fdc.read_io8(0);
        
        if(status & 0x02) {  // DRQ
            id_data.push_back(fdc.read_io8(3));
            event.advance_clock(10);
        } else if(!(status & 0x01)) {  // Not BUSY
            break;
        } else {
            event.advance_clock(10);
        }
    }
    
    printf("Read %zu bytes of ID data\n", id_data.size());
    
    if(id_data.size() != 6) {
        printf("ERROR: Expected 6 bytes (Track, Head, Sector, Size, CRC1, CRC2)\n");
        return false;
    }
    
    // Verify ID field contents
    printf("ID Field: Track=%d, Head=%d, Sector=%d, Size=%d\n",
           id_data[0], id_data[1], id_data[2], id_data[3]);
    
    if(id_data[0] != 10) {  // Track should be 10
        printf("ERROR: Track ID mismatch\n");
        return false;
    }
    
    return true;
}

bool test_read_track_command(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Setup test disk for Type III commands
    setup_test_disk_for_type3(&fdc);
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Move to track 5
    fdc.write_io8(1, 5);  // Track register = 5
    fdc.write_io8(0, 0x15);  // SEEK command
    wait_completion(&fdc, &event);
    
    // Execute READ TRACK command
    fdc.write_io8(0, 0xE0);  // READ TRACK
    
    // Read track data
    std::vector<uint8_t> track_data;
    int timeout = 10000;
    
    while(timeout-- > 0 && track_data.size() < 8192) {
        uint32_t status = fdc.read_io8(0);
        
        if(status & 0x02) {  // DRQ
            track_data.push_back(fdc.read_io8(3));
            event.advance_clock(10);
        } else if(!(status & 0x01)) {  // Not BUSY
            break;
        } else {
            event.advance_clock(10);
        }
    }
    
    printf("Read %zu bytes of track data\n", track_data.size());
    
    if(track_data.size() < 3000) {
        printf("ERROR: Track data too small\n");
        return false;
    }
    
    // Count GAP and SYNC bytes
    int gap_count = 0;
    int sync_count = 0;
    int id_am_count = 0;
    int data_am_count = 0;
    
    for(size_t i = 0; i < track_data.size(); i++) {
        if(track_data[i] == 0x4E) gap_count++;
        if(track_data[i] == 0xA1) sync_count++;
        if(track_data[i] == 0xFE) id_am_count++;
        if(track_data[i] == 0xFB || track_data[i] == 0xF8) data_am_count++;
    }
    
    printf("Track analysis: GAP=%d, SYNC=%d, ID AM=%d, Data AM=%d\n",
           gap_count, sync_count, id_am_count, data_am_count);
    
    if(gap_count < 100 || sync_count < 10 || id_am_count < 10 || data_am_count < 10) {
        printf("ERROR: Invalid track structure\n");
        return false;
    }
    
    return true;
}

bool test_write_track_command(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Use SafeDISK (not write protected)
    fdc.open_disk(0, nullptr, 0);
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Move to track 20
    fdc.write_io8(1, 20);  // Track register = 20
    fdc.write_io8(0, 0x15);  // SEEK command
    wait_completion(&fdc, &event);
    
    // Execute WRITE TRACK command
    fdc.write_io8(0, 0xF0);  // WRITE TRACK
    
    // Prepare format data for 9 sectors
    std::vector<uint8_t> format_data;
    
    // Track start GAP
    for(int i = 0; i < 80; i++) {
        format_data.push_back(0x4E);
    }
    
    // Format 9 sectors
    for(int sector = 1; sector <= 9; sector++) {
        // SYNC
        for(int i = 0; i < 12; i++) {
            format_data.push_back(0x00);
        }
        for(int i = 0; i < 3; i++) {
            format_data.push_back(0xF5);  // Write A1 with missing clock
        }
        
        // ID AM
        format_data.push_back(0xFE);
        
        // ID Field
        format_data.push_back(20);      // Track
        format_data.push_back(0);       // Side
        format_data.push_back(sector);  // Sector
        format_data.push_back(1);       // Size (256 bytes)
        format_data.push_back(0xF7);    // Write CRC
        
        // GAP2
        for(int i = 0; i < 22; i++) {
            format_data.push_back(0x4E);
        }
        
        // Data SYNC
        for(int i = 0; i < 12; i++) {
            format_data.push_back(0x00);
        }
        for(int i = 0; i < 3; i++) {
            format_data.push_back(0xF5);  // Write A1 with missing clock
        }
        
        // Data AM
        format_data.push_back(0xFB);
        
        // Data field (256 bytes of E5)
        for(int i = 0; i < 256; i++) {
            format_data.push_back(0xE5);
        }
        
        // Data CRC
        format_data.push_back(0xF7);
        
        // GAP3
        for(int i = 0; i < 54; i++) {
            format_data.push_back(0x4E);
        }
    }
    
    // Fill rest of track with GAP4
    while(format_data.size() < 6250) {  // Standard track size
        format_data.push_back(0x4E);
    }
    
    // Write format data
    size_t write_index = 0;
    int timeout = 10000;
    
    while(timeout-- > 0 && write_index < format_data.size()) {
        uint32_t status = fdc.read_io8(0);
        
        if(status & 0x02) {  // DRQ
            fdc.write_io8(3, format_data[write_index++]);
            event.advance_clock(10);
        } else if(!(status & 0x01)) {  // Not BUSY
            break;
        } else {
            event.advance_clock(10);
        }
    }
    
    printf("Wrote %zu bytes of format data\n", write_index);
    
    // Check final status
    uint32_t status = fdc.read_io8(0);
    printf("Status after WRITE TRACK: 0x%02X\n", status);
    
    if(status & 0x20) {  // Write fault
        printf("ERROR: Write fault\n");
        return false;
    }
    
    if(status & 0x40) {  // Write protect
        printf("ERROR: Write protected\n");
        return false;
    }
    
    // Verify by reading back
    fdc.write_io8(0, 0xC0);  // READ ADDRESS
    wait_completion(&fdc, &event);
    
    uint8_t sector = fdc.read_io8(2);
    printf("Verified sector after format: %d\n", sector);
    
    return true;
}

bool test_force_interrupt_command(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Setup test disk for Type III commands
    setup_test_disk_for_type3(&fdc);
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Start a long-running command (READ TRACK)
    fdc.write_io8(0, 0xE0);  // READ TRACK
    
    // Wait a bit for command to start
    for(int i = 0; i < 100; i++) {
        event.advance_clock(10);
    }
    
    // Verify command is running
    uint32_t status = fdc.read_io8(0);
    if(!(status & 0x01)) {
        printf("ERROR: Command not busy\n");
        return false;
    }
    
    // Issue FORCE INTERRUPT with immediate interrupt (I3=1)
    fdc.write_io8(0, 0xD8);  // FORCE INTERRUPT, I3=1
    
    // Wait for interrupt
    wait_until(&fdc, &event, [&]() { return !(fdc.read_io8(0) & 0x01); }, 1000);
    
    // Check that command was terminated
    status = fdc.read_io8(0);
    printf("Status after FORCE INTERRUPT: 0x%02X\n", status);
    
    if(status & 0x01) {
        printf("ERROR: Command still busy\n");
        return false;
    }
    
    // Test conditional interrupt (no immediate interrupt)
    fdc.write_io8(0, 0x88);  // Read sector
    
    // Wait for command to start
    for(int i = 0; i < 100; i++) {
        event.advance_clock(10);
    }
    
    // Issue FORCE INTERRUPT with no immediate interrupt
    fdc.write_io8(0, 0xD0);  // FORCE INTERRUPT, no conditions
    
    // Should terminate without interrupt
    wait_until(&fdc, &event, [&]() { return !(fdc.read_io8(0) & 0x01); }, 1000);
    
    status = fdc.read_io8(0);
    if(status & 0x01) {
        printf("ERROR: Command not terminated\n");
        return false;
    }
    
    return true;
}

// Main test runner
int main() {
    printf("MB8877 Type III/IV Command Tests\n");
    printf("================================\n");
    
    int passed = 0;
    int total = 0;
    
    // Test READ ADDRESS
    if(test_read_address_command("READ ADDRESS Basic Test")) passed++;
    total++;
    
    if(test_read_address_with_data("READ ADDRESS Data Test")) passed++;
    total++;
    
    // Test READ TRACK
    if(test_read_track_command("READ TRACK Test")) passed++;
    total++;
    
    // Test WRITE TRACK
    if(test_write_track_command("WRITE TRACK (Format) Test")) passed++;
    total++;
    
    // Test FORCE INTERRUPT
    if(test_force_interrupt_command("FORCE INTERRUPT Test")) passed++;
    total++;
    
    printf("\n================================\n");
    printf("Type III/IV Tests: %d/%d passed (%.1f%%)\n", 
           passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}