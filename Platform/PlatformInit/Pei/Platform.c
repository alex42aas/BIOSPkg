/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

/** @file
  This driver produces LynxPoint Platform initialization


**/


#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>

#ifdef EFI_NO_INTERFACE_DECL
  #define EFI_FORWARD_DECLARATION(x)
#else
  #define EFI_FORWARD_DECLARATION(x) typedef struct _##x x
#endif

#include "SmbusPolicy.h"
#include "Ppi/Stall.h"
#include <Ppi/PciCfg.h>
#include <Ppi/MasterBootMode.h>
#include <Foundation/Ppi/BaseMemoryTest/BaseMemoryTest.h>
#include <Ppi/PlatformMemoryRange/PlatformMemoryRange.h>
#include <Ppi/PlatformMemorySize/PlatformMemorySize.h>
#include "MeChipset.h"
#include "PchRegsLpc.h"
#include "PchRegsRcrb.h"
#include "PchRegsSmbus.h"
#include "PchRegsThermal.h"
#include "SaRegs.h"
#include "HeciRegs.h"

#include <CommonAddresses.h>
#include <PlatformDataLib.h>

#define	PCI_FUNCTION_NUMBER_PCH_HECI	22
// fed0 0000 - fed0 3fff занимает HPET, дальше свободно:
#define	HECI_BASE	0xfed04000		// 16 bytes

#define	MMIO_READ32(addr)		*(UINT32*)(addr)
#define	MMIO_WRITE32(addr, data)	*(UINT32*)(addr) = data

#define PCI_MEM_ADDRESS(bus, dev, func, off)	(PCD_EDKII_GLUE_PciExpressBaseAddress + PCI_LIB_ADDRESS(bus, dev, func, off))
#define MemRead8(addr)			*(UINT8*)(addr)
#define MemWrite8(addr, data)		*(UINT8*)(addr) = (data)
#define MemRead16(addr)			*(UINT16*)(addr)
#define MemWrite16(addr, data)		*(UINT16*)(addr) = (data)
#define MemRead32(addr)			*(UINT32*)(addr)
#define MemWrite32(addr, data)		*(UINT32*)(addr) = (data)
#define MemOr32(addr, data)		*(UINT32*)(addr) |= (data)

//=========================================================================
//
//		SMBUS
//
#define		SMBUS_BASE	0xefa0		// значение из ACPI-таблиц Intel


EFI_GUID  gPeiSmbusPolicyPpiGuid = PEI_SMBUS_POLICY_PPI_GUID;

PEI_SMBUS_POLICY_PPI mSmbusPoicy = {
  SMBUS_BASE,				// Base Adress
  PEI_PCI_CFG_ADDRESS(0, 31, 3, 0),     // Pci controller address D31:F3
  0,					// NumRsvdAddress
  NULL					// RsvdAddress
};


//=========================================================================
//
//		STALL
//
EFI_GUID  gPeiStallPpiGuid = EFI_PEI_STALL_PPI_GUID;


EFI_STATUS
EFIAPI Stall(
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN CONST EFI_PEI_STALL_PPI    *This,
  IN UINTN                      Microseconds
  )
{
#define HPET_1US             0x000F
#define HPET_1MS             0x37EF

  UINT32 Start, Finish;
  UINT32 volatile Now;
  BOOLEAN Done = FALSE;


  Start = MMIO_READ32( PCH_HPET_BASE_ADDRESS + 0xF0);
  Finish = Start + ((UINT32)Microseconds) * HPET_1US;


  do {
    Now = MMIO_READ32( PCH_HPET_BASE_ADDRESS + 0xF0);
    if (Finish > Start) {
      if (Now >= Finish) {
        Done = TRUE;
      }
    } else {
      if ((Now < Start) && (Now >= Finish)) {
        Done = TRUE;
      }
    }
  } while (Done == FALSE);


  return EFI_SUCCESS;
}



EFI_PEI_STALL_PPI mStall = {
  1,
  Stall
};

//=========================================================================
//	Platform Memory size
//

EFI_GUID  gPeiPlatformMemorySizePpiGuid = PEI_PLATFORM_MEMORY_SIZE_PPI_GUID;


#define  MEMORY_TEST_COVER_SPAN         0x40000 
#define  MEMORY_TEST_PATTERN            0x5A5A5A5A 

EFI_STATUS
EFIAPI 
PeiGetMinimumPlatformMemorySize (
IN EFI_PEI_SERVICES **PeiServices,
IN PEI_PLATFORM_MEMORY_SIZE_PPI * This,
IN OUT UINT64 *MemorySize
) 

{
  *MemorySize = 512 * 1024 * 1024;
  return EFI_SUCCESS;
}


PEI_PLATFORM_MEMORY_SIZE_PPI mPlatformMemorySize = {
  PeiGetMinimumPlatformMemorySize
}; 


//=========================================================================
//	Base Memory Test
//

EFI_GUID gPeiBaseMemoryTestPpiGuid = PEI_BASE_MEMORY_TEST_GUID;

EFI_STATUS 
EFIAPI 
PeiBaseMemoryTest ( 
  IN  EFI_PEI_SERVICES                   **PeiServices, 
  IN  PEI_BASE_MEMORY_TEST_PPI           *This,  
  IN  EFI_PHYSICAL_ADDRESS               BeginAddress, 
  IN  UINT64                             MemoryLength, 
  IN  PEI_MEMORY_TEST_OP                 Operation, 
  OUT EFI_PHYSICAL_ADDRESS               *ErrorAddress 
  ) 
/*++ 
 
Routine Description: 
 
  This function checks the memory range in PEI.  
 
Arguments: 
 
  PeiServices     Pointer to PEI Services. 
  This            Pei memory test PPI pointer. 
  BeginAddress    Beginning of the memory address to be checked. 
  MemoryLength    Bytes of memory range to be checked. 
  Operation       Type of memory check operation to be performed. 
  ErrorAddress    Return the address of the error memory address. 
     
Returns: 
   
  EFI_SUCCESS         The operation completed successfully. 
  EFI_DEVICE_ERROR    Memory test failed. It's not safe to use this range of 
memory. 
 
--*/   
{ 
  UINT32                                 TestPattern; 
  UINT32                                 TestMask; 
  UINT32                                 SpanSize; 
  EFI_PHYSICAL_ADDRESS                   TempAddress; 

//  REPORT_STATUS_CODE( EFI_PROGRESS_CODE, EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_TEST);
  TestPattern = MEMORY_TEST_PATTERN; 
  TestMask = 0; 
  SpanSize = 0; 
 
  // 
  // Make sure we don't try and test anything above the max physical address range 
  // 
  ASSERT ( BeginAddress + MemoryLength < MAX_ADDRESS); 
 
  switch (Operation) { 
    case Extensive: 
      SpanSize = 0x4; 
      break; 
    case Sparse: 
    case Quick: 
      SpanSize = MEMORY_TEST_COVER_SPAN; 
      break; 
    case Ignore: 
      goto Done; 
      break; 
  } 
   
  // 
  // Write the test pattern into memory range 
  // 
  TempAddress = BeginAddress; 
  while (TempAddress < BeginAddress + MemoryLength) { 
    (*(UINT32*)(UINTN)TempAddress) = TestPattern; 
    TempAddress += SpanSize; 
  } 
   
  // 
  // Read pattern from memory and compare it 
  // 
  TempAddress = BeginAddress; 
  while (TempAddress < BeginAddress + MemoryLength){ 
    if ((*(UINT32*)(UINTN)TempAddress) != TestPattern) { 
        *ErrorAddress = TempAddress; 
//        REPORT_STATUS_CODE( EFI_ERROR_CODE | EFI_ERROR_UNRECOVERED,  EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_UNCORRECTABLE);

      return EFI_DEVICE_ERROR; 
    } 
    TempAddress += SpanSize; 
  } 
 
Done: 
 
  return EFI_SUCCESS; 
} 



PEI_BASE_MEMORY_TEST_PPI mBaseMemoryTest = {
  PeiBaseMemoryTest
};


//=========================================================================
//	Platform Memory Range
//

EFI_GUID  gPeiPlatformMemoryRangePpiGuid = PEI_PLATFORM_MEMORY_RANGE_PPI_GUID;

EFI_STATUS 
EFIAPI 
PeiChooseRanges ( 
  IN EFI_PEI_SERVICES                     **PeiServices, 
  IN PEI_PLATFORM_MEMORY_RANGE_PPI        *This, 
  IN OUT PEI_MEMORY_RANGE_OPTION_ROM      *OptionRomMask, 
  IN OUT PEI_MEMORY_RANGE_SMRAM           *SmramMask, 
  IN OUT PEI_MEMORY_RANGE_GRAPHICS_MEMORY *GraphicsMemoryMask, 
  IN OUT PEI_MEMORY_RANGE_PCI_MEMORY      *PciMemoryMask 
  ) 
/*++ 
 
Routine Description: 
 
  Fill in bit masks to specify reserved memory ranges. 
 
Arguments: 
 
  PeiServices         - General purpose services available to every PEIM. 
  This                - Pointer to the PEI_PLATFORM_MEMORY_RANGE_PPI 
instance. 
  OptionRomMask       - Reserved memory ranges for Legacy Option ROM. 

  SmramMask           - Reserved memory ranges for SMRAM. 
  GraphicsMemoryMask  - Reserved memory ranges for Graphic Memory. 
  PciMemoryMask       - Reserved memory ranges for PCI. 
 
Returns: 
 
  EFI_SUCCESS - Operation success. 
 
--*/ 
{ 
	
  UINT16   GraphicsControlRegister; 
 
  // 
  // Choose regions to reserve for Option ROM use 
  // 
  *OptionRomMask = PEI_MR_OPTION_ROM_NONE; 
 
  // 
  // Choose regions to reserve for SMRAM 
  // 
  *SmramMask = PEI_MR_SMRAM_NONE; 
 
  // 
  // Choose one or none of the following: 
  //   PEI_MR_SMRAM_ABSEG_128K_NOCACHE 
  //   PEI_MR_SMRAM_HSEG_128K_CACHE 
  //   PEI_MR_SMRAM_HSEG_128K_NOCACHE 
  // 
//  *SmramMask |= PEI_MR_SMRAM_ABSEG_128K_NOCACHE;		// Asgard
 
  // 
  // Choose one or none of the following: 
  //   PEI_MR_SMRAM_TSEG_512K_CACHE 
  //   PEI_MR_SMRAM_TSEG_512K_NOCACHE 
  //   PEI_MR_SMRAM_TSEG_1024K_CACHE 
  //   PEI_MR_SMRAM_TSEG_1024K_NOCACHE 
  // 
//  *SmramMask |= PEI_MR_SMRAM_SIZE_8192K_MASK;			// Asgard

//  *SmramMask |= PEI_MR_SMRAM_SIZE_8192K_MASK | PEI_MR_SMRAM_TSEG_MASK;
//  *SmramMask |= PEI_MR_SMRAM_TSEG_1024K_CACHE; 
  *SmramMask |= (PEI_MR_SMRAM_CACHEABLE_MASK | PEI_MR_SMRAM_TSEG_MASK | PEI_MR_SMRAM_SIZE_8192K_MASK);
 
 // 
  // Choose regions to reserve for Graphics Memory 
  // Choose one or none of the following: 
  //    1MB, 4MB, 8MB, 16MB, 32MB, 48MB, 64MB, 128MB, 256MB 
  // 
  
  GraphicsControlRegister = MemRead16 (PCI_MEM_ADDRESS(0, 0, 0, R_SA_GGC)); 

  if ((GraphicsControlRegister & B_SA_GGC_IVD_MASK) == 0) { 
    switch ((GraphicsControlRegister & B_SA_GGC_GMS_MASK) >> N_SA_GGC_GMS_OFFSET) { 
      case V_SA_GGC_GMS_480MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_480M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_448MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_448M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_416MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_416M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_384MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_384M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_352MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_352M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_320MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_320M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_288MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_288M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_224MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_224M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_192MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_192M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_160MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_160M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_96MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_96M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_512MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_512M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_256MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_256M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_128MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_128M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_64MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_64M_NOCACHE; 
        break; 
      case V_SA_GGC_GMS_32MB: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_32M_NOCACHE; 
        break; 

      default: 
        *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_NONE; 
        break; 
    } 
      //  
      // Add GGMS memory to reserved graphics memory 
      // 
      switch ((GraphicsControlRegister & B_SA_GGC_GGMS_MASK) >> N_SA_GGC_GGMS_OFFSET) { 
        case V_SA_GGC_GGMS_1MB: 
          *GraphicsMemoryMask += PEI_MR_GRAPHICS_MEMORY_1M_NOCACHE; 
          break; 
        case V_SA_GGC_GGMS_2MB: 
          *GraphicsMemoryMask += (PEI_MR_GRAPHICS_MEMORY_1M_NOCACHE * 2); 
          break; 
      } 
  } else { 
    *GraphicsMemoryMask = PEI_MR_GRAPHICS_MEMORY_NONE; 
  } 

  // 
  // Choose regions to reserve for PCI 
  // 
  *PciMemoryMask = 0; 

  return EFI_SUCCESS; 
} 


PEI_PLATFORM_MEMORY_RANGE_PPI mPlatformMemoryRange = {
  PeiChooseRanges
}; 

//=========================================================================

CONST EFI_PEI_PPI_DESCRIPTOR     mPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI),
    &gPeiSmbusPolicyPpiGuid,
    &mSmbusPoicy
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI),
    &gEfiPeiStallPpiGuid,
    &mStall
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI),
    &gPeiPlatformMemorySizePpiGuid,
    &mPlatformMemorySize
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI),
    &gPeiBaseMemoryTestPpiGuid,
    &mBaseMemoryTest
  },

  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gPeiPlatformMemoryRangePpiGuid,
    &mPlatformMemoryRange
  }

};



//===============================================================================
//
//	Задаем BootMode, устанавливаем PEI_MASTER_BOOT_MODE_PEIM_PPI
//	(скопировано из GByte-H77)
//

#ifndef _REDIRECT_DEBUG_LIB_H_


int	inport(int addr){
int	res;

	__asm	mov	edx, addr
	__asm	xor	eax, eax
	__asm	in	ax, dx
	__asm	mov	res, ax

	return	res;
}

#endif // !_PLATFORM_DEBUG_LIB_H_

int	testS4(UINT32 PmBase){

        if((inport(PmBase) & 0x1c00) != 0x1800)
							return FALSE;
        return TRUE;
}

int	testS3(UINT32 PmBase){
	if(MemRead8(PCI_MEM_ADDRESS(0, 
				    PCI_DEVICE_NUMBER_PCH_LPC, 
				    PCI_FUNCTION_NUMBER_PCH_LPC, 
				    R_PCH_LPC_GEN_PMCON_3)) 
							& 0x02)
          { //Power Failure
            return FALSE;
          }
	if(inport(PmBase) & 0x800)
          { // Power Button Override occurs:
            return FALSE;
          }
	if((inport(PmBase) >> 15) == 0)
          { // wake event occurs
            return FALSE;
          }
        if((inport(PmBase + 4) & 0x1c00) != 0x1400)
                                       return FALSE;
        return TRUE;
}


EFI_GUID  gPeiMasterBootModePpiGuid = EFI_PEI_MASTER_BOOT_MODE_PEIM_PPI;

EFI_PEI_PPI_DESCRIPTOR     BootModePpiDsc = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gPeiMasterBootModePpiGuid,
    NULL
};


void    setBootMode(CONST EFI_PEI_SERVICES **PeiServices){
UINT32	mode;
UINT32	PmBase;

	PmBase = MemRead32(PCI_MEM_ADDRESS(0, 
				    PCI_DEVICE_NUMBER_PCH_LPC, 
				    PCI_FUNCTION_NUMBER_PCH_LPC, 
				    R_PCH_LPC_ACPI_BASE)) & ~1;

	if(testS3(PmBase) == TRUE)
          {
             mode = BOOT_ON_S3_RESUME;
             goto	next;
          }
	if(testS4(PmBase) == TRUE)
          {
             mode = BOOT_ON_S4_RESUME;
             goto	next;
          }

	mode = BOOT_WITH_FULL_CONFIGURATION;
next:
	(*PeiServices)->SetBootMode(PeiServices, mode);
	(*PeiServices)->InstallPpi(PeiServices, &BootModePpiDsc);

}
//===============================================================================



void	getCpuid(UINT32 index, UINT32 *peax, UINT32 *pebx, UINT32 *pecx, UINT32 *pedx){

	__asm	pushad
	__asm	mov	eax, index
	__asm	cpuid
	__asm	push	ecx
	__asm	mov	ecx, peax
	__asm	jecxz	l1
	__asm	mov	[ecx], eax	;
//	--------------------------------
l1:
	__asm	pop	eax
	__asm	mov	ecx, pebx
	__asm	jecxz	l2
	__asm	mov	[ecx], ebx
//	--------------------------------
l2:
	__asm	mov	ecx, pecx
	__asm	jecxz	l3
	__asm	mov	[ecx], eax
//	--------------------------------
l3:
	__asm	mov	ecx, pedx
	__asm	jecxz	l4
	__asm	mov	[ecx], edx
//	--------------------------------
l4:
	__asm	popad


}


EFI_STATUS
EFIAPI
PlatformEntryPoint (
  IN EFI_FFS_FILE_HEADER *FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS		Status = EFI_SUCCESS;
  UINT32		Capabilities;
  UINT32		RootComplexBar;
  EFI_RESOURCE_ATTRIBUTE_TYPE Attributes;
  int			tmp;
/*

  It is strongly recommended that 

	*** NOT enable any SATA port in the platform code during PEI phase.  ***

  It’s to avoid enabling any SATA port before SATA HSIO initialization 
is done.
	(from Pch Reference Code Manual, 5.5.2  PEI PCH Initialization)
*/

//  PostCode(0x21);

  // 1. "Intel® 8 Series/C220 Series Chipset Family, Lynx Point-LP Platform Controller Hub (PCH) Framework", 
  //	5.5.1  SEC PCH Initialization:
  RootComplexBar = MemRead32(PCI_MEM_ADDRESS(0, 
				    PCI_DEVICE_NUMBER_PCH_LPC, 
				    PCI_FUNCTION_NUMBER_PCH_LPC, 
				    R_PCH_LPC_RCBA)) & ~1;

  // read dword 0 from FVEC:
  // 0 -> index:
  MemWrite32(PCI_MEM_ADDRESS(0, 
			     PCI_DEVICE_NUMBER_PCH_LPC, 
			     PCI_FUNCTION_NUMBER_PCH_LPC, 
			     R_PCH_LPC_FVECIDX), 
	     0);
  // read data from [0]:
  Capabilities =  MemRead32(PCI_MEM_ADDRESS(0, 
					    PCI_DEVICE_NUMBER_PCH_LPC, 
					    PCI_FUNCTION_NUMBER_PCH_LPC, 
					    R_PCH_LPC_FVECD));

  if(Capabilities & 0x2) 
  { // disable PCI-to-PCI bridge(?):
	  MemOr32 (RootComplexBar + R_PCH_RCRB_FUNC_DIS, 0x2);
	  MemRead32 (RootComplexBar + R_PCH_RCRB_FUNC_DIS);	// ; Reads back for posted write to take effect

  }

// 2. SMBUS:
	MemWrite32(PCI_MEM_ADDRESS(0, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_SMBUS, R_PCH_SMBUS_BASE), SMBUS_BASE | 1);

// 3. HECI:
	MemWrite32(PCI_MEM_ADDRESS(0, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, R_HECIMBAR ), HECI_BASE | 0x4);
	tmp = MemRead16(PCI_MEM_ADDRESS(0, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, R_COMMAND ));
	// Set CMD.BME and CMD.MSE:
	MemWrite16(PCI_MEM_ADDRESS(0, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, R_COMMAND ), (UINT16)(tmp | 0x6));


// Install all services:
  Status = (*PeiServices)->InstallPpi(PeiServices, mPpiList);
  ASSERT_EFI_ERROR(Status);

// выясняем и сообщаем всем режим загрузки BootMode:
  setBootMode(PeiServices);

	Attributes = (EFI_RESOURCE_ATTRIBUTE_PRESENT    
		        |  EFI_RESOURCE_ATTRIBUTE_INITIALIZED 
				|  EFI_RESOURCE_ATTRIBUTE_TESTED 
					|  EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE);

	// Add reserved and memory mapped IO spaces
	// под HPET, RCBA, MCHBAR etc.  (уже роздано)
	BuildResourceDescriptorHob (
		EFI_RESOURCE_MEMORY_MAPPED_IO,
		Attributes,
		(EFI_PHYSICAL_ADDRESS) 0xFED00000,
		(EFI_PHYSICAL_ADDRESS) 0x00100000
		);  
	// под PCI-E (уже роздано)
	BuildResourceDescriptorHob (
		EFI_RESOURCE_MEMORY_MAPPED_IO,
		Attributes,
		(EFI_PHYSICAL_ADDRESS) 0xF8000000,
		(EFI_PHYSICAL_ADDRESS) 0x04000000
		);  

	BuildResourceDescriptorHob (
		EFI_RESOURCE_FIRMWARE_DEVICE,
		 Attributes,
		FixedPcdGet32(PcdBiosFlashBase),
		FixedPcdGet32(PcdBiosFlashSize)
		);

    // NvStorages:
	BuildFvHob ( 
			FixedPcdGet32(PcdFlashNvStorageVariableBase),
			FixedPcdGet32(PcdFlashNvStorageVariableSize) +  
			FixedPcdGet32(PcdFlashNvStorageFtwWorkingSize) +
			FixedPcdGet32(PcdFlashNvStorageFtwSpareSize)
			);

	BuildFvHob ( 
			FixedPcdGet32(PcdFlashFvRecoveryBase),
			FixedPcdGet32(PcdFlashFvRecoverySize)
			);

	if (FixedPcdGet32(PcdMicrocodeSize) != 0) {
	    BuildFvHob (
			FixedPcdGet32(PcdMicrocodeBase),
			FixedPcdGet32(PcdMicrocodeSize)
			);
	  }


    //	PEI:
	BuildFvHob ( 
			FixedPcdGet32(PcdBootFirmwareVolumeBase),
			FixedPcdGet32(PcdBootFirmwareVolumeSize)
			);

    //	SystemVol:
	if (FixedPcdGet32(PcdSystemVolumeSize) != 0) {
	    BuildFvHob (
			FixedPcdGet32(PcdSystemVolumeBase),
			FixedPcdGet32(PcdSystemVolumeSize)
			);
	  }


	if (FixedPcdGet32(PcdNewFvSize) != 0) {
		BuildFvHob (
			FixedPcdGet32(PcdNewFvBase),
			FixedPcdGet32(PcdNewFvSize)
		);
	 }


{	
int	Index;
	
	for(Index = 0; Index < IRQ_ROUTE_SIZE; Index++) 
	  {
		MMIO_WRITE32(RootComplexBar + 0x3100 + Index * 4, IrqRouteTable[Index]);
	  }
}
	// PIRQ[n]_ROUTs:
	MemWrite32(PCI_MEM_ADDRESS(0, 
			     PCI_DEVICE_NUMBER_PCH_LPC, 
			     PCI_FUNCTION_NUMBER_PCH_LPC, 
			     R_PCH_LPC_PIRQA_ROUT), 
	     PirqRoute[0]);
	MemWrite32(PCI_MEM_ADDRESS(0, 
			     PCI_DEVICE_NUMBER_PCH_LPC, 
			     PCI_FUNCTION_NUMBER_PCH_LPC, 
			     R_PCH_LPC_PIRQE_ROUT), 
	     PirqRoute[1]);

	// Set PIRQA to PINA to IGD:
	MemWrite8(PCI_MEM_ADDRESS(0, 2, 0, 0x3C), (UINT8)PirqRoute[0]);

	// Build CPU hob
{
UINT32	Reg;

	getCpuid(0x80000008, &Reg, NULL, NULL, NULL);
	Reg &= 0xff;
	BuildCpuHob((UINT8)Reg, 16);
}

  return Status;
}
