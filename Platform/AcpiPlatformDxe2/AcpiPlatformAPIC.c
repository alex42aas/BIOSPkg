/** @file
  Sample ACPI Platform Driver

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 
//-----------------------------------------------------------------------------
/*
	Сделан честно только список записей [Processor Local APIC] на основании о
	собираемых данных по логическим процессорам. Остальные записи ([I/O APIC], 
	[Interrupt Source Override], [Local APIC NMI]) взяты из
	шаблона (AcpiPlatformAPICRes.h) полученного на основе созданных при загрузке
	ОС с оригинальным BIOS.
*/
//-----------------------------------------------------------------------------
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

#include <Protocol/FrameworkMpService.h>

#include <Protocol/AcpiTable.h>
#include <IndustryStandard/Acpi.h>

#include "AcpiPlatformAPIC.h"
#include "AcpiPlatformAPICRes.h"

//-----------------------------------------------------------------------------
/**
	Функция создания и добавления таблицы MADT в системный набор таблиц ACPI

  @param VOID

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
AcpiPlatformAPICInstall (
  VOID
  )
{
	EFI_ACPI_TABLE_PROTOCOL* pACPiProt;
	FRAMEWORK_EFI_MP_SERVICES_PROTOCOL *pMPServProt;
	EFI_MP_PROC_CONTEXT MPContext;
	EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_STRUCTURE *pMPApic;
	PACPI_PLATFORM_APIC_TMPL pAPIcTmpl;
	EFI_STATUS efiSTS;
	INTN iTmp, iTmp0, nrCpus;
	UINT8 *pbData, rgbApic[64];

	DEBUG ((EFI_D_INFO, "\nEntry "__FUNCTION__".%d\n", __LINE__));//%%%%

	//Получение доступа к сервису взаимодействия с процессором
	efiSTS = gBS->LocateProtocol (
			&gFrameworkEfiMpServiceProtocolGuid,
			NULL,
			&pMPServProt
			);

	DEBUG ((EFI_D_INFO, __FUNCTION__".Get MPServiceProtocol.Status:%d.%d\n", efiSTS, __LINE__));//%%%%

	if (EFI_ERROR (efiSTS)) { efiSTS = EFI_ABORTED; goto fin; }

	//Получение количества логических процессоров
	efiSTS = pMPServProt->GetGeneralMPInfo (
									pMPServProt,
									&nrCpus,
									NULL, NULL, NULL, NULL
									);  

	for ( iTmp = 0; iTmp < nrCpus; iTmp++ )
	{//Собираем данные конфигурации процессоров 
		iTmp0 = sizeof(MPContext);
		efiSTS = pMPServProt->GetProcessorContext (
									pMPServProt,
									iTmp,
									&iTmp0,
									&MPContext
									);

		DEBUG ((EFI_D_INFO, __FUNCTION__".GetProcessorContext.Status:%d.%d\n", efiSTS, __LINE__));//%%%%

		if (EFI_ERROR (efiSTS)) { efiSTS = EFI_ABORTED; goto fin; }

		DEBUG(( EFI_D_INFO, __FUNCTION__".ApicId:%x, Enabled:%x, AP/BSP:%d, SecondaryCpu:%d.%d\n",
							MPContext.ApicID, MPContext.Enabled, MPContext.Designation,
										MPContext.bSecondaryCpu, __LINE__ ));

		/*Создание карты логических процесоров. По требованию к порядку включения логических
		процессоров вторичные процессоры должны включаться после первичных, поэтому APIC вторичных
		процессоров численно смещается относительно первичных, чтобы последующая сортировка дала
		нужный порядок*/
		rgbApic[iTmp] = (UINT8)(!MPContext.bSecondaryCpu ? MPContext.ApicID: MPContext.ApicID | 0x80);
	}

	for ( iTmp = 0; iTmp < nrCpus - 1; iTmp++ )
	{//Сортируем карту логических процесоров
		for ( iTmp0 = iTmp + 1; iTmp0 < nrCpus; iTmp0++ )
		{
			if ( rgbApic[iTmp] > rgbApic[iTmp0] )
			{
				rgbApic[iTmp0] ^= rgbApic[iTmp];
				rgbApic[iTmp] ^= rgbApic[iTmp0];
				rgbApic[iTmp0] ^= rgbApic[iTmp];
			}
		}
	}
	//Удаление вспомогательного смещения из карты логических процесоров
	for ( iTmp = 0; iTmp < nrCpus; iTmp++ )rgbApic[iTmp] &= 0x7F;

	//Вычисляем требуемый размер для MAD (Multiple APIC Description Table)
	iTmp = sizeof(ACPI_PLATFORM_APIC_TMPL) +
						(nrCpus - 1 ) * sizeof(EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_STRUCTURE);

	//Выделяем память для MAD
	pAPIcTmpl = (PACPI_PLATFORM_APIC_TMPL) AllocatePool ( iTmp	);
	ASSERT (pAPIcTmpl);

	/*Инициализация MAD*/
	pAPIcTmpl->MADHdr = resMADHdr;
	pAPIcTmpl->MADHdr.Header.Length = (UINT32)iTmp;
	pAPIcTmpl->IOApic = resIOApic;
	pAPIcTmpl->rgINTSrcOvr[0] = resINTSrcOvr0;
	pAPIcTmpl->rgINTSrcOvr[1] = resINTSrcOvr1;
	pAPIcTmpl->LOCApicNMI = resLOCApicNMI;

	for ( iTmp = 0, pMPApic = pAPIcTmpl->rgMPApic; iTmp < nrCpus; iTmp++, pMPApic++ )
	{//Заполняем [Processor Local APIC] используя карту 
		*pMPApic = resMPApic0;
		pMPApic->AcpiProcessorId = (UINT8)(iTmp + 1);
		pMPApic->ApicId = (UINT8)rgbApic[iTmp];
	}

	//Считаем и сохраняем контрольную сумму таблицы
	iTmp = pAPIcTmpl->MADHdr.Header.Length;
	pbData = (UINT8*)pAPIcTmpl;
	for ( iTmp0 = 0 ; iTmp ; iTmp-- )iTmp0 += *pbData++;
	pAPIcTmpl->MADHdr.Header.Checksum = (UINT8) (0xff - iTmp0 + 1);

#if 0//Отладочный дамп таблицы MADT
	DEBUG ((EFI_D_INFO, __FUNCTION__".Dump MADT.%d\n", __LINE__));//%%%%

	iTmp = pAPIcTmpl->MADHdr.Header.Length;
	pbData = (UINT8*)pAPIcTmpl;
	while ( iTmp )
	{
		for ( iTmp0 = 0; iTmp0 < 16 && iTmp; iTmp0++, iTmp-- )
		{
			DEBUG ((EFI_D_INFO, "%02X ", *pbData++));//%%%%
		}
		DEBUG ((EFI_D_INFO, "\n"));//%%%%
	}
#endif

	//Получение доступа к сервису работы с ACPI
	efiSTS = gBS->LocateProtocol ( &gEfiAcpiTableProtocolGuid, NULL, (VOID**)&pACPiProt );

	DEBUG ((EFI_D_INFO, __FUNCTION__".Get.ACPiProtocol.Status:%d.%d\n", efiSTS, __LINE__));//%%%%

	if (EFI_ERROR (efiSTS)) { efiSTS = EFI_ABORTED; goto fin; }

	//Устанавливаем таблицу
	iTmp = pAPIcTmpl->MADHdr.Header.Length;
	efiSTS = pACPiProt->InstallAcpiTable( pACPiProt, pAPIcTmpl, iTmp, &iTmp0 );

	DEBUG ((EFI_D_INFO, __FUNCTION__".InstallAcpiTable.Status:%d.%d\n", efiSTS, __LINE__));//%%%%

	FreePool( pAPIcTmpl );

	efiSTS = EFI_SUCCESS;
fin:
	
  return efiSTS;
}

