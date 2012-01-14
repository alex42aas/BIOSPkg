/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

﻿/** 
* @file PlatformInfo.c
* Драйвер сбора информации о периферийных устройствах.
**/

#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/DevicePathLib.h>
#include <Include/Library/FsUtils.h> 

#include <Protocol/ScsiIo.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/BlockIo.h>

#include <Include/Library/CommonUtils.h>

#include <CommonAddresses.h>
#include <PlatformDataLib.h>

#include <Include\Library\BootMngrLib.h>

#include "Protocol\PeripheralInfo\PeripheralInfo.h"

//=============================================================================

EFI_GUID gEfiPeripheralInfoProtocolGuid = EFI_PERIPHERAL_INFO_PROTOCOL_GUID; 

#define ELEMENTS_IN(x) (sizeof(x)/sizeof(x[0]))

//=============================================================================

//
// Определения, необходмые для идментификации SCSI-устройств (ATAPI-CDROM)
//

#define ATA_INFO_CMD_RESULT_SIZE 512
#define ATA_INFO_MODEL_NAME_OFFSET 54
#define ATA_INFO_MODEL_NAME_SIZE 40

//=============================================================================

CHAR16 *
	GetDescriptionFromDevicePath(
	IN EFI_DEVICE_PATH_PROTOCOL *DevPath
	)
{
	EFI_DEVICE_PATH_PROTOCOL *PrePrevDP = DevPath;
	EFI_DEVICE_PATH_PROTOCOL *DP;
	UINTN Idx;

	if (DevPath == NULL) {
		return NULL;
	}

	for (DP = DevPath, Idx = 0; !IsDevicePathEnd(DP); 
		DP = NextDevicePathNode(DP), Idx++) {
			if (Idx >= 2) {
				PrePrevDP = NextDevicePathNode(PrePrevDP);
			}    
	}

	return DevicePathToStr(PrePrevDP);
}

VOID
	FixDescriptionFromDevicePath(
	IN OUT CHAR16 *DpStr
	)
{
	UINTN Len, TmpLen;
	CHAR16 *EndStr;

	if (DpStr == NULL) {
		return;
	}
	TmpLen = Len = StrLen(DpStr);
	EndStr = &DpStr[Len - 1];
	if (*EndStr != L')') {
		return;
	}
	while (*EndStr != '(') {
		TmpLen--;
		EndStr--;
		if (TmpLen == 0) {
			return;
		}
	}

	while (*EndStr != L',') {
		TmpLen++;
		if (TmpLen >= Len) {
			return;
		}
		EndStr++;
	}
	*EndStr++ = L')';
	*EndStr = 0;
}

//=============================================================================
EFI_STATUS FindSataInfo(IN PSSataInfo pSataInfo)
{
	EFI_STATUS Status;
	
	UINTN NumberHandles;
	UINTN Index;
	
	EFI_DEVICE_PATH_PROTOCOL * DevicePath;
	EFI_BLOCK_IO_PROTOCOL * pBlockIo;
	EFI_DISK_INFO_PROTOCOL * pDiskInfo;
	
	EFI_HANDLE *pFsp;
	
	CHAR16 *wszDeviceName = NULL;
	CHAR16 *TempStr;
	  
	CHAR8 * pDiskInfoResult;
	UINT32 nDiskInfoResultSize;

	UINT64 nSize = 0;
	  
	EFI_DEVICE_PATH_PROTOCOL *dpFullDevicePath;
	EFI_DEVICE_PATH_PROTOCOL *dpCurrentNode;

	BOOLEAN bFound;
	
	UINT32 nSataDeviceCount = 0;

	gBS->LocateHandleBuffer (
		ByProtocol,
		&gEfiBlockIoProtocolGuid,
		NULL,
		&NumberHandles,
		&pFsp
		);

	DEBUG((EFI_D_INFO, "%a.%d NumberHandles = %d\n", 
		__FUNCTION__, __LINE__, NumberHandles));


	//
	// Обход всех устройств с BlockIo
	//
	
	for (Index = 0; Index < NumberHandles; Index++) 
	{
		wszDeviceName = NULL;
		pBlockIo = NULL;
		  
		Status = gBS->HandleProtocol (
		  pFsp[Index],
		  &gEfiDevicePathProtocolGuid,
		  (VOID *) &DevicePath);

		if (EFI_ERROR (Status)) {
		  DEBUG((EFI_D_ERROR, "%a.%d Error! Index=%d\n", 
			__FUNCTION__, __LINE__, Index));
		  continue;
		}

		bFound = FALSE;
		dpFullDevicePath = DuplicateDevicePath(DevicePath);		
		dpCurrentNode = dpFullDevicePath;

		while (!IsDevicePathEnd(dpCurrentNode))
		{
			// 
			if (dpCurrentNode->Type == MESSAGING_DEVICE_PATH
				&& dpCurrentNode->SubType == MSG_SATA_DP)
			{				
				bFound = TRUE;
				break;
			}
						
			// Переход к следующему узлу
			dpCurrentNode = NextDevicePathNode(dpCurrentNode);
		}

		if (!bFound)
		{
			DEBUG((EFI_D_INFO, "not sata device\n", __FUNCTION__, __LINE__));
			// не SATA-устрйоство
			continue;			
		}

		// Переход к следующему узлу
		dpCurrentNode = NextDevicePathNode(dpCurrentNode);

		if (!IsDevicePathEnd(dpCurrentNode))
		{
			// это логическое устройство, т.к. у него есть дополнительная часть пути
			// (раздел, логический диск)
			DEBUG((EFI_D_INFO, "logical device\n", __FUNCTION__, __LINE__));	
			continue;	
		}

		//
		// Открытие протокола BlockIo для получения информации об устройстве
		//

		Status = gBS->HandleProtocol (
		  pFsp[Index],
		  &gEfiBlockIoProtocolGuid,
		  (VOID *) &pBlockIo);
		ASSERT(Status == EFI_SUCCESS);

		ASSERT(pBlockIo->Media);

		// не показываем емкость для съемных накопителей (CD-ROM)
		// TODO! Найти более правильный способ определения
		if (pBlockIo->Media->RemovableMedia)
		{
			nSize = 0;
			DEBUG((EFI_D_INFO, "Removable device, force size=0 \n"));
		}
		else
		{
			nSize = pBlockIo->Media->BlockSize * pBlockIo->Media->LastBlock;
			DEBUG((EFI_D_INFO, "Size = 0x%08x 0x%08x \n", nSize));
		}

		//
		// Получение имени устройства
		//

		TempStr = DevicePathToStr (dpFullDevicePath);
		DEBUG((EFI_D_INFO, "%a.%d DevicePath = %s\n", 
		  __FUNCTION__, __LINE__, TempStr));
		  
		TempStr = GetDeviceName(pFsp[Index], dpFullDevicePath);
		DEBUG((EFI_D_INFO, "GetDeviceName %s\n", TempStr));
		if (TempStr) 
		{
			wszDeviceName = AllocateCopyPool (StrSize(TempStr), TempStr);
			ASSERT(wszDeviceName != NULL);
		} 
		else 
		{
			TempStr = GetDescriptionFromDevicePath(dpFullDevicePath);
			DEBUG((EFI_D_INFO, "GetDescriptionFromDevicePath %s\n", TempStr));
			if (TempStr) {
				wszDeviceName = AllocateCopyPool (StrSize(TempStr), TempStr);        
				ASSERT(wszDeviceName != NULL);
			}      
			FixDescriptionFromDevicePath(wszDeviceName);
			DEBUG((EFI_D_INFO, "FixDescriptionFromDevicePath %s\n", wszDeviceName));
		}

		//
		// Для SCSI дисков существует другой способ получения имени устройства
		// (Так как драйвер SCSI не устанавливает имя устройства, а формирует строку 
		// SCSI Disk Device).
		// 
		// Пока эта операция выполняется для всех устройств, имеющих протокол ScsiIo.
		//

		Status = gBS->HandleProtocol (
			pFsp[Index],
			&gEfiScsiIoProtocolGuid,
			(VOID *) &pDiskInfo);
		if (!EFI_ERROR(Status))
		{
			DEBUG((EFI_D_INFO, "ScsiIo supported \n"));

			// Получение протокола с информацией о носителе

			Status = gBS->HandleProtocol (
				pFsp[Index],
				&gEfiDiskInfoProtocolGuid,
				(VOID *) &pDiskInfo);
			if (!EFI_ERROR(Status))
			{
				DEBUG((EFI_D_INFO, "DiskInfo supported \n"));

				pDiskInfoResult = AllocatePool (ATA_INFO_CMD_RESULT_SIZE);
				nDiskInfoResultSize = ATA_INFO_CMD_RESULT_SIZE;

				Status = pDiskInfo->Identify(pDiskInfo, pDiskInfoResult, &nDiskInfoResultSize);
				if (!EFI_ERROR(Status))
				{
					UINTN Index;
					CHAR8* pExchangeByte = &pDiskInfoResult[ATA_INFO_MODEL_NAME_OFFSET];
					CHAR8  cTempExchangeByte;

					//
					// Смена порядка следования байт in-place
					//

					ASSERT(!(ATA_INFO_MODEL_NAME_SIZE % 2));

					for (Index = 0; Index < ATA_INFO_MODEL_NAME_SIZE; Index += 2) 
					{
						cTempExchangeByte = pExchangeByte[Index + 1];
						pExchangeByte[Index + 1] = pExchangeByte[Index];
						pExchangeByte[Index] = cTempExchangeByte;
					}

					pDiskInfoResult[ATA_INFO_MODEL_NAME_OFFSET + ATA_INFO_MODEL_NAME_SIZE] = '\0';
					DEBUG((EFI_D_INFO, "GetDeviceName %s\n", &pDiskInfoResult[ATA_INFO_MODEL_NAME_OFFSET]));

					AsciiStrToUnicodeStr(&pDiskInfoResult[ATA_INFO_MODEL_NAME_OFFSET], wszDeviceName);
				}

				FreePool(pDiskInfoResult);
			}
		}

		if (wszDeviceName)
		{
			//
			// Удаление завершающих пробелов
			//
			
			UINTN nLength = StrLen(wszDeviceName);
			UINT16* pSpaceTest = wszDeviceName;
			
			if (nLength)
			{
				pSpaceTest = pSpaceTest + nLength - 1;
				while (*pSpaceTest == L' ')
				{
					pSpaceTest--;
				}
				pSpaceTest++;
				*pSpaceTest = L'\0';
			}
		}
		
		//
		// Копирование собранной информации в протокол
		//
		
		pSataInfo->_aSataDevice[nSataDeviceCount]._wszModel = 
			AllocatePool(ATA_INFO_MODEL_NAME_SIZE * sizeof(*pSataInfo->_aSataDevice[nSataDeviceCount]._wszModel));
		ASSERT(pSataInfo->_aSataDevice[nSataDeviceCount]._wszModel);

		StrCpy(pSataInfo->_aSataDevice[nSataDeviceCount]._wszModel, wszDeviceName);
		pSataInfo->_aSataDevice[nSataDeviceCount]._nSizeInBytes = nSize;
		nSataDeviceCount++;

		if (nSataDeviceCount == ELEMENTS_IN(pSataInfo->_aSataDevice))
		{
			// ASSERT срабатывает, когда обнаружено слишком много устройств
			ASSERT(0);
			break;
		}
	}
	
	pSataInfo->_nDeviceCount = nSataDeviceCount;
	
	return EFI_SUCCESS;
}


/**
  The user Entry Point for this module.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval Others            Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PeripheralInfoEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
	EFI_STATUS               Status = EFI_SUCCESS;
	EFI_PERIPHERAL_INFO_PROTOCOL*  pPeriphInfoProtocol;

	LOG((EFI_D_INFO, "\r\nPeripheralInfoDxe: entry"));

	gBS = SystemTable->BootServices;

	//
	// Выделение памяти для протокола
	//

	pPeriphInfoProtocol = (EFI_PERIPHERAL_INFO_PROTOCOL*)AllocatePool(sizeof(*pPeriphInfoProtocol));
	if (!pPeriphInfoProtocol)
	{
		LOG ((EFI_D_INFO, "PlatformInfoEntryPoint: AllocatePool failed"));
		return EFI_OUT_OF_RESOURCES;
	}
	SetMem(pPeriphInfoProtocol, 0x0, sizeof(*pPeriphInfoProtocol));
	pPeriphInfoProtocol->_Version = PERIPHERAL_INFO_VERSION_1_0;
	
	//
  	LOG((EFI_D_INFO, "\r\nPeripheralInfoDxe: FindSataInfo...")); 
	//
  
 	FindSataInfo(&pPeriphInfoProtocol->_PeripheralInfo._SataInfo); 
  
	//
	LOG((EFI_D_INFO, "\r\nPeripheralInfoDxe: InstallProtocol..."));
	//

	//
	// Установка протокола
	// 

	Status = gBS->InstallMultipleProtocolInterfaces (
		&ImageHandle,
		&gEfiPeripheralInfoProtocolGuid,
		pPeriphInfoProtocol,
		NULL
		);
	if (EFI_ERROR (Status)) 
	{
		LOG((EFI_D_INFO, "\r\nPeripheralInfoDxe InstallProtocol failed "));
		FreePool(pPeriphInfoProtocol);
	}
	
	LOG((EFI_D_INFO, "\r\nPeripheralInfoDxe OK ")); 
  
	return Status;
}
