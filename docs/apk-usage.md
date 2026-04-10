# APK Usage

The Android APK expects machine resources to be placed under the Android
download directory.

## Resource Layout

- BIOS, font files, and similar resources:
  - `/sdcard/Download/emulator/<machine>ROM/`
- Floppy disk images:
  - `/sdcard/Download/emulator/<machine>ROM/DISK/`
- Tape images:
  - `/sdcard/Download/emulator/<machine>ROM/TAPE/`

For example, `X1turboZ` uses:

- `/sdcard/Download/emulator/x1turbozROM/`

## Input

- Android's default keyboard is often not enough for emulator use.
- The wiki recommends Hacker's Keyboard for symbol keys, cursor keys, and function keys.

## Notes

- The wiki mentions real-device testing on an AQUOS SH-01K running Android 8.
- For many systems, boot confirmation was done without BIOS or font files when compatible files were not available.

## Related

- [Wiki page](https://github.com/medamap/CommonSourceCodeProject/wiki/APK%E3%81%AB%E3%81%A4%E3%81%84%E3%81%A6)
