AptioFix
========

AptioFixPkg drivers fixing certain UEFI APTIO Firmware issues relevant to booting macOS.

**WARNING**: The code in this repository should be considered to be a proof of concept draft quality, and is only intended to be used as a software implementation guide. Due to the lack of time, this codebase may contain partially understood reverse-engineering samples, almost no documentation, hacks, and absolute ignorance of EDK2 codestyle.

## AptioInputFix 

Reference driver to shim AMI APTIO proprietary mouse & keyboard protocols for File Vault 2 GUI input support.

#### Features
- Sends pressed keys to APPLE_KEY_MAP_DATABASE_PROTOCOL
- Fixes mouse movement via EFI_SIMPLE_POINTER_PROTOCOL

## AptioMemoryFix

Fork of the original [OsxAptioFix2](https://sourceforge.net/p/cloverefiboot/code/HEAD/tree/OsxAptioFixDrv/) driver with a cleaner (yet still terrible) codebase and improved stability and functionality.

#### Features (compared to the original)
- Automatically finds the available memory region for boot.efi to use
- Implements KASLR support for systems where certain slides cannot be used
- Adds Safe Mode support on systems with used lower addresses
- Ensures no slide argument leak to the operating system
- Attempts to more properly handle the memory mappings
- Implements new mapping code when waking from hibernation (still not stable enough)

#### Credits
- [Apple](https://www.apple.com) for macOS
- [AMI](https://ami.com) for APTIO firmware
- [dmazar](https://sourceforge.net/u/dmazar/profile/), [apianti](https://sourceforge.net/u/apianti/), [CodeRush](https://github.com/NikolajSchlej), [night199uk](https://github.com/night199uk), [Slice](https://sourceforge.net/u/slice2009/) for developing the original OsxAptioFix driver
- [CupertinoNet](https://github.com/CupertinoNet) team for continuous support
- [Download-Fritz](https://github.com/Download-Fritz) for implimenting some of this code and invaluable suggestions
- [vit9696](https://github.com/vit9696) for all the mess
- Big thanks go to all the contributors and researchers involved in AMI APTIO exploration!
- Additional thanks go to people on [AppleLife](http://applelife.ru) and [InsanelyMac](http://insanelymac.com) who helped to test things!
