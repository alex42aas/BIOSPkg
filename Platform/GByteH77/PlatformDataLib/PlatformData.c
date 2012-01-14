/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PlatformDataLib.h>
//#include <PchRegsRcrb.h>


//===================================================================================
// for LegacyBiosPlatform:

// таблица ссответствия PIRQx -> IRQy
//	PIRQ:	      A   B    C   D   E   F     G   H	
UINT8 PirqTable[] = {11, 11, 11, 10, 11, 0x80, 10, 10};		//

//===================================================================================
// for PlatfromInit PEI:
//
UINT32
IrqRouteTable[32] = {
  // GByteH77:
  // RCBA + [3100...317c]
  0x03243200,  0x00000000,  0x00014321,  0x43214321,		// 3100
  0x00000001,  0x00004321,  0x00000001,  0x00000000,		// 3110
  0x00000000,  0x00002321,  0x00000001,  0x00000000,		// 3120
  0x00000000,  0x00000000,  0x00000000,  0x00000000,		// 3130

  0x00000230,  0x32102037,  0x00003216,  0x00003250,		// 3140
  0x00007654,  0x00003210,  0x00000000,  0x00001230,		// 3150
  0x00003210,  0x00000000,  0x00000000,  0x00000000,		// 3160
  0x00000000,  0x00000000,  0x00000000,  0x00000000,		// 3170
};

UINT32	PirqRoute[2] = {0x0a0b0b0b,		//
			0x0a0a800b		//
			};			// PIRQ[n]_ROUTs /d31:f0:60, d31:f0:68/


UINT32 mGpioData[32] =  { 
  // GByteH77 (from addr 0x1c00):
//+0x0000
  0x996ba9ff,
  0x8edf4eff,
  0x00000000,
  0xe8db7fff,
//+0x0010
  0x00000000,	
  0x00000000,	
  0x00040000,	
  0x00000000,	
//+0x0020
  0x00080000,	
  0x00000000,	
  0x00000000,	
  0x00000800,
//+0x0030
  0x02ff00ff,
  0x1f57fff4,
  0xbeff7fa7,
  0x00000000,
//+0x0040
  0x00000130,
  0x00000bf0,
  0x000009df,
  0x00000000,
//+0x0050
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
//+0x0060
  0x01000000,
  0x00000000,
  0x00000000,
  0x00000000,
//+0x0070
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
};  
