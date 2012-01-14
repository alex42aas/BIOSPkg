/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

﻿/** @file
  This driver produces Ibex peak policy protocol.


**/
#include "EdkIIGlueDxe.h"
#include "Common\EdkIIGlueDependencies.h"


#include "Protocol\GlobalNvsArea\GlobalNvsArea.h"
#include "Protocol\PpmGlobalNvsArea\PpmGlobalNvsArea.h"

#include "PchRegsLpc.h"
#include "AcpiGnvsInitLib.h"
#include "PchRegsThermal.h"

#include <CommonAddresses.h>
#include <PlatformDataLib.h>

#include <Setup.h>
#include <SaGlobalNvsArea\SaGlobalNvsArea.h>


#include <RedirectDebugLib.h>

#include <Protocol/Smbios.h>
//#include <MdePkg/Include/IndustryStandard/Smbios.h>
#define SMBIOS_HANDLE_PI_RESERVED 0xFFFE

EFI_GUID  gVendorGuid			= { 0x2fb3abd9, 0x65fb, 0x4db4, 0x87, 0x2, 0x42, 0xdc, 0xa4, 0x55, 0xa8, 0x12 };
EFI_GUID  gEfiSmbiosProtocolGuid	= EFI_SMBIOS_PROTOCOL_GUID;
EFI_GUID  gSaGlobalNvsAreaProtocolGuid	= SYSTEM_AGENT_GLOBAL_NVS_AREA_PROTOCOL_GUID;


EFI_BOOT_SERVICES  *gBS;


// Protocols that are installed 
EFI_GLOBAL_NVS_AREA_PROTOCOL      mGlobalNvsArea        = { 0 };


EFI_STATUS
ObtainSetupEfiVar(
  IN OUT SETUP **SetupPtr
  )
{
  STATIC SETUP *Setup;
  EFI_STATUS Status;
  UINTN Size;

  if (gRT == NULL) {
    DEBUG((EFI_D_ERROR, "%a.%d Error!\n", __FUNCTION__, __LINE__));
    return EFI_ABORTED;
  }

  Size = 0;

  if (Setup != NULL) {
    FreePool(Setup);
    Setup = NULL;
  }
  
  Status = gRT->GetVariable(SETUP_VARIABLE_NAME, &gVendorGuid, NULL,
    &Size, NULL);
  if (Status == EFI_NOT_FOUND) {
    *SetupPtr = NULL;
    return Status;
  }
  Setup = (SETUP *)AllocateZeroPool(Size);
  if (Setup == NULL) {
    *SetupPtr = NULL;
    return EFI_OUT_OF_RESOURCES;
  }
  Status = gRT->GetVariable(SETUP_VARIABLE_NAME, &gVendorGuid, NULL,
    &Size, Setup);
  *SetupPtr = Setup;
  return Status;
}

//===================================================================================

EFI_STATUS
InstallGlobalNvsAreaProtocol(
    VOID
    )
{
EFI_STATUS Status;
EFI_HANDLE Handle;

	Status = AcpiGnvsInit (&mGlobalNvsArea.Area, gBS);
	DEBUG((EFI_D_INFO, "\r\nPlatformDxe: InstallGlobalNvsAreaProtocol: AcpiGnvsInit status = %x", Status));
	ASSERT_EFI_ERROR (Status);
	//
	// Install protocols to to allow access to this Policy.
	//
	Handle = NULL;
	Status = gBS->InstallMultipleProtocolInterfaces (
		&Handle,
		&gEfiGlobalNvsAreaProtocolGuid,
		&mGlobalNvsArea,
		NULL
		);
	DEBUG((EFI_D_INFO, "\r\nPlatformDxe: InstallGlobalNvsAreaProtocol: install status = %x, Area = %llx", Status, mGlobalNvsArea.Area));
	ASSERT_EFI_ERROR (Status);

	return Status;  
}


EFI_STATUS	InstallPlatformRegions( VOID );

#define  SIZE_4GB    0x0000000100000000ULL

EFI_STATUS 
InstallPlatformRegions(
    VOID
    )
{
EFI_STATUS	Status;

	DEBUG((EFI_D_INFO, "\r\nInstallPlatformRegions: entry"));

  
	// 
	// Reserve PciExpress.
	// 
	Status = gDS->SetMemorySpaceAttributes ( 0xF8000000, 0x04000000, EFI_MEMORY_UC | EFI_MEMORY_RUNTIME );

	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "\r\nInstallPlatformRegions: SetMemorySpaceAttributes 0xF8000000 status = %x", Status));
	return Status;
	}

	// RCBA, MMIO:
	Status = gDS->SetMemorySpaceAttributes ((EFI_PHYSICAL_ADDRESS)0xFED00000, (UINT64)0x00100000, EFI_MEMORY_UC | EFI_MEMORY_RUNTIME );
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "\r\nInstallPlatformRegions: SetMemorySpaceAttributes 0xFED00000 status = %x", Status));
	return Status;
	}

	// APIC:
	Status = gDS->AddMemorySpace( EfiGcdMemoryTypeMemoryMappedIo, (EFI_PHYSICAL_ADDRESS)0xFEE00000, (UINT64)0x00001000, EFI_MEMORY_UC );
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "\r\nInstallPlatformRegions: AddMemorySpace 0xFEE00000 status = %x", Status));
	return Status;
	}
	Status = gDS->SetMemorySpaceAttributes ((EFI_PHYSICAL_ADDRESS)0xFEE00000, (UINT64)0x00001000, EFI_MEMORY_UC | EFI_MEMORY_RUNTIME );
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "\r\nInstallPlatformRegions: SetMemorySpaceAttributes 0xFEE00000 status = %x", Status));
	return Status;
	}
	
	Status = gDS->SetMemorySpaceAttributes ((EFI_PHYSICAL_ADDRESS)0xFF800000, (UINT64)0x00800000, EFI_MEMORY_UC | EFI_MEMORY_RUNTIME );
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "\r\nInstallPlatformRegions: SetMemorySpaceAttributes 0xFF800000 status = %x", Status));
	return Status;
	}

	DEBUG((EFI_D_INFO, "\r\nInstallPlatformRegions: AddMemory.."));
{
EFI_PEI_HOB_POINTERS	ResHob;

	// Add upper memory, if exist.
	ResHob.Raw = GetHobList ();

	DEBUG((EFI_D_INFO, "\r\nInstallPlatformRegions: GetHobList = %llx", (UINT32)(UINTN)ResHob.Raw));
	if(ResHob.Raw == NULL)
	{
	  return EFI_INVALID_PARAMETER;
	}

	while ((ResHob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, ResHob.Raw)) != NULL) {
	  DEBUG((EFI_D_INFO, "\r\nInstallPlatformRegions: GetNextHob %llx OK", (UINT32)(UINTN)ResHob.Raw));
	  if((ResHob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) &&
			(ResHob.ResourceDescriptor->PhysicalStart >= SIZE_4GB) ) 
	  {
	    gDS->RemoveMemorySpace( ResHob.ResourceDescriptor->PhysicalStart, 
							ResHob.ResourceDescriptor->ResourceLength);

	    if ((ResHob.ResourceDescriptor->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE)) 
	    {

	      Status = gDS->AddMemorySpace(EfiGcdMemoryTypeSystemMemory, ResHob.ResourceDescriptor->PhysicalStart, ResHob.ResourceDescriptor->ResourceLength,
									EFI_MEMORY_UC | EFI_MEMORY_WC | EFI_MEMORY_WT | EFI_MEMORY_WB | EFI_MEMORY_UCE );

	      if(EFI_ERROR(Status)) 
	      {
		DEBUG((EFI_D_ERROR, "\r\nInstallPlatformRegions: AddMemorySpace status = %x", Status));
		break;
	      }

	      Status = gDS->SetMemorySpaceAttributes( ResHob.ResourceDescriptor->PhysicalStart, ResHob.ResourceDescriptor->ResourceLength,
										EFI_MEMORY_WB | EFI_MEMORY_RUNTIME );

	      if(EFI_ERROR(Status)) 
	      {
		DEBUG((EFI_D_ERROR, "\r\nInstallPlatformRegions: SetMemorySpaceAttr status = %x", Status));
		break;	
	      }
	      DEBUG((EFI_D_INFO, "\r\nInstallPlatformRegions: above 4Gb hob size = %llx", ResHob.ResourceDescriptor->ResourceLength));
	    }  // if(... CACHEABLE )
	    else 
	    {
	      Status = gDS->AddMemorySpace( EfiGcdMemoryTypeReserved, ResHob.ResourceDescriptor->PhysicalStart, ResHob.ResourceDescriptor->ResourceLength,
									EFI_MEMORY_UC );
	      DEBUG((EFI_D_INFO, "\r\nInstallPlatformRegions: AddMemorySpace status = %x", Status));
	    } // else(.. CACHABLE) == UNCACHABLE
	  }

	  ResHob.Raw = GET_NEXT_HOB (ResHob);
	} // while(Hob)
}

	DEBUG((EFI_D_INFO, "\r\nInstallPlatformRegions: exit with status = %x", Status));

	return Status;
}

#ifdef	SMBIOS_SUPPORT

int	SmbiosStructureTypes[] = {
				2,	// Baseboard
				3,	// System Enclosure (Chassic)
				8,	// Port Connector
				10,	// OnboardDevice
				11,	// OEM String
				12,	// System Configuration Options
				-1	// конец таблицы
				};

VOID	AddSmbiosStructures (VOID) {
EFI_STATUS		Status;
EFI_SMBIOS_PROTOCOL     *Smbios;
EFI_SMBIOS_HANDLE	SmbiosHandle;
int			Index;
void			**struct_table = structTable;
EFI_SMBIOS_TABLE_HEADER *Record;


	DEBUG((EFI_D_INFO, "\r\n%a.%d: entry", __FUNCTION__, __LINE__));
	//
	// Initialize the mSmbios to contain the SMBIOS protocol,
	//
	Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &Smbios);
	if(EFI_ERROR(Status))
	{
	  DEBUG((EFI_D_ERROR, "\r\n%a.%d: locate EfiSmbiosProtocol failed", __FUNCTION__, __LINE__));
	}
	ASSERT_EFI_ERROR (Status);

	

	while(1)
	{ // перебираем все структуры, присутствующие в нашей таблице structTable[]:
	  Record = *struct_table;
	  if(Record == NULL)
				break;
	  Index = 0;
	  while(1)
	  { // проверяем, присутствует ли тип структуры в нашем массиве SmbiosStructureTypes[]:
	    if(SmbiosStructureTypes[Index] < 0)
						break;
	    if(SmbiosStructureTypes[Index] == Record->Type)
	    { // добавляем очередную структуру в таблицу SMBIOS:
	      SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
	      Status = Smbios->Add (Smbios, NULL, &SmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *) Record);
	      if(EFI_ERROR(Status))
	      {
	        DEBUG((EFI_D_ERROR, "\r\n%a.%d: add structure failed (type %d)", __FUNCTION__, __LINE__, Record->Type));
	      }
	      else
	      {
	        DEBUG((EFI_D_INFO, "\r\n%a.%d: add structure (type %d) OK, handle = %d", __FUNCTION__, __LINE__, Record->Type, SmbiosHandle));
	      }
	    } // if(.. == Record->Type)
            Index++;
	  } // while
	  struct_table++;
	  
	} // while

}

#endif	/* SMBIOS_SUPPORT */

/**
  The user Entry Point for this module.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval Others            Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PlatformEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
EFI_STATUS         Status;
UINTN              Index;
UINT32             GpioBase;

	DEBUG((EFI_D_INFO, "\r\nPlatformDxe: entry"));

	// setup gBs:
	UefiBootServicesTableLibConstructor(ImageHandle, SystemTable);
	ASSERT (gBS != NULL);
	DxeServicesTableLibConstructor(ImageHandle, SystemTable);
	ASSERT (gDS != NULL);

	Status = HobLibConstructor (ImageHandle, SystemTable);
	ASSERT(!EFI_ERROR(Status));
  
// Thermal:
	PciWrite32(PCI_LIB_ADDRESS(0, PCI_DEVICE_NUMBER_PCH_THERMAL, PCI_FUNCTION_NUMBER_PCH_THERMAL, R_PCH_THERMAL_TBARBH ), 0);
	PciWrite32(PCI_LIB_ADDRESS(0, PCI_DEVICE_NUMBER_PCH_THERMAL, PCI_FUNCTION_NUMBER_PCH_THERMAL, R_PCH_THERMAL_TBARB ), THERMAL_BASEB | 0x4);

	DEBUG((EFI_D_INFO, "\r\nPlatformDxe: InstallGlobalNvsAreaProtocol.."));
	Status = InstallGlobalNvsAreaProtocol();
	DEBUG((EFI_D_INFO, "\r\nPlatformDxe: InstallGlobalNvsAreaProtocol status = %x", Status));

	Status = InstallPlatformRegions();
	ASSERT (!EFI_ERROR(Status));

#ifdef	SMBIOS_SUPPORT
	AddSmbiosStructures();
#endif	/* SMBIOS_SUPPORT */

	//	GPIO:
	PciOr8(PCI_LIB_ADDRESS(0,
			 PCI_DEVICE_NUMBER_PCH_LPC, 
			 PCI_FUNCTION_NUMBER_PCH_LPC, 
			 R_PCH_LPC_GPIO_CNT), 
				B_PCH_LPC_GPIO_CNT_GPIO_EN);
	GpioBase = (PciRead32(PCI_LIB_ADDRESS(0, 
					PCI_DEVICE_NUMBER_PCH_LPC, 
					PCI_FUNCTION_NUMBER_PCH_LPC, 
					R_PCH_LPC_GPIO_BASE)) 
							& (~1)) & 0xFFFF;
 
	for(Index = 0; Index < GPIO_DATA_SIZE; Index++) {
		IoWrite32(GpioBase + Index * 4, mGpioData[Index]);
	}


	return Status;
}
