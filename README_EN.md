# CommonSourceCodeProject

<div align="center">

**Comprehensive Multi-Platform Retro Computer Emulator Collection**

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Android%20%7C%20macOS%20%7C%20iOS%20%7C%20iPadOS-green)](https://github.com/takeda-toshiya/common_source_code_project)

*Preserving computing heritage through accurate emulation of 130+ vintage computers and game consoles*

</div>

## 🎯 Overview

**CommonSourceCodeProject** is an open-source retro computer emulator collection that provides accurate emulation of over **130 different vintage computers and game consoles** from the 1970s-1990s. The project features a shared codebase designed for cross-platform compatibility across Windows, Android, and Apple platforms (macOS, iOS, iPadOS).

### Key Features

- 🖥️ **130+ Emulated Systems** - Japanese home computers, business systems, game consoles, and educational machines
- 🎮 **Multiple Media Support** - Floppy disks, cassette tapes, hard drives, cartridges
- 🎨 **Advanced Graphics** - Accurate video output with scanline effects, blur filters, and accessibility features
- 🔊 **Authentic Audio** - Precise sound chip emulation for original audio experience
- 💾 **Save States** - Save and resume emulation at any point
- 🌐 **Cross-Platform** - Consistent experience across Windows, Android, macOS, iOS, and iPadOS

## 🖥️ Supported Systems

<details>
<summary><strong>🇯🇵 Japanese Home Computers (Click to expand)</strong></summary>

### Sharp X1 Series
- **X1** - Original X1 computer
- **X1 turbo** - Enhanced model with increased performance
- **X1 turboZ** - Advanced graphics capabilities
- **X1 twin** - Dual-system compatibility

### NEC PC Series
- **PC-6001 Series**: PC-6001, PC-6001mkII, PC-6001mkIISR
- **PC-8001 Series**: PC-8001, PC-8001mkII, PC-8001mkIISR
- **PC-8801 Series**: PC-8801, PC-8801mkII, PC-8801MA
- **PC-9801 Series**: PC-9801, PC-9801E/F/M, PC-9801VF/VM/VX, PC-9801RA, PC-98XA/XL/RL/DO/LT/HA

### Fujitsu FM Series
- **FM-7/77 Series**: FM-7, FM-77, FM77AV, FM77AV40, FM77AV40EX, FM77L4
- **FM-8** - Early 8-bit computer
- **FM16** - 16-bit systems (FM16β, FM16π)

### Sharp MZ Series
- **Early Models**: MZ-80A/B/K/C, MZ-700, MZ-800, MZ-1200
- **Advanced Models**: MZ-1500, MZ-2200, MZ-2500, MZ-2800, MZ-3500, MZ-5500, MZ-6500, MZ-6550

### MSX Systems
- **MSX1** - Original MSX standard
- **MSX2** - Enhanced graphics and memory
- **MSX2+** - Advanced MSX system
</details>

<details>
<summary><strong>🏢 Business & Professional Systems</strong></summary>

### Fujitsu FMR Series
- FMR-30, FMR-50, FMR-60, FMR-70, FMR-80

### TOSHIBA Systems
- J-3100GT, J-3100SL, PASOPIA, PASOPIA7

### EPSON Systems
- HC-20/HX-20, HC-40/PX-4, HC-80/PX-8, QC-10/QX-10
</details>

<details>
<summary><strong>🎮 Game Consoles & Entertainment</strong></summary>

### SEGA Systems
- Master System, Game Gear, SC-3000, SCV (Super Cassette Vision)

### NEC Gaming
- PC Engine/TurboGrafx-16 (with CD-ROM² support)

### Nintendo
- Family BASIC

### Other Gaming Systems
- ColecoVision, PV-1000, PV-2000, TV BOY
</details>

<details>
<summary><strong>📱 Portable & Educational Systems</strong></summary>

### Handheld Computers
- CASIO: FP-200, FP-1100, FX-9000P
- CANON: X-07, BX-1
- EPSON: PX-7

### Educational & Training
- TK-80BS, TK-85, MP-85 (microprocessor training systems)
- Babbage-2nd (educational computer)
- Z80 TV GAME SYSTEM (homebrew console)
</details>

## 🚀 Platform Support

### ✅ Windows (Primary Platform)
- **Build System**: Microsoft Visual C++ 2008/2017
- **Graphics**: DirectX 9.0 with DirectInput
- **Status**: Fully functional with complete GUI
- **Features**: All emulation features available

### ✅ Android
- **Build System**: Android Studio with Gradle 8.3.1  
- **Graphics**: OpenGL ES
- **Audio**: Oboe high-performance audio library
- **Status**: Functional with touch-optimized interface
- **Features**: Mobile-specific UI adaptations

### 🚧 macOS/iOS/iPadOS (Active Development)
- **Build System**: CMake + Xcode
- **Graphics**: Metal API for modern GPU acceleration
- **Audio**: Core Audio/AudioUnit integration (planned)
- **Status**: Advanced development with working prototypes

#### Current Apple Platform Progress:
- ✅ Metal rendering implementation
- ✅ Command-line interface with media mounting
- ✅ 130+ machine support via CMake build system
- ✅ Batch build system for automated compilation
- 🔄 GUI application development in progress
- 📅 Core Audio integration planned

## 📂 Project Structure

```
CommonSourceCodeProject/
├── src/                           # Core source code
│   ├── vm/                        # Virtual machine cores (CPUs, sound chips, etc.)
│   ├── menu/                      # Machine-specific configuration menus
│   ├── win32/                     # Windows-specific implementation
│   ├── Android/                   # Android-specific implementation
│   └── Xcode/                     # Apple platforms implementation
├── androidstudio/                 # Android build system
│   └── buildbatch/               # Android batch build scripts
├── xcode/                         # Apple platforms build system
│   ├── buildbatch/               # Apple batch build scripts
│   ├── Machines/                 # Machine configuration files
│   └── build_*.sh               # Build scripts
├── vc++2008/                      # Visual C++ 2008 project files
├── vc++2017/                      # Visual C++ 2017 project files
└── res/                           # Resources (icons, etc.)
```

## 🔨 Building Instructions

### Windows
```bash
# Requirements: Visual C++ 2008 SP1 or Visual C++ 2017
# DirectX SDK required (9.0 recommended)

# Open project files in Visual Studio:
# - vc++2008/*.vcproj (for VS 2008)
# - vc++2017/*.vcxproj (for VS 2017)
```

### Android
```bash
cd androidstudio

# Build single model
./buildbatch/allexecute.sh -d x1turbo -m ReleaseBuild

# Build multiple models from CSV
./buildbatch/allexecute.sh -i models/x1.csv -m ReleaseBuild

# Build all models
./gradlew assembleRelease
```

### macOS/iOS/iPadOS
```bash
cd xcode

# Build single machine
./build_machine.sh x1turbo

# Build via batch system (unsigned)
./buildbatch/allexecute.sh -d x1turbo -m MacFree

# Build multiple machines from CSV
./buildbatch/allexecute.sh -i models/x1.csv -m MacFree

# Manual CMake build
mkdir build_x1turbo && cd build_x1turbo
cmake -DMACHINE=x1turbo ..
make -j4
```

#### Apple Platform Build Modes
- **MacFree**: Unsigned macOS executables (for development/testing)
- **MacDeveloper**: Signed with Apple Developer Program certificates
- **iOSFree**: iOS builds with personal team certificates
- **iOSDeveloper**: iOS builds with Apple Developer Program certificates

## 🎮 Usage Examples

### Command Line Interface (macOS/Linux)
```bash
# Basic X1 Turbo startup
./x1turbo

# Load floppy disk
./x1turbo -fdd0 "cz8fb01.2d"

# Load cassette tape
./x1turbo -tape0 "cz8fb01.tap"

# Multiple media mounting
./x1turbo \
  -fdd0 "cz8fb01.d88" \
  -fdd1 "datadisk.d88" \
  -tape0 "usertape.tap"

# MSX with cartridge
./msx1 -cart0 "gamecart.rom"
```

### Supported File Formats
- **Floppy Disks**: `.d88`, `.2d`, `.fdi`
- **Cassette Tapes**: `.tap`, `.t77`
- **Hard Disks**: Various `.hd?` formats
- **Cartridges**: `.rom`, `.bin`

## 🏗️ Technical Architecture

### Core Design Philosophy
- **Shared Codebase**: Common emulation core across all platforms
- **Platform Abstraction**: OS-specific layers for UI, graphics, and input
- **Modular Design**: Machine-specific implementations in separate modules
- **Clean Separation**: Core emulation logic independent of platform rendering

### Key Components

#### Virtual Machine Core (`src/vm/`)
- **CPU Emulations**: Z80, 8080, 6502, 68000, x86 series
- **Sound Chips**: YM2203, YM2151, SN76489, AY-3-8910
- **Graphics Controllers**: TMS9918, HD46505, uPD7220
- **I/O Controllers**: Various PIO, SIO, FDC implementations

#### Platform-Specific Layers
- **OSD (Operating System Dependent)**: Screen, sound, input, file I/O
- **Menu Systems**: Machine-specific configuration and controls
- **Build Systems**: Platform-appropriate compilation and packaging

## 🌟 Recent Development Highlights

### Android Port Achievements
- Complete conversion from Windows batch build system to shell scripts
- Gradle-based build system supporting all 130+ machines
- Touch-optimized interface with virtual controls
- High-performance audio via Oboe library

### Apple Platforms Implementation (2024-2025)
- **Metal API Integration**: Modern GPU-accelerated rendering
- **CMake Build System**: Supporting all 130+ vintage computer models
- **Command-Line Interface**: Full media mounting capabilities (floppy, tape, cartridge, HDD)
- **Batch Build System**: Automated compilation for multiple machine types
- **Cross-Platform OSD**: Apple-specific optimizations while maintaining core compatibility

### Recent Core Improvements
- Color vision accessibility support (5 different vision types)
- SMC-777/70 border color support  
- MZ series MIDI sound source CMU-800 support
- Palette usage optimizations and bug fixes
- Analog palette border color implementation

## 🤝 Contributing & Community

### Contributors
The project benefits from contributions by numerous developers:
- **Original creator and primary maintainer** - TAKEDA, toshiya
- **Mr. Artane** - FM-7/77 series implementation
- **Mr. tanam** - MSX, PC-6001, Game Gear implementations
- **Mr. umaiboux** - MSX2+, various machine contributions
- **Mr. GORRY** - MICOM MAHJONG emulator
- **Mr. Meister** - X1 extensions and enhancements
- **Many others** - CPU cores, sound chips, machine-specific implementations

### License
This project is released under the **GNU General Public License Version 2**.
See [COPYING.txt](license/COPYING.txt) for complete license terms.

### Attribution
Extensive attribution is provided in the source code for:
- Third-party CPU cores (MAME, MESS projects)
- Sound synthesis libraries (fmgen, various chip implementations)
- Platform-specific optimizations and enhancements
- Hardware documentation and reverse engineering efforts

## 📚 Documentation & Support

### Project Documentation
- **English**: This README and inline code documentation
- **Japanese**: Original documentation in `readme.txt` and source comments
- **Wiki**: Additional information available in project wiki (see online repository)

### Development Documentation
- **Apple Platforms**: Detailed implementation progress in `claude/macOS_implementation_status.md`
- **Build Systems**: Platform-specific build documentation in respective directories
- **Architecture**: Core emulation architecture documented in source headers

## 🔮 Future Roadmap

### Apple Platforms (Immediate Priority)
1. **Core Audio Integration** - Authentic sound reproduction
2. **GUI Application** - Native macOS/iOS interface
3. **App Store Compatibility** - Distribution via official channels
4. **iCloud Integration** - Seamless file synchronization
5. **Touch Interface Optimization** - iPad-specific enhancements

### Cross-Platform Enhancements
1. **Save State Management** - Enhanced save/load capabilities
2. **Network Connectivity** - Multiplayer and file sharing features
3. **Modern Display Support** - High-DPI and HDR compatibility
4. **Performance Optimizations** - GPU acceleration improvements

## 📧 Original

- **Website**: http://takeda-toshiya.my.coocan.jp/
- **License**: GNU GPL Version 2
- **Last Update**: December 31, 2023

---

<div align="center">

**Preserving Computing History for Future Generations**

*CommonSourceCodeProject represents one of the most comprehensive efforts to preserve and provide access to vintage computing systems, ensuring that the rich history of personal computing remains accessible on modern hardware.*

</div>