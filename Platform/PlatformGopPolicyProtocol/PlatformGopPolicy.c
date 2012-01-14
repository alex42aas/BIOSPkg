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

#include <Base.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h> 

#include "PlatformGopPolicy.h"
#include <Guid/LegacyBios.h>
#include <Guid/ImageAuthentication.h>
#include <Library/BaseLib.h>
#include <Library/CommonUtils.h>

EFI_GUID gGopVbtGuid;
EFI_GUID gPlatformGopPolicyProtocolGuid;
EFI_GUID gGopDisplayBrightnessProtocolGuid;



#define PLATFORM_GOP_POLICY_PROTOCOL_REVISION_02    0x0222
#define GOP_DISPLAY_BRIGHTNESS_PROTOCOL_REVISION_01 0x01
#define PLATFORM_MAX_BRIGHTNESS_LEVEL               0x80

#define VBT_HEADER_SIZE                             0x30

EFI_PHYSICAL_ADDRESS	mVbtAddress = 0;
UINT32			mVbtSize = 0;

EFI_STATUS
LoadRom(
    EFI_GUID        *FileName,
    UINTN           *RomAddress,
    UINTN           *RomSize
    );

//
// Global for the Legacy BIOS Platform Protocol that is produced by this driver
//

PLATFORM_GOP_POLICY_PROTOCOL mPlatformGopPolicyProtocol = {
  PLATFORM_GOP_POLICY_PROTOCOL_REVISION_02,
  GetPlatformLidStatus,
  GetVbtData,
  GetPlatformDockStatus
};

GOP_DISPLAY_BRIGHTNESS_PROTOCOL mGopDisplayBrightnessProtocol = {
  GOP_DISPLAY_BRIGHTNESS_PROTOCOL_REVISION_01,
  GetMaxBrightnessLevel,
  GetCurrentBrightnessLevel,
  SetBrightnessLevel
};

//
// Global for the handle that the Legacy Interrupt Protocol is installed
//

EFI_HANDLE                    mPlatformGopPolicyHandle = NULL;

UINT32                        PlatformCurrentBrightnessLevel = 0x80;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
GetPlatformLidStatus(
  OUT LID_STATUS    *CurrentLidStatus
)
{
  DEBUG((EFI_D_INFO, "GetPlatformLidStatus\n"));
  return EFI_UNSUPPORTED;
}

EFI_STATUS
GetVbtData(
  OUT EFI_PHYSICAL_ADDRESS    *VbtAddress,
  OUT UINT32                  *VbtSize
)
{
  EFI_STATUS  Status;
  UINTN       *RomAddress;
  UINTN       RomSize;
  UINT16      Flags = 0;

  DEBUG((EFI_D_INFO, "GetVbtData\n"));
  if ((VbtAddress == NULL) | (VbtSize == NULL)){
    return EFI_INVALID_PARAMETER;
  }

  if(mVbtAddress)
  { // already loaded:
    *VbtAddress = mVbtAddress;
    *VbtSize = mVbtSize;
    return EFI_SUCCESS;
  }



  // We have to prevent launch if GOP driver is not used
  Status = GetSetupFlags(&Flags);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "%a.%d\n", __FUNCTION__, __LINE__));
    return EFI_NOT_FOUND;
  }

  if (!(Flags & SETUP_FLAG_GOP_VIDEO_USE)) {
    DEBUG((EFI_D_ERROR, "%a.%d\n", __FUNCTION__, __LINE__));
    return EFI_NOT_FOUND;
  }



  RomSize = SIZE_8KB;
  RomAddress = (UINTN *) AllocateRuntimeZeroPool(RomSize);
  Status = LoadRom(&gGopVbtGuid, RomAddress, &RomSize);
  ASSERT_EFI_ERROR(Status);
  if EFI_ERROR(Status) {
    DEBUG((EFI_D_INFO, "Failed to find Vbt data\n"));
    return EFI_NOT_FOUND;
  } else {
    *VbtAddress = (EFI_PHYSICAL_ADDRESS)RomAddress;
    *VbtSize = (UINT32)RomSize;
    DEBUG((EFI_D_INFO, "Return Vbt data at %x %x bytes\n", *VbtAddress, *VbtSize));
    mVbtAddress = *VbtAddress;
    mVbtSize = *VbtSize;

    return EFI_SUCCESS;
  }
}

EFI_STATUS
GetPlatformDockStatus(
  OUT DOCK_STATUS CurrentDockStatus
)
{
  DEBUG((EFI_D_INFO, "GetPlatformDockStatus\n"));
  return EFI_UNSUPPORTED;
}

EFI_STATUS
GetMaxBrightnessLevel(
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  OUT UINT32                          *MaxBrightnessLevel
)
{
  DEBUG((EFI_D_INFO, "GetMaxBrightnessLevel\n"));

  if (MaxBrightnessLevel == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  *MaxBrightnessLevel = PLATFORM_MAX_BRIGHTNESS_LEVEL;
  return EFI_SUCCESS;
}

EFI_STATUS
GetCurrentBrightnessLevel(
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  OUT UINT32                          *CurrentBrightnessLevel
)
{
  DEBUG((EFI_D_INFO, "GetCurrentBrightnessLevel\n"));
  if (CurrentBrightnessLevel == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  *CurrentBrightnessLevel = PlatformCurrentBrightnessLevel;
  return EFI_SUCCESS;
}

EFI_STATUS
SetBrightnessLevel(
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  IN UINT32                          BrightnessLevel
)
{
  DEBUG((EFI_D_INFO, "SetBrightnessLevel\n"));
  PlatformCurrentBrightnessLevel = BrightnessLevel;
  return EFI_SUCCESS;
}

/**
Routine Description:
  Install Driver to produce Platform Gop Policy protocol. 
Arguments:
  ImageHandle     Handle for this drivers loaded image protocol.
  SystemTable     EFI system table.
Returns: 
  EFI_SUCCESS - Legacy Bios Platform protocol installed
  Other       - No protocol installed, unload driver.

**/
EFI_STATUS
EFIAPI
PlatformGopPolicyInstall (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  DEBUG((EFI_D_INFO, "Install GOP driver auxillary protocols\ngPlatformGopPolicyProtocolGuid %g\ngGopDisplayBrightnessProtocolGuid %g\ngGopVbtGuid %g\n", gPlatformGopPolicyProtocolGuid, gGopDisplayBrightnessProtocolGuid, gGopVbtGuid));

  //
  // Make sure the Legacy Interrupt Protocol is not already installed in the system
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gPlatformGopPolicyProtocolGuid);
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gGopDisplayBrightnessProtocolGuid);

  //
  // Make a new handle and install the protocol
  //
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPlatformGopPolicyHandle,
                  &gPlatformGopPolicyProtocolGuid,
                  &mPlatformGopPolicyProtocol,
                  &gGopDisplayBrightnessProtocolGuid,
                  &mGopDisplayBrightnessProtocol,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  
  Status = EFI_SUCCESS;

  return Status;
}
