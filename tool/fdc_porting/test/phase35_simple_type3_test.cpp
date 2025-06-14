/*
    Simple Type III/IV Command Test
    
    Tests basic functionality of Type III/IV commands
*/

#include <stdio.h>
#include <string.h>
#include "mb8877_test_wrapper.h"
#include "mock_environment.h"

// Simple test for READ ADDRESS command
void test_read_address() {
    printf("\n=== Testing READ ADDRESS (0xC0) ===\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Execute READ ADDRESS command
    printf("Executing READ ADDRESS command...\n");
    fdc.write_io8(0, 0xC0);  // READ ADDRESS
    
    // Wait a bit
    for(int i = 0; i < 100; i++) {
        event.advance_clock(10);
    }
    
    // Check status
    uint32_t status = fdc.read_io8(0);
    printf("Status after READ ADDRESS: 0x%02X\n", status);
    
    if(status & 0x01) {
        printf("Command still busy\n");
    } else {
        printf("Command completed\n");
    }
    
    if(status & 0x10) {
        printf("Record not found\n");
    }
}

// Simple test for READ TRACK command
void test_read_track() {
    printf("\n=== Testing READ TRACK (0xE0) ===\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Execute READ TRACK command
    printf("Executing READ TRACK command...\n");
    fdc.write_io8(0, 0xE0);  // READ TRACK
    
    // Wait a bit
    for(int i = 0; i < 100; i++) {
        event.advance_clock(10);
    }
    
    // Check status
    uint32_t status = fdc.read_io8(0);
    printf("Status after READ TRACK: 0x%02X\n", status);
    
    if(status & 0x01) {
        printf("Command still busy\n");
    } else {
        printf("Command completed\n");
    }
}

// Simple test for WRITE TRACK command
void test_write_track() {
    printf("\n=== Testing WRITE TRACK (0xF0) ===\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Execute WRITE TRACK command
    printf("Executing WRITE TRACK command...\n");
    fdc.write_io8(0, 0xF0);  // WRITE TRACK
    
    // Wait a bit
    for(int i = 0; i < 100; i++) {
        event.advance_clock(10);
    }
    
    // Check status
    uint32_t status = fdc.read_io8(0);
    printf("Status after WRITE TRACK: 0x%02X\n", status);
    
    if(status & 0x01) {
        printf("Command still busy\n");
    } else {
        printf("Command completed\n");
    }
    
    if(status & 0x40) {
        printf("Write protected\n");
    }
}

// Simple test for FORCE INTERRUPT command
void test_force_interrupt() {
    printf("\n=== Testing FORCE INTERRUPT (0xD0) ===\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Start a long command (READ TRACK)
    printf("Starting READ TRACK command...\n");
    fdc.write_io8(0, 0xE0);  // READ TRACK
    
    // Wait a bit
    for(int i = 0; i < 50; i++) {
        event.advance_clock(10);
    }
    
    // Check if busy
    uint32_t status = fdc.read_io8(0);
    printf("Status during READ TRACK: 0x%02X\n", status);
    
    // Execute FORCE INTERRUPT
    printf("Executing FORCE INTERRUPT command...\n");
    fdc.write_io8(0, 0xD8);  // FORCE INTERRUPT with immediate interrupt
    
    // Wait a bit
    for(int i = 0; i < 50; i++) {
        event.advance_clock(10);
    }
    
    // Check status
    status = fdc.read_io8(0);
    printf("Status after FORCE INTERRUPT: 0x%02X\n", status);
    
    if(status & 0x01) {
        printf("ERROR: Command still busy after FORCE INTERRUPT\n");
    } else {
        printf("Command successfully interrupted\n");
    }
}

int main() {
    printf("MB8877 Simple Type III/IV Command Test\n");
    printf("======================================\n");
    
    test_read_address();
    test_read_track();
    test_write_track();
    test_force_interrupt();
    
    printf("\n======================================\n");
    printf("Test completed\n");
    
    return 0;
}