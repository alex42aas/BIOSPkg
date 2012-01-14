This part of UEFI BIOS for Intel based board (gigabyte only).

This is my student work, done so far from 2008 till now (2012), may be some
would find it usefull.

Licensed under BSD License.

No warranty, no bugfixing, some things not working or buggy :-)
Use at your own risk :-)

This code is my try to dig deep into x86/x64 platform via writing my own 
BIOS. I use UEFI framework TianoCore (tianocore.org) and original 
AMI UEFI bios from gigabyte boards for reverse engineering.
I use binary mrc and some binary pei modules to bootstrap motherborard.

To use this pkgs clone edk2 tree, put BIOSPkg into cloned tiano tree 
as a submodule, then include dedicated pkg inf files into build.

Compiled only under MS Windows, using VC compiler and WINDDK. Compilation
under Linux not tested.

Also you can use my ApplicationPkg as UEFI application for bds phase.

Everything as one single commit (sorry for that).

