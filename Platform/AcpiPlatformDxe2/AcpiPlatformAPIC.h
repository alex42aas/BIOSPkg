/** @file
  
  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 
//-----------------------------------------------------------------------------
#ifndef _ACPI_PLATFORM_APIC_H_
#define _ACPI_PLATFORM_APIC_H_

#pragma pack(1)

typedef struct _ACPI_PLATFORM_APIC_TMPL
{
	EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER MADHdr;
	EFI_ACPI_3_0_IO_APIC_STRUCTURE IOApic;
	EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE rgINTSrcOvr[2];
	EFI_ACPI_3_0_LOCAL_APIC_NMI_STRUCTURE LOCApicNMI;
	EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_STRUCTURE rgMPApic[1];
} ACPI_PLATFORM_APIC_TMPL, *PACPI_PLATFORM_APIC_TMPL;

#pragma pack()
//-----------------------------------------------------------------------------
EFI_STATUS
EFIAPI
AcpiPlatformAPICInstall (
  VOID
  );
#endif //!_ACPI_PLATFORM_APIC_H_
