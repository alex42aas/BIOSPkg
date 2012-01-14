/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef	COMMON_ADDRESSES_H
#define	COMMON_ADDRESSES_H


#define MCH_BASE_ADDRESS		0xfed10000

#define PCH_TEMP_MEM_BASE_ADDRESS       0xDFFF0000	// 64k под временные (на время PEI) MMIO для PCI-устройств

#define PCH_HPET_BASE_ADDRESS           0xFED00000	// 16k

#define	THERMAL_BASEB			0xfed06000	// 4k



#endif	/* COMMON_ADDRESSES_H	*/