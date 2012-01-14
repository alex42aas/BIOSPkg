/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PCI_PLATFORM_LIB_H
#define _PCI_PLATFORM_LIB_H

#include <Protocol/PciPlatform.h>

typedef 
EFI_STATUS
(EFIAPI *CUSTOM_GET_ROM) (
    IN UINTN Seg, 
    IN UINTN Bus,
    IN UINTN Dev,
    IN UINTN Func,
    OUT PHYSICAL_ADDRESS *BaseAddress,
    OUT PHYSICAL_ADDRESS *Length
    );

typedef enum {
  RomFileLoadFromFv,
  RomFileCustomLoad,
  TerminatePciRomList
} PCI_PLATFORM_OPROM_ATTRIBUTES;

typedef struct _PCI_PLATFORM_OPROME_TABLE {
  PCI_PLATFORM_OPROM_ATTRIBUTES  Attribute;
  UINT16  VendorId;
  UINT16  DeviceId;
  EFI_GUID *FileGuid;
  CUSTOM_GET_ROM CustomGetRom;
} PCI_PLATFORM_OPROME_TABLE;
 
extern PCI_PLATFORM_OPROME_TABLE gPlatformOpromTable[];
extern EFI_PCI_PLATFORM_POLICY gPciPlatformPolicy;

#endif
