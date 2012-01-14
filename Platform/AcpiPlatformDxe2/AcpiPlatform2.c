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

#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/AcpiSupport.h>
#include <AcpiTable.h>
#include <AcpiSupport.h>
#include <Universal/Acpi/AcpiSupportDxe/AcpiSupport.h>
#include <Protocol/FirmwareVolume2.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

#include <Protocol/FrameworkMpService.h>

#include <IndustryStandard/Acpi.h>

#ifdef EFI_NO_INTERFACE_DECL
  #define EFI_FORWARD_DECLARATION(x)
#else
  #define EFI_FORWARD_DECLARATION(x) typedef struct _##x x
#endif
#include <Protocol/GlobalNvsArea/GlobalNvsArea.h>

#include "AcpiPlatformAPIC.h"

#pragma pack(1)	// выравнивание побайтно
typedef struct _AMLI_DATA_32 {
	UINT8 Prefix;
	UINT32 Dword;
} AMLI_DATA_32; 

typedef struct _AMLI_DATA_16 {
	UINT8 Prefix;
	UINT16 Word;
} AMLI_DATA_16; 
#pragma pack()

/**
  Entrypoint of Acpi Platform driver.

  @param  ImageHandle
  @param  SystemTable

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES
**/
EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  EFI_ACPI_SUPPORT_PROTOCOL      *AcpiSupport;
  INTN                           Instance;
  EFI_ACPI_COMMON_HEADER         *CurrentTable;
  UINTN                          TableHandle;
  EFI_ACPI_SUPPORT_INSTANCE	 *AcpiSupportInstance;

  Instance     = 0;
  CurrentTable = NULL;
  TableHandle  = 0;

  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**)&AcpiTable);
  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }

  //
  // Find the AcpiSupport protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, (VOID**)&AcpiSupport);
  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }

  AcpiSupportInstance = (EFI_ACPI_BUNCH_FROM_ACPI_SUPPORT_THIS(AcpiSupport))->AcpiSupportInstance;

	//Cоздание и добавление таблицы MADT в системный набор таблиц ACPI
	Status = AcpiPlatformAPICInstall();
	ASSERT_EFI_ERROR (Status);

{
EFI_ACPI_SDT_PROTOCOL      *SdtProtocol;
EFI_ACPI_SDT_HEADER        *Table;
EFI_ACPI_TABLE_VERSION     Version;
UINTN                      TableKey;
UINTN                      Index;
UINT8                      *Data;
UINTN                      DataSize;
EFI_ACPI_DATA_TYPE         DataType;
EFI_GLOBAL_NVS_AREA_PROTOCOL               *GlobalNvsAreaProtocol;
EFI_GLOBAL_NVS_AREA                        *GlobalNvsArea;

	Status = gBS->LocateProtocol (&gEfiGlobalNvsAreaProtocolGuid , NULL, &GlobalNvsAreaProtocol);
	ASSERT_EFI_ERROR (Status);

	GlobalNvsArea  = GlobalNvsAreaProtocol->Area;

	Status = gBS->LocateProtocol(&gEfiAcpiSdtProtocolGuid, NULL, &SdtProtocol);
	ASSERT_EFI_ERROR (Status);

	Index = 0;

	do {
		Status = SdtProtocol->GetAcpiTable(Index, &Table, &Version, &TableKey);

		if (EFI_ERROR(Status)) {
			break;
		}
		
		if(Table->Signature == SIGNATURE_32('D','S','D','T')) {
EFI_ACPI_HANDLE   TableHandle;
EFI_ACPI_HANDLE   GnvsHandle;
AMLI_DATA_32      GnvsAreaOffset;
AMLI_DATA_16      GnvsAreaLength;

			Status = SdtProtocol->OpenSdt(TableKey, &TableHandle);
			ASSERT_EFI_ERROR (Status);

			Status = SdtProtocol->FindPath(TableHandle, "\\GNVS", &GnvsHandle);
			ASSERT_EFI_ERROR (Status);


			// 1. Offset
			Status = SdtProtocol->GetOption( GnvsHandle, 3, &DataType, &Data, &DataSize );
			ASSERT_EFI_ERROR (Status);

			CopyMem(&GnvsAreaOffset, Data, sizeof(GnvsAreaOffset));
			ASSERT (GnvsAreaOffset.Prefix == 0xC); // DWordPrefix

			GnvsAreaOffset.Dword = (UINT32) (UINTN) (GlobalNvsAreaProtocol->Area);

			Status = SdtProtocol->SetOption( GnvsHandle, 3, &GnvsAreaOffset, DataSize );
			ASSERT_EFI_ERROR (Status);

			// 2. Size
			Status = SdtProtocol->GetOption( GnvsHandle, 4, &DataType, &Data, &DataSize );
			ASSERT_EFI_ERROR (Status);

			CopyMem(&GnvsAreaLength, Data, sizeof(GnvsAreaLength));
			ASSERT (GnvsAreaLength.Prefix == 0xB); // WordPrefix

			GnvsAreaLength.Word = (UINT16)sizeof(*GlobalNvsAreaProtocol->Area);

			Status = SdtProtocol->SetOption( GnvsHandle, 4, &GnvsAreaLength, DataSize );
			ASSERT_EFI_ERROR (Status);

			SdtProtocol->Close(GnvsHandle);

			SdtProtocol->Close(TableHandle);
		}


		Index++;
	} while (TRUE);
}

  return EFI_SUCCESS;
}
