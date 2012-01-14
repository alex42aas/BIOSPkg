/** @file
  This contains the installation function for the driver.

Copyright (c) 2005 - 2009, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "LegacyBiosPlatform.h"
#include <Guid/LegacyBios.h>
#include <Library/PciLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseLib.h>
#include <Protocol/PciIo.h>
#include <Protocol/UsbIo.h>
#include <Protocol/VgaMiniPort.h>
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Smbios.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/LegacyRegion.h>
#include <Protocol/SmmAccess.h>
#include <Protocol/UsbLegacyInitProtocol.h>
#include <Setup.h>

#include <PchRegsRcrb.h>
#include <PlatformDataLib.h>

#define  MAX_REGIONS_NUM    4

EFI_STATUS
LoadRom(
    EFI_GUID        *FileName,
    UINT32          RomAddress,
    UINT32          RomSize
    );

//
// Global for the Legacy BIOS Platform Protocol that is produced by this driver
//

EFI_LEGACY_BIOS_PLATFORM_PROTOCOL mLegacyBiosPlatform = {
  GetPlatformInfo,
  GetPlatformHandle,
  SmmInit,
  PlatformHooks,
  GetRoutingTable,
  TranslatePirq,
  PrepareToBoot
};

USB_LEGACY_INIT_PROTOCOL mUsbLegacyInitProtocol = {
  NULL,     // StartEhciDxe
  NULL,     // StopEhciDxe
  NULL,     // EhciDxeImageHandle
  NULL,     // EhciDxeSystemTable
  NULL,     // StartUsbBusDxe
  NULL,     // UsbBusDxeImageHandle
  NULL,     // UsbBusDxeSystemTable
  NULL,     // StartUsbKbDxe
  NULL,     // UsbKbDxeImageHandle
  NULL,     // UsbKbDxeSystemTable
  NULL,     // EhcMonitorAsyncRequests
  0,        // Flags
  NULL,     // Ehc[0]
  NULL      // Ehc[1]
};

//
// Global for the handle that the Legacy Interrupt Protocol is installed
//

EFI_HANDLE                    mLegacyBiosPlatformHandle = NULL;
EFI_HANDLE                    mUsbLegacyInitProtocolHandle = NULL;
    
//------------------------------
// legacy bios platform
//------------------------------

/**
Routine Description:
  Finds the binary data or other platform information.
Arguments:
  This - Indicates the EFI_LEGACY_BIOS_PLATFORM_PROTOCOL instance.
  Mode - Specifies what data to return.
	GetMpTable
	GetOemIntData
	GetOem16Data
	GetOem32Data
	GetTpmBinary
	GetSystemRom
	GetPciExpressBase
	GetPlatformPmmSize
	GetPlatformEndOpromShadowAddr
  Table - Pointer to OEM legacy 16-bit code or data.
  TableSize - Size of data.
  Location - Location to place table. 0x00 – Either 0xE0000 or 0xF0000 64 KB blocks.
	Bit 0 = 1 0xF0000 64 KB block.
	Bit 1 = 1 0xE0000 64 KB block.
	Multiple bits can be set.
  Alignment - Bit-mapped address alignment granularity. The first nonzero bit from the right is the address granularity.
  LegacySegment - Segment where EfiCompatibility code will place the table or data.
  LegacyOffset - Offset where EfiCompatibility code will place the table or data.
Returns: 
  EFI_SUCCESS - The data was returned successfully.
  EFI_UNSUPPORTED - Mode is not supported on this platform.
  EFI_NOT_FOUND - Binary image not found.

**/
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
)
{
UINTN pTable;

  switch (Mode)
  {
  case EfiGetPlatformBinaryMpTable: // Returns the multiprocessor (MP) table information
    return EFI_UNSUPPORTED;           // No support yet
    break;

  case EfiGetPlatformBinaryOemIntData:  // Returns any OEM-specific code and/or data.
    return EFI_UNSUPPORTED;               // No support yet
    break;

  case EfiGetPlatformBinaryOem16Data:   // Returns any 16-bit OEM-specific code and/or data.
    return EFI_UNSUPPORTED;               // No support yet
    break;

  case EfiGetPlatformBinaryOem32Data:   // Returns any 32-bit OEM-specific code and/or data.
    return EFI_UNSUPPORTED;               // No support yet
    break;

  case EfiGetPlatformBinaryTpmBinary:   // Gets the TPM (Trusted Platform Module) binary image associated with the onboard TPM device.
    return EFI_UNSUPPORTED;               // No support yet
    break;

  case EfiGetPlatformBinarySystemRom:   // Finds the Compatibility16 “ROM".
                                        // Should fill the Table & TableSize variables with ROM image

    pTable = 0x000F0000;
    *Table = (UINTN *)pTable;
    *TableSize  = SIZE_64KB;

    LoadRom(&gLegacyBiosRomGuid, 0xF0000, SIZE_64KB); 

    return EFI_SUCCESS;
    break;

  case EfiGetPlatformPciExpressBase:  // Gets the PciExpress base address
    return EFI_UNSUPPORTED;             // No support yet
    break;

  case EfiGetPlatformPmmSize:
    return EFI_UNSUPPORTED;

  case EfiGetPlatformEndOpromShadowAddr:
    *Location = 0xDFFFF;
    return EFI_SUCCESS;

  default:
    return EFI_SUCCESS;
    break;
  }
}


static EFI_STATUS
ObtainSetupEfiVar(
  IN OUT SETUP **SetupPtr
  )
{
  static SETUP *Setup;
  EFI_STATUS Status;
  UINTN Size;
  extern EFI_RUNTIME_SERVICES  *gRT; 

  if (gRT == NULL) {
    DEBUG((EFI_D_ERROR, "\r\n%a.%d Error!\n", __FUNCTION__, __LINE__));
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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
Routine Description:
  Returns a buffer of handles for the requested sub-function.
Arguments:
  This - Indicates the EFI_LEGACY_BIOS_PLATFORM_PROTOCOL instance.
  Mode - Specifies what handle to return.
	GetVgaHandle
	GetIdeHandle
	GetIsaBusHandle
	GetUsbHandle
  Type - Handle Modifier – Mode specific
  HandleBuffer - Pointer to buffer containing all Handles matching the specified criteria. Handles are sorted in priority order.
  HandleCount - Number of handles in HandleBuffer.
  AdditionalData - Pointer to additional data returned – mode specific.
Returns: 
  EFI_SUCCESS - The handle is valid.
  EFI_UNSUPPORTED - Mode is not supported on this platform.
  EFI_NOT_FOUND - The handle is not known.

**/
EFI_STATUS
EFIAPI
GetPlatformHandle (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN EFI_GET_PLATFORM_HANDLE_MODE Mode,
  IN UINT16 Type,
  OUT EFI_HANDLE **HandleBuffer,
  OUT UINTN *HandleCount,
  OUT VOID OPTIONAL **AdditionalData
)
{
  EFI_PCI_IO_PROTOCOL       *PciIo;
  EFI_STATUS                Status;
  UINTN                     LocalHandleCount;
  EFI_HANDLE                *LocalHandleBuffer;

  UINTN     Index;
  UINTN     ExitIndex;
  UINT8     PciClass;
  UINT8     PciSubclass;
  UINT8     PciProgrammInterface;
  UINTN     PciSegment;
  UINTN     PciBus;
  UINTN     PciDevice;
  UINTN     PciFunction;

#ifndef INT13_SMM_HANDLER
  UINT8     PciIntLine;
  UINT8     Sirq;
  UINT16    PciTemp16;
  HDD_INFO  *LocalHddInfo;
  UINT32    PciBootableMassStorageController;
#endif	// INT13_SMM_HANDLER

  EFI_HANDLE *ResultBuffer;

  DEBUG((EFI_D_INFO, "\r\n%a.%d: entry\n", __FUNCTION__, __LINE__));

  switch (Mode) {  
  case EfiGetPlatformVgaHandle: // Returns the handle for the VGA device that should be used during a Compatibility16 boot.
    DEBUG ((EFI_D_INFO, "EfiGetPlatformVgaHandle...\n"));
    
    LocalHandleCount = 0;
    LocalHandleBuffer = NULL;
    ExitIndex = 0;

    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiPciIoProtocolGuid,
                    NULL,
                    &LocalHandleCount,
                    &LocalHandleBuffer
                    );

    if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Error in locating handle buffer\n"));
        return Status;
    }

    *HandleBuffer = AllocatePool (sizeof(EFI_HANDLE) * LocalHandleCount);
    if (*HandleBuffer == NULL) {
      if (LocalHandleBuffer != NULL) {
        FreePool (LocalHandleBuffer);
      }
      DEBUG((EFI_D_ERROR, "Unsufficient memory to allocate handle\n"));
      return EFI_ABORTED;
    }
    ResultBuffer = *HandleBuffer;
    
    for (Index=0;Index < LocalHandleCount;Index++) {
      Status = gBS->HandleProtocol (
                  LocalHandleBuffer[Index],
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo
                  );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Error in handling protocol %x\n", Index));
        continue;
      }
      
      Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_CLASSCODE_OFFSET + 2,
                        1,
                        &PciClass
                        );
      Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_CLASSCODE_OFFSET + 1,
                        1,
                        &PciSubclass
                        );
      Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_CLASSCODE_OFFSET,
                        1,
                        &PciProgrammInterface
                        );
      if (
          (PciClass == 0x03) && (PciSubclass == 0x00) && (PciProgrammInterface == 0x00) ||
          (PciClass == 0x00) && (PciSubclass == 0x01) && (PciProgrammInterface == 0x01)
         ) {
        // VGA-compatible display controller
        DEBUG((EFI_D_INFO, "VGA display controller found\n"));

        PciIo->GetLocation (
           PciIo,
           &PciSegment,
           &PciBus,
           &PciDevice,
           &PciFunction
           );
        DEBUG ((EFI_D_INFO, "PciSegment = %x PciBus = %x PciDevice = %x PciFunction = %x\n", PciSegment, PciBus, PciDevice, PciFunction));
        DEBUG ((EFI_D_INFO, "PciClass = %x PciSubclass = %x PciProgrammInterface = %x\n", PciClass, PciSubclass, PciProgrammInterface));
	// возвращаем первый найденный контроллер, так как:
	// - если IGD активен(включен), он будет найден первым;
	// - если IGD выключен, то его в списке LocalHandleBuffer[] не окажется и будет найден внешний видеоконтроллер;
	// - случай с двумя внешними контроллерами пока не рассматриваем.
	DEBUG ((EFI_D_INFO, "VGA found and added\n"));
        ResultBuffer[ExitIndex] = LocalHandleBuffer[Index];
        ExitIndex++;
        break;
      }
    }

    *HandleCount = ExitIndex;

    if (LocalHandleBuffer != NULL) {
      FreePool (LocalHandleBuffer);
    }    
    

    if (*HandleCount > 0) {
      DEBUG ((EFI_D_INFO, "GetVgaHandle return %x handle(s)\n", *HandleCount));
      return EFI_SUCCESS;
    } else {
      DEBUG ((EFI_D_INFO, "GetVgaHandle exit with error\n"));      
      return EFI_UNSUPPORTED; // No handles return
    }
  break;

//****************************************************************************

  case EfiGetPlatformIdeHandle:   // Returns the handle for the IDE controller that should be used during a Compatibility16 boot.
    DEBUG ((EFI_D_INFO, "EfiGetPlatformIdeHandle...\n"));


    LocalHandleCount = 0;
    LocalHandleBuffer = NULL;
#ifndef INT13_SMM_HANDLER
    ExitIndex = 0;
    PciBootableMassStorageController = PcdGet32 (BootableMassStorageController);
    
    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiPciIoProtocolGuid,
                    NULL,
                    &LocalHandleCount,
                    &LocalHandleBuffer
                    );

    if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Error in locating handle buffer\n"));
        return Status;
    }
    DEBUG ((EFI_D_INFO, "%a.%d LocalHandleCount=%d\n", __FUNCTION__, 
      __LINE__, LocalHandleCount));
    *HandleBuffer = AllocatePool (sizeof(EFI_HANDLE) * LocalHandleCount);
    if (*HandleBuffer == NULL) {
      DEBUG((EFI_D_ERROR, "Unsufficient memory to allocate handle\n"));
      return EFI_ABORTED;
    }
    ResultBuffer = *HandleBuffer;

    for (Index = 0; Index < LocalHandleCount; Index++) {
      Status = gBS->HandleProtocol (
                    LocalHandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Error in handling protocol %x\n", Index));
        continue;
      }

      PciIo->GetLocation (
        PciIo,
        &PciSegment,
        &PciBus,
        &PciDevice,
        &PciFunction
        );

      // search for Primary SATA controller
      if ((PciSegment == (UINT8) (PciBootableMassStorageController>>24)) && (PciBus == (UINT8) (PciBootableMassStorageController>>16)) 
			&& (PciDevice == (UINT8) (PciBootableMassStorageController>>8)) && (PciFunction == (UINT8) (PciBootableMassStorageController)) ) {      
        // get it!
        if (AdditionalData != NULL) {
          LocalHddInfo = *AdditionalData;

        }

        // commented 02.02.2012 - skip for Compact Flash
        Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_CLASSCODE_OFFSET,
                        1,
                        &PciProgrammInterface
                        );
#if 1
        PciProgrammInterface &= 0xfa;       // set SATA controller to legacy mode
#else
        PciProgrammInterface |= 0x05;       // set SATA controller to native mode
#endif
        Status = PciIo->Pci.Write (
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_CLASSCODE_OFFSET,
                        1,
                        &PciProgrammInterface
                        );
        

        // Set IntLine
        PciIo->Pci.Read (
                  PciIo,
                  EfiPciIoWidthUint8,
                  0x3c,
                  1,
                  &PciIntLine
                  );
        DEBUG ((EFI_D_INFO, "PciIdeIntLine = %x\n", PciIntLine));
        
        Sirq = PciRead8(PCI_LIB_ADDRESS(0x00,0x1f,0x00,0x64));
        DEBUG((EFI_D_INFO, "SIrq %x\n", Sirq));
       
        PciTemp16 = PciRead16(PCI_LIB_ADDRESS(0x00, 0x1f, 0x02, 0x00));
        DEBUG((EFI_D_INFO, "PciSataVendorId %x\n", PciTemp16));
        PciTemp16 = PciRead16(PCI_LIB_ADDRESS(0x00, 0x1f, 0x02, 0x02));
        DEBUG((EFI_D_INFO, "PciSataDeviceId %x\n", PciTemp16));

        ResultBuffer[ExitIndex] = LocalHandleBuffer[Index];
        ExitIndex++;
      }
    }

    if (LocalHandleBuffer != NULL) {
      FreePool (LocalHandleBuffer);
    }    

    *HandleCount = ExitIndex;  

    if (*HandleCount > 0) {
      DEBUG ((EFI_D_INFO, "GetIdeHandle return %x handle(s)\n", *HandleCount));
    } else {
      DEBUG ((EFI_D_INFO, "GetIdeHandle didn't find appropriate handles\n"));
      return EFI_UNSUPPORTED;   // no handles found
    }

    return EFI_SUCCESS;       // return OK

#else

    return EFI_NOT_FOUND;  

#endif		// INT13_SMM_HANDLER
    break;

  case EfiGetPlatformIsaBusHandle:    // Returns the handle for the ISA bus controller that should be used during a Compatibility16 boot.
    DEBUG ((EFI_D_INFO, "EfiGetPlatformIsaBusHandle...\n"));
    return EFI_UNSUPPORTED;           // No support yet
    break;

  case EfiGetPlatformUsbHandle:       // Returns the handle for the USB device that should be used during a Compatibility16 boot.
    DEBUG ((EFI_D_INFO, "EfiGetPlatformUsbHandle...\n"));
    return EFI_UNSUPPORTED;           // No support yet
    break;

  default:
    DEBUG ((EFI_D_ERROR, "%a.%d: undefined Mode 0x%x\n", __FUNCTION__, __LINE__, Mode));
    return EFI_SUCCESS;
    break;
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
Routine Description:
  Loads and registers the Compatibility16 handler with the EFI SMM code.
Arguments:
  This - Indicates the EFI_LEGACY_BIOS_PLATFORM_PROTOCOL instance.
  EfiToCompatibility16BootTable - The boot table passed to the Compatibility16. Allows the SmmInit() function to update EFI_TO_COMPATIBILITY16_BOOT_TABLE.SmmTable.
Returns: 
  EFI_SUCCESS - The SMM code loaded.
  EFI_DEVICE_ERROR - The SMM code failed to load.

**/
EFI_STATUS
EFIAPI
SmmInit (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN VOID *EfiToCompatibility16BootTable
)
{
  EFI_STATUS  Status;
  SETUP       *pSetup;
  BOOLEAN     UsbLegacyEnable = FALSE;

  DEBUG((EFI_D_INFO, "\r\n%a.%d: entry\n", __FUNCTION__, __LINE__));
  Status = ObtainSetupEfiVar(&pSetup);

  if (!EFI_ERROR(Status) && pSetup != NULL && (pSetup->Flags & SETUP_FLAG_USB_LEGACY_ENABLE)) {
    UsbLegacyEnable = TRUE;
  }

  if (UsbLegacyEnable) {
    // run EhciRuntimeDxe
    if (mUsbLegacyInitProtocol.StartEhciDxe != NULL) {
      DEBUG((EFI_D_INFO, "Start EhciRuntimeDxe\n"));
      Status = mUsbLegacyInitProtocol.StartEhciDxe(mUsbLegacyInitProtocol.EhciDxeImageHandle, mUsbLegacyInitProtocol.EhciDxeSystemTable);
      if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
      }
    }

    // run UsbIoRuntimeDxe
    if (mUsbLegacyInitProtocol.StartUsbBusDxe != NULL) {
      DEBUG((EFI_D_INFO, "Start UsbIoRuntimeDxe\n"));    
      Status = mUsbLegacyInitProtocol.StartUsbBusDxe(mUsbLegacyInitProtocol.UsbBusDxeImageHandle, mUsbLegacyInitProtocol.UsbBusDxeSystemTable);
      if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
      }
    }

    // run UsbKbRuntimeDxe
    if (mUsbLegacyInitProtocol.StartUsbKbDxe != NULL) {
      DEBUG((EFI_D_INFO, "Start UsbKbRuntimeDxe\n"));    
      Status = mUsbLegacyInitProtocol.StartUsbKbDxe(mUsbLegacyInitProtocol.UsbKbDxeImageHandle, mUsbLegacyInitProtocol.UsbKbDxeSystemTable);
      if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
      }
    }

    // Set USB up for legacy operation
    PciOr32 (PCI_LIB_ADDRESS(0x00,0x1f,0x00,0x94),0x00000010);    // set SMI on USB IRQ Enable (USBSMIEN)
  
    PciOr32(PCI_LIB_ADDRESS(0x00,0x1a,0x00,0x6c), 0x00000001);    // Set USB SMI generate on Complete
    PciOr32(PCI_LIB_ADDRESS(0x00,0x1d,0x00,0x6c), 0x00000001);  

    PciOr32(PCI_LIB_ADDRESS(0x00,0x1a,0x00,0x6c), 0x00002000);    // Set USB SMI generate on ownership change
    PciOr32(PCI_LIB_ADDRESS(0x00,0x1d,0x00,0x6c), 0x00002000);  

    // Set UsbLegacyEnable bit in UsbLegacyInitProtocol
    mUsbLegacyInitProtocol.Flags |= USB_LEGACY_ENABLED;
  }

  DEBUG((EFI_D_INFO, "%a.%d: exit OK\n", __FUNCTION__, __LINE__));

  return EFI_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
Routine Description:
  Allows platform to perform any required action after a LegacyBios operation.
Arguments:
  This - Indicates the EFI_LEGACY_BIOS_PLATFORM_PROTOCOL instance.
  Mode - Specifies what handle to return.
	PrepareToScanRom
	ShadowServiceRoms
	AfterRomInit
  Type - Mode specific.
  DeviceHandle - List of PCI devices in the system.
  Shadowaddress - First free OpROM area, after other OpROMs have been dispatched.
Returns: 
  EFI_SUCCESS - The operation performed successfully.
  EFI_UNSUPPORTED - Mode is not supported on this platform.
  EFI_SUCCESS - Mode specific.

**/
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
)
{

  EFI_SMM_ACCESS_PROTOCOL         *mSmmAccess;
  EFI_STATUS                      Status;
  UINTN                           Index;
  EFI_SMRAM_DESCRIPTOR            SmramDescrioptors[MAX_REGIONS_NUM];
  UINTN                           SmramMapSize;
  BOOLEAN                         bFound;
  UINTN                           mRegionIndex = 0;

  DEBUG((EFI_D_INFO, "\r\n%a.%d: entry\n", __FUNCTION__, __LINE__));

  switch (Mode)
  {
  case EfiPlatformHookPrepareToScanRom:	// Allows any preprocessing before scanning OpROMs.

    DEBUG ((EFI_D_INFO, "EfiPlatformHookPrepareToScanRom..\n"));
    
    // Close SMRAM window in AB legacy memory
    Status = gBS->LocateProtocol (
                  &gEfiSmmAccessProtocolGuid,
                  NULL,
                  &mSmmAccess
                  );
    ASSERT_EFI_ERROR(Status);

    SmramMapSize = sizeof(SmramDescrioptors);
    Status = mSmmAccess->GetCapabilities( 
                  mSmmAccess,
                  &SmramMapSize,
                  &SmramDescrioptors[0]
                  );
    ASSERT_EFI_ERROR(Status);

    //
    // Search for AB segment
    //
    bFound = FALSE;
    for (Index = 0; Index < SmramMapSize/sizeof(EFI_SMRAM_DESCRIPTOR); Index++) {
      if(SmramDescrioptors[Index].PhysicalStart < 0x100000) {
        mRegionIndex = Index;
        bFound = TRUE;
        break;
      }
    }

    //ASSERT(bFound == TRUE);

    if (bFound) {
      Status = mSmmAccess->Close(mSmmAccess, mRegionIndex);
    }

    // Set PciIdeTimeReg for SataControllers to enable IDE decode
    PciOr32(PCI_LIB_ADDRESS(0x00,0x1f,0x02,0x40), BIT15);
    PciOr32(PCI_LIB_ADDRESS(0x00,0x1f,0x05,0x40), BIT15);

    break;

  case EfiPlatformHookShadowServiceRoms:	// Shadows legacy OpROMS that may not have a physical device associated with them. Examples are PXE base code and BIS.
    DEBUG ((EFI_D_INFO, "EfiPlatformHookShadowServiceRoms: UNSUPPORTED\n"));
	return EFI_UNSUPPORTED;				// No support yet
	break;
	
  case EfiPlatformHookAfterRomInit:		// Allows platform to perform any required operation after an OpROM has completed its initialization.
    DEBUG ((EFI_D_INFO, "EfiPlatformHookAfterRomInit: UNSUPPORTED\n"));
	return EFI_UNSUPPORTED;				// No support yet
	break;

  default:
    DEBUG ((EFI_D_ERROR, "%a.%d: undefined hook 0x%x\n", __FUNCTION__, __LINE__, Mode));
	break;
  }

	return EFI_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef	struct {
int	Bus;
int	Device;
int	rcbaOffset;
}	DXXR;

// адреса регистров В RCBA, описывающих соответствие INTx -> PIRQy
DXXR	DxxR[] = {
	{0, 31, R_PCH_RCRB_D31IR},
	{0, 29, R_PCH_RCRB_D29IR},
	{0, 28, R_PCH_RCRB_D28IR},
	{0, 27, R_PCH_RCRB_D27IR},
	{0, 26, R_PCH_RCRB_D26IR},
	{0, 25, R_PCH_RCRB_D25IR},
	{0, 22, R_PCH_RCRB_D22IR},
	{0, 20, R_PCH_RCRB_D20IR},
	};

// убрать hardcoding:
#define		TMP_RCBA		0xfed1c000

/**
Routine Description:
  Returns information associated with PCI IRQ routing.
Arguments:
  This - Indicates the EFI_LEGACY_BIOS_PLATFORM_PROTOCOL instance.
  RoutingTable - Pointer to the PCI IRQ routing table. This location is the $PIR table minus the header. 
                 The contents are described by the PCI IRQ Routing Table Specification and consist of RoutingTableEntries of EFI_LEGACY_IRQ_ROUTING_ENTRY.
  RoutingTableEntries - Number of entries in the PCI IRQ routing table.
  LocalPirqTable - $PIR table. It consists of EFI_LEGACY_PIRQ_TABLE_HEADER, immediately followed by RoutingTable.
  PirqTableSize - Size of $PIR table.
  LocalIrqPriorityTable - A priority table of IRQs to assign to PCI. This table consists of IrqPriorityTableEntries of EFI_LEGACY_IRQ_PRIORITY_TABLE_ENTRY and is used to prioritize the allocation of IRQs to PCI.
  IrqPriorityTableEntries - Number of entries in the priority table.
Returns: 
  EFI_SUCCESS - Data was returned successfully.

**/
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
)
{
  EFI_PCI_IO_PROTOCOL       *PciIo;
  EFI_STATUS                Status;
  UINTN                     LocalHandleCount;
  EFI_HANDLE                *LocalHandleBuffer;

  EFI_LEGACY_PIRQ_TABLE_HEADER          *LocalRoutingTableHeader;
  EFI_LEGACY_IRQ_ROUTING_ENTRY          *LocalRoutingTableEntry;
  EFI_LEGACY_IRQ_PRIORITY_TABLE_ENTRY   *LocalIrqPriorityTableEntry;

  UINTN     Index;
  UINT8     PciClass;
  UINT8     PciSubclass;
  UINT8     PciProgrammInterface;
  UINTN     PciSegment;
  UINTN     PciBus;
  UINTN     PciDevice;
  UINTN     PciFunction;
  UINT8     PciDeviceIndex = 0;

  EFI_LEGACY_IRQ_ROUTING_ENTRY          *LocalRoutingTableEntry0;
  UINTN		Index2;
  int		flagNext;
  int		tmpPirq;
  int		tmpIrq;

  DEBUG((EFI_D_INFO, "\r\n%a.%d: entry\n", __FUNCTION__, __LINE__));

  LocalRoutingTableHeader = (EFI_LEGACY_PIRQ_TABLE_HEADER*) AllocateZeroPool(0x3000);
  LocalRoutingTableEntry = (EFI_LEGACY_IRQ_ROUTING_ENTRY*) (((UINTN)LocalRoutingTableHeader) + 0x20);
  LocalRoutingTableEntry0 = LocalRoutingTableEntry;
  LocalIrqPriorityTableEntry = (EFI_LEGACY_IRQ_PRIORITY_TABLE_ENTRY*) (((UINTN)LocalRoutingTableHeader) + 0xE00);

  //exit params
  if (LocalPirqTable != NULL) {
    *LocalPirqTable = LocalRoutingTableHeader;
  }
  if (RoutingTable != NULL) {
    *RoutingTable = LocalRoutingTableEntry;
  }
  if (LocalIrqPriorityTable != NULL) {
    *LocalIrqPriorityTable = LocalIrqPriorityTableEntry;
  }

// Further action - replace table creation to other once-driven code, leave here just address return

  // Fill PIR table header
  LocalRoutingTableHeader->Signature = EFI_LEGACY_PIRQ_TABLE_SIGNATURE;
  LocalRoutingTableHeader->MinorVersion = 0x00;
  LocalRoutingTableHeader->MajorVersion = 0x01;
  LocalRoutingTableHeader->Bus = 0;
  LocalRoutingTableHeader->DevFun = 0xF8;        // Consist PCI interrupt router bus located on Bus 0 Device 1f Function 0

  LocalRoutingTableHeader->PciOnlyIrq = 0;		//  

  LocalRoutingTableHeader->CompatibleVid =  0x0000;   // 0x8086;
  LocalRoutingTableHeader->CompatibleDid = 0x0000;    // 0x3D07;
  LocalRoutingTableHeader->Miniport = 0;

  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiPciIoProtocolGuid,
                NULL,
                &LocalHandleCount,
                &LocalHandleBuffer
                );

  for (Index = 0; Index < LocalHandleCount; Index++) {
    Status = gBS->HandleProtocol (
                  LocalHandleBuffer[Index],
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo
                  );
     
    Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint8,
                      PCI_CLASSCODE_OFFSET + 2,
                      1,
                      &PciClass
                      );
    Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint8,
                      PCI_CLASSCODE_OFFSET + 1,
                      1,
                      &PciSubclass
                      );
    Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint8,
                      PCI_CLASSCODE_OFFSET,
                      1,
                      &PciProgrammInterface
                      );

    PciIo->GetLocation (
      PciIo,
      &PciSegment,
      &PciBus,
      &PciDevice,
      &PciFunction
      );
    DEBUG ((EFI_D_INFO, "PciSegment = %x PciBus = %x PciDevice = %x PciFunction = %x\n", PciSegment, PciBus, PciDevice, PciFunction));
    // check if Bus:Device entry already included:	
    flagNext = FALSE;
    for(Index2 = 0; Index2 < PciDeviceIndex; Index2++)
    {
      if( (LocalRoutingTableEntry0[Index2].Bus == (UINT8) PciBus)
		&& (LocalRoutingTableEntry0[Index2].Device == (UINT8) (PciDevice << 3)) )
	{
	  flagNext = TRUE;
	  break;
	}
    }
    if(flagNext)
		continue;	// Bus:Device already included

    // Fill IrqRoutingEntry info
    LocalRoutingTableEntry->Bus = (UINT8) PciBus;
    LocalRoutingTableEntry->Device = (UINT8) PciDevice<<3;

    // find DxxR for Bus:Device
    for(Index2 = 0; Index2 < (sizeof(DxxR) / sizeof(DxxR[0])); Index2++)
    {
      if( (DxxR[Index2].Bus == (UINT8) PciBus)
		&& (DxxR[Index2].Device == (UINT8)PciDevice) )
	{ // Bus:Device found:
	  break;
	}

    }
    if(Index2 >= (sizeof(DxxR) / sizeof(DxxR[0])))
    { // Bus:Device not found: set trivial options
      LocalRoutingTableEntry->PirqEntry[0].Pirq = 1;
      LocalRoutingTableEntry->PirqEntry[0].IrqMask = (1 << PirqTable[0]);
      LocalRoutingTableEntry->PirqEntry[1].Pirq = 0;
      LocalRoutingTableEntry->PirqEntry[2].Pirq = 0;
      LocalRoutingTableEntry->PirqEntry[3].Pirq = 0;
    }
    else
    { // Bus:Device found in DxxR: Bus:Device -> DxxR -> PirqTable
      tmpPirq = *(UINT16*)(UINTN)(TMP_RCBA + DxxR[Index2].rcbaOffset);

      LocalRoutingTableEntry->PirqEntry[0].Pirq = 1;
      tmpIrq = PirqTable[(tmpPirq >> 0) & 0x7];
      if(tmpIrq & 0x80)
	LocalRoutingTableEntry->PirqEntry[0].IrqMask = 0;
      else
	LocalRoutingTableEntry->PirqEntry[0].IrqMask = (1 << tmpIrq);

      LocalRoutingTableEntry->PirqEntry[1].Pirq = 2;
      tmpIrq = PirqTable[(tmpPirq >> 4) & 0x7];
      if(tmpIrq & 0x80)
	LocalRoutingTableEntry->PirqEntry[1].IrqMask = 0;
      else
	LocalRoutingTableEntry->PirqEntry[1].IrqMask = (1 << tmpIrq);

      LocalRoutingTableEntry->PirqEntry[2].Pirq = 3;
      tmpIrq = PirqTable[(tmpPirq >> 8) & 0x7];
      if(tmpIrq & 0x80)
	LocalRoutingTableEntry->PirqEntry[2].IrqMask = 0;
      else
	LocalRoutingTableEntry->PirqEntry[2].IrqMask = (1 << tmpIrq);

      LocalRoutingTableEntry->PirqEntry[3].Pirq = 4;
      tmpIrq = PirqTable[(tmpPirq >> 12) & 0x7];
      if(tmpIrq & 0x80)
	LocalRoutingTableEntry->PirqEntry[3].IrqMask = 0;
      else
	LocalRoutingTableEntry->PirqEntry[3].IrqMask = (1 << tmpIrq);
    }

    LocalRoutingTableEntry->Slot = 0;

    PciDeviceIndex++;
    LocalRoutingTableEntry++;
  }

  LocalRoutingTableHeader->TableSize = (0x20 + (0x10 * PciDeviceIndex));



  //Fill IrqPriorityTable
  for(Index = 0; Index < 4; Index++)
  {
    if(PirqTable[Index] & 0x80)
    {
	LocalIrqPriorityTableEntry->Irq = 0;
	LocalIrqPriorityTableEntry->Used = PCI_UNUSED;
    }
    else
    {
	LocalIrqPriorityTableEntry->Irq = PirqTable[Index];
	LocalIrqPriorityTableEntry->Used = PCI_USED;
    }
  }

  // exit params
  if (RoutingTableEntries != NULL) {
    *RoutingTableEntries = PciDeviceIndex;
  }
  if (PirqTableSize != NULL) {
  *PirqTableSize = LocalRoutingTableHeader->TableSize;
  }
  if (IrqPriorityTableEntries != NULL) {
    *IrqPriorityTableEntries = 4;
  }

  if (LocalHandleBuffer != NULL) {
    FreePool (LocalHandleBuffer);
  }    

  DEBUG((EFI_D_INFO, "%a.%d: GetRoutingTable: exit\n", __FUNCTION__, __LINE__));
  return EFI_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
Routine Description:
  Translates the given PIRQ accounting for bridges.
Arguments:
  This -Indicates the EFI_LEGACY_BIOS_PLATFORM_PROTOCOL instance.
  PciBus - PCI bus number for this device.
  PciDevice - PCI device number for this device.
  PciFunction - PCI function number for this device.
  IntX -The PciIrq (PCI IRQ pin - INTA..INTD). INTA = 0, INTB = 1, and so on.
  PirqIrq - IRQ assigned to the indicated PIRQ.
Returns: 
  EFI_SUCCESS - The PIRQ was translated.

**/
EFI_STATUS
EFIAPI
TranslatePirq (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN UINTN PciBus,
  IN UINTN PciDevice,
  IN UINTN PciFunction,
  IN OUT UINT8 *IntX,
  OUT UINT8 *PciIrq
)
{
int	Index;
UINTN	Pirq;			// PIRQx

  DEBUG ((EFI_D_INFO, "\r\n%a.%d: entry for Bus %x, Device %x, Function %x Pin %x\n", __FUNCTION__, __LINE__, PciBus, PciDevice >> 3, PciFunction, *IntX));

	if(*IntX > 3)
	{
		DEBUG((EFI_D_INFO, "Legacy: TranslatePirq: IntX > 3\n"));
		return EFI_UNSUPPORTED;
	}

	// find DxxR for Bus:Device
	for(Index = 0; Index < (sizeof(DxxR) / sizeof(DxxR[0])); Index++)
	{
	  if( (DxxR[Index].Bus == (UINT8) PciBus)
			&& (DxxR[Index].Device == (UINT8)(PciDevice >>3)) )
	  { // Bus:Device found:
		break;
	  }
	}

	if(Index >= (sizeof(DxxR) / sizeof(DxxR[0])))
	{ // Bus:Device not found in DxxR[] table => NorthBridge device: set trivial options INTA->PIRQA
	  if(*IntX != 0)
	  {
	    DEBUG((EFI_D_INFO, "Legacy: TranslatePirq: IntX for NorthBridge != 0\n"));
	    return  EFI_UNSUPPORTED;
	  }
          Pirq = 0;		// = PIRQA
	}
	else
	{ // Bus:Device found in DxxR[]: tmpPirq = DxxR[Bus:Device]
	  Pirq = *(UINT16*)(UINTN)(TMP_RCBA + DxxR[Index].rcbaOffset);	// Pirq for Bus:Device:{INTA..INTD} from DxxR[]
	  Pirq >>= (4 * (*IntX));					
	  Pirq &= 0x07;							// Pirq for Bus:Device:IntX
	}

	*PciIrq = PirqTable[Pirq];
	DEBUG((EFI_D_INFO, "%a.%d: return Irq %x for IntX %x\n", __FUNCTION__, __LINE__, (UINT32)(*PciIrq), *IntX));

	return EFI_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
EFIAPI
ConnectAllSataDevices(VOID) {
  EFI_PCI_IO_PROTOCOL       *PciIo;
  EFI_STATUS                Status;
  UINTN                     LocalHandleCount;
  EFI_HANDLE                *LocalHandleBuffer;
  UINTN                     Index;
  UINT8                     PciClass;


  LocalHandleCount = 0;
  LocalHandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiPciIoProtocolGuid,
                 NULL,
                 &LocalHandleCount,
                 &LocalHandleBuffer
                 );

  if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Error in locating handle buffer\n"));
      return Status;
  }

  for (Index=0;Index < LocalHandleCount;Index++) {
    Status = gBS->HandleProtocol(
			LocalHandleBuffer[Index],
			&gEfiPciIoProtocolGuid,
			(VOID **) &PciIo
			);

    if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error in handling protocol %x\n", Index));
    continue;
    }
    
    Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint8,
                      PCI_CLASSCODE_OFFSET + 2,
                      1,
                      &PciClass
                      );

    //TODO -поменять условие на более адекватное. 
    if (PciClass == PCI_CLASS_MASS_STORAGE) {
	gBS->ConnectController(LocalHandleBuffer[Index], NULL, NULL, TRUE);
    }
  }

  return EFI_SUCCESS;
}


/**
Routine Description:
  Attempts to boot a traditional OS.
Arguments:
  This - Indicates the EFI_LEGACY_BIOS_PLATFORM_PROTOCOL instance.
  BbsDevicePath - EFI Device Path from BootXXXX variable.
  BbsTable - A list of BBS entries of type BBS_TABLE.
  LoadOptionsSize - Size of LoadOption in bytes.
  LoadOptions - LoadOption from BootXXXX variable.
  EfiToLegacyBootTable - Pointer to EFI_TO_COMPATIBILITY16_BOOT_TABLE.
Returns:
  EFI_SUCCESS - Ready to boot.
**/
EFI_STATUS
EFIAPI
PrepareToBoot (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *This,
  IN BBS_BBS_DEVICE_PATH *BbsDevicePath,
  IN VOID *BbsTable,
  IN UINT32 LoadOptionsSize,
  IN VOID *LoadOptions,
  IN VOID *EfiToLegacyBootTable
)
{
  CHAR8     *BootDeviceString;
  CHAR8     *BbsDeviceString;  
  UINTN     BbsDeviceStringAddr;
  UINT8     Index, Index1;
  BBS_TABLE	*BbsTableEntry;
  EFI_TO_COMPATIBILITY16_BOOT_TABLE   *BootTable;

  SMBIOS_TABLE_ENTRY_POINT	*SmbiosTable;
  UINT8		SmbiosLength;
  UINT8		*SmbiosTableInF;

  DEBUG ((EFI_D_INFO, "\r\n%a.%d: entry\n", __FUNCTION__, __LINE__));

#ifdef INT13_SMM_HANDLER
  ConnectAllSataDevices();
#endif
  
  BootDeviceString = (CHAR8*) BbsDevicePath->String;
  BbsTableEntry = (BBS_TABLE*) BbsTable;

  BootTable = (EFI_TO_COMPATIBILITY16_BOOT_TABLE*) EfiToLegacyBootTable;

  // find and copy Smbios table to F-segment
  SmbiosTable = (SMBIOS_TABLE_ENTRY_POINT*) (UINTN) BootTable->SmbiosTable;
  SmbiosLength = SmbiosTable->EntryPointLength;
  DEBUG((EFI_D_INFO, "Smbios placed at %x size %x bytes\n", SmbiosTable, SmbiosLength));
  SmbiosTableInF = (UINT8 *) (UINTN) 0x000FDD80;
  CopyMem (SmbiosTableInF, SmbiosTable, SmbiosLength);

  // search this string in BBS Table
  for (Index = 0; Index < BootTable->NumberBbsEntries; Index++ ) {
    BbsDeviceStringAddr = (BbsTableEntry[Index].DescStringSegment * 16 + BbsTableEntry[Index].DescStringOffset);
    if (BbsDeviceStringAddr == 0) {
      continue;
    }
    BbsDeviceString = (CHAR8*) BbsDeviceStringAddr;
    if (AsciiStrCmp(BbsDeviceString, BootDeviceString) == 0) {
      // set highest priority for this BBS Table entry
      DEBUG((EFI_D_INFO, "BBS entry found at index %x\n", Index));
      BbsTableEntry[Index].BootPriority = 0;
           if (BbsTableEntry[Index].DeviceType == BBS_BEV_DEVICE) {
                     BbsTableEntry[Index].AssignedDriveNumber |= 0x70;
           }
           
      break;
    }
  }

  if (Index == BootTable->NumberBbsEntries) {
    DEBUG((EFI_D_ERROR, "%a.%d: Failed to find BBS entry for BootOption\n", __FUNCTION__, __LINE__));
    return EFI_UNSUPPORTED;
  } else {
    // set all other priorities to BBS_LOWEST_PRIORITY
    for (Index1 = 0; Index1 < BootTable->NumberBbsEntries; Index1++) {
      if (Index1 == Index) {
        continue;
      }
      if (BbsTableEntry[Index1].BootPriority < BBS_UNPRIORITIZED_ENTRY) {
        BbsTableEntry[Index1].BootPriority = BBS_LOWEST_PRIORITY;
      }
    }
    DEBUG ((EFI_D_INFO, "%a.%d: exit OK\n", __FUNCTION__, __LINE__));
    return EFI_SUCCESS;
  }


}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
Routine Description:
  Install Driver to produce Legacy Bios Platform protocol. 
Arguments:
  ImageHandle     Handle for this drivers loaded image protocol.
  SystemTable     EFI system table.
Returns: 
  EFI_SUCCESS - Legacy Bios Platform protocol installed
  Other       - No protocol installed, unload driver.

**/
EFI_STATUS
EFIAPI
LegacyBiosPlatformInstall (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  DEBUG ((EFI_D_INFO, "\r\n%a.%d: entry\n", __FUNCTION__, __LINE__));

  //
  // Make sure the Legacy Interrupt Protocol is not already installed in the system
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiLegacyBiosPlatformProtocolGuid);

  //
  // Make a new handle and install the protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mLegacyBiosPlatformHandle,
                  &gEfiLegacyBiosPlatformProtocolGuid,
                  &mLegacyBiosPlatform,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  // also, intall UsbLegacyInitProtocol
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mUsbLegacyInitProtocolHandle,
                  &gUsbLegacyInitProtocolGuid,
                  &mUsbLegacyInitProtocol,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "%a.%d: exit OK\n", __FUNCTION__, __LINE__));

  return Status;
}
