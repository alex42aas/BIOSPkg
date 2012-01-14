/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef	PLATFORM_H
#define	PLATFORM_H

//
// DEVICE 0 (Memory Controller Hub)
//
#define MC_BUS                     0x00 

#define MCH_PCI_EXPRESS_BASE_ADDRESS  PCD_EDKII_GLUE_PciExpressBaseAddress

//#define MC_MCHBAR_OFFSET           0x48    // GMCH Register Range Base Address



#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)MCH_PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )


#define MmPci64Ptr( Segment, Bus, Device, Function, Register ) \
  ( (volatile UINT64 *)MmPciAddress( Segment, Bus, Device, Function, Register ) )

#define MmPci64( Segment, Bus, Device, Function, Register ) \
  *MmPci64Ptr( Segment, Bus, Device, Function, Register )

#define McD0PciCfg64(Register)	MmPci64(0, MC_BUS, 0, 0, Register)


#endif	/* PLATFORM_H	*/