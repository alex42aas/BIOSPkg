/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

/** @file
  

**/ 

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <PciPlatformLib.h>


extern EFI_GUID gVideoRomGuid;
extern EFI_GUID gSataAhciRomGuid;
/*
extern EFI_GUID gPxeRomGuidNic0;
extern EFI_GUID gPxeRomGuidNic1;
*/

//EFI_PCI_PLATFORM_POLICY gPciPlatformPolicy = EFI_RESERVE_ISA_IO_ALIAS | EFI_RESERVE_VGA_IO_ALIAS;
EFI_PCI_PLATFORM_POLICY gPciPlatformPolicy = EFI_RESERVE_ISA_IO_ALIAS;

PCI_PLATFORM_OPROME_TABLE gPlatformOpromTable[] = {
  // SATA:	
  { RomFileLoadFromFv, 0x8086, 0x8c02, &gSataAhciRomGuid, NULL },			// Series 8 Desktop Sata Ahci
  { RomFileLoadFromFv, 0x8086, 0x8c03, &gSataAhciRomGuid, NULL },			// Series 8 Mobile Sata Ahci

  // VIDEO:
  { RomFileLoadFromFv, 0x8086, 0x0102, &gVideoRomGuid, NULL },				// CPU-2, Datasheet

  { RomFileLoadFromFv, 0x8086, 0x0402, &gVideoRomGuid, NULL },				// CPU-4, Datasheet

  // из VideoBios'a, CPU-4:
  { RomFileLoadFromFv, 0x8086, 0x0406, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x040A, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0412, &gVideoRomGuid, NULL },				// Cpu
  { RomFileLoadFromFv, 0x8086, 0x0416, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x041A, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x041B, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x041E, &gVideoRomGuid, NULL },				//

  // из VideoBios'a, на всякий случай:
  { RomFileLoadFromFv, 0x8086, 0x0a06, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0a0e, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0a16, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0a1e, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0a26, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0a2e, &gVideoRomGuid, NULL },				//

  { RomFileLoadFromFv, 0x8086, 0x0d06, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0d16, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0d26, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0d12, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0d22, &gVideoRomGuid, NULL },				//
  
  { RomFileLoadFromFv, 0x8086, 0x0bd0, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0bd1, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0bd2, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0bd3, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x0bd4, &gVideoRomGuid, NULL },				//


  { RomFileLoadFromFv, 0x8086, 0x1606, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x1616, &gVideoRomGuid, NULL },				//
  { RomFileLoadFromFv, 0x8086, 0x1626, &gVideoRomGuid, NULL },				//

  { TerminatePciRomList, 0 }
};

