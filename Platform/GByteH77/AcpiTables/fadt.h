/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef	FADT_H
#define	FADT_H

//#include	"Acpi30.h"
#include	"Acpi50.h"


//
// ACPI table information used to initialize tables.
//
#define EFI_ACPI_OEM_ID           'A','L','A','S','K','A'   // OEMID 6 bytes long
#define EFI_ACPI_OEM_TABLE_ID     SIGNATURE_64('A',' ','M',' ','I',0,0,0) // OEM table id 8 bytes long
#define EFI_ACPI_OEM_REVISION     0x01072009
#define EFI_ACPI_CREATOR_ID       SIGNATURE_32('A','M','I',' ')
#define EFI_ACPI_CREATOR_REVISION 0x00010013

#define INT_MODEL       0x01
#define PM_PROFILE      0x01
#define SCI_INT_VECTOR  0x0009 
#define SMI_CMD_IO_PORT 0xB2 // If SMM was supported, then this would be 0xB2
#define ACPI_ENABLE     0x0A0
#define ACPI_DISABLE    0x0A1
#define S4BIOS_REQ      0x00
#define PM1a_EVT_BLK    0x00001800
#define PM1b_EVT_BLK    0x00000000
#define PM1a_CNT_BLK    0x00001804
#define PM1b_CNT_BLK    0x00000000
#define PM2_CNT_BLK     0x00001850
#define PM_TMR_BLK      0x00001808
#define GPE0_BLK        0x00001820
#define GPE1_BLK        0x00000000
#define PM1_EVT_LEN     0x04
#define PM1_CNT_LEN     0x02
#define PM2_CNT_LEN     0x01
#define PM_TM_LEN       0x04
#define GPE0_BLK_LEN    0x10
#define GPE1_BLK_LEN    0x00
#define GPE1_BASE       0x00
#define RESERVED        0x00
#define P_LVL2_LAT      0x0065
#define P_LVL3_LAT      0x0439			// must be disabled!
#define FLUSH_SIZE      0x0400
#define FLUSH_STRIDE    0x0010
#define DUTY_OFFSET     0x00
#define DUTY_WIDTH      0x00
#define DAY_ALRM        0x0D
#define MON_ALRM        0x00
#define CENTURY         0x32

#define	 RST_CNT	0x00000CF9
//#define FLAG            ((UINT32)(EFI_ACPI_2_0_WBINVD | EFI_ACPI_2_0_PWR_BUTTON)) 

#endif

