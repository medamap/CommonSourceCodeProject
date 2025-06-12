#include <cstdio>
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

int main() {
    printf("Starting simple MB8877 test...\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    printf("Created FDC\n");
    
    fdc.set_context_event_manager(&event, 0, 0, 0);
    printf("Set event manager\n");
    
    fdc.initialize();
    printf("Initialized FDC\n");
    
    fdc.reset();
    printf("Reset FDC\n");
    
    // Open a disk  
    fdc.open_disk(0, _T("/Volumes/PoppoSSD2T/Projects/ClaudeCodeProjects/Fdc-Porting/CommonSourceCodeProject/test/test.d88"), 0);
    printf("Opened disk\n");
    
    // Turn on motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    printf("Motor on\n");
    
    // Check if disk is inserted
    bool inserted = fdc.is_disk_inserted(0);
    printf("Disk inserted: %s\n", inserted ? "YES" : "NO");
    
    // Try to issue restore command
    fdc.write_io8(0, 0x00);
    printf("Issued restore command\n");
    
    // Check status
    uint32_t status = fdc.read_io8(0);
    printf("Status: 0x%02X\n", status);
    printf("BUSY: %s\n", (status & 0x01) ? "YES" : "NO");
    printf("NOT READY: %s\n", (status & 0x80) ? "YES" : "NO");
    
    // Advance clock to let events fire
    printf("\nAdvancing clock...\n");
    event.advance_clock(60000);
    
    // Check status again
    status = fdc.read_io8(0);
    printf("\nAfter advance_clock:\n");
    printf("Status: 0x%02X\n", status);
    printf("BUSY: %s\n", (status & 0x01) ? "YES" : "NO");
    printf("TRACK00: %s\n", (status & 0x04) ? "YES" : "NO");
    
    return 0;
}