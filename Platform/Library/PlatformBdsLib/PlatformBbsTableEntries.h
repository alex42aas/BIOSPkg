/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PLATFORM_BBS_TABLE_ENTRIES_H_
#define _PLATFORM_BBS_TABLE_ENTRIES_H_

#include <PiDxe.h>
#include <Protocol/LegacyBios.h>
#include <Library/GenericBdsLib.h>
#include <Library/PlatformBdsLib.h>
#include <Library/CommonUtils.h>
#include <Protocol/ScsiIo.h>
#include <Protocol/DiskInfo.h>

#include <Csm/LegacyBiosDxe/LegacyBiosInterface.h>


#define ATA_INFO_CMD_RESULT_SIZE	512
#define ATA_INFO_MODEL_NAME_OFFSET	54
#define BBS_NAME_LEN				32
#define	BBS_ENTRIES_NUMBER			16 


EFI_STATUS PlatformUpdateBbsTable (VOID);





#endif //_PLATFORM_BBS_TABLE_ENTRIES_H_