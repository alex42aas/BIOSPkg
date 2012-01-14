/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Mobile Silicon Support Module" and is
  licensed for Intel Mobile CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchSmmCore.c

Abstract:

  This driver is responsible for the registration of child drivers
  and the abstraction of the PCH SMI sources.

--*/


#include "Int13LegacySmmHandler.h"
#include <Protocol\Legacy8259.h>
#include <Protocol\Timer.h>
#include <Library\TimerLib.h>
#include <Protocol\ScsiPassThru.h>
#include <Protocol\ScsiIo.h>
#include <Protocol\DiskInfo.h>

enum {
	AH_00 = 0x00,
	AH_02 = 0x02,
	AH_08 = 0x08,
	AH_15 = 0x15,
	AH_42 = 0x42,
	AH_43 = 0x43,
	AH_41 = 0x41,
	AH_48 = 0x48,
	AH_4B = 0x4B
};


//Пока что логика такая. 
//Первый HardDriveBlockIo[0]  = 80h
//Если грузимся с жесткого диска, то первым будет выбранный. Остальные в порядке, в котором будут найдены
//если грузимся с CD-ROM, то жесткие диски просто в порядке нахождения. 

EFI_BLOCK_IO_PROTOCOL*  CdBlockIo = NULL; //будет только один
CDROM_SPEC_PACKET       SpecPacket;


EFI_BLOCK_IO_PROTOCOL*  HardDriveBlockIo[MAX_HDD_SUPPORTED] = { NULL };
UINT16                  ActualHardDriveCount = 0;
EFI_BLOCK_IO_PROTOCOL*  BootDevice = NULL;


INT13_PRIVATE           *Int13Private = NULL;
SAVE_REGS               *Regs = NULL;
BBS_TABLE               *BbsEntry;




EFI_STATUS	FindSataBlockIo(UINT32 Port, UINT32 Device, UINT32 Function, UINT16 BbsDeviceType) {

  EFI_STATUS                Status;
  EFI_STATUS                Ret = EFI_NOT_FOUND;
  UINTN                     Index;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *dpFullDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *dpPciNode;
  EFI_DEVICE_PATH_PROTOCOL  *dpCurrentNode;
  EFI_DEVICE_PATH_PROTOCOL  *dpPrevNode;
  EFI_HANDLE                *BlockIoHandles;
  UINTN                     NumberHandles;
  UINTN                     bFound;
  SATA_DEVICE_PATH          *Sata;
  PCI_DEVICE_PATH           *Pci;
  CHAR16                    *TempStr;
  UINTN                     HardDriveIndex = 0;
  BOOLEAN                   isCdromDevice = FALSE;
  EFI_SCSI_IO_PROTOCOL      *pScsiIo;
  EFI_BLOCK_IO_PROTOCOL     *blockio;
  

  if (BbsDeviceType != BBS_CDROM && BbsDeviceType != BBS_HARDDISK) {
	  DEBUG((EFI_D_ERROR, "%a.%d: Invalid device type\n", __FUNCTION__, __LINE__));
	  return EFI_INVALID_PARAMETER;
  }

  
  // 1. Все BlockIo протоколы:
  gBS->LocateHandleBuffer (
		ByProtocol,
		&gEfiBlockIoProtocolGuid,
		NULL,
		&NumberHandles,
		&BlockIoHandles
		);


  for (Index = 0; Index < NumberHandles; Index++) {

    Status = gBS->HandleProtocol (
		  BlockIoHandles[Index],
		  &gEfiDevicePathProtocolGuid,
		  (VOID**)&DevicePath);

    if (EFI_ERROR (Status)) {
      continue;
    }

    bFound = FALSE;
    dpFullDevicePath = DuplicateDevicePath(DevicePath);		
    dpPciNode = DevicePath;		
    dpCurrentNode = DevicePath;

    while (!IsDevicePathEnd(dpCurrentNode)) {

      if (dpCurrentNode->Type == MESSAGING_DEVICE_PATH
				&& dpCurrentNode->SubType == MSG_SATA_DP) {				
		bFound = TRUE;
		break;
      }
						
      // Переход к следующему узлу
      dpPciNode = dpCurrentNode;
      dpCurrentNode = NextDevicePathNode(dpCurrentNode);
    } // while

    TempStr = ConvertDevicePathToText (dpFullDevicePath, TRUE, TRUE);

    if (!bFound) {
		// не SATA-устройство
        FreePool(dpFullDevicePath);
		continue;			
    }

    // Переход к следующему узлу
    dpPrevNode = dpCurrentNode;
    dpCurrentNode = NextDevicePathNode(dpCurrentNode);

    if (!IsDevicePathEnd(dpCurrentNode)) {
	// это логическое устройство, т.к. у него есть дополнительная часть пути
	// (раздел, логический диск)
        FreePool(dpFullDevicePath);
		continue;	
    } 

	isCdromDevice = FALSE;
	Status = gBS->HandleProtocol (
				BlockIoHandles[Index],
				&gEfiScsiIoProtocolGuid,
				(VOID**) &pScsiIo);
	if (!EFI_ERROR(Status)) {
		isCdromDevice = TRUE;
	}

	Status = gBS->HandleProtocol (
				BlockIoHandles[Index],
				&gEfiBlockIoProtocolGuid,
				(VOID**)&blockio);
	ASSERT_EFI_ERROR(Status);


	if (isCdromDevice) {
		CdBlockIo = blockio;
	} else {
		//hard drive
		if ((HardDriveIndex + 1) >= MAX_HDD_SUPPORTED) {
			//TODO: подумать, как сделать нормальную обработку такой ситуации. ибо дисков может быть много
			DEBUG((EFI_D_ERROR, "%a.%d Too many hard drives\n", __FUNCTION__, __LINE__));
			ASSERT(FALSE);
			continue;	
		}

		HardDriveBlockIo[HardDriveIndex++] = blockio;
		++ActualHardDriveCount;
	}
			
    Sata = (SATA_DEVICE_PATH*)dpPrevNode;
	Pci =  (PCI_DEVICE_PATH*)dpPciNode;
	
    if((Port == Sata->HBAPortNumber) && (Device == Pci->Device) && (Function == Pci->Function))
    { // нашли требуемое устройство:

		BootDevice = blockio;
		Ret = EFI_SUCCESS;
    }

  }

  //если грузимся с жесткого диска, нужно выставить его первым по приоритету
  if (BbsDeviceType == BBS_HARDDISK) {

	blockio = HardDriveBlockIo[0];
	for (HardDriveIndex = 1; HardDriveIndex < ActualHardDriveCount; HardDriveIndex++) {

		if (BootDevice == HardDriveBlockIo[HardDriveIndex]) {

			HardDriveBlockIo[HardDriveIndex] = blockio;
			HardDriveBlockIo[0] = BootDevice;

			break;
		}
	}
  }

  return Ret;
}






EFI_STATUS FirstCall(VOID) {

  UINT32      BdaInt13Private = 0;
  EFI_STATUS  Status = EFI_SUCCESS;
  EFI_TPL     OldTpl;

  // первый вызов: получаем указатели
  BdaInt13Private = *(UINT32*)(UINTN) INT13_PRIVATE_ADDR;
  Int13Private = (INT13_PRIVATE*)(UINTN) (((BdaInt13Private >> 12) & 0xffff0) + (BdaInt13Private & 0xffff));
  Regs = &Int13Private->SaveRegs;
  BbsEntry = (BBS_TABLE*)(UINTN) (((Int13Private->BbsEntry >> 12) & 0xffff0) + (Int13Private->BbsEntry & 0xffff));

  DEBUG((EFI_D_INFO, "\r\n BdaInt13Private = %x, Int13Private = %lx, BbsEntry = %08x\n", BdaInt13Private, Int13Private, BbsEntry));

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(TPL_APPLICATION);

  Status = FindSataBlockIo( BbsEntry->BootHandlerOffset,  BbsEntry->Device,  BbsEntry->Function, BbsEntry->DeviceType);

  gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(OldTpl);

  return EFI_SUCCESS;
}




EFI_STATUS
SwDispatchInt13Hahdler (
  IN EFI_HANDLE    DispatchHandle,
  IN EFI_SMM_SW_DISPATCH_CONTEXT   * DispatchContext
    )
{
  EFI_STATUS             Status = EFI_SUCCESS;
  EFI_BLOCK_IO_PROTOCOL  *blockio = NULL;
  

  if(Regs == NULL)
  { 
	 return FirstCall();

  }

  if (!BootDevice) {
	  DEBUG((EFI_D_ERROR, "Block io is null! Exit...\n"));
	  return EFI_SUCCESS;
  }


  if(Int13Private->BbsDeviceType == BBS_CDROM && Regs->ax.val == CD_DVD_BOOT_EVENT) {

    DEBUG((EFI_D_INFO, "\r\n%a.%d: Boot from CD/DVD\n", __FUNCTION__, __LINE__));
	Status = PerformCdBoot(CdBlockIo);

	 if(EFI_ERROR(Status)) {
		Regs->flags.val |= 1;	// flagC
	 } else {
		 Regs->flags.val &= (UINT16)(~1);
	 }

	 return EFI_SUCCESS;
  }


  blockio = getPreferredBlockIo(Regs->dx.reg.lo);
  if (!blockio) {

	  Regs->flags.val |= 1;	// flagC
	  Regs->ax.reg.hi = DISK_RET_EPARAM;
	  return EFI_SUCCESS;
  }

  switch(Regs->ax.reg.hi)  {
  case AH_00:
	Regs->ax.reg.hi = DISK_RET_SUCCESS;
	Status = EFI_SUCCESS;
	break;

  case AH_02:
	Status = ReadSector(blockio, Regs->ax.reg.lo, Regs->cx.reg.hi, Regs->dx.reg.hi,  Regs->cx.reg.lo, SEGMENT_TO_LINEAR(Regs->es.val, Regs->bx.val));  
	Regs->ax.reg.hi = DISK_RET_SUCCESS;
    break;

  case AH_08:
    //WARNING: возможное проблемное место
	Regs->ax.reg.hi = DISK_RET_SUCCESS;
	Status = EFI_SUCCESS;
	Regs->cx.val = 0xFFFF;
	Regs->dx.val = ActualHardDriveCount;
    Regs->dx.reg.hi = 0xFF;
	break;

  case AH_15:
	Regs->ax.reg.hi = 0x03;
	Status = EFI_SUCCESS;
	break;

  case AH_41:
	Status = EFI_SUCCESS;
	Regs->bx.val = 0x0aa55;
	Regs->cx.val = 0x05; 
	Regs->ax.reg.hi = 0x30;
	break;

  case AH_42:
	Status = ExtPerformSectorOperation(blockio, (DAP*)(UINTN)SEGMENT_TO_LINEAR(Regs->ds.val, Regs->si.val), TRUE);
	Regs->ax.reg.hi = DISK_RET_SUCCESS;
	break;

  case AH_43:
	Status = Status = ExtPerformSectorOperation(blockio, (DAP*)(UINTN)SEGMENT_TO_LINEAR(Regs->ds.val, Regs->si.val), FALSE);
	Regs->ax.reg.hi = DISK_RET_SUCCESS;
	break;

  case AH_48:
	Status = GetDriveParameters(blockio, SEGMENT_TO_LINEAR(Regs->ds.val, Regs->si.val));
	Regs->ax.val = DISK_RET_SUCCESS;
	break;
  case AH_4B:
	if (Regs->dx.reg.lo == PRIMARY_CDROM_IDENT) {
		Status = CdRomFillSpecPacket(SEGMENT_TO_LINEAR(Regs->ds.val, Regs->si.val));
		Regs->ax.val = DISK_RET_SUCCESS;
	} else {
		Status = EFI_NOT_FOUND;
		Regs->ax.reg.hi = DISK_RET_EPARAM;
	}
	break;


  default:
    DEBUG((EFI_D_ERROR, "\r\n%a.%d: Invalid Ah value; Value = 0x%x\n", __FUNCTION__, __LINE__, Regs->ax.reg.hi));
    DEBUG((EFI_D_ERROR, "\r\n Regs: ax = %04x, bx = %04x, cx = %04x, dx = %04x \n", Regs->ax.val, Regs->bx.val, Regs->cx.val, Regs->dx.val));
    DEBUG((EFI_D_ERROR, "\r\n Regs: si = %04x, di = %04x, bp = %04x, sp = %04x \n", Regs->si.val, Regs->di.val, Regs->bp.val, Regs->sp.val));
    DEBUG((EFI_D_ERROR, "\r\n Regs: ds = %04x, es = %04x, flags = %04x \n", Regs->ds.val, Regs->es.val, Regs->flags.val));
	Status = EFI_NOT_AVAILABLE_YET;
    Regs->ax.reg.hi = DISK_RET_EPARAM;
    break;
  }

  if(EFI_ERROR(Status)) {

     Regs->flags.val |= 1;	// flagC
	 Regs->ax.reg.hi = DISK_RET_EPARAM;

  } else {

	  Regs->flags.val &= (UINT16)(~1);
  }
  return EFI_SUCCESS;
}


EFI_STATUS ReadSector(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN UINTN Count, IN UINT8 Cilynder, IN UINT8 Head, IN UINT64 Sector, IN UINT32 OutputAddress) {

  UINTN LBA = Sector - 1;
  EFI_STATUS Status = EFI_SUCCESS;

  if (!blockIo->Media->MediaPresent) {
    return EFI_NO_MEDIA;
  }

  ASSERT(blockIo != NULL);
  DEBUG((EFI_D_INFO, "Read Sector Func; Count = 0x%x; Sector = 0x%x; Cilynder = 0x%x; Head = 0x%x; OutputAddress = 0x%x\n", Count, Sector, Cilynder, Head, OutputAddress));

  Status =  ReadBlocksFromLba(blockIo, Count, LBA, OutputAddress);
  return Status;
}


EFI_STATUS ExtPerformSectorOperation(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN DAP* Descriptor, BOOLEAN isRead) {

  UINTN OutputAddress = (UINTN)SEGMENT_TO_LINEAR(Descriptor->BufferSegment, Descriptor->BufferOffset);
  UINTN FirstSector = Descriptor->FirstSector;


  if (!blockIo->Media->MediaPresent) {
    return EFI_NO_MEDIA;
  }

  if (Descriptor->DapSize == 0x18) {
    OutputAddress = Descriptor->FlatAddress;	
  }

  if (isRead)
	return ReadBlocksFromLba(blockIo, Descriptor->NumberOfSectors, FirstSector, OutputAddress);
  else
	return WriteBlocksFromLba(blockIo, Descriptor->NumberOfSectors, FirstSector, OutputAddress);
}


EFI_STATUS ReadBlocksFromLba(IN EFI_BLOCK_IO_PROTOCOL* blockIo, 
                             IN UINTN Count, 
                             IN UINTN Start, 
                             IN UINTN OutputAddress) {
  EFI_TPL     OldTpl;
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT32      SectorSize = blockIo->Media->BlockSize;

  if (!blockIo->Media->MediaPresent) {
    return EFI_NO_MEDIA;
  }

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(TPL_APPLICATION | DISABLE_TPL_NOTIFICATION);

  Status = blockIo->ReadBlocks(blockIo, blockIo->Media->MediaId, Start,  Count*SectorSize, (VOID*)(UINTN)OutputAddress);

  gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(OldTpl);

  ASSERT_EFI_ERROR(Status);

  return Status;
}


EFI_STATUS WriteBlocksFromLba(IN EFI_BLOCK_IO_PROTOCOL* blockIo, 
                              IN UINTN Count, 
                              IN UINTN Start, 
                              IN UINTN OutputAddress) {

  EFI_TPL     OldTpl;
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT32      SectorSize = blockIo->Media->BlockSize;

  if (!blockIo->Media->MediaPresent) {
    return EFI_NO_MEDIA;
  }

  if (blockIo->Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(TPL_APPLICATION | DISABLE_TPL_NOTIFICATION);

  Status = blockIo->WriteBlocks(blockIo, blockIo->Media->MediaId, Start,  Count*SectorSize, (VOID*)(UINTN)OutputAddress);

  gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(OldTpl);

  ASSERT_EFI_ERROR(Status);

  return Status;
}


EFI_STATUS GetDriveParameters(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN UINT32 OutputAddr) {

  DRIVE_PARAMETERS* params = (DRIVE_PARAMETERS*)(UINTN)OutputAddr;

  if (!blockIo->Media->MediaPresent) {
    DEBUG((EFI_D_ERROR, "Media is not present\n"));
    return EFI_NO_MEDIA;
  }


  if (params->BufferSize < 26)
		return EFI_BUFFER_TOO_SMALL;
  else if (params->BufferSize < 30)
		params->BufferSize = 26;
       else if (params->BufferSize < 0x42)
		params->BufferSize = 30;
            else 
		params->BufferSize = 0x42;


  params->BytesPerSector = (UINT16)blockIo->Media->BlockSize;
  params->InformationFlags = 0;
  params->PhysicalCylinderCount = 0; //not valid
  params->PhysicalHeadsCount = 0; //not valid
  params->PhysicalSectorPerTrackCount = 0; //not valid
	
  params->SectorCount = blockIo->Media->LastBlock + 1;

  if (params->BufferSize > 26) {
    params->PointerToEDDConfig = 0xFFFFFFFF;
  }


  if (params->BufferSize > 30) {
    params->DevicePathPresent = 0xBEDD;
    params->DevicePathLength = 36;
    params->Reversed1 = 0;
    params->Reversed2 = 0;
    params->Signature = 0x00494350;
    params->InterfaceType = 0x00415451;
    params->Interfacepath.pci.Bus = (UINT8)BbsEntry->Bus;
    params->Interfacepath.pci.Slot = (UINT8)BbsEntry->Device;
    params->Interfacepath.pci.Function = (UINT8)BbsEntry->Function;
    params->DevicePath = 0; //not implemented yet
    params->Reversed3 = 0;
    params->CheckSum = 0;
  }

  return EFI_SUCCESS;
}


EFI_STATUS PerformCdBoot(IN EFI_BLOCK_IO_PROTOCOL* cdBlockIo) {

  UINT32                   BootImageLba;
  EFI_STATUS               Status = EFI_SUCCESS;
  EFI_TPL                  OldTpl;
  CDROM_VOLUME_DESCRIPTOR  *Descriptor = (CDROM_VOLUME_DESCRIPTOR*)(UINTN)BOOT_SECTOR_OUTPUT_ADDR;
  ELTORITO_CATALOG         *Catalog = (ELTORITO_CATALOG*)(UINTN)BOOT_SECTOR_OUTPUT_ADDR;
  VOID                     *BootSectorAddress = (VOID*)(UINTN)BOOT_SECTOR_OUTPUT_ADDR;
  UINT32                   BlockSize = cdBlockIo->Media->BlockSize;
  UINT32                   BootCatalogSectror;

  ASSERT(cdBlockIo != NULL);
  ASSERT(BlockSize == 2048);

  if (!cdBlockIo->Media->MediaPresent) {
    return EFI_NO_MEDIA;
  }

  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(TPL_CALLBACK | DISABLE_TPL_NOTIFICATION);

  Status = cdBlockIo->ReadBlocks(cdBlockIo, cdBlockIo->Media->MediaId, 0x11, BlockSize, Descriptor);

  if (EFI_ERROR(Status)) 
		return Status;

  BootCatalogSectror = *(UINT32*)Descriptor->BootRecordVolume.EltCatalog;
	
  Status = cdBlockIo->ReadBlocks(cdBlockIo, cdBlockIo->Media->MediaId, BootCatalogSectror, BlockSize, Catalog);

  if (EFI_ERROR(Status)) 
		return Status;

  if (Catalog->Catalog.Id55AA != 0xAA55) {
    DEBUG((EFI_D_ERROR, "Wrong validation entry!!!\n"));
    return EFI_NOT_FOUND;
  }

  ++Catalog;

  //Initial boot entry
  if (Catalog->Boot.Indicator != 0x88) {
    return EFI_NOT_FOUND;
  }

  BootImageLba = Catalog->Boot.Lba; 

  DEBUG((EFI_D_INFO, "BootImageLba = 0x%x\n", BootImageLba));
  DEBUG((EFI_D_INFO, "EmulationType = 0x%x\n", Catalog->Boot.MediaType));
  DEBUG((EFI_D_INFO, "SectorCount = 0x%x\n", Catalog->Boot.SectorCount));
  DEBUG((EFI_D_INFO, "LoadingSegment = 0x%x\n", Catalog->Boot.LoadSegment));

  if (Catalog->Boot.LoadSegment != 0) {
    BootSectorAddress = (VOID*)(UINTN)SEGMENT_TO_LINEAR(Catalog->Boot.LoadSegment, 0);
  }

  SpecPacket.BootMediaType = Catalog->Boot.MediaType;
  SpecPacket.LbaOfDiskEmulated = BootImageLba;
  SpecPacket.LoadSegment = Catalog->Boot.LoadSegment;
  SpecPacket.SectorCount =  Catalog->Boot.SectorCount;
  SpecPacket.PacketSize = 0x13;
  SpecPacket.DriveNumber = PRIMARY_CDROM_IDENT;
  SpecPacket.ControllerIndex = 0;
  SpecPacket.DeviceSpecification = 0;
  SpecPacket.UserBufferSegment = 0;
  SpecPacket.AH_08_CilynderCount = 0;
  SpecPacket.AH_08_SectorCount = 0;
  SpecPacket.AH_08_HeadCount = 0;

  if (Catalog->Boot.MediaType != 0) {
    DEBUG((EFI_D_ERROR, "Emulation mode for cd boot not supported yet...\n"));
    return EFI_UNSUPPORTED;
  }

  Status = cdBlockIo->ReadBlocks(cdBlockIo, cdBlockIo->Media->MediaId, BootImageLba, BlockSize*(Catalog->Boot.SectorCount / 4), BootSectorAddress);

  if (EFI_ERROR(Status)) 
		return Status;
	
  gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gBS->RestoreTPL(OldTpl);

  return Status;
}



EFI_STATUS CdRomFillSpecPacket(IN UINTN OutputAddress) {

  CDROM_SPEC_PACKET *spec = (CDROM_SPEC_PACKET*)OutputAddress;

  CopyMem(spec, &SpecPacket, sizeof(CDROM_SPEC_PACKET));
  return EFI_SUCCESS;
}


EFI_BLOCK_IO_PROTOCOL* getPreferredBlockIo(UINT8 DriveNumber) {

  if (DriveNumber == PRIMARY_CDROM_IDENT)
		return CdBlockIo;

  if (DriveNumber >= 0x80 && DriveNumber < 0x90) {
    return HardDriveBlockIo[DriveNumber & 0x0F];
  }
  return NULL;
}
