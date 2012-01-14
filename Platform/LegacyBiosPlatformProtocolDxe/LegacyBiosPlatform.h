/** @file
    Driver implementing the Tiano Legacy Bios Platform Protocol

    Copyright (c) 2005 - 2009, Intel Corporation. All rights reserved.<BR>
    This program and the accompanying materials
    are licensed and made available under the terms and conditions of the BSD License
    which accompanies this distribution.  The full text of the license may be found at
    http://opensource.org/licenses/bsd-license.php.

    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _LEGACY_BIOS_PLATFORM_H__
#define _LEGACY_BIOS_PLATFORM_H__

#include <Protocol/LegacyBiosPlatform.h>
#include <Protocol/LegacyBios.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

//#include <PchRegs.h>
#include <Library/PciLib.h>

#define MAX_PCI_BRIDGES 0x20
#define CSM_GOP_FLAG    0x4E6
// Protocol Function Prototypes

EFI_STATUS
EFIAPI
GetPlatformInfo (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN EFI_GET_PLATFORM_INFO_MODE Mode,
  IN OUT VOID **Table,
  IN OUT UINTN *TableSize,
  IN OUT UINTN *Location,
  OUT UINTN *Alignment,
  IN UINT16 LegacySegment,
  IN UINT16 LegacyOffset
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
EFIAPI
GetPlatformHandle (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN EFI_GET_PLATFORM_HANDLE_MODE Mode,
  IN UINT16 Type,
  OUT EFI_HANDLE **HandleBuffer,
  OUT UINTN *HandleCount,
  OUT VOID OPTIONAL **AdditionalData
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
EFIAPI
SmmInit (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN VOID *EfiToCompatibility16BootTable
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
EFIAPI
PlatformHooks (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN EFI_GET_PLATFORM_HOOK_MODE Mode,
  IN UINT16 Type,
  IN EFI_HANDLE OPTIONAL DeviceHandle,
  IN OUT UINTN OPTIONAL *ShadowAddress,
  IN EFI_COMPATIBILITY16_TABLE OPTIONAL *Compatibility16Table,
  OUT VOID OPTIONAL **AdditionalData
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
EFIAPI
GetRoutingTable (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  OUT VOID **RoutingTable,
  OUT UINTN *RoutingTableEntries,
  OUT VOID OPTIONAL **LocalPirqTable,
  OUT UINTN OPTIONAL *PirqTableSize,
  OUT VOID OPTIONAL **LocalIrqPriorityTable,
  OUT UINTN OPTIONAL *IrqPriorityTableEntries
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
EFIAPI
TranslatePirq (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN UINTN PciBus,
  IN UINTN PciDevice,
  IN UINTN PciFunction,
  IN OUT UINT8 *Pirq,
  OUT UINT8 *PciIrq
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
EFIAPI
PrepareToBoot (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN BBS_BBS_DEVICE_PATH *BbsDevicePath,
  IN VOID *BbsTable,
  IN UINT32 LoadOptionsSize,
  IN VOID *LoadOptions,
  IN VOID *EfiToLegacyBootTable
);

#endif
