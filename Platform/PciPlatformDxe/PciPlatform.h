/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PCI_LOCAL_PLATFORM_H_
#define _PCI_LOCAL_PLATFORM_H_

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/CpuLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <IndustryStandard/Pci30.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciPlatform.h>
#include <Protocol/FirmwareVolume2.h>
#include <PciPlatformLib.h>
#include <Library/CommonUtils.h>

extern EFI_GUID gSataAhciRomGuid;

#endif

