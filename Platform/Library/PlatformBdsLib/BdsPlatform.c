/** @file
  Platform BDS customizations.

  Copyright (c) 2004 - 2009, Intel Corporation. <BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include "BdsPlatform.h"

#include <Protocol/LegacyBios.h> 
#include <Protocol/LegacyBiosPlatform.h>
#include <Protocol/LegacyRegion.h> 
#include <Protocol/Legacy8259.h> 
#include <Protocol/LegacyInterrupt.h>
#include <Library/LocalApicLib.h>
#include <Library/MtrrLib.h>
#include <Library/TimerLib.h>

#include <Setup.h>
#include <RedirectDebugLib.h>

#ifdef INT13_SMM_HANDLER
#include "PlatformBbsTableEntries.h"
#endif


VOID                         *mEfiDevPathNotifyReg;
EFI_EVENT                    mEfiDevPathEvent;
VOID                         *mEmuVariableEventReg;
EFI_EVENT                    mEmuVariableEvent;
BOOLEAN                      mDetectVgaOnly;
EFI_LEGACY_BIOS_PROTOCOL     *mLegacyBios = NULL;
EFI_LEGACY_BIOS_PLATFORM_PROTOCOL     *mLegacyBiosPlatform;


//
// Type definitions
//

typedef
EFI_STATUS
(EFIAPI *PROTOCOL_INSTANCE_CALLBACK)(
  IN EFI_HANDLE           Handle,
  IN VOID                 *Instance,
  IN VOID                 *Context
  );

/**
  @param[in]  Handle - Handle of PCI device instance
  @param[in]  PciIo - PCI IO protocol instance
  @param[in]  Pci - PCI Header register block
**/
typedef
EFI_STATUS
(EFIAPI *VISIT_PCI_INSTANCE_CALLBACK)(
  IN EFI_HANDLE           Handle,
  IN EFI_PCI_IO_PROTOCOL  *PciIo,
  IN PCI_TYPE00           *Pci
  );


//
// Function prototypes
//

EFI_STATUS
VisitAllInstancesOfProtocol (
  IN EFI_GUID                    *Id,
  IN PROTOCOL_INSTANCE_CALLBACK  CallBackFunction,
  IN VOID                        *Context
  );

EFI_STATUS
VisitAllPciInstancesOfProtocol (
  IN VISIT_PCI_INSTANCE_CALLBACK CallBackFunction
  );

VOID
InstallDevicePathCallback (
  VOID
  );

VOID
LoadVideoRom (
  VOID
  );

EFI_STATUS
PciRomLoadEfiDriversFromRomImage (
  IN EFI_PHYSICAL_ADDRESS    Rom,
  IN UINTN                   RomSize
  );

//
// BDS Platform Functions
//
VOID
EFIAPI
PlatformBdsInit (
  VOID
  )
/*++

Routine Description:

  Platform Bds init. Incude the platform firmware vendor, revision
  and so crc check.

Arguments:

Returns:

  None.

--*/
{
	EFI_STATUS Status;
	SETUP Setup;
	UINTN nVarSize;
	EFI_GUID PAThConsType;
	int i;
	enum { SERIAL_PC_ANSI, SERIAL_VT100, SERIAL_VT100_PLUS, SERIAL_UTF8, SERIAL_NONE };
	EFI_GUID rgPAThConsType[] =
	{//Возможные типы консолей
		DEVICE_PATH_MESSAGING_PC_ANSI,
		DEVICE_PATH_MESSAGING_VT_100,
		DEVICE_PATH_MESSAGING_VT_100_PLUS,
		DEVICE_PATH_MESSAGING_VT_UTF8
	};

	DEBUG ((EFI_D_INFO, "\nEntry "__FUNCTION__".%d\n", __LINE__));//%%%%
  // Check for SETUP var present,  if no write defaults.

  nVarSize = 0;
  Status   = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gVendorGuid,
                    NULL,
                    &nVarSize,
                    NULL
                    );

  if (Status == EFI_NOT_FOUND)
  {
    ZeroMem(&Setup, sizeof(Setup));
    Setup.MemoryTestMode = 0;
    Setup.InteractiveBootMenu = 1;
  
      Status = gRT->SetVariable (
          SETUP_VARIABLE_NAME,
          &gVendorGuid,
          (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
          sizeof(Setup),
          &Setup
          ); 
  }

	/*Работа с настройками "Тип терминала" раздела "Чипсет"*/
	//Читаем значение переменной
	nVarSize = sizeof(PAThConsType);
	Status = gRT->GetVariable (
						CONSOLE_TYPE_VARIABLE_NAME,
						&gVendorGuid,
						NULL,
						&nVarSize,
						&PAThConsType );

	DEBUG ((EFI_D_INFO, __FUNCTION__".Status=%d.%d\n", Status, __LINE__));//%%%%

	if ( Status == EFI_NOT_FOUND )
	{//Если переменной еще нет создаем ее
		EFI_GUID PAThConsTypeTemp = DEVICE_PATH_MESSAGING_VT_UTF8;
		PAThConsType = PAThConsTypeTemp;

		Status = gRT->SetVariable (
				CONSOLE_TYPE_VARIABLE_NAME,
				&gVendorGuid,
				(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
				sizeof(PAThConsType),
				&PAThConsType );

		DEBUG ((EFI_D_INFO, __FUNCTION__".Status=%d.%d\n", Status, __LINE__));//%%%%
		if ( EFI_ERROR(Status) )return;
	}

	DEBUG((EFI_D_INFO, __FUNCTION__".ConsoleTypeGuid = %g.%d\n", &PAThConsType, __LINE__));

	for ( i = SERIAL_PC_ANSI; i < SERIAL_NONE; i++ )
	{//Поиск индекса в таблице возможных типов консолей по текущему значению из переменной
		if ( CompareGuid ( &PAThConsType, &rgPAThConsType[i] ) )break;
	}

	switch ( i )
	{//Установка переменной gTerminalTypeDeviceNode используемой для инициализации
		case SERIAL_PC_ANSI:
		{
			VENDOR_DEVICE_PATH VENdorDevicePath = gPcAnsiTerminal;
			gTerminalTypeDeviceNode = VENdorDevicePath; break;
		}
		case SERIAL_VT100:
		{
			VENDOR_DEVICE_PATH VENdorDevicePath = gVT100Terminal;
			gTerminalTypeDeviceNode = VENdorDevicePath; break;
		}
		case SERIAL_VT100_PLUS:
		{
			VENDOR_DEVICE_PATH VENdorDevicePath = gVT100PlusTerminal;
			gTerminalTypeDeviceNode = VENdorDevicePath; break;
		}
		case SERIAL_UTF8://По умолчанию UTF-8
		default:
		{
			VENDOR_DEVICE_PATH VENdorDevicePath = gUtf8Terminal;
			gTerminalTypeDeviceNode = VENdorDevicePath; break;
		}
	}

	DEBUG ((EFI_D_INFO, __FUNCTION__".bConsoleTypeInx=%d.%d\n", i, __LINE__));//%%%%
}



EFI_STATUS
ConnectRootBridge (
  VOID
  )
/*++

Routine Description:

  Connect RootBridge

Arguments:

  None.

Returns:

  EFI_SUCCESS             - Connect RootBridge successfully.
  EFI_STATUS              - Connect RootBridge fail.

--*/
{
  EFI_STATUS                Status;
  EFI_HANDLE                RootHandle;
  //
  // Make all the PCI_IO protocols on PCI Seg 0 show up
  //
  BdsLibConnectDevicePath (gPlatformRootBridges[0]);

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &gPlatformRootBridges[0],
                  &RootHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->ConnectController (RootHandle, NULL, NULL, FALSE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
PrepareLpcBridgeDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add IsaKeyboard to ConIn,
  add IsaSerial to ConOut, ConIn, ErrOut.
  LPC Bridge: 06 01 00

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.

Returns:

  EFI_SUCCESS             - LPC bridge is added to ConOut, ConIn, and ErrOut.
  EFI_STATUS              - No LPC bridge is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  CHAR16                    *DevPathStr;

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  TempDevicePath = DevicePath;

  //
  // Register Keyboard
  //
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnpPs2KeyboardDeviceNode);

  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);

  //
  // Register COM1
  //
  DevicePath = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 0;
  
  gUartDeviceNode.BaudRate = PcdGet64(PcdUartDefaultBaudRate);

  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartFlowControlDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  //
  // Print Device Path
  //

  DevPathStr = DevicePathToStr(DevicePath);

  DEBUG((
    EFI_D_INFO,
    "BdsPlatform.c+%d: COM%d DevPath: %s\n",
    __LINE__,
    gPnp16550ComPortDeviceNode.UID + 1,
    DevPathStr
    ));
  FreePool(DevPathStr);

  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarErrorOut, DevicePath, NULL);

  //
  // Register COM2
  //
  DevicePath = TempDevicePath;
  gPnp16550ComPortDeviceNode.UID = 1;

  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gPnp16550ComPortDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartFlowControlDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);

  //
  // Print Device Path
  //
  DevPathStr = DevicePathToStr(DevicePath);
  DEBUG((
    EFI_D_INFO,
    "BdsPlatform.c+%d: COM%d DevPath: %s\n",
    __LINE__,
    gPnp16550ComPortDeviceNode.UID + 1,
    DevPathStr
    ));
  FreePool(DevPathStr);

  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarErrorOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
GetGopDevicePath (
   IN  EFI_DEVICE_PATH_PROTOCOL *PciDevicePath,
   OUT EFI_DEVICE_PATH_PROTOCOL **GopDevicePath
   )
{
  UINTN                           Index;
  EFI_STATUS                      Status;
  EFI_HANDLE                      PciDeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL        *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *TempPciDevicePath;
  UINTN                           GopHandleCount;
  EFI_HANDLE                      *GopHandleBuffer;

  DEBUG((EFI_D_INFO, "\r\nGetGopDevicePath 1\n"));
  if (PciDevicePath == NULL || GopDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the GopDevicePath to be PciDevicePath
  //
  *GopDevicePath    = PciDevicePath;
  TempPciDevicePath = PciDevicePath;

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &TempPciDevicePath,
                  &PciDeviceHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG((EFI_D_INFO, "GetGopDevicePath 2\n"));

  //
  // Try to connect this handle, so that GOP dirver could start on this
  // device and create child handles with GraphicsOutput Protocol installed
  // on them, then we get device paths of these child handles and select
  // them as possible console device.
  //
  gBS->ConnectController (PciDeviceHandle, NULL, NULL, FALSE);

  DEBUG((EFI_D_INFO, "GetGopDevicePath 3\n"));

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &GopHandleCount,
                  &GopHandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
  DEBUG((EFI_D_INFO, "GetGopDevicePath 4\n"));

  //
  // Add all the child handles as possible Console Device
  //
  for (Index = 0; Index < GopHandleCount; Index++) {
    DEBUG((EFI_D_INFO, "GetGopDevicePath 5\n"));
    Status = gBS->HandleProtocol (GopHandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID*)&TempDevicePath);
    if (EFI_ERROR (Status)) {
	DEBUG((EFI_D_INFO, "GetGopDevicePath 9\n"));
        continue;
    }
    if (CompareMem (
            PciDevicePath,
            TempDevicePath,
            GetDevicePathSize (PciDevicePath) - END_DEVICE_PATH_LENGTH
            ) == 0) {
        //
        // In current implementation, we only enable one of the child handles
        // as console device, i.e. sotre one of the child handle's device
        // path to variable "ConOut"
        // In futhure, we could select all child handles to be console device
        //
	DEBUG((EFI_D_INFO, "GetGopDevicePath 6\n"));
        *GopDevicePath = TempDevicePath;
        //
        // Delete the PCI device's path that added by GetPlugInPciVgaDevicePath()
        // Add the integrity GOP device path.
        //
        BdsLibUpdateConsoleVariable (VarConsoleOutDev, NULL, PciDevicePath);
        BdsLibUpdateConsoleVariable (VarConsoleOutDev, TempDevicePath, NULL);

	DEBUG((EFI_D_INFO, "GetGopDevicePath 7\n"));
      }
      DEBUG((EFI_D_INFO, "GetGopDevicePath 8\n"));
    }
    gBS->FreePool (GopHandleBuffer);
  }

  return EFI_SUCCESS;
}




EFI_STATUS
PreparePciVgaDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add PCI VGA to ConOut.
  PCI VGA: 03 00 00

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.

Returns:

  EFI_SUCCESS             - PCI VGA is added to ConOut.
  EFI_STATUS              - No PCI VGA device is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *GopDevicePath;

  DEBUG((EFI_D_INFO, "\r\nPreparePciVgaDevicePath invoked\n"));

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG((EFI_D_INFO, "PreparePciVgaDevicePath case 1\n"));
 
  GetGopDevicePath (DevicePath, &GopDevicePath);

  DEBUG((EFI_D_INFO, "PreparePciVgaDevicePath case 2\n"));

  DevicePath = GopDevicePath;


  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);


  DEBUG((EFI_D_INFO, "PreparePciVgaDevicePath exit ok\n"));

  return EFI_SUCCESS;
}



EFI_STATUS
PreparePciSerialDevicePath (
  IN EFI_HANDLE                DeviceHandle
  )
/*++

Routine Description:

  Add PCI Serial to ConOut, ConIn, ErrOut.
  PCI Serial: 07 00 02

Arguments:

  DeviceHandle            - Handle of PCIIO protocol.

Returns:

  EFI_SUCCESS             - PCI Serial is added to ConOut, ConIn, and ErrOut.
  EFI_STATUS              - No PCI Serial device is added.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *DevPathStr;

  DEBUG ((EFI_D_INFO, "\r\n%a.%d: Entry \n", __FUNCTION__, __LINE__));

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gUartDeviceNode);
  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *)&gTerminalTypeDeviceNode);
  //
  // Print Device Path
  //
  DevPathStr = DevicePathToStr(DevicePath);
  DEBUG((
    EFI_D_INFO,
    "BdsPlatform.c+%d: PCI Serial DevPath: %s\n",
    __LINE__,
    DevPathStr
    ));

  FreePool(DevPathStr);


  BdsLibUpdateConsoleVariable (VarConsoleOut, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarConsoleInp, DevicePath, NULL);
  BdsLibUpdateConsoleVariable (VarErrorOut, DevicePath, NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
VisitAllInstancesOfProtocol (
  IN EFI_GUID                    *Id,
  IN PROTOCOL_INSTANCE_CALLBACK  CallBackFunction,
  IN VOID                        *Context
  )
{
  EFI_STATUS                Status;
  UINTN                     HandleCount;
  EFI_HANDLE                *HandleBuffer;
  UINTN                     Index;
  VOID                      *Instance;

  DEBUG((EFI_D_INFO, "\r\nVisitAllInstancesOfProtocol invoked\n"));

  //
  // Start to check all the PciIo to find all possible device
  //
  HandleCount = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  Id,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
	DEBUG((EFI_D_INFO, ">> VisitAllInstancesOfProtocol: return error status %x\n", Status));
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], Id, &Instance);
    if (EFI_ERROR (Status)) {
      continue;
    }

  DEBUG((EFI_D_INFO, "VisitInstancesOfProtocol index %x\n", Index));

    Status = (*CallBackFunction) (
               HandleBuffer[Index],
               Instance,
               Context
               );
  }

  gBS->FreePool (HandleBuffer);

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
VisitingAPciInstance (
  IN EFI_HANDLE  Handle,
  IN VOID        *Instance,
  IN VOID        *Context
  )
{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                Pci;

  DEBUG((EFI_D_INFO, "\r\nVisitingAPciInstance invoked\n"));

  PciIo = (EFI_PCI_IO_PROTOCOL*) Instance;

  //
  // Check for all PCI device
  //
  Status = PciIo->Pci.Read (
                    PciIo,
                    EfiPciIoWidthUint32,
                    0,
                    sizeof (Pci) / sizeof (UINT32),
                    &Pci
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return (*(VISIT_PCI_INSTANCE_CALLBACK)(UINTN) Context) (
           Handle,
           PciIo,
           &Pci
           );

}



EFI_STATUS
VisitAllPciInstances (
  IN VISIT_PCI_INSTANCE_CALLBACK CallBackFunction
  )
{
  return VisitAllInstancesOfProtocol (
           &gEfiPciIoProtocolGuid,
           VisitingAPciInstance,
           (VOID*)(UINTN) CallBackFunction
           );
}


/**
  Do platform specific PCI Device check and add them to
  ConOut, ConIn, ErrOut.

  @param[in]  Handle - Handle of PCI device instance
  @param[in]  PciIo - PCI IO protocol instance
  @param[in]  Pci - PCI Header register block

  @retval EFI_SUCCESS - PCI Device check and Console variable update successfully.
  @retval EFI_STATUS - PCI Device check or Console variable update fail.

**/
EFI_STATUS
DetectAndPreparePlatformPciDevicePath (
  IN EFI_HANDLE           Handle,
  IN EFI_PCI_IO_PROTOCOL  *PciIo,
  IN PCI_TYPE00           *Pci
  )
{
  EFI_STATUS               Status;

  DEBUG((EFI_D_INFO, "\r\n>> DetectAndPreparePlatformPciDevicePath: entry\n"));

  Status = PciIo->Attributes (
    PciIo,
    EfiPciIoAttributeOperationEnable,
    EFI_PCI_DEVICE_ENABLE,
    NULL
    );
  ASSERT_EFI_ERROR (Status);

  if (!mDetectVgaOnly) {
    //
    // Here we decide whether it is LPC Bridge
    //
    if (IS_PCI_LPC (Pci)) {
      //
      // Add IsaKeyboard to ConIn,
      // add IsaSerial to ConOut, ConIn, ErrOut
      //
      DEBUG ((EFI_D_INFO, "Found LPC Bridge device\n"));
      PrepareLpcBridgeDevicePath (Handle);
      return EFI_SUCCESS;
    }

    //
    // Here we decide which Serial device to enable in PCI bus
    //
    if (IS_PCI_16550SERIAL (Pci)) {
      //
      // Add them to ConOut, ConIn, ErrOut.
      //
      DEBUG ((EFI_D_INFO, "Found PCI 16550 SERIAL device\n"));
      PreparePciSerialDevicePath (Handle);
      return EFI_SUCCESS;
    }
    
  }


  //
  // Here we decide which VGA device to enable in PCI bus
  //

  // commented - was done earlier

  if (IS_PCI_VGA (Pci)) {
    //
    // Add them to ConOut.
    //
    DEBUG ((EFI_D_INFO, "Found PCI VGA device\n"));
    PreparePciVgaDevicePath (Handle);
    return EFI_SUCCESS;
  }

  return Status;
}


/**
  Do platform specific PCI Device check and add them to ConOut, ConIn, ErrOut

  @param[in]  DetectVgaOnly - Only detect VGA device if it's TRUE.

  @retval EFI_SUCCESS - PCI Device check and Console variable update successfully.
  @retval EFI_STATUS - PCI Device check or Console variable update fail.

**/
EFI_STATUS
DetectAndPreparePlatformPciDevicePaths (
  BOOLEAN DetectVgaOnly
  )
{
  mDetectVgaOnly = DetectVgaOnly;
  return VisitAllPciInstances (DetectAndPreparePlatformPciDevicePath);
}


EFI_STATUS
PlatformBdsConnectConsole (
  IN BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  )
/*++

Routine Description:

  Connect the predefined platform default console device. Always try to find
  and enable the vga device if have.

Arguments:

  PlatformConsole         - Predfined platform default console device array.

Returns:

  EFI_SUCCESS             - Success connect at least one ConIn and ConOut
                            device, there must have one ConOut device is
                            active vga device.

  EFI_STATUS              - Return the status of
                            BdsLibConnectAllDefaultConsoles ()

--*/
{
  EFI_STATUS                         Status;
  UINTN                              Index;
  EFI_DEVICE_PATH_PROTOCOL           *VarConout;
  EFI_DEVICE_PATH_PROTOCOL           *VarConin;
  UINTN                              DevicePathSize;
  //  UINT8                              BackLightValue;
  //
  // Connect RootBridge
  //

  DEBUG((EFI_D_INFO, "\r\nPlatformBdsConnectConsole invoked\n"));

  VarConout = BdsLibGetVariableAndSize (
                VarConsoleOut,
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );
  VarConin = BdsLibGetVariableAndSize (
               VarConsoleInp,
               &gEfiGlobalVariableGuid,
               &DevicePathSize
               );

  if (VarConout == NULL || VarConin == NULL) {
	DEBUG((EFI_D_INFO, "PlatformBdsConnectConsole NULL console detected\n"));
    //
    // Do platform specific PCI Device check and add them to ConOut, ConIn, ErrOut
    //
    DetectAndPreparePlatformPciDevicePaths (FALSE);
//    DetectAndPreparePlatformPciDevicePaths (TRUE);	// VGA only

    //
    // Have chance to connect the platform default console,
    // the platform default console is the minimue device group
    // the platform should support
    //
    for (Index = 0; PlatformConsole[Index].DevicePath != NULL; ++Index) {
      //
      // Update the console variable with the connect type
      //
      if ((PlatformConsole[Index].ConnectType & CONSOLE_IN) == CONSOLE_IN) {
        BdsLibUpdateConsoleVariable (VarConsoleInp, PlatformConsole[Index].DevicePath, NULL);
      }
      if ((PlatformConsole[Index].ConnectType & CONSOLE_OUT) == CONSOLE_OUT) {
        BdsLibUpdateConsoleVariable (VarConsoleOut, PlatformConsole[Index].DevicePath, NULL);
      }
      if ((PlatformConsole[Index].ConnectType & STD_ERROR) == STD_ERROR) {
        BdsLibUpdateConsoleVariable (VarErrorOut, PlatformConsole[Index].DevicePath, NULL);
      }
    }
  } else {
    //
    // Only detect VGA device and add them to ConOut
    //
    DEBUG((EFI_D_INFO, "PlatformBdsConnectConsole case 1\n"));
    DetectAndPreparePlatformPciDevicePaths (TRUE);
  }

  //
  // Connect the all the default console with current cosole variable
  //
  DEBUG((EFI_D_INFO, "PlatformBdsConnectConsole case 2\n"));
  Status = BdsLibConnectAllDefaultConsoles ();
  DEBUG((EFI_D_INFO, "PlatformBdsConnectConsole case 3\n"));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}



VOID
PciInitialization ( 
    VOID
  )
{

}


EFI_STATUS
EFIAPI
ConnectRecursivelyIfPciMassStorage (
  IN EFI_HANDLE           Handle,
  IN EFI_PCI_IO_PROTOCOL  *Instance,
  IN PCI_TYPE00           *PciHeader
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *DevPathStr;


  if (IS_CLASS1 (PciHeader, PCI_CLASS_MASS_STORAGE)) {
    DevicePath = NULL;
    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiDevicePathProtocolGuid,
                    (VOID*)&DevicePath
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Print Device Path
    //
    DevPathStr = DevicePathToStr (DevicePath);
    DEBUG((
      EFI_D_INFO,
      "Found Mass Storage device: %s\n",
      DevPathStr
      ));
    FreePool(DevPathStr);

    Status = gBS->ConnectController (Handle, NULL, NULL, TRUE);
    if (EFI_ERROR (Status)) {
      return Status;
    }

  }

  return EFI_SUCCESS;
}


VOID
PlatformBdsConnectSequence (
  VOID
  )
/*++

Routine Description:

  Connect with predeined platform connect sequence,
  the OEM/IBV can customize with their own connect sequence.

Arguments:

  None.

Returns:

  None.

--*/
{
  UINTN Index;

  DEBUG ((EFI_D_INFO, "\r\nPlatformBdsConnectSequence: entry\n"));

  Index = 0;

  //
  // Here we can get the customized platform connect sequence
  // Notes: we can connect with new variable which record the
  // last time boots connect device path sequence
  //
  while (gPlatformConnectSequence[Index] != NULL) {
    //
    // Build the platform boot option
    //
    BdsLibConnectDevicePath (gPlatformConnectSequence[Index]);
    Index++;
  }

  //
  // Just use the simple policy to connect all devices
  //
  DEBUG((EFI_D_INFO, ">> PlatformBdsConnectSequence: BdsLibConnectAll()..\n"));
  BdsLibConnectAll ();

  DEBUG((EFI_D_INFO, ">> PlatformBdsConnectSequence: exit\n"));
  //
  // Clear the logo after all devices are connected.
  //
  //gST->ConOut->ClearScreen (gST->ConOut);
}

VOID
PlatformBdsDiagnostics (
  IN EXTENDMEM_COVERAGE_LEVEL    MemoryTestLevel,
  IN BOOLEAN                     QuietBoot,
  IN BASEM_MEMORY_TEST           BaseMemoryTest
  )
/*++

Routine Description:

  Perform the platform diagnostic, such like test memory. OEM/IBV also
  can customize this fuction to support specific platform diagnostic.

Arguments:

  MemoryTestLevel  - The memory test intensive level

  QuietBoot        - Indicate if need to enable the quiet boot

  BaseMemoryTest   - A pointer to BaseMemoryTest()

Returns:

  None.

--*/
{
  EFI_STATUS  Status;

  DEBUG ((EFI_D_INFO, "PlatformBdsDiagnostics\n"));

  //
  // Here we can decide if we need to show
  // the diagnostics screen
  // Notes: this quiet boot code should be remove
  // from the graphic lib
  //
  if (QuietBoot) {
    EnableQuietBoot (PcdGetPtr(PcdLogoFile));
    //
    // Perform system diagnostic
    //
    Status = BaseMemoryTest (MemoryTestLevel);
    if (EFI_ERROR (Status)) {
      DisableQuietBoot ();
    }

    return ;
  }
  //
  // Perform system diagnostic
  //
  Status = BaseMemoryTest (MemoryTestLevel);
  DEBUG ((EFI_D_INFO, "PlatformBdsDiagnostics OK\n"));
  
}

// ----------------------------------------------------------------------------------------------

/**
  Build the on flash shell boot option with the handle parsed in.

  @param  Handle                 The handle which present the device path to create
                                 on flash shell boot option
  @param  BdsBootOptionList      The header of the link list which indexed all
                                 current boot options

**/
VOID
EFIAPI
BdsLibBuildOptionFromFileOnFlash (
  IN EFI_HANDLE                  Handle,
  IN OUT LIST_ENTRY              *BdsBootOptionList,
  IN EFI_GUID                    *FileNameGuid,
  IN UINT16                      *EntryName 
  )
{
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH FileNode;

  DevicePath = DevicePathFromHandle (Handle);

  //
  // Build the shell device path
  //
  EfiInitializeFwVolDevicepathNode (&FileNode, FileNameGuid);

  DevicePath = AppendDevicePathNode (DevicePath, (EFI_DEVICE_PATH_PROTOCOL *) &FileNode);

  //
  // Create and register the shell boot option
  //
  BdsLibRegisterNewOption (BdsBootOptionList, DevicePath, EntryName, L"BootOrder");

}

EFI_GUID gBootLoaderNameGuid = 
{ 0xA3126D42, 0x4471, 0xF4E4, { 0x9A, 0x21, 0x71, 0xA3, 0x43, 0xFB, 0x2D, 0x13 } };

VOID
EFIAPI
PlatformBdsAddInternalBootloader(
  IN OUT LIST_ENTRY                  *BootOptionList
  )
{
  UINTN Index;
  EFI_STATUS Status;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINT32                        AuthenticationStatus;
  UINTN                         FvHandleCount;
  EFI_HANDLE                    *FvHandleBuffer;
  EFI_FV_FILETYPE               Type;
  UINTN                         Size;

  Fv = NULL;

  gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiFirmwareVolume2ProtocolGuid,
        NULL,
        &FvHandleCount,
        &FvHandleBuffer
        );
  for (Index = 0; Index < FvHandleCount; Index++) {
    gBS->HandleProtocol (
          FvHandleBuffer[Index],
          &gEfiFirmwareVolume2ProtocolGuid,
          (VOID **) &Fv
          );

    ASSERT( Fv != NULL );

    Status = Fv->ReadFile (
                  Fv,
                  &gBootLoaderNameGuid, // PcdGetPtr(PcdShellFile)
                  NULL,
                  &Size,
                  &Type,
                  &Attributes,
                  &AuthenticationStatus
                  );
    if (EFI_ERROR (Status)) {
      //
      // Skip if no shell file in the FV
      //
      continue;
    }
    //
    // Build the shell boot option
    //
    BdsLibBuildOptionFromFileOnFlash (
	FvHandleBuffer[Index], 
	BootOptionList, 
	&gBootLoaderNameGuid,
	L"Internal Bootloader" 
	);
  }

  if (FvHandleCount != 0) {
    FreePool (FvHandleBuffer);
  }

  //
  // Make sure every boot only have one time
  // boot device enumerate
  //
//  Status = BdsLibBuildOptionFromVar (BootOptionList, L"BootOrder");
}

// ----------------------------------------------------------------------------------------------
VOID
PIrqInitialization ( 
    VOID
  )
{

  // Enable SIRQ
  PciOr8( PCI_LIB_ADDRESS(0, 0x1f, 0, 0x64), 0xC2 );

  // Disable TCO WDT
  {
    UINT32 Rcba;
    UINT16 Pmbase;
    UINT16 Tcobase;

    Rcba = PciRead32(PCI_LIB_ADDRESS(0,0x1f,0,0xF0)) & 0xFFFFC000;
    Pmbase = (UINT16) (PciRead32(PCI_LIB_ADDRESS(0,0x1f,0,0x40)) & 0xFFF8);
    Tcobase = Pmbase + 0x60;

    MmioOr32(Rcba + 0x3410, (1 << 5));
    IoOr16(Tcobase+8, (1 << 11));  // TCO1_CNT
    IoOr16(Tcobase+4, 0);
    
  }
  DEBUG((EFI_D_ERROR, "PciInitialization OK\n"));
}


#if	0
/*
	Отображаем имена драйверов из списка DriverNameList,
	если эти драйверы присутствуют среди Binding-протоколов
*/
VOID	findDriverBinding(CHAR16 **DriverNameList){
EFI_STATUS                     	Status;
UINTN				DriverBindingHandleCount;
EFI_HANDLE     			*DriverBindingHandleBuffer;
EFI_DRIVER_BINDING_PROTOCOL	*DriverBinding;
UINTN				index;
EFI_COMPONENT_NAME2_PROTOCOL	*EfiComponentName2Protocol;
CHAR16				*DriverName;
int				indexName = 0;

	// запрашиваем все Binding-протоколы, зарегистрированные в ядре:
	Status = gBS->LocateHandleBuffer (
             ByProtocol,
             &gEfiDriverBindingProtocolGuid,
             NULL,
             &DriverBindingHandleCount,
             &DriverBindingHandleBuffer
             );
	DEBUG((EFI_D_INFO, "\r\n%a.%d: Locate EfiDriverBindingProtocol's, Count = %d, Status = %x\n", __FUNCTION__, __LINE__, DriverBindingHandleCount, Status));
	if(EFI_ERROR(Status))
			return;

	// для каждого протокола запрашиваем информацию по драйверу:
	for(index = 0; index < DriverBindingHandleCount; index++)
	{
	  // получаем собственно хэндл протокола:
	  Status = gBS->HandleProtocol (
                  DriverBindingHandleBuffer[index],
                  &gEfiDriverBindingProtocolGuid,
                  (VOID **) &DriverBinding
                  );
	  if(EFI_ERROR(Status))
			continue;

	  // для протокола получаем имя:
	  Status = gBS->HandleProtocol (
                DriverBinding->DriverBindingHandle,
                &gEfiComponentName2ProtocolGuid,
                (VOID **) &EfiComponentName2Protocol
                );
          if (!EFI_ERROR (Status) && EfiComponentName2Protocol != NULL) 
	  { // имя существует:
	    EfiComponentName2Protocol->GetDriverName(
					EfiComponentName2Protocol,
					EfiComponentName2Protocol->SupportedLanguages,
					&DriverName);
	    if(DriverName == NULL)
				continue;
//		if(StrStr(DriverName, L"NVIDIA") || StrStr(DriverName, L"BIOS[INT10]"))

	    indexName = 0;
	    while(1)
	    {
	      if(DriverNameList[indexName] == NULL)
						break;
	      if(StrStr(DriverName, DriverNameList[indexName]))
	      { // нашли - отображаем:
		DEBUG((EFI_D_INFO, "%a.%d: Driver \"%ls\" found \n", __FUNCTION__, __LINE__, DriverName));
		break;
	      }
	      indexName++;
	    } // while

	  } // if
	} // for



}

CHAR16	*driverNameList[] = {	L"NVIDIA",
				L"BIOS[INT10]",
				L"GOP",
				NULL
			};

#endif // 0







// ----------------------------------------------------------------------------
VOID
EFIAPI
PlatformBdsPolicyBehavior (
  IN OUT LIST_ENTRY                  *DriverOptionList,
  IN OUT LIST_ENTRY                  *BootOptionList,
  IN PROCESS_CAPSULES                ProcessCapsules,
  IN BASEM_MEMORY_TEST               BaseMemoryTest
  )
/*++

Routine Description:

  The function will excute with as the platform policy, current policy
  is driven by boot mode. IBV/OEM can customize this code for their specific
  policy action.

Arguments:

  DriverOptionList - The header of the driver option link list

  BootOptionList   - The header of the boot option link list

  ProcessCapsules  - A pointer to ProcessCapsules()

  BaseMemoryTest   - A pointer to BaseMemoryTest()

Returns:

  None.

--*/
{

  EFI_STATUS                         	Status;
  UINT16                             	Timeout;
  EFI_EVENT                          	UserInputDurationTime;
  LIST_ENTRY                         	*Link;
  BDS_COMMON_OPTION                  	*BootOption;


  EFI_BOOT_MODE                      	BootMode;
  UINTN                                	HandleCount;
  EFI_HANDLE                           	*HandleBuffer;
  EFI_PCI_IO_PROTOCOL	  		*PciIo;

  DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[1]: entry"));


  UserInputDurationTime = NULL;

  PIrqInitialization();

  ConnectRootBridge ();

#if	0
  findDriverBinding(driverNameList);
#endif	// 0

  Status = gBS->LocateProtocol(&gEfiLegacyBiosPlatformProtocolGuid, NULL, &mLegacyBiosPlatform);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &mLegacyBios);

// first, set VGA device
  HandleCount = 0;
  HandleBuffer = NULL;

  Status = mLegacyBiosPlatform->GetPlatformHandle (
                                          mLegacyBiosPlatform,
                                          EfiGetPlatformVgaHandle,
                                          0,
                                          &HandleBuffer,
                                          &HandleCount,
                                          NULL
                                          );
  DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior: get EfiGetPlatformVgaHandle, status = %x", Status));
  DEBUG((EFI_D_INFO, "\r\n%a.%d: Count = %d", __FUNCTION__, __LINE__, HandleCount));

  if (!EFI_ERROR (Status)) {
	// get PciIo on thet handle, and enable it
	Status = gBS->HandleProtocol (
                  HandleBuffer[0],
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo
                  );
	DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior: HandleProtocol PciIo for LegacyBiosPlatform, status = %x", Status));
	
	ASSERT_EFI_ERROR (Status);

	Status = PciIo->Attributes (
    	PciIo,
	    EfiPciIoAttributeOperationEnable,
    	EFI_PCI_DEVICE_ENABLE,
	    NULL
    	);
	DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior: set PciIoAttributeOperationEnable, status = %x", Status));
	ASSERT_EFI_ERROR (Status);

	PreparePciVgaDevicePath(HandleBuffer[0]);
	DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[2]: PreparePciVgaDevicePath OK"));
  }	


  if (HandleCount>0) {
	FreePool(HandleBuffer);
  }



// Next, shadow all other PCI ROMs
  if (mLegacyBios != NULL) {
	DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior: ShadowAllLegacyOproms"));
  	mLegacyBios->ShadowAllLegacyOproms(mLegacyBios);
  }

  //
  // Init the time out value
  //
#if 0  
  Timeout = PcdGet16 (PcdPlatformBootTimeOut);
#else
  Timeout = 0xFFFF; // the default timeout value
#endif

  //
  // Get current Boot Mode
  //
  Status = BdsLibGetBootMode (&BootMode);

  //
  // Go the different platform policy with different boot mode
  // Notes: this part code can be change with the table policy
  //
  ASSERT (BootMode == BOOT_WITH_FULL_CONFIGURATION);

  //
  // Connect platform console
  //
  Status = PlatformBdsConnectConsole (gPlatformConsole);
  DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[3]: PlatformBdsConnectConsole status = %x", Status));
  if (EFI_ERROR (Status)) {
    //
    // Here OEM/IBV can customize with defined action
    //
    PlatformBdsNoConsoleAction ();
  }

  //
  // Memory test and Logo show
  //
  if (PcdGetBool(UseGop) == TRUE)
	EnableQuietBoot (PcdGetPtr(PcdGopLogoFile));

  //
  // Perform some platform specific connect sequence
  //
  PlatformBdsConnectSequence ();
  DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[4]: PlatformBdsConnectSequence"));

  //
  DEBUG ((EFI_D_INFO, "BdsLibConnectAll\n"));
  BdsLibConnectAll ();
  DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[5]: BdsLibConnectAll OK"));

#ifdef INT13_SMM_HANDLER
  PlatformUpdateBbsTable();
#endif

  DEBUG ((EFI_D_INFO, "BdsLibEnumerateAllBootOption\n"));
  BdsLibEnumerateAllBootOption (BootOptionList);
  DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[6]: BdsLibEnumerateAllBootOption OK"));

  //
  // Please uncomment above ConnectAll and EnumerateAll code and remove following first boot
  // checking code in real production tip.
  //
  // In BOOT_WITH_FULL_CONFIGURATION boot mode, should always connect every device
  // and do enumerate all the default boot options. But in development system board, the boot mode
  // cannot be BOOT_ASSUMING_NO_CONFIGURATION_CHANGES because the machine box
  // is always open. So the following code only do the ConnectAll and EnumerateAll at first boot.
  //
  // Status = BdsLibBuildOptionFromVar (BootOptionList, L"BootOrder");
  // DEBUG ((EFI_D_INFO, "BdsLibBuildOptionFromVar Status:%r\n", Status));

  if (EFI_ERROR(Status)) {
    //
    // If cannot find "BootOrder" variable,  it may be first boot.
    // Try to connect all devices and enumerate all boot options here.
    //
    BdsLibConnectAll ();
    BdsLibEnumerateAllBootOption (BootOptionList);
    DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[6.5]: BdsLibEnumerateAllBootOption-2 OK"));
  }

#ifndef BOOT_ONLY_MBLOADER

  //
  // To give the User a chance to enter Setup here, if user set TimeOut is 0.
  // BDS should still give user a chance to enter Setup
  //
  // Connect first boot option, and then check user input before exit
  //
  for (Link = BootOptionList->ForwardLink; Link != BootOptionList;Link = Link->ForwardLink) {
    BootOption = CR (Link, BDS_COMMON_OPTION, Link, BDS_LOAD_OPTION_SIGNATURE);
    if (!IS_LOAD_OPTION_TYPE (BootOption->Attribute, LOAD_OPTION_ACTIVE)) {
      //
      // skip the header of the link list, becuase it has no boot option
      //
      continue;
    } else {
      

      //
      // Make sure the boot option device path connected, but ignore the BBS device path
      //
      DEBUG((EFI_D_INFO, "\r\n%a.%d: DevicePath type:%x\n", __FUNCTION__, __LINE__, DevicePathType (BootOption->DevicePath)));

      if (DevicePathType (BootOption->DevicePath) != BBS_DEVICE_PATH) {
        BdsLibConnectDevicePath (BootOption->DevicePath);
      }
      break;
    }
  }
#else

  for (Link = BootOptionList->ForwardLink; Link != BootOptionList;Link = Link->ForwardLink) {
     BootOption = CR (Link, BDS_COMMON_OPTION, Link, BDS_LOAD_OPTION_SIGNATURE);
      // Boot only from internal firmware volume

     DEBUG((EFI_D_INFO, "\r\n%a.%d: DevicePath type:%x stype:%x Attr:%x Name:%s\n", __FUNCTION__, __LINE__,
           DevicePathType (BootOption->DevicePath), 
           DevicePathSubType (BootOption->DevicePath),
           BootOption->Attribute,
           BootOption->OptionName
           ));
      if ((DevicePathType (BootOption->DevicePath) == HARDWARE_DEVICE_PATH) &&
          (DevicePathSubType (BootOption->DevicePath) == HW_MEMMAP_DP)
          ) {
        UINTN             ExitDataSize;
        CHAR16            *ExitData;

        BootOption->Attribute |= LOAD_OPTION_ACTIVE; 
        BdsLibConnectDevicePath (BootOption->DevicePath);

        Status = BdsLibBootViaBootOption (
            BootOption, 
            BootOption->DevicePath, 
            &ExitDataSize, 
            &ExitData);
        ASSERT(0);

      }
  }
#endif

  DEBUG((EFI_D_INFO, "\r\n>> PlatformBdsPolicyBehavior[7]: exit OK\n"));

  return ;
}


VOID
EFIAPI
PlatformBdsBootSuccess (
  IN  BDS_COMMON_OPTION   *Option
  )
/*++

Routine Description:

  Hook point after a boot attempt succeeds. We don't expect a boot option to
  return, so the EFI 1.0 specification defines that you will default to an
  interactive mode and stop processing the BootOrder list in this case. This
  is alos a platform implementation and can be customized by IBV/OEM.

Arguments:

  Option - Pointer to Boot Option that succeeded to boot.

Returns:

  None.

--*/
{
  CHAR16  *TmpStr;

  DEBUG ((EFI_D_INFO, "PlatformBdsBootSuccess\n"));
  //
  // If Boot returned with EFI_SUCCESS and there is not in the boot device
  // select loop then we need to pop up a UI and wait for user input.
  //
  TmpStr = Option->StatusString;
  if (TmpStr != NULL) {
    BdsLibOutputStrings (gST->ConOut, TmpStr, Option->Description, L"\n\r", NULL);
    FreePool (TmpStr);
  }
}


VOID
EFIAPI
PlatformBdsBootFail (
  IN  BDS_COMMON_OPTION  *Option,
  IN  EFI_STATUS         Status,
  IN  CHAR16             *ExitData,
  IN  UINTN              ExitDataSize
  )
/*++

Routine Description:

  Hook point after a boot attempt fails.

Arguments:

  Option - Pointer to Boot Option that failed to boot.

  Status - Status returned from failed boot.

  ExitData - Exit data returned from failed boot.

  ExitDataSize - Exit data size returned from failed boot.

Returns:

  None.

--*/
{
  CHAR16  *TmpStr;

  DEBUG ((EFI_D_INFO, "PlatformBdsBootFail\n"));

  //
  // If Boot returned with failed status then we need to pop up a UI and wait
  // for user input.
  //
  TmpStr = Option->StatusString;
  if (TmpStr != NULL) {
    BdsLibOutputStrings (gST->ConOut, TmpStr, Option->Description, L"\n\r", NULL);
    FreePool (TmpStr);
  }
}


EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  )

/*++

Routine Description:

  This function is remained for IBV/OEM to do some platform action,
  if there no console device can be connected.

Arguments:

  None.

Returns:

  EFI_SUCCESS      - Direct return success now.

--*/

{
  DEBUG ((EFI_D_INFO, "PlatformBdsNoConsoleAction\n"));
  return EFI_SUCCESS;
}


VOID
EFIAPI
PlatformBdsLockNonUpdatableFlash (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "PlatformBdsLockNonUpdatableFlash\n"));
  return;
}



/**
  This notification function is invoked when an instance of the
  EFI_DEVICE_PATH_PROTOCOL is produced.

  @param  Event                 The event that occured
  @param  Context               For EFI compatiblity.  Not used.

**/
VOID
EFIAPI
NotifyDevPath (
  IN  EFI_EVENT Event,
  IN  VOID      *Context
  )
{
  EFI_HANDLE                            Handle;
  EFI_STATUS                            Status;
  UINTN                                 BufferSize;
  EFI_DEVICE_PATH_PROTOCOL             *DevPathNode;
  ATAPI_DEVICE_PATH                    *Atapi;

  //
  // Examine all new handles
  //
  for (;;) {
    //
    // Get the next handle
    //
    BufferSize = sizeof (Handle);
    Status = gBS->LocateHandle (
              ByRegisterNotify,
              NULL,
              mEfiDevPathNotifyReg,
              &BufferSize,
              &Handle
              );

    //
    // If not found, we're done
    //
    if (EFI_NOT_FOUND == Status) {
      break;
    }

    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Get the DevicePath protocol on that handle
    //
    Status = gBS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid, (VOID **)&DevPathNode);
    ASSERT_EFI_ERROR (Status);

    while (!IsDevicePathEnd (DevPathNode)) {
      //
      // Find the handler to dump this device path node
      //
      if (
           (DevicePathType(DevPathNode) == MESSAGING_DEVICE_PATH) &&
           (DevicePathSubType(DevPathNode) == MSG_ATAPI_DP)
         ) {
        Atapi = (ATAPI_DEVICE_PATH*) DevPathNode;
      }

      //
      // Next device path node
      //
      DevPathNode = NextDevicePathNode (DevPathNode);
    }
  }

  return;
}


VOID
InstallDevicePathCallback (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "Registered NotifyDevPath Event\n"));
  mEfiDevPathEvent = EfiCreateProtocolNotifyEvent (
                          &gEfiDevicePathProtocolGuid,
                          TPL_CALLBACK,
                          NotifyDevPath,
                          NULL,
                          &mEfiDevPathNotifyReg
                          );
}

/**
  Lock the ConsoleIn device in system table. All key
  presses will be ignored until the Password is typed in. The only way to
  disable the password is to type it in to a ConIn device.

  @param  Password        Password used to lock ConIn device.

  @retval EFI_SUCCESS     lock the Console In Spliter virtual handle successfully.
  @retval EFI_UNSUPPORTED Password not found

**/
EFI_STATUS
EFIAPI
LockKeyboards (
  IN  CHAR16    *Password
  )
{
    return EFI_UNSUPPORTED;
}


