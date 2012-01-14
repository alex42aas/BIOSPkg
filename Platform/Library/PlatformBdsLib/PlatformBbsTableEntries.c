/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "PlatformBbsTableEntries.h"
#include <IndustryStandard\ElTorito.h>

extern EFI_LEGACY_BIOS_PROTOCOL     *mLegacyBios;




/**
  Update list of Bev or BCV table entries.

  @param  Private                Protocol instance pointer.
  @param  PciIo                  Instance of PCI I/O Protocol

  @retval EFI_SUCCESS            Always should succeed.

**/
EFI_STATUS
PlatformUpdateBbsTable (VOID) {

  EFI_STATUS                      Status;
  EFI_PCI_IO_PROTOCOL             *PciIo;
  BBS_TABLE                       *BbsTable;
  UINTN                           BbsIndex;
  UINTN							  dIndex;
  UINTN                           Segment;
  UINTN                           Bus;
  UINTN                           Device;
  UINTN                           Function;
  UINT8                           Class;
  UINT8                           SubClass;
  UINT16                          DeviceType;
  LEGACY_BIOS_INSTANCE            *Private;
  EFI_DEVICE_PATH_PROTOCOL	      *DevicePath;
  EFI_DISK_INFO_PROTOCOL          *pDiskInfo;
  EFI_HANDLE			          *pBlockIoHandles;
  EFI_HANDLE			          *pDevicePathHandles;
  UINTN				               NumberHandles;
  UINTN				               Index;
  UINTN				               bFound;
  EFI_DEVICE_PATH_PROTOCOL	       *dpFullDevicePath;
  EFI_DEVICE_PATH_PROTOCOL	       *pciDevicePath;
  EFI_DEVICE_PATH_PROTOCOL	       *dpCurrentNode;
  EFI_DEVICE_PATH_PROTOCOL	       *dpPrevNode;
  UINTN				               Port;
  CHAR16			               *TempStr;
  CHAR8				               *pDiskInfoResult;
  UINT32			               nDiskInfoResultSize;
  CHAR8				               DeviceName[BBS_NAME_LEN];
  CHAR8				               FullDeviceName[BBS_NAME_LEN * 2];
  EFI_PHYSICAL_ADDRESS		       PhysicalAddress;
  CHAR8				               *Descriptors;
  UINTN				               bSCSI;
  SATA_DEVICE_PATH                 *Sata;
  UINTN		                        dpIndex;
  UINTN		                        dpNumberHandles;


  Segment     = 0;
  Bus         = 0;
  Device      = 0;
  Function    = 0;
  Class       = 0;
  SubClass    = 0;
  DeviceType  = BBS_UNKNOWN;

  Private = LEGACY_BIOS_INSTANCE_FROM_THIS(mLegacyBios);
  BbsIndex  = Private->IntThunk->EfiToLegacy16BootTable.NumberBbsEntries;
  BbsTable  = (BBS_TABLE*)(UINTN) Private->IntThunk->EfiToLegacy16BootTable.BbsTable;

  gBS->LocateHandleBuffer (
		ByProtocol,
		&gEfiBlockIoProtocolGuid,
		NULL,
		&NumberHandles,
		&pBlockIoHandles
		);


  PhysicalAddress = CONVENTIONAL_MEMORY_TOP;
  Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiBootServicesCode,
                    EFI_SIZE_TO_PAGES (BBS_ENTRIES_NUMBER * BBS_NAME_LEN),
                    &PhysicalAddress
                    );
  if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a.%d: no space for descriptors\n", __FUNCTION__, __LINE__));
      ASSERT(FALSE);
  }

  dIndex = 0;
  Descriptors = (CHAR8*)PhysicalAddress;  

  for (Index = 0; Index < NumberHandles; Index++) 
  {

    Status = gBS->HandleProtocol (
		  pBlockIoHandles[Index],
		  &gEfiDevicePathProtocolGuid,
		  (VOID**)&DevicePath);

    if (EFI_ERROR (Status)) {
      continue;
    }

    bFound = FALSE;
    dpFullDevicePath = DuplicateDevicePath(DevicePath);		
    pciDevicePath = DuplicateDevicePath(DevicePath);		
    dpCurrentNode = pciDevicePath;

    while (!IsDevicePathEnd(dpCurrentNode))
    {
      if (dpCurrentNode->Type == MESSAGING_DEVICE_PATH
				&& dpCurrentNode->SubType == MSG_SATA_DP)
      {				
		bFound = TRUE;
		break;
      }
						
      //    
      dpCurrentNode = NextDevicePathNode(dpCurrentNode);
    } // while

	TempStr = DevicePathToStr (dpFullDevicePath);

	if (!bFound) {
	    FreePool(dpFullDevicePath);
	    FreePool(pciDevicePath);
		continue;			
	}


	dpPrevNode = dpCurrentNode;
	dpCurrentNode = NextDevicePathNode(dpCurrentNode);

	if (!IsDevicePathEnd(dpCurrentNode)) {
	    FreePool(dpFullDevicePath);
	    FreePool(pciDevicePath);
		continue;	
	}

    if(dIndex >= BBS_ENTRIES_NUMBER) {

      DEBUG((EFI_D_ERROR, "%a.%d: error: Number of BBS-devices > %d\n", __FUNCTION__, __LINE__, BBS_ENTRIES_NUMBER));
      break;
    }


	Sata = (SATA_DEVICE_PATH*)dpPrevNode;
	Port = Sata->HBAPortNumber;

    TempStr = DevicePathToStr (dpPrevNode);

	DeviceName[0] = '\0';

    SetDevicePathEndNode(dpPrevNode);

	TempStr = DevicePathToStr (pciDevicePath);

	TempStr = GetDeviceName(pBlockIoHandles[Index], dpFullDevicePath);
	if (TempStr) {

	  TempStr[BBS_NAME_LEN] = L'\0';
      UnicodeStrToAsciiStr(TempStr, DeviceName);

	} 

	bSCSI = FALSE;
	//  SCSI (CD-ROM):
	Status = gBS->HandleProtocol (
			pBlockIoHandles[Index],
			&gEfiScsiIoProtocolGuid,
			(VOID**)&pDiskInfo);
	if (!EFI_ERROR(Status)) {

		Status = gBS->HandleProtocol (
				pBlockIoHandles[Index],
				&gEfiDiskInfoProtocolGuid,
				(VOID *) &pDiskInfo);
		if (!EFI_ERROR(Status))
		{
			bSCSI = TRUE;

			pDiskInfoResult = AllocatePool (ATA_INFO_CMD_RESULT_SIZE);
			nDiskInfoResultSize = ATA_INFO_CMD_RESULT_SIZE;

			Status = pDiskInfo->Identify(pDiskInfo, pDiskInfoResult, &nDiskInfoResultSize);
			if (!EFI_ERROR(Status))
			{
				UINTN Index;
				CHAR8* pExchangeByte = &pDiskInfoResult[ATA_INFO_MODEL_NAME_OFFSET];
				CHAR8  cTempExchangeByte;

				ASSERT(!(BBS_NAME_LEN % 2));

				for (Index = 0; Index < BBS_NAME_LEN; Index += 2) 
				{
					cTempExchangeByte = pExchangeByte[Index + 1];
					pExchangeByte[Index + 1] = pExchangeByte[Index];
					pExchangeByte[Index] = cTempExchangeByte;
				}

				pDiskInfoResult[ATA_INFO_MODEL_NAME_OFFSET + BBS_NAME_LEN - 1] = '\0';
				CopyMem(DeviceName, &pDiskInfoResult[ATA_INFO_MODEL_NAME_OFFSET], BBS_NAME_LEN);
			}

			FreePool(pDiskInfoResult);
		}
	} // if(Scsi)

	AsciiSPrint(FullDeviceName, BBS_NAME_LEN, "P%d-", Port);
	AsciiSPrint(FullDeviceName + AsciiStrLen(FullDeviceName), BBS_NAME_LEN, "%a", DeviceName);
        FullDeviceName[BBS_NAME_LEN - 1] = '\0';
	CopyMem(Descriptors, FullDeviceName, BBS_NAME_LEN);
	

    gBS->LocateHandleBuffer(
		ByProtocol,
		&gEfiDevicePathProtocolGuid,
		NULL,
		&dpNumberHandles,
		&pDevicePathHandles
		);

    PciIo = NULL;
    for (dpIndex = 0; dpIndex < dpNumberHandles; dpIndex++) {

	Status = gBS->HandleProtocol (
			pDevicePathHandles[dpIndex],
			&gEfiDevicePathProtocolGuid,
			(VOID**)&DevicePath);

	if(CompareMem(DevicePath, pciDevicePath, GetDevicePathSize(pciDevicePath)) == 0)
	{
	  Status = gBS->HandleProtocol (
			pDevicePathHandles[dpIndex],
			&gEfiPciIoProtocolGuid,
			(VOID**)&PciIo);
	  break;
	}
    } // for(dpIndex)

    FreePool(dpFullDevicePath);
    FreePool(pciDevicePath);

    if(PciIo)
    {
      PciIo->GetLocation (
             PciIo,
             &Segment,
             &Bus,
             &Device,
             &Function
             );
      PciIo->Pci.Read (
                 PciIo,
                 EfiPciIoWidthUint8,
                 0x0b,
                 1,
                 &Class
                 );
      PciIo->Pci.Read (
                 PciIo,
                 EfiPciIoWidthUint8,
                 0x0a,
                 1,
                 &SubClass
                 );
    }
    if (Class == PCI_CLASS_MASS_STORAGE) {
      DeviceType = BBS_HARDDISK;
    } else {
      if (Class == PCI_CLASS_NETWORK) {
        DeviceType = BBS_EMBED_NETWORK;
      }
    }

    BbsTable[BbsIndex].BootPriority             = BBS_UNPRIORITIZED_ENTRY;
    BbsTable[BbsIndex].DeviceType               = DeviceType;
    if(bSCSI) {
	    BbsTable[BbsIndex].DeviceType               = BBS_CDROM;
	}

    BbsTable[BbsIndex].Bus                      = (UINT32) Bus;
    BbsTable[BbsIndex].Device                   = (UINT32) Device;
    BbsTable[BbsIndex].Function                 = (UINT32) Function;
    BbsTable[BbsIndex].StatusFlags.OldPosition  = 0;
    BbsTable[BbsIndex].StatusFlags.Reserved1    = 0;
    BbsTable[BbsIndex].StatusFlags.Enabled      = 0;
    BbsTable[BbsIndex].StatusFlags.Failed       = 0;
    BbsTable[BbsIndex].StatusFlags.MediaPresent = 0;
    BbsTable[BbsIndex].StatusFlags.Reserved2    = 0;
    BbsTable[BbsIndex].Class                    = Class;
    BbsTable[BbsIndex].SubClass                 = SubClass;	

    BbsTable[BbsIndex].BootHandlerOffset = (UINT8)Port;


    BbsTable[BbsIndex].DescStringOffset = (UINT16)((UINTN)Descriptors & 0xffff);
    BbsTable[BbsIndex].DescStringSegment = (UINT16)(((UINTN)Descriptors >> 4) & 0xf000);
    BbsTable[BbsIndex].MfgStringOffset = (UINT16)((UINTN)Descriptors & 0xffff);
    BbsTable[BbsIndex].MfgStringSegment = (UINT16)(((UINTN)Descriptors >> 4) & 0xf000);

    BbsIndex++;
    Descriptors += BBS_NAME_LEN;
    dIndex++;
  } // for(Index)

  //TODO: Добавить загрузку с устройств PCI (анкад, например)

  BbsTable[BbsIndex].BootPriority = BBS_IGNORE_ENTRY;
  Private->IntThunk->EfiToLegacy16BootTable.NumberBbsEntries = (UINT32) BbsIndex;

  return EFI_SUCCESS;
}
