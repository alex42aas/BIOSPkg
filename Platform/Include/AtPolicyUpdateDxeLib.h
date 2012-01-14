/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef	AT_POLICY_UPDATE_DXE_LIB_H
#define	AT_POLICY_UPDATE_DXE_LIB_H

#include "EdkIIGlueDxe.h"
#include EFI_PROTOCOL_PRODUCER (AtPlatformPolicy)

void	AtPolicyUpdateDxe (DXE_AT_POLICY_PROTOCOL *mAtPolicyData); 

#endif	/* 	AT_POLICY_UPDATE_DXE_LIB_H */
