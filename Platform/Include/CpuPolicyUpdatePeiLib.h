/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef	CPU_POLICY_UPDATE_PEI_LIB_H
#define	CPU_POLICY_UPDATE_PEI_LIB_H


#include "EdkIIGluePeim.h"
#include EFI_PPI_PRODUCER (CpuPlatformPolicy)

void	CpuPolicyUpdatePei(  
	IN EFI_PEI_SERVICES    **PeiServices, 
	IN PEI_CPU_PLATFORM_POLICY_PPI	*CpuPlatformPolicyPpi);

#endif	/* 	CPU_POLICY_UPDATE_PEI_LIB_H */
