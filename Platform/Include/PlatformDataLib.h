/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PLATFORM_DATA_LIB_H_
#define _PLATFORM_DATA_LIB_H_


#ifdef MEMORY_INIT_TYPES
typedef	U32	UINT32;
typedef	U16	UINT16;
typedef	U8	UINT8;
#endif // MEMORY_INIT_TYPES

extern	UINT8	PirqTable[8];

#define	IRQ_ROUTE_SIZE	32
extern	UINT32	IrqRouteTable[];
extern	UINT32	PirqRoute[];

extern	UINT32	mGpioData[];

typedef struct {
UINT8	type;
UINT8	length;
UINT16	handle;
}	SMBIOS_STRUCT_HEADER;

extern	void	*structTable[];

#endif	/*	_PLATFORM_DATA_LIB_H_	*/
