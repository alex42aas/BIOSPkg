/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

/**@file
  Platform PEI driver

**/

//
// The package level header files this module uses
//

#include <PiPei.h>

/*
//
// The Library classes this module consumes
//
*/
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/MtrrLib.h>


#define MIN(a,b)             (((a) < (b)) ? (a) : (b))
#define MAX(a,b)             (((a) > (b)) ? (a) : (b))


#define MTRR_VAR_BASE       0x200
#define MTRR_DEF_TYPE       0x2FF
#define NO_EVICT_MODE       0x2E0
#define MTRR_DEF_TYPE_E     0x800
#define CR0_CACHE_DISABLE   0x40000000
#define CR0_CACHE_NO_WRITE  0x20000000


#define MAX_MTRR_NUMBER       8

VOID
ExitCarMode(
		VOID
		)
{

//  DEBUG((EFI_D_ERROR, "ExitCarMode\n"));

  __asm {
    mov  edx, 0E00F80DCh
    mov  byte ptr [edx], 0x4            ; Disable SPI cache

    mov ecx, MTRR_DEF_TYPE
    rdmsr 
    and eax, ~MTRR_DEF_TYPE_E
    wrmsr

    mov ecx, NO_EVICT_MODE
    rdmsr 
    btr eax, 1
    wrmsr

    mov ecx, NO_EVICT_MODE
    rdmsr 
    btr eax, 0
    wrmsr
    
    invd

    mov eax, cr0
    or  eax, CR0_CACHE_DISABLE | CR0_CACHE_NO_WRITE
    mov cr0, eax

    mov  ecx, 0x200
clearloop:
    xor  eax, eax
    xor  edx, edx
    wrmsr
    inc  ecx 
    cmp  ecx, 0x210
    jb   clearloop

    mov  edx, 0E00F80DCh
    mov  byte ptr [edx], 0x8            ; Enable SPI cache
  }
  
//  DEBUG((EFI_D_ERROR, "ExitCarMode OK\n"));
}

/**   
 * Merge continious ranges
 *
 */
UINT32
MergeMemoryRange( 
    IN VARIABLE_MTRR *VarMtrr, 
    IN UINT32 MtrrMaxNumber,
    IN EFI_PHYSICAL_ADDRESS Address,
    IN EFI_PHYSICAL_ADDRESS Length,
    IN MTRR_MEMORY_CACHE_TYPE Type
    )
{
  UINT32 Index;
  EFI_PHYSICAL_ADDRESS NewBaseAddress;
  EFI_PHYSICAL_ADDRESS NewEndAddress;

  for(Index = 0; Index < MtrrMaxNumber; Index++) {
    if(VarMtrr[Index].Used) {
      if(!((Address > (VarMtrr[Index].BaseAddress + VarMtrr[Index].Length)) ||
            ((Address + Length) < VarMtrr[Index].BaseAddress )) 
          && (VarMtrr[Index].Type == Type)
        ) {

        NewBaseAddress = MIN(VarMtrr[Index].BaseAddress, Address);
        NewEndAddress = MAX((Address + Length), (VarMtrr[Index].BaseAddress + VarMtrr[Index].Length));
        VarMtrr[Index].BaseAddress = NewBaseAddress;
        VarMtrr[Index].Length = NewEndAddress - NewBaseAddress;
        break;
      }
    } else {
      VarMtrr[Index].Used = TRUE;
      VarMtrr[Index].BaseAddress = Address;
      VarMtrr[Index].Length = Length;
      VarMtrr[Index].Type = Type;
      break;
    }
  }

/*  DEBUG((EFI_D_ERROR, "MergeMemoryRange: %d  %llx:%llx T:%lld\n", 
      Index,
      VarMtrr[Index].BaseAddress,
      VarMtrr[Index].Length,
      VarMtrr[Index].Type));*/

  return Index;
}

/**
  Perform Platform PEI initialization.

  @param  FileHandle      Handle of the file being invoked.
  @param  PeiServices     Describes the list of possible PEI Services.

  @return EFI_SUCCESS     The PEIM initialized successfully.

**/
EFI_STATUS
EFIAPI
PlatformMtrr (
  IN EFI_FFS_FILE_HEADER *FfsHeader,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                  Status;
  EFI_PEI_HOB_POINTERS        HobPointer;
  EFI_HOB_RESOURCE_DESCRIPTOR *Resource;
  UINTN                       Reg;
  UINTN                       Index; 
  VARIABLE_MTRR               VarMtrrs[MAX_MTRR_NUMBER];

  ZeroMem(&VarMtrrs, sizeof(VARIABLE_MTRR) * MAX_MTRR_NUMBER);

  ExitCarMode();

  // Setup MTRR's
  HobPointer.Raw = GetHobList();

  while( (HobPointer.Raw = 
		GetNextHob(  EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, HobPointer.Raw)) != NULL) {
      MTRR_MEMORY_CACHE_TYPE CacheType;
      
      Resource = HobPointer.ResourceDescriptor;


      if((Resource->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE)) {
        CacheType = CacheWriteBack;
      } else 
        if ((Resource->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTED)) {
           CacheType = CacheWriteProtected;
        }  else if ((Resource->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE) &&
                    !(Resource->ResourceAttribute & EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE)
                   ) {
           CacheType = CacheWriteCombining;
        }  else {
           CacheType = CacheUncacheable;
        } 
    
      if((CacheType == CacheWriteProtected) || 
          (CacheType == CacheWriteBack)     ||
          (CacheType == CacheWriteCombining)) {
        MergeMemoryRange( 
            VarMtrrs,
            MAX_MTRR_NUMBER,
            Resource->PhysicalStart,
            Resource->ResourceLength,
            CacheType
            );
      }

      HobPointer.Raw = GET_NEXT_HOB(HobPointer);
  }

  for(Index = 0; Index < MAX_MTRR_NUMBER; Index++) {
    if(VarMtrrs[Index].Used == TRUE) {
      Status = MtrrSetMemoryAttribute(
          VarMtrrs[Index].BaseAddress, 
          VarMtrrs[Index].Length,
          (MTRR_MEMORY_CACHE_TYPE) VarMtrrs[Index].Type
          );
      
	ASSERT_EFI_ERROR(Status);
    }
  }

  AsmEnableCache();
  // Build CPU hob
  AsmCpuid(0x80000008, &Reg, NULL, NULL, NULL);
  Reg &= 0xff;
  BuildCpuHob((UINT8)Reg, 16);

  Status = EFI_SUCCESS;
  return Status;
}
