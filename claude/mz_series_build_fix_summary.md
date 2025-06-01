# MZ Series Build Configuration Fix Summary

## Overview
This document summarizes the investigation and fixes for building MZ series emulators (MZ-80A, MZ-80B, MZ-80K, MZ-700, MZ-800, MZ-1500, MZ-2200, MZ-2500) in the CommonSourceCodeProject for macOS/Xcode.

## Investigation Findings

### 1. QuickDisk Support
- QuickDisk is implemented in `src/vm/mz700/quickdisk.cpp` and `quickdisk.h`
- Supported file extensions: `.mzt`, `.q20`, `.qdf`
- The Android implementation properly handles these extensions in file dialogs
- QuickDisk support is enabled for MZ-800, MZ-1500, and MZ-2200 (via `SUPPORT_QUICK_DISK` define)

### 2. Machine Implementations
- **MZ-80K/80A**: Implemented in `src/vm/mz80k/` directory
  - MZ-80A uses `_MZ80A` preprocessor define and includes floppy disk support (MB8877)
  - MZ-80K uses basic configuration without floppy support
- **MZ-80B**: Implemented as part of MZ-2500 in `src/vm/mz2500/mz80b.cpp`
  - Uses `_MZ80B` preprocessor define
- **MZ-700/800/1500**: Implemented in `src/vm/mz700/` directory
  - Share common codebase with different feature sets
  - MZ-800 and MZ-1500 include QuickDisk support
- **MZ-2200**: Also uses MZ-2500 implementation with `_MZ2200` define
- **MZ-2500**: Has its own directory `src/vm/mz2500/`

### 3. Menu Files
All MZ series have corresponding menu implementation files in `src/menu/`:
- mz80k.cpp, mz80a.cpp, mz80b.cpp
- mz700.cpp, mz800.cpp, mz1500.cpp
- mz2200.cpp, mz2500.cpp
- mz2800.cpp, mz3500.cpp, mz5500.cpp, mz6500.cpp, mz6550.cpp

## Created Configuration Files

The following Xcode/CMake configuration files were created in `/xcode/Machines/`:

### 1. _MZ80K.txt
- Basic Z80-based configuration
- Includes tape support
- No floppy disk support

### 2. _MZ80A.txt
- Extended MZ-80K configuration
- Includes MB8877 floppy disk controller
- Uses AND gate for interrupt handling

### 3. _MZ80B.txt
- Uses MZ-2500 implementation base
- Includes calendar, CMT, CRTC modules
- Floppy disk support via MB8877

### 4. _MZ800.txt
- Based on MZ-700 implementation
- Includes QuickDisk support
- SN76489AN sound chip
- Z80PIO and Z80SIO for I/O

### 5. _MZ1500.txt
- Similar to MZ-800 configuration
- Includes QuickDisk support
- Printer support via MZ1P17

### 6. _MZ2200.txt
- Based on MZ-2500/MZ-80B implementation
- Includes QuickDisk support
- More advanced features than MZ-80B

## Key Components by Machine

### Common Components (All MZ Series)
- Z80 CPU
- I8253 timer
- I8255 PIO
- Datarec (tape interface)
- PCM1BIT sound

### Machine-Specific Components
- **MZ-80A**: MB8877 (floppy), AND gate
- **MZ-80B/2200**: Calendar, CRTC, advanced memory management
- **MZ-700/800/1500**: QuickDisk, SN76489AN (sound), Z80PIO/SIO
- **MZ-2500**: Full feature set including CMU800 MIDI support

## Build Instructions

To build any MZ series machine:

```bash
cd xcode
mkdir build_mz80k
cd build_mz80k
cmake -DMACHINE=mz80k ..
make
```

Replace `mz80k` with the desired machine name (mz80a, mz80b, mz700, mz800, mz1500, mz2200).

## Notes

1. All configuration files follow the same pattern as existing machines (X1TURBO, MSX1, etc.)
2. The MZ-80B and MZ-2200 share implementation with MZ-2500 but use different preprocessor defines
3. QuickDisk support is properly implemented for machines that historically had it
4. The configurations include all necessary header and source files for each machine
5. Menu implementations are machine-specific and handle different feature sets appropriately

## Testing Required

The configurations should be tested by:
1. Running CMake with each machine configuration
2. Building the resulting projects
3. Verifying that all features work correctly (tape, disk, QuickDisk where applicable)
4. Testing file loading for each media type