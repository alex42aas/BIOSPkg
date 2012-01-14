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
EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER resMADHdr =
{
	{//EFI_ACPI_DESCRIPTION_HEADER
		EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE,//Signature
		0,//Length
		3,//Revision
		0,//Checksum
		{ 'A','L','T','E','L','L' },//OemId[6]
		0,//OemTableId
		0,//OemRevision
		'A'|'L'<<8|'T'<<16|' '<<24,//CreatorId
		0,//CreatorRevision
	},
	0xFEE00000,//LocalApicAddress;
	EFI_ACPI_3_0_PCAT_COMPAT,//Flags;
};

EFI_ACPI_3_0_IO_APIC_STRUCTURE resIOApic =
{
	EFI_ACPI_3_0_IO_APIC,//Type;
	sizeof(EFI_ACPI_3_0_IO_APIC_STRUCTURE),//Length;
	0x02,//IoApicId;
	0x0,//Reserved;
	0xFEC00000,//IoApicAddress;
	0x0,//GlobalSystemInterruptBase;	
};

EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE resINTSrcOvr0 =
{
	EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE,//Type;
	sizeof(EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE),//Length;
	0x0,//Bus;
	0x0,//Source;
	0x02,//GlobalSystemInterrupt;
	0x0,//Flags;	
};

EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE resINTSrcOvr1 =
{
	EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE,//Type;
	sizeof(EFI_ACPI_3_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE),//Length;
	0x0,//Bus;
	0x09,//Source;
	0x09,//GlobalSystemInterrupt;
	0x0D,//Flags;	
};

EFI_ACPI_3_0_LOCAL_APIC_NMI_STRUCTURE resLOCApicNMI =
{
	EFI_ACPI_3_0_LOCAL_APIC_NMI,//Type;
	sizeof(EFI_ACPI_3_0_LOCAL_APIC_NMI_STRUCTURE),//Length;
	0xFF,//AcpiProcessorId;
	0x05,//Flags;
	0x01,//LocalApicLint;	
};

EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_STRUCTURE resMPApic0 =
{
	EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC,//Type;
	sizeof(EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_STRUCTURE),//Length;
	0x01,//AcpiProcessorId;
	0x0,//ApicId;
	0x01,//Flags;	
};
