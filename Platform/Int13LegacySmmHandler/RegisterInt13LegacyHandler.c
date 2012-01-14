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

  RegisterInt13LegacyHandler.c

Abstract:

  This driver is responsible for the registration of child drivers
  and the abstraction of the PCH SMI sources.

--*/

#include <Protocol/SmmBase.h>
#include <Library/SmmServicesTableLib.h>


#include "Int13LegacySmmHandler.h"



EFI_SMM_SW_DISPATCH_CONTEXT  mSmmSwInt13DispatchContext;
EFI_HANDLE                   mSmmSwInt13DispatchHandle;


/*++

Routine Description:

  Initializes the Smm handler for Legacy Int13

Arguments:
  
  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:
  Status        - EFI_SUCCESS

--*/

//
// Driver entry point
//
EFI_STATUS
EFIAPI
RegisterInt13LegacySmmHandler (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )

{
  EFI_STATUS                    Status;
  EFI_SMM_SW_DISPATCH_PROTOCOL  *SmmSwDispatch;


  DEBUG((EFI_D_INFO, "%a.%d: entry\n", __FUNCTION__, __LINE__));

  
  Status = gBS->LocateProtocol (&gEfiSmmSwDispatchProtocolGuid, NULL, (VOID**)&SmmSwDispatch);
  DEBUG((EFI_D_INFO, "%a.%d: gEfiSmmSwDispatchProtocol, Status = %x\n", __FUNCTION__, __LINE__, Status));
 
  mSmmSwInt13DispatchContext.SwSmiInputValue = APM_CNT_INT13_CMD; 
  Status = SmmSwDispatch->Register (
    SmmSwDispatch,
    SwDispatchInt13Hahdler,
    &mSmmSwInt13DispatchContext,
    &mSmmSwInt13DispatchHandle
  );
  
  ASSERT_EFI_ERROR(Status);

  

  return EFI_SUCCESS;

}