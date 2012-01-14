/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include	"PchPolicyUpdatePeiLib.h"
#include	"CommonAddresses.h"


void	PchPolicyUpdatePei (  
	IN EFI_PEI_SERVICES    **PeiServices, 
	IN PCH_PLATFORM_POLICY_PPI	*PchPlatformPolicyPpi){
// здесь можно модифицировать поля PchPlatformPolicyPpi:
	PchPlatformPolicyPpi->PlatformData->TempMemBaseAddr = THERMAL_BASEB;

#ifdef GBYTEH77_BOARD
	PchPlatformPolicyPpi->GbeConfig->EnableGbe = 1;
#endif 
}
