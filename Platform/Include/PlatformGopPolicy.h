/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PLATFORM_GOP_POLICY_H__
#define _PLATFORM_GOP_POLICY_H__

//#include <Base.h>
//#include <Library/UefiBootServicesTableLib.h>
//#include <Library/MemoryAllocationLib.h>
//#include <Library/DebugLib.h>
//#include <Library/UefiLib.h>

extern	EFI_GUID gGopVbtGuid;
extern	EFI_GUID gPlatformGopPolicyProtocolGuid;
extern	EFI_GUID gGopDisplayBrightnessProtocolGuid;

typedef enum {
  LidClosed,
  LidOpen,
  LidStatusMax
} LID_STATUS;

typedef enum {
  Docked,
  UnDocked,
  DockStatusMax
} DOCK_STATUS;

typedef
EFI_STATUS
(EFIAPI *GET_PLATFORM_LID_STATUS) (
  OUT LID_STATUS *CurentLidStatus
);

typedef
EFI_STATUS
(EFIAPI *GET_VBT_DATA) (
  OUT EFI_PHYSICAL_ADDRESS  *VbrAddress,
  OUT UINT32                *VbtSize
);

typedef
EFI_STATUS
(EFIAPI *GET_PLATFORM_DOCK_STATUS) (
  OUT DOCK_STATUS CurrentDockStatus
);

typedef
EFI_STATUS
(EFIAPI *DUMMY_FUNC) (
  void
);

typedef struct {
  UINT32                            Revision;
  GET_PLATFORM_LID_STATUS           GetPlatformLidStatus;
  GET_VBT_DATA                      GetVbtData;
  GET_PLATFORM_DOCK_STATUS          GetPlatformDockStatus;
  DUMMY_FUNC                        DummyFunc;
} PLATFORM_GOP_POLICY_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *GET_MAXIMUM_BRIGHTNESS_LEVEL) (
  IN    VOID                            *This,
  OUT   UINT32                          *MaxBrightnessLevel
);

typedef
EFI_STATUS
(EFIAPI *GET_CURRENT_BRIGHTNESS_LEVEL) (
  IN    VOID                           *This,
  OUT   UINT32                         *CurrentBrightnessLevel
);

typedef
EFI_STATUS
(EFIAPI *SET_BRIGHTNESS_LEVEL) (
  IN    VOID                            *This,
  IN    UINT32                          BrightnessLevel
);

typedef struct {
  UINT32                            Revision;
  GET_MAXIMUM_BRIGHTNESS_LEVEL      GetMaxBrightnessLevel;
  GET_CURRENT_BRIGHTNESS_LEVEL      GetCurrentBrightnessLevel;
  SET_BRIGHTNESS_LEVEL              SetBrightnessLevel;
}GOP_DISPLAY_BRIGHTNESS_PROTOCOL;

// Protocol Function Prototypes

EFI_STATUS
GetPlatformLidStatus(
  OUT LID_STATUS    *CurrentLidStatus
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
GetVbtData(
  OUT EFI_PHYSICAL_ADDRESS    *VbtAddress,
  OUT UINT32                  *VbtSize
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EFI_STATUS
GetPlatformDockStatus(
  OUT DOCK_STATUS CurrentDockStatus
);

EFI_STATUS
GetMaxBrightnessLevel(
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  OUT UINT32                          *MaxBrightnessLevel
);

EFI_STATUS
GetCurrentBrightnessLevel(
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  OUT UINT32                          *CurrentBrightnessLevel
);

EFI_STATUS
SetBrightnessLevel(
  IN  GOP_DISPLAY_BRIGHTNESS_PROTOCOL *This,
  IN UINT32                          BrightnessLevel
);

EFI_STATUS
DummyFunc(
  void
);
#endif
