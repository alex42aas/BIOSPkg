/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef	FWBLOCK_SERVICE_PARM_H
#define	FWBLOCK_SERVICE_PARM_H

//	для W25Q64:
#define	G_SPI_INIT_DATA		{\
	{ 0x06, 0x00 },\
	{\
	{ EnumSpiCycle20MHz,      EnumSpiOperationProgramData_1_Byte },\
	{ EnumSpiCycle20MHz,      EnumSpiOperationReadData },\
	{ EnumSpiCycle20MHz,      EnumSpiOperationWriteDisable },\
	{ EnumSpiCycle20MHz,      EnumSpiOperationReadStatus },\
	{ EnumSpiCycle20MHz,      EnumSpiOperationWriteEnable },\
	{ EnumSpiCycle20MHz,      EnumSpiOperationFastRead },\
	{ EnumSpiCycle20MHz,      EnumSpiOperationJedecId },\
	{ EnumSpiCycle20MHz,      EnumSpiOperationErase_4K_Byte }\
	},\
	NULL,\
	0,\
	0\
	}
#endif	/* FWBLOCK_SERVICE_PARM_H */
