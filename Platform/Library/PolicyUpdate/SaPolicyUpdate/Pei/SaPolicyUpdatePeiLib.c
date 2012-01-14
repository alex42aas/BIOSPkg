/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include	"SaPolicyUpdatePeiLib.h"

#include	<Setup.h>
#include	<Ppi/Variable2/Variable2.h>

EFI_GUID	gEfiPeiReadOnlyVariable2PpiGuid = { 0x2ab86ef5, 0xecb5, 0x4134, 0xb5, 0x56, 0x38, 0x54, 0xca, 0x1f, 0xe1, 0xb4 };
EFI_GUID	gVendorGuid = { 0x2fb3abd9, 0x65fb, 0x4db4, 0x87, 0x2, 0x42, 0xdc, 0xa4, 0x55, 0xa8, 0x12 };




void	SaPolicyUpdatePei (  
	IN EFI_PEI_SERVICES    **PeiServices, 
	IN SA_PLATFORM_POLICY_PPI	*SaPlatformPolicyPpi){
// здесь можно модифицировать поля SaPlatformPolicyPpi:
GT_CONFIGURATION              *GtConfig = SaPlatformPolicyPpi->GtConfig;

//
// Initialize the Graphics configuration
//

#define GTT_SIZE_2MB 2
#define IGD_ENABLE 1
#define IGD_DISABLE 0
#define IGD 0
#define PEG 1
#define APERTURE_SIZE_256MB 2

  GtConfig->GttSize           = GTT_SIZE_2MB;
  GtConfig->IgdDvmt50PreAlloc = V_SA_GGC_GMS_128MB;

  DEBUG((EFI_D_INFO, "%a.%d GtConfig->InternalGraphics=%X\n", 
    __FUNCTION__, __LINE__, GtConfig->InternalGraphics));
  DEBUG((EFI_D_INFO, "%a.%d GtConfig->PrimaryDisplay=%X\n", 
    __FUNCTION__, __LINE__, GtConfig->PrimaryDisplay));

  
  GtConfig->ApertureSize      = APERTURE_SIZE_256MB;


}