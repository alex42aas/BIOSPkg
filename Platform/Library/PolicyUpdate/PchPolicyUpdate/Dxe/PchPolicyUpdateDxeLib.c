/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include	"PchPolicyUpdateDxeLib.h"


void	PchPolicyUpdateDxe (DXE_PCH_PLATFORM_POLICY_PROTOCOL *mPchPolicyData)  {
// здесь можно модифицировать поля mCpuPolicyData:

#ifdef GBYTEH77_BOARD
	mPchPolicyData->DeviceEnabling->Lan = PCH_DEVICE_ENABLE;
#endif 

}
