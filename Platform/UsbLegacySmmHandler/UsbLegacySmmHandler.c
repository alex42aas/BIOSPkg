/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Mobile Silicon Support Module" and is
  licensed for Intel Mobile CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchSmmCore.c

Abstract:

  This driver is responsible for the registration of child drivers
  and the abstraction of the PCH SMI sources.

--*/

#include <Protocol/SmmUsbDispatch.h>
#include <Protocol/UsbLegacyInitProtocol.h>
#include <Protocol/PciIo.h>
#include <Protocol/SmmBase.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/ExitPmAuth.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>

#include <PchRegsLpc.h>
#include <PchRegsUsb.h>

#define   SMI_ON_OS_OWNERSHIP_CHANGE      BIT29

typedef struct _EXIT_PM_AUTH_PROTOCOL  EXIT_PM_AUTH_PROTOCOL;
typedef struct _EXIT_PM_AUTH_PROTOCOL {
  UINTN   Dummy;
};

EFI_HANDLE		mExitPmAuthProtocolHandle = NULL;

EXIT_PM_AUTH_PROTOCOL	mExitPmAuthProtocol = {
  0
};

extern EFI_GUID		gEfiEventLegacyBootGuid;

//
// /////////////////////////////////////////////////////////////////////////////
// MODULE / GLOBAL DATA
//
// Module variables used by the both the main dispatcher and the source dispatchers
// Declared in PchSmmSources.h
//
EFI_SMM_USB_DISPATCH_PROTOCOL	*mSmmUsbDispatch;
EFI_SMM_USB_DISPATCH_CONTEXT	mSmmUsbDispatchContext;
EFI_HANDLE			mSmmUsbDispatchHandle;
EFI_HANDLE			mDriverImageHandle;
EFI_SMM_BASE_PROTOCOL		*mSmmBase;
EFI_SMM_SYSTEM_TABLE		*mSmst;

USB_LEGACY_INIT_PROTOCOL	*mUsbLegacyInitProtocol;

VOID				*mEhc;

VOID
UsbLegacyDispatch (
  IN  EFI_HANDLE                             DispatchHandle,
  IN  EFI_SMM_USB_DISPATCH_CONTEXT	     *DispatchContext
  );

//
// Driver entry point
//
EFI_STATUS
EFIAPI
RegisterUsbLegacySmmHandler (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initializes the PCH SMM Dispatcher

Arguments:
  
  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:
  Status        - EFI_SUCCESS

--*/
{
  EFI_STATUS			Status;
  UINT32			PmBase;
  UINT32			DataSts;
  EFI_DEVICE_PATH_PROTOCOL 	*DevPath;
  EFI_PCI_IO_PROTOCOL		*PciIo;

  EFI_HANDLE			*HandleBuffer;
  UINTN				HandleCount;
  UINTN				Index;
  UINTN				PciSegment;
  UINTN				PciBus;
  UINTN				PciDevice;
  UINTN				PciFunction;

  BOOLEAN			InSmm;
  EFI_HANDLE			Handle;
  EFI_LOADED_IMAGE_PROTOCOL	*LoadedImage;
  EFI_DEVICE_PATH_PROTOCOL	*CompleteFilePath;
  EFI_DEVICE_PATH_PROTOCOL	*ImageDevicePath;

  DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler entry\n"));

  //
  // Initialize the EFI SMM driver library
  //
  mDriverImageHandle = ImageHandle;
  
  DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler locate protocol gEfiSmmBaseProtocolGuid\n"));

  //
  // Find the SMM base protocol
  //
  Status = gBS->LocateProtocol (&gEfiSmmBaseProtocolGuid, NULL, &mSmmBase);
  ASSERT_EFI_ERROR (Status);

  DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler GetSmstLocation\n"));

  //
  // We're the SMM RAM copy. Initialize global variables.
  //
  mSmmBase->GetSmstLocation (mSmmBase, &mSmst);

  DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler InSmm call \n"));

  mSmmBase->InSmm (mSmmBase, &InSmm);

  DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler InSmm call OK \n"));

  if (!InSmm) {

	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler Not in SMM \n"));

    //
    // We need to perform our initialization from within the SMM context.
    // Unfortunately, we're not in SMM mode right now.  We need to load
    // this image into SMRAM using the SmmBase Protocol and then trigger
    // an SMI with the SmmControl Protocol.
    //
    //
    // Load the image into (SMRAM) memory
    //
    Status = gBS->HandleProtocol (
                    ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    &LoadedImage
                    );
    ASSERT_EFI_ERROR (Status);

	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler ImageDevicePath \n"));

    Status = gBS->HandleProtocol (
                    LoadedImage->DeviceHandle,
                    &gEfiDevicePathProtocolGuid,
                    (VOID *) &ImageDevicePath
                    );
    ASSERT_EFI_ERROR (Status);

    CompleteFilePath = AppendDevicePath (
                        ImageDevicePath,
                        LoadedImage->FilePath
                        );

	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler Register SMM \n"));

    //
    // Load it into SMRAM & make it available for callbacks
    //
    mSmmBase->Register (mSmmBase, CompleteFilePath, NULL, 0, &Handle, FALSE);

  } else {

  	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler Register USB Legacy SMM handler\n"));
  
	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler locate gUsbLegacyInitProtocolGuid\n"));

  	//
  	// get required EHCI driver function
  	//
  	Status = gBS->LocateProtocol (&gUsbLegacyInitProtocolGuid, NULL, &mUsbLegacyInitProtocol);
  	ASSERT_EFI_ERROR (Status);
  
  	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler locate gEfiSmmUsbDispatchProtocolGuid\n"));

  	//
    // Initialize the EFI Runtime Library
    //
    Status = gBS->LocateProtocol (&gEfiSmmUsbDispatchProtocolGuid, NULL, &mSmmUsbDispatch);
    ASSERT_EFI_ERROR (Status);
  
	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler locate gEfiPciIoProtocolGuid\n"));

  	// find EHCI handle to install DevicePath to
  	DevPath = NULL;
    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiPciIoProtocolGuid,
                    NULL,
                    &HandleCount,
                    &HandleBuffer
                    );
  
  	ASSERT_EFI_ERROR(Status);

	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler LocateHandleBuffer %d\n", HandleCount));
  
    for (Index=0;Index < HandleCount;Index++) 
    {
      Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
  
      PciIo->GetLocation (
        PciIo,
        &PciSegment,
        &PciBus,
        &PciDevice,
        &PciFunction
        );
  
      if ( (PciSegment == 0) && (PciBus == 0) && ((PciDevice == PCI_DEVICE_NUMBER_PCH_USB_EXT) || (PciDevice == PCI_DEVICE_NUMBER_PCH_USB)) && (PciFunction == 0) ) 
      {
	DEBUG((EFI_D_INFO, "RegisterUsbLegacySmmHandler found PCI device\n"));

  	//EHCI 
  	// get DevicePath for handle
  	Status = gBS->HandleProtocol (
  					HandleBuffer[Index],
  					&gEfiDevicePathProtocolGuid,
  					(VOID **) &DevPath
  					);
  
  	if (EFI_ERROR(Status)) 
	{
  	  DEBUG((EFI_D_ERROR, "Failed to open DevicePath for controller %x\n", Index));
  	  continue;
  	}
  
  	//
  	// Register a callback function to handle subsequent SMIs.  This callback
  	// will be called by SmmCoreDispatcher.
  	//
  	mSmmUsbDispatchContext.Type = UsbLegacy;
  	mSmmUsbDispatchContext.Device = DevPath/*NULL*/;
    	
	DEBUG((EFI_D_INFO, "mSmmUsbDispatch->Register %x\n", Index));		
  		
  	Status = mSmmUsbDispatch->Register(mSmmUsbDispatch, UsbLegacyDispatch, &mSmmUsbDispatchContext, &mSmmUsbDispatchHandle);
  	if (EFI_ERROR(Status)) {
  	  DEBUG((EFI_D_ERROR, "Failed to register SMI for controller %x\n", Index));
  	  continue;			
  	}
      }
    }
    PmBase = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_ACPI_BASE));
    PmBase &= ~0x7f;

    DataSts = IoRead32(PmBase + R_PCH_SMI_STS);
    IoWrite32(PmBase + R_PCH_SMI_STS, DataSts);  

    DataSts = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_ULKMC));
    PciWrite32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_ULKMC), DataSts);
  
    IoOr32(PmBase + R_PCH_SMI_EN, B_PCH_SMI_EN_LEGACY_USB3 | B_PCH_SMI_EN_INTEL_USB2 | B_PCH_SMI_EN_LEGACY_USB2 | B_PCH_SMI_EN_LEGACY_USB | B_PCH_SMI_EN_EOS | B_PCH_SMI_EN_GBL_SMI);
  }

  return EFI_SUCCESS;
}

VOID
UsbLegacyDispatch (
  IN  EFI_HANDLE			DispatchHandle,
  IN  EFI_SMM_USB_DISPATCH_CONTEXT	*DispatchContext
  )
/*++

Routine Description:

  The callback function to handle subsequent SMIs.  This callback will be called by SmmCoreDispatcher.

--*/
{
  EFI_STATUS	Status;
  UINT32	Ulkmc;
  UINT32	UsbSts0, UsbSts1;
  UINT32	UsbCmd0, UsbCmd1;
  UINT32	UsbLegacyConSts0, UsbLegacyConSts1;
  UINT32	LegacyExtSupport;
  UINT32	PmBase;
  UINT32	DataSts;
  UINT32	EhciBase0;
  UINT32	EhciBase1;

  PmBase = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_ACPI_BASE));
  PmBase &= ~0x7f;
  DataSts = IoRead32(PmBase + R_PCH_SMI_STS);
  DEBUG((EFI_D_INFO, "\r\n%a.%d: entry with SMI_STS = %08x\n", __FUNCTION__, __LINE__, DataSts));

  Ulkmc = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_ULKMC));

  if (((Ulkmc & B_PCH_LPC_ULKMC_SMIBYENDPS) && (Ulkmc & B_PCH_LPC_ULKMC_SMIATENDPS)) || 
      ((Ulkmc & B_PCH_LPC_ULKMC_TRAPBY64W) && (Ulkmc & B_PCH_LPC_ULKMC_64WEN)) || 
      ((Ulkmc & B_PCH_LPC_ULKMC_TRAPBY64R) && (Ulkmc & B_PCH_LPC_ULKMC_64REN)) || 
      ((Ulkmc & B_PCH_LPC_ULKMC_TRAPBY60W ) && (Ulkmc & B_PCH_LPC_ULKMC_60WEN)) || 
      ((Ulkmc & B_PCH_LPC_ULKMC_TRAPBY60R ) && (Ulkmc & B_PCH_LPC_ULKMC_60REN))) 
  {
    DEBUG((EFI_D_INFO, "SMI caused by ULKMC\n"));
    // clear all ULKMC interrupt requests
    PciWrite32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_ULKMC), Ulkmc);
  } 

  UsbLegacyConSts0 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CAP));
  UsbLegacyConSts1 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CAP));

  UsbLegacyConSts0 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CS));
  UsbLegacyConSts1 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CS));

  if ( (UsbLegacyConSts0 & SMI_ON_OS_OWNERSHIP_CHANGE) || (UsbLegacyConSts1 & SMI_ON_OS_OWNERSHIP_CHANGE) ) {
    DEBUG((EFI_D_INFO, "SMI caused by USB ownership change\n"));
    // SMI caused by USB ownership change
    if (UsbLegacyConSts0 & SMI_ON_OS_OWNERSHIP_CHANGE) {
      // first controller
      DEBUG((EFI_D_INFO, "Shutdown 1st EHCI\n"));
      LegacyExtSupport = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CAP));
      if (LegacyExtSupport & HC_OS_OWNERSHIP) {
	// OS requested HostController ownership
	DEBUG((EFI_D_INFO, "OS requested ownership on this controller\n"));
	// 1 - disconnect USB stack
	Status = mUsbLegacyInitProtocol->StopEhciDxe(mUsbLegacyInitProtocol->Ehc[0]);
	// 2 - clear SMI request
	DEBUG((EFI_D_INFO, "Clear SMI request\n"));
	PciWrite32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CS), UsbLegacyConSts0);
	// 3 - disable all SMI on this conroller
	DEBUG((EFI_D_INFO, "Disable SMI\n"));
	PciOr32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CS), 0xE0000000);
	PciAnd32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CS), 0xFFFF0000);
      }
    } else {
      // second controller
      DEBUG((EFI_D_INFO, "Shutdown 2nd EHCI\n"));
      LegacyExtSupport = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CAP));
      if (LegacyExtSupport & HC_OS_OWNERSHIP) {
	// OS requested HostController ownership
	DEBUG((EFI_D_INFO, "OS requested ownership on this controller\n"));
	// 1 - disconnect USB stack
	Status = mUsbLegacyInitProtocol->StopEhciDxe(mUsbLegacyInitProtocol->Ehc[1]);
	// 2 - clear SMI request
	PciWrite32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CS), UsbLegacyConSts1);
	// 3 - disable all SMI on this conroller
	PciOr32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CS), 0xE0000000);
	PciAnd32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CS), 0xFFFF0000);
      }
    }
    mUsbLegacyInitProtocol->Flags &= ~USB_LEGACY_ENABLED;
    return;
  }

  EhciBase0 = PciRead32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_MEM_BASE)) & ~0xf;
  EhciBase1 = PciRead32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_MEM_BASE)) & ~0xf;

  // SPECIAL_SMI—Intel® Specific USB 2.0 SMI Register:
  UsbSts0 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_SPCSMI));
  UsbSts1 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_SPCSMI));
  if (UsbSts0) {
    DEBUG((EFI_D_INFO, "\r\n%a.%d: Device %d,  EHCI_SPCSMI = %08x\n", __FUNCTION__, __LINE__, PCI_DEVICE_NUMBER_PCH_USB_EXT, UsbSts0));
    PciWrite32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_SPCSMI), UsbSts0);
  }
  if (UsbSts1) {
    DEBUG((EFI_D_INFO, "\r\n%a.%d: Device %d,  EHCI_SPCSMI = %08x\n", __FUNCTION__, __LINE__, PCI_DEVICE_NUMBER_PCH_USB, UsbSts1));
    PciWrite32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_SPCSMI), UsbSts1);
  }

  UsbCmd0 = MmioRead32 (EhciBase0 + R_PCH_EHCI_USB2CMD);
  UsbCmd1 = MmioRead32 (EhciBase1 + R_PCH_EHCI_USB2CMD);

  UsbSts0 = MmioRead32 (EhciBase0 + R_PCH_EHCI_USB2STS);
  UsbSts1 = MmioRead32 (EhciBase1 + R_PCH_EHCI_USB2STS);

  if ( (UsbSts0 & BIT0) || (UsbSts1 & BIT0) ) {
    DEBUG((EFI_D_INFO, "SMI caused by UsbSts\n"));
    // clear all UsbSts interrupt requests
    MmioWrite32 (EhciBase0 + R_PCH_EHCI_USB2STS, UsbSts0);
    MmioWrite32 (EhciBase1 + R_PCH_EHCI_USB2STS, UsbSts1);

    if ( (UsbSts0 & BIT0) || (UsbSts1 & BIT0)) {
      if (mUsbLegacyInitProtocol->EhcMonitorAsyncRequests == NULL) {
	return;
      }
      // handle interrupt with EhcMonitorAsyncRequests
      if (UsbSts0 & BIT0) {
	mEhc = mUsbLegacyInitProtocol->Ehc[0];
      } else {
	mEhc = mUsbLegacyInitProtocol->Ehc[1];
      }
      if ((mUsbLegacyInitProtocol->Flags & USB_LEGACY_ENABLED) != USB_LEGACY_ENABLED) {
        return;
      }
      
      if (mUsbLegacyInitProtocol->EhcMonitorAsyncRequests != NULL) 
      {
	DEBUG((EFI_D_INFO, "Invoke EhcMonitorAsyncRequests %08lx\n", mUsbLegacyInitProtocol->EhcMonitorAsyncRequests));
  	mUsbLegacyInitProtocol->EhcMonitorAsyncRequests ( (EFI_EVENT) ((UINTN) EVT_RUNTIME), mEhc);
      }
      return;
    }
  }

  // LEG_EXT_CS—USB EHCILegacy Support Extended Control/Status Register:
  UsbSts0 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CS));
  UsbSts0 &= 0xFFFF0000;
  UsbSts1 = PciRead32 (PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CS));
  UsbSts1 &= 0xFFFF0000;

  // EHCI 26:
  if (UsbSts0) {
    DEBUG((EFI_D_INFO, "\r\n%a.%d: Device %d,  LEG_EXT_CS = %08x\n", __FUNCTION__, __LINE__, PCI_DEVICE_NUMBER_PCH_USB_EXT, UsbSts0));
    // clear SMI requests:
    if(UsbSts0 & 0xE0000000)
    { // clear in EHCI_LEGEXT_CS:
      UsbSts0 &= 0xE0000000;
      PciOr32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB_EXT, PCI_FUNCTION_NUMBER_PCH_EHCI2, R_PCH_EHCI_LEGEXT_CS), UsbSts0);
    }
    else
    { // clear in EHCI_USB2STS:
      UsbSts0 &= 0x3F0000;
      UsbSts0 >>= 16;
      MmioOr32(EhciBase0 + R_PCH_EHCI_USB2STS, UsbSts0);
    }
  }

  // EHCI 29:
  if (UsbSts1) {
    DEBUG((EFI_D_INFO, "\r\n%a.%d: Device %d,  LEG_EXT_CS = %08x\n", __FUNCTION__, __LINE__, PCI_DEVICE_NUMBER_PCH_USB, UsbSts1));
    // clear SMI requests:
    if(UsbSts1 & 0xE0000000)
    { // clear in EHCI_LEGEXT_CS:
      UsbSts1 &= 0xE0000000;
      PciOr32(PCI_LIB_ADDRESS(0x00, PCI_DEVICE_NUMBER_PCH_USB, PCI_FUNCTION_NUMBER_PCH_EHCI, R_PCH_EHCI_LEGEXT_CS), UsbSts1);
    }
    else
    { // clear in EHCI_USB2STS:
      UsbSts1 &= 0x3F0000;
      UsbSts1 >>= 16;
      MmioOr32(EhciBase1 + R_PCH_EHCI_USB2STS, UsbSts1);
    }
  }

  DEBUG((EFI_D_INFO, "%a.%d exit\n", __FUNCTION__, __LINE__));
}
