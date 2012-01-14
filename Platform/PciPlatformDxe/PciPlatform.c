/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include "PciPlatform.h"

#define	CDROM_WAIT_DELAY_MARKER     "CDRMWTDL"
#define	CDROM_WAIT_DELAY_MARKER_LEN 8


EFI_STATUS
EFIAPI
CheckAndPatchSataAhciRom (
	IN UINT8* BiosBuffer, 
	IN UINTN  BiosSize
	)
{
  EFI_STATUS Status;
  SETUP	*pSetup;
  EFI_PCI_ROM_HEADER	RomHeader;
  PCI_3_0_DATA_STRUCTURE	*Pcir;
  UINT8 *Checksum;
  UINT8 NewChecksum;
  UINT8 *PatchPlace;
  UINT16 *Delay;
  BOOLEAN Found;
  CHAR8 *Marker = CDROM_WAIT_DELAY_MARKER;
  UINTN MarkerLen = CDROM_WAIT_DELAY_MARKER_LEN;

  ASSERT (BiosBuffer != NULL);
  ASSERT (BiosSize != 0);

  DEBUG ((EFI_D_INFO, "%a.%d BiosBuffer = 0x%p, BiosSize = 0x%X\n", __FUNCTION__, __LINE__, BiosBuffer, BiosSize));

  Status = ObtainSetupEfiVar(&pSetup);
  if (EFI_ERROR(Status) || pSetup == NULL) {
    DEBUG ((EFI_D_ERROR, "%a.%d: *** error: ObtainSetupEfiVar(): Status = %r, pSetup = 0x%p\n", __FUNCTION__, __LINE__, Status, pSetup));
    return EFI_NOT_FOUND;
  }

  RomHeader.Raw = BiosBuffer;
  if (RomHeader.Generic->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
    DEBUG ((EFI_D_ERROR, "%a.%d: *** error: RomHeader.Generic->Signature = 0x%X\n", __FUNCTION__, __LINE__, RomHeader.Generic->Signature));
    return EFI_NOT_FOUND;
  }
  if ((RomHeader.Generic->PcirOffset == 0) ||
    ((RomHeader.Generic->PcirOffset & 3) !=0 )||
    (BiosSize < (RomHeader.Generic->PcirOffset + sizeof (PCI_DATA_STRUCTURE)))) {
      DEBUG ((EFI_D_ERROR, "%a.%d: *** error: RomHeader.Generic->PcirOffset = 0x%X\n", __FUNCTION__, __LINE__, RomHeader.Generic->PcirOffset));
      return EFI_NOT_FOUND;
  }
  Pcir = (PCI_3_0_DATA_STRUCTURE *) (RomHeader.Raw + RomHeader.Generic->PcirOffset);
  if (Pcir->Signature != PCI_DATA_STRUCTURE_SIGNATURE) {
    DEBUG ((EFI_D_ERROR, "%a.%d: *** error: Pcir->Signature = 0x%X\n", __FUNCTION__, __LINE__, Pcir->Signature));
    return EFI_NOT_FOUND;
  }
  if ((Pcir->ImageLength * 512) > BiosSize) {
    DEBUG ((EFI_D_ERROR, "%a.%d: *** error: Pcir->ImageLength = 0x%X\n", __FUNCTION__, __LINE__, Pcir->ImageLength));
    return EFI_NOT_FOUND;
  }
  if (Pcir->CodeType != PCI_CODE_TYPE_PCAT_IMAGE) {
    DEBUG ((EFI_D_ERROR, "%a.%d: *** error: Pcir->CodeType = 0x%X\n", __FUNCTION__, __LINE__, Pcir->CodeType));
    return EFI_NOT_FOUND;
  }
  Checksum = (UINT8*)Pcir + Pcir->Length + sizeof(UINT16) + sizeof(UINT16) + sizeof(UINT8);
  if ((UINTN)(Checksum - BiosBuffer) > BiosSize) {
    DEBUG ((EFI_D_ERROR, "%a.%d: *** error: &Checksum = 0x%p\n", __FUNCTION__, __LINE__, Checksum));
    return EFI_NOT_FOUND;
  }
  for (Found = FALSE, PatchPlace = (Checksum + 1); 
    PatchPlace < ((BiosBuffer + BiosSize) - MarkerLen - sizeof(UINT16)); 
    PatchPlace++) {
      if (CompareMem (PatchPlace, Marker, MarkerLen) == 0) {
        Found = TRUE;
        break;
      }
  }
  if (!Found) {
    DEBUG ((EFI_D_ERROR, "%a.%d: *** error: Found = %d\n", __FUNCTION__, __LINE__, Found));
    return EFI_NOT_FOUND;
  }

  PatchPlace += MarkerLen;
  Delay = (UINT16*)PatchPlace;
  DEBUG ((EFI_D_INFO, "%a.%d PatchPlace = 0x%p\n", __FUNCTION__, __LINE__, PatchPlace));
  DEBUG ((EFI_D_INFO, "%a.%d OldChecksum = 0x%X\n", __FUNCTION__, __LINE__, *Checksum));
  DEBUG ((EFI_D_INFO, "%a.%d OldDelay = %d\n", __FUNCTION__, __LINE__, *Delay));
  DEBUG ((EFI_D_INFO, "%a.%d SETUP_FLAG_SATA_AHCI_SLOW = %d\n", __FUNCTION__, __LINE__, ((pSetup->Flags & SETUP_FLAG_SATA_AHCI_SLOW) != 0)));

  NewChecksum = *Checksum + *PatchPlace + *(PatchPlace + 1);
  if ((pSetup->Flags & SETUP_FLAG_SATA_AHCI_SLOW)) {
    if (*Delay != PcdGet16(SataAhciSlowDelay)) {
      *Delay = PcdGet16(SataAhciSlowDelay);
    } else {
      DEBUG ((EFI_D_INFO, "%a.%d: *** error: SataAhciSlowDelay = %d\n", __FUNCTION__, __LINE__, PcdGet16(SataAhciSlowDelay)));
      return EFI_SUCCESS;
    }
  } else {
    if (*Delay != PcdGet16(SataAhciFastDelay)) {
      *Delay = PcdGet16(SataAhciFastDelay);
    } else {
      DEBUG ((EFI_D_INFO, "%a.%d: *** error: SataAhciFastDelay = %d\n", __FUNCTION__, __LINE__, PcdGet16(SataAhciFastDelay)));
      return EFI_SUCCESS;
    }
  }

  *Checksum = NewChecksum - *PatchPlace - *(PatchPlace + 1);
  DEBUG ((EFI_D_INFO, "%a.%d: *** error: NewDelay = %d\n", __FUNCTION__, __LINE__, *Delay));
  DEBUG ((EFI_D_INFO, "%a.%d: *** error: NewChecksum = 0x%X\n", __FUNCTION__, __LINE__, *Checksum));

  return EFI_SUCCESS;
}




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
//      DEBUG((EFI_D_ERROR, "Read Section Buf:%p %p\n", SourceBuffer, *SourceSize));
      FreePool (HandleBuffer);
      return SourceBuffer;
    }
    
  }  

  FreePool(HandleBuffer);
  
  return NULL;
}


/**
  The notification from the PCI bus enumerator to the platform that it is
  about to enter a certain phase during the enumeration process.

  The PlatformNotify() function can be used to notify the platform driver so that
  it can perform platform-specific actions. No specific actions are required.
  Eight notification points are defined at this time. More synchronization points
  may be added as required in the future. The PCI bus driver calls the platform driver
  twice for every Phase-once before the PCI Host Bridge Resource Allocation Protocol
  driver is notified, and once after the PCI Host Bridge Resource Allocation Protocol
  driver has been notified.
  This member function may not perform any error checking on the input parameters. It
  also does not return any error codes. If this member function detects any error condition,
  it needs to handle those errors on its own because there is no way to surface any
  errors to the caller.

  @param[in] This           The pointer to the EFI_PCI_PLATFORM_PROTOCOL instance.
  @param[in] HostBridge     The handle of the host bridge controller.
  @param[in] Phase          The phase of the PCI bus enumeration.
  @param[in] ExecPhase      Defines the execution phase of the PCI chipset driver.

  @retval EFI_SUCCESS   The function completed successfully.

**/
EFI_STATUS
EFIAPI 
PciPlatformPhaseNotify(
  IN EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN EFI_HANDLE                                     HostBridge,
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE  Phase,
  IN EFI_PCI_EXECUTION_PHASE                        ExecPhase
  )
{
  return EFI_SUCCESS;
}

/**
  The notification from the PCI bus enumerator to the platform for each PCI
  controller at several predefined points during PCI controller initialization.

  The PlatformPrepController() function can be used to notify the platform driver so that
  it can perform platform-specific actions. No specific actions are required.
  Several notification points are defined at this time. More synchronization points may be
  added as required in the future. The PCI bus driver calls the platform driver twice for
  every PCI controller-once before the PCI Host Bridge Resource Allocation Protocol driver
  is notified, and once after the PCI Host Bridge Resource Allocation Protocol driver has
  been notified.
  This member function may not perform any error checking on the input parameters. It also
  does not return any error codes. If this member function detects any error condition, it
  needs to handle those errors on its own because there is no way to surface any errors to
  the caller.

  @param[in] This           The pointer to the EFI_PCI_PLATFORM_PROTOCOL instance.
  @param[in] HostBridge     The associated PCI host bridge handle.
  @param[in] RootBridge     The associated PCI root bridge handle.
  @param[in] PciAddress     The address of the PCI device on the PCI bus.
  @param[in] Phase          The phase of the PCI controller enumeration.
  @param[in] ExecPhase      Defines the execution phase of the PCI chipset driver.

  @retval EFI_SUCCESS   The function completed successfully.

**/

EFI_STATUS
EFIAPI 
PciPlatformPreprocessController (
  IN EFI_PCI_PLATFORM_PROTOCOL                     *This,
  IN EFI_HANDLE                                    HostBridge,
  IN EFI_HANDLE                                    RootBridge,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS   PciAddress,
  IN EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE  Phase,
  IN EFI_PCI_EXECUTION_PHASE                       ExecPhase
  )
{
  return EFI_SUCCESS;
}

/**
  Retrieves the platform policy regarding enumeration.

  The GetPlatformPolicy() function retrieves the platform policy regarding PCI
  enumeration. The PCI bus driver and the PCI Host Bridge Resource Allocation Protocol
  driver can call this member function to retrieve the policy.

  @param[in]  This        The pointer to the EFI_PCI_PLATFORM_PROTOCOL instance.
  @param[out] PciPolicy   The platform policy with respect to VGA and ISA aliasing.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   PciPolicy is NULL.

**/

EFI_STATUS
EFIAPI 
PciPlatformGetPlatformPolicy (
  IN  CONST EFI_PCI_PLATFORM_PROTOCOL  *This,
  OUT       EFI_PCI_PLATFORM_POLICY    *PciPolicy
  )
{
  if(PciPolicy == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *PciPolicy = gPciPlatformPolicy; 
  return EFI_SUCCESS;
}


/**
  Gets the PCI device's option ROM from a platform-specific location.

  The GetPciRom() function gets the PCI device's option ROM from a platform-specific location.
  The option ROM will be loaded into memory. This member function is used to return an image
  that is packaged as a PCI 2.2 option ROM. The image may contain both legacy and EFI option
  ROMs. See the UEFI 2.0 Specification for details. This member function can be used to return
  option ROM images for embedded controllers. Option ROMs for embedded controllers are typically
  stored in platform-specific storage, and this member function can retrieve it from that storage
  and return it to the PCI bus driver. The PCI bus driver will call this member function before
  scanning the ROM that is attached to any controller, which allows a platform to specify a ROM
  image that is different from the ROM image on a PCI card.

  @param[in]  This        The pointer to the EFI_PCI_PLATFORM_PROTOCOL instance.
  @param[in]  PciHandle   The handle of the PCI device.
  @param[out] RomImage    If the call succeeds, the pointer to the pointer to the option ROM image.
                          Otherwise, this field is undefined. The memory for RomImage is allocated
                          by EFI_PCI_PLATFORM_PROTOCOL.GetPciRom() using the EFI Boot Service AllocatePool().
                          It is the caller's responsibility to free the memory using the EFI Boot Service
                          FreePool(), when the caller is done with the option ROM.
  @param[out] RomSize     If the call succeeds, a pointer to the size of the option ROM size. Otherwise,
                          this field is undefined.

  @retval EFI_SUCCESS            The option ROM was available for this device and loaded into memory.
  @retval EFI_NOT_FOUND          No option ROM was available for this device.
  @retval EFI_OUT_OF_RESOURCES   No memory was available to load the option ROM.
  @retval EFI_DEVICE_ERROR       An error occurred in obtaining the option ROM.

**/

EFI_STATUS
EFIAPI 
PciPlatformGetPciRom
(
  IN  CONST EFI_PCI_PLATFORM_PROTOCOL  *This,
  IN        EFI_HANDLE                 PciHandle,
  OUT       VOID                       **RomImage,
  OUT       UINTN                      *RomSize
  )
{

  EFI_PCI_IO_PROTOCOL           *PciIo;
  EFI_STATUS                    Status;
  UINTN                         Seg;
  UINTN                         Bus;
  UINTN                         Dev;
  UINTN                         Func;
  UINTN                         Index;
  UINT16                        VendorId;
  UINT16                        DeviceId;

	Status = gBS->HandleProtocol( PciHandle, &gEfiPciIoProtocolGuid, &PciIo );

	if(EFI_ERROR(Status)) {
		return EFI_INVALID_PARAMETER;
	}

	Status = PciIo->GetLocation( PciIo, &Seg, &Bus, &Dev, &Func );

	if(EFI_ERROR(Status)) {
		return EFI_INVALID_PARAMETER;
	}

	Index = 0;

	PciIo->Pci.Read(
			PciIo,
			EfiPciIoWidthUint16,
			0,
			1,
			&VendorId
			);
	
	PciIo->Pci.Read(
			PciIo,
			EfiPciIoWidthUint16,
			2,
			1,
			&DeviceId
			);

	while(gPlatformOpromTable[Index].Attribute != TerminatePciRomList) {

	  if((gPlatformOpromTable[Index].VendorId == VendorId) &&
			(gPlatformOpromTable[Index].DeviceId == DeviceId)) {

	    if(gPlatformOpromTable[Index].Attribute == RomFileLoadFromFv ) {
  EFI_COMMON_SECTION_HEADER       *BiosSection;
  UINT8                           *BiosBuffer;
  UINTN                            BiosSize;

	      DEBUG((EFI_D_INFO, "\r\n PciPlatformGetPciRom: load for\r\n {%08x - %08x - %08x - %08x}", 
		((UINT32*)gPlatformOpromTable[Index].FileGuid)[0],
		((UINT32*)gPlatformOpromTable[Index].FileGuid)[1],
		((UINT32*)gPlatformOpromTable[Index].FileGuid)[2],
		((UINT32*)gPlatformOpromTable[Index].FileGuid)[3]
			));

	      BiosSection = GetSectionInAnyFv( 
				gPlatformOpromTable[Index].FileGuid, 
				EFI_SECTION_ALL, 
				&BiosSize 
				);
	      if(BiosSection) {
		if (BiosSection->Type == EFI_SECTION_GUID_DEFINED) {
EFI_GUID_DEFINED_SECTION        *GuidedSection;

		  GuidedSection = (EFI_GUID_DEFINED_SECTION*) BiosSection;
		  BiosSize -= sizeof(EFI_COMMON_SECTION_HEADER);
		  BiosSize -= GuidedSection->DataOffset;
		  BiosBuffer = ((UINT8*) BiosSection) + sizeof(EFI_COMMON_SECTION_HEADER) + GuidedSection->DataOffset;
		} 
		else {
		  BiosBuffer = ((UINT8*) BiosSection) + sizeof(EFI_COMMON_SECTION_HEADER);
		  BiosSize -= sizeof(EFI_COMMON_SECTION_HEADER);
		}

		if (CompareGuid(gPlatformOpromTable[Index].FileGuid,&gSataAhciRomGuid)) {
		  Status = CheckAndPatchSataAhciRom (BiosBuffer, BiosSize);
		  if(EFI_ERROR(Status))
			return EFI_NOT_FOUND;
		}

	        *RomImage = (VOID*) BiosBuffer;
		*RomSize = BiosSize;
		DEBUG((EFI_D_INFO, "%a Image loaded %p len %p\n", __FUNCTION__, *RomImage,  *RomSize ));
		return EFI_SUCCESS;
	      } // if(BiosSection) 

	    } // if(... == RomFileLoadFromFv )

	    if(gPlatformOpromTable[Index].Attribute == RomFileLoadFromFv) {
  PHYSICAL_ADDRESS BaseAddress;
  PHYSICAL_ADDRESS Length;
        
	      if(gPlatformOpromTable[Index].CustomGetRom != NULL) {

		Status = gPlatformOpromTable[Index].CustomGetRom(Seg, Bus, Dev, Func, &BaseAddress, &Length);
		if ((Status == EFI_SUCCESS) && 
				CompareGuid(gPlatformOpromTable[Index].FileGuid, &gSataAhciRomGuid)) {
		  CheckAndPatchSataAhciRom ((UINT8*) (UINTN) BaseAddress, (UINTN)Length);
		}

		*RomImage = (VOID*) (UINTN) BaseAddress;
		*RomSize = (UINTN) Length;
		return Status;
	      } // if(gPlatformOpromTable[Index].CustomGetRom != NULL)
	    } // if(.. == RomFileLoadFromFv) 
	  } // if((.. == VendorId)
	  Index++;    
	} // while

	return EFI_NOT_FOUND;
}



EFI_PCI_PLATFORM_PROTOCOL mPciPlatform = {
    PciPlatformPhaseNotify,
    PciPlatformPreprocessController,
    PciPlatformGetPlatformPolicy,
    PciPlatformGetPciRom    
};

/**
  The module Entry Point.

  
  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval Others            Some error occurs when executing this entry point.

**/

EFI_STATUS
EFIAPI
PciPlatformEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;
  
  Handle = NULL;
  Status = gBS->InstallProtocolInterface(
      &Handle,
      &gEfiPciPlatformProtocolGuid,
      EFI_NATIVE_INTERFACE,
      &mPciPlatform
      );

  return Status;
}

