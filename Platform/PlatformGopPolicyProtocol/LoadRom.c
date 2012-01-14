/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
//#include <Library/LocalApicLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Library/CacheMaintenanceLib.h>
//#include <Csm.h>

#include <Protocol/AcpiS3Save.h> 
#include <Protocol/FirmwareVolume2.h> 
#include <Protocol/LegacyRegion.h> 
#include <Protocol/Legacy8259.h> 
#include <Protocol/LegacyInterrupt.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/Acpi30.h>
#include <Guid/Acpi.h>

// ------------------------------------------------------------------------------
#define EFI_CPU_EFLAGS_IF 0x200

/**
  Searches all Firmware Volumes for the first file matching FileType and SectionType and returns the section data.

  @param   FileType                FileType to search for within any of the firmware volumes in the platform.
  @param   SectionType             SectionType to search for within any of the matching FileTypes in the firmware volumes in the platform.
  @param   SourceSize              Return the size of the returned section data..

  @retval  != NULL                 Pointer to the allocated buffer containing the section data.
  @retval  NULL                    Section data was not found.

 **/

EFI_COMMON_SECTION_HEADER *
GetSectionInAnyFv (
    IN  EFI_GUID            *NameGuid,
    IN  EFI_SECTION_TYPE    SectionType,
    OUT UINTN               *SourceSize
    )
{
  EFI_STATUS                    Status;
  UINTN                         HandleCount;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
  VOID                          *SourceBuffer;
  UINT32                        AuthenticationStatus;

  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
      ByProtocol,
      &gEfiFirmwareVolume2ProtocolGuid,
      NULL,
      &HandleCount,
      &HandleBuffer
      );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
        HandleBuffer[Index],
        &gEfiFirmwareVolume2ProtocolGuid,
        (VOID **)&Fv
        );

    if (EFI_ERROR (Status)) {
      continue;
    }

    SourceBuffer = NULL;

    Status = Fv->ReadSection (
        Fv, 
        NameGuid, 
        SectionType, 
        0, 
        &SourceBuffer, 
        SourceSize, 
        &AuthenticationStatus
        );

    if (!EFI_ERROR (Status)) {
      DEBUG((EFI_D_INFO, "Read Section Buf:%p %p\n", SourceBuffer, *SourceSize));
      FreePool (HandleBuffer);
      return SourceBuffer;
    }

  }  

  FreePool(HandleBuffer);
  return NULL;
}

EFI_STATUS
LoadRom(
    EFI_GUID        *FileName,
    UINTN           *RomAddress,
    UINTN           *RomSize
    )
{
  UINT8                           *BiosBuffer;
  UINTN                           BiosSize;
  EFI_COMMON_SECTION_HEADER       *BiosSection;
  EFI_GUID_DEFINED_SECTION        *GuidedSection;

  DEBUG((EFI_D_INFO, "LoadRom %g to address %x size %x\n", FileName, RomAddress, *RomSize));
  BiosSection = GetSectionInAnyFv( FileName, EFI_SECTION_ALL, &BiosSize );

  ASSERT( BiosSection != NULL );
  if (BiosSection->Type == EFI_SECTION_GUID_DEFINED) {
    GuidedSection = (EFI_GUID_DEFINED_SECTION*)BiosSection;
    BiosSize -= sizeof(EFI_COMMON_SECTION_HEADER);
    BiosSize -= GuidedSection->DataOffset;
    BiosBuffer = ((UINT8*) BiosSection) + sizeof(EFI_COMMON_SECTION_HEADER) + GuidedSection->DataOffset;
  } else {
    BiosBuffer = ((UINT8*) BiosSection) + sizeof(EFI_COMMON_SECTION_HEADER);
    BiosSize -= sizeof(EFI_COMMON_SECTION_HEADER);
  }

  CopyMem(RomAddress, BiosBuffer, BiosSize);

  ASSERT(CompareMem(RomAddress, BiosBuffer, BiosSize) == 0);
  *RomSize = BiosSize;
  return EFI_SUCCESS;
}
