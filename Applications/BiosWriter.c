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


#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>
#include <Library/FsUtils.h>
#include <Protocol/BlockIo2.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/EdidOverride.h>
#include <PlatformGopPolicy.h>
#include <Protocol/SaPlatformpolicy/SaPlatformPolicy.h>
#include "Protocol/GlobalNvsArea/GlobalNvsArea.h"
#include <IncludePrivate/Protocol/PchNvsArea.h>
#include <IncludePrivate/Protocol/PchNvs.h>
#include <Protocol/SaGlobalNvsArea.h>

#include "IgdOpRegion.h"
#include "SaGlobalNvsArea.h"

#include <stdio.h> 

#include "BiosWriter.h"
#include "spi_flash.h"

#include <Register/PchRegsSpi.h>
#include <Register/PchRegsPmc.h>


//#define	TEST_MODE
//#define	GIGABYTE_H77
//#define	GET_MTRR_INFO
//#define	GRAPHICS_MODE_INFO
//#define	TEST_STORAGE_MODE



#ifdef GRAPHICS_MODE_INFO
#include <Protocol/LegacyBios.h>
#include <Protocol/LegacyBiosPlatform.h>
#include <PlatformGopPolicy.h>
#endif // GRAPHICS_MODE_INFO

/*
//
// The Library classes this module consumes
//
*/
//#include <Library/DebugLib.h>
//#include <Library/BaseMemoryLib.h>
//#include <Library/HobLib.h>
//#include <Library/MtrrLib.h>
//#include <Library/RedirectDebugLib.h>
#include <Library/HobLib.h>
#define	__HOB__H__
#include <Guid/SaDataHob/SaDataHob.h>


EFI_GUID gDxePlatformSaPolicyGuid = DXE_PLATFORM_SA_POLICY_GUID;
EFI_GUID gEfiGlobalNvsAreaProtocolGuid = EFI_GLOBAL_NVS_AREA_PROTOCOL_GUID;

void		dumpSaPeiPlatformPolicy(void){

  SA_DATA_HOB               *SaDataHob;

  SaDataHob              = (SA_DATA_HOB *)GetFirstGuidHob (&gSaDataHobGuid);

}


//-------------------------------------------------------------------------------------------------------------
typedef	UINT64	size_t;

#define	READ_PCI_8(bus, device, function, offset)	*(UINT8*)((size_t)(PCD_EDKII_GLUE_PciExpressBaseAddress + (bus << 20) + (device << 15) + (function << 12) + offset))
#define	READ_PCI_16(bus, device, function, offset)	*(UINT16*)((size_t)(PCD_EDKII_GLUE_PciExpressBaseAddress + (bus << 20) + (device << 15) + (function << 12) + offset))

#define	READ_PCI_32(device, function, offset)		*(UINT32*)((size_t)(PCD_EDKII_GLUE_PciExpressBaseAddress + (device << 15) + (function << 12) + offset))

#define	WRITE_PCI_32(device, function, offset, data)	*(UINT32*)((size_t)(PCD_EDKII_GLUE_PciExpressBaseAddress + ((device) << 15) + ((function) << 12) + (offset))) = data


//========================================================
#define	SIZE_8M		0x00800000
#define	SIZE_16M	0x01000000
#define	SIZE_MAX	SIZE_16M

#define	BUFFER_SIZE	0x00100000		// рабочий буфер

int	gFlagUsb = FALSE;
UINT8	gFs[16];
UINT8	*gBuf = NULL;	



//=========================================================


int	getch(
	IN int timeOut)
{
EFI_STATUS	Status;
EFI_INPUT_KEY	key;
int	chr;
int	i = 0;

	while(1)
	{
	  if(asktti())
	  {
	    chr = tti();
	    if(chr != 0xff)
			break;
	  }

	  Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
	  if(Status != EFI_NOT_READY)
	  {
	    chr = key.UnicodeChar;
	    break;
	  }

	  if(timeOut > 0)
	  {
	    i++;
	    if(i > timeOut)
	    {
		chr = 0;
		break;
	    }
	  }
	  
	}
	return chr; 

}

#define	PRINT_BUF_SIZE	1024
UINT16	tmpPrintBuf16[PRINT_BUF_SIZE];
UINT8	tmpPrintBuf8[PRINT_BUF_SIZE];

int	gFlagUprintf = FALSE;

void	print(char *format, ...){
VA_LIST argptr;
int	i = 0;

	VA_START(argptr, format);
	vsprintf((char*)tmpPrintBuf8, format, argptr);
	if(gFlagUprintf)
		_uprintf("\r\n%s", tmpPrintBuf8);
	while(1){
	tmpPrintBuf16[i] = tmpPrintBuf8[i];
	if(tmpPrintBuf8[i] == 0)
				break;
	i++;
	}
	Print(L"\r\n%s", tmpPrintBuf16);
}

void	_print(char *format, ...){
VA_LIST argptr;
int	i = 0;

	VA_START(argptr, format);
	vsprintf((char*)tmpPrintBuf8, format, argptr);
	if(gFlagUprintf)
		_uprintf("%s", tmpPrintBuf8);
	while(1){
	tmpPrintBuf16[i] = tmpPrintBuf8[i];
	if(tmpPrintBuf8[i] == 0)
				break;
	i++;
	}
	Print(L"%s", tmpPrintBuf16);
}


typedef	struct{
DWORD	address;
DWORD	size;
} BIOS_REGION;

#define	FILE_IN			"BiosRom.bin"				// файл для обновления прошивки
#define	FILE_OUT		"BIOS\\BiosDump.bin"			// для сохранения текущей прошивки

// файлы для изучения работы БИОСа:
#define	FILE_ACPI_TABLE		"BIOS\\AcpiDump.bin"			// для сохранения ACPI-таблиц
#define	FILE_SMBIOS_TABLE	"BIOS\\SmbiosStructureTable.bin"	// для сохранения таблиц SMBIOS
#define	FILE_SMBIOS_HEADER	"BIOS\\SmbiosHeader.bin"		// для сохранения заголовка SMBIOS
#define	FILE_GPIO_PIRQ		"BIOS\\GpioPirq.c"			// для сохранения данных по GPIO и PIRQ
#define	FILE_VBT		"BIOS\\vbt.bin"			// для сохранения VBT
#define	FILE_VBIOS_VBT		"BIOS\\vbios_vbt.bin"			// для сохранения VBT из VideoBIOS'a
#define	FILE_GOP_VBT		"BIOS\\GOP_vbt.bin"			// для сохранения VBT из PlatformGopPolicy
#define	FILE_EDID		"BIOS\\Edid.bin"			// для сохранения EDID из EdidDiscovered и EdidActive
#define	FILE_SA_DATA		"BIOS\\SaData.bin"			// для сохранения PlatformSaPolicy и SaGlobalNvsArea 
#define	FILE_GNVS_DATA		"BIOS\\GnvsData.bin"			// для сохранения GNVS
#define	FILE_PCH_DATA		"BIOS\\PchNvsData.bin"		// для сохранения PCH_NVS

#define FILE_SIO_DUMP		"BIOS\\SioDump.txt"			// для сохранения дампа регистров SIO
#ifdef GET_MTRR_INFO
#define	FILE_MTRR		"BIOS\\MtrrData.txt"			// для сохранения данных по MTRR
#endif // GET_MTRR_INFO


#define	ACPI_TAB_SIZE	0x20000				// 128k


#define	FILE_IMG	"BiosWriter.efi"			// файл, откуда загружались

#define	FS2		"fs2:\\"
#define	FS3		"fs3:\\"
#define	FS4		"fs4:\\"
#define	FS5		"fs5:\\"
#define	FS6		"fs6:\\"
#define	FS7		"fs7:\\"
#define	FS8		"fs8:\\"

UINT8	*fDev[] = {FS2, FS3, FS4, FS5, FS6, FS7, FS8, NULL};

#define	PATH_SIZE	256
UINT8	fileImg[PATH_SIZE + 10];
UINT8	fileIn[PATH_SIZE + 10];
UINT8	fileOut[PATH_SIZE + 10];
UINT8	gFileOut[PATH_SIZE + 10];



#define	BIOS_REGION_OFFSET	0x44	// смещение в дескрипторе
//=============================================================================================

EFI_BOOT_SERVICES	*mBootServices;
EFI_RUNTIME_SERVICES	*mRuntimeServices;

/*
EFI_STATUS  EFIAPI	UefiBootServicesTableLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );
*/

void	Stall(UINTN delay){

	mBootServices->Stall(delay);
}

#define	TRACE(arg)	\
{	tto('\r'); \
	tto('\n'); \
	tto(0x30 + arg); \
	tto(0x2e);\
}


UINTN  EFIAPI myPrint (
  IN CONST CHAR16  *Format,
  ...
  );


void	str16to8(UINT8 * dst, UINT16 *src){
int	i;

	for(i = 0; i < PATH_SIZE; i++)
	  {
	      if(src[i] == 0)
				break;
	      dst[i] = (UINT8)src[i];
	  }
	dst[i] = 0;
}


//=============================================================================================

int	sioIndexPort = 0x2e;
int	sioDataPort = 0x2f;

VOID	SioWriteData(UINT8 Index, UINT8 Data)
{
	outportb(sioIndexPort, Index);  
	outportb(sioDataPort, Data);  

}

enum {
	DMODE_WINBOND = 0,
	DMODE_ITE
};

VOID	SioEnterPnp( int mode, int sioBaseAddress)
{
	sioIndexPort = sioBaseAddress;
	sioDataPort = sioIndexPort + 1;
	switch(mode) {
	case DMODE_WINBOND:
		outportb(sioIndexPort, 0x87);  
		outportb(sioIndexPort, 0x87);
		break;
	case DMODE_ITE:
		outportb(sioIndexPort, 0x87);  
		outportb(sioIndexPort, 0x01);
		outportb(sioIndexPort, 0x55);
		outportb(sioIndexPort, 0x55);
		break;
	}
}

VOID	SioExitPnp(int mode)
{
	switch(mode) {
	case DMODE_WINBOND:
		outportb(sioIndexPort, 0xaa);  
		break;
	case DMODE_ITE:
		SioWriteData(0x2, 0x2);
		break;
	}
}

// для GigabyteH77:
VOID	_SioEnterPnp(VOID)
{
	SioEnterPnp(DMODE_ITE, 0x2e);
}

VOID	_SioExitPnp(VOID)
{
	SioExitPnp(DMODE_ITE);
}



//=====================================================================================
void	dumpBuf2(void *buf0, int len){
int	i;
UINT32	bufAddr = ((UINT32)(UINTN)buf0) & ~0xf;
UINT8	*buf = (UINT8*)bufAddr;

	print ("%08x:", (UINT32)(UINTN)buf); 
	for(i = 0; i < len; i++)
	{
	  if((i & 0xf) == 0)
		print("%02x: ", i);
	  if((i & 0x3) == 0)
		_print(" ");
	  _print(" %02x", buf[i]);
	}
}

void	dumpBuf(void *buf0){
	dumpBuf2(buf0, 256);
}


#ifdef TEST_MODE

UINT8	tmp_buf[256];
UINT32	mask[2] = {0x11009100, 0x0000000b};	// биты-кандидаты на управление CS0/CS1
// считать gpioBase (d31:f0:48)
// считать текущие состояния интересующих битов (как-то выделить? вывести таблицу по маскам?)
// механизм изменения: b<Y>te.b<I>t, <+>/<->, <1>/<0>

void	biosDump(UINT8 *BiosStart){
int	i;
	print ("BIOS dump:"); 
	print ("descriptor [0x10]:"); 
	CopyMem(tmp_buf, BiosStart + 0x10, 16);
	for(i = 0; i < 16; i++)
	{
	  _print(" %02x", tmp_buf[i]);
	}
	print ("last segment [0xfffffff0]:"); 
	CopyMem(tmp_buf, (UINT8*)(UINTN)0xfffffff0, 16);
	for(i = 0; i < 16; i++)
	{
	  _print(" %02x", tmp_buf[i]);
	}

}


VOID	Test(UINT8 *BiosStart){
int	gpioBase = readPci0(31, 0, 0x48) & ~1;
int	chr;

	print("GPIO: %08x  %08x", inportd(gpioBase + 0xc), inportd(gpioBase + 0x38));
	biosDump(BiosStart);

	print("invert GPIO? <i>");
	_print(" set [fe] |= 1? <1>, set [f1] |= 4? <4>");
	chr = getch(0);
	if(chr == 'i')
	{
	outportd(gpioBase + 0xc, inportd(gpioBase + 0xc) ^ mask[0]);
	outportd(gpioBase + 0xc, inportd(gpioBase + 0x38) ^ mask[1]);

	print("GPIO: %08x  %08x", inportd(gpioBase + 0xc), inportd(gpioBase + 0x38));
	biosDump(BiosStart);
	chr = getch(0);

	outportd(gpioBase + 0xc, inportd(gpioBase + 0xc) ^ mask[0]);
	outportd(gpioBase + 0xc, inportd(gpioBase + 0x38) ^ mask[1]);
	}

	if(chr == '1')
	{
int	tmp;
	  _SioEnterPnp();
	  SioWriteData(7, 7);	// LogDev = 7
	  outportb(sioIndexPort, 0xef);
	  tmp = inportb(sioDataPort);
	  print(" [ef] = %02x", tmp);
	  tmp |= 1;
	  outportb(sioDataPort, tmp);
	  _SioExitPnp();
	print("GPIO: %08x  %08x", inportd(gpioBase + 0xc), inportd(gpioBase + 0x38));
	biosDump(BiosStart);
	chr = getch(0);
	  _SioEnterPnp();
	  SioWriteData(7, 7);	// LogDev = 7
	  outportb(sioIndexPort, 0xef);
	  tmp = inportb(sioDataPort);
	  print(" [ef] = %02x", tmp);
	  tmp &= ~1;
	  outportb(sioDataPort, tmp);
	  _SioExitPnp();

	}

	if(chr == '4')
	{
int	tmp;
	  _SioEnterPnp();
	  SioWriteData(7, 4);	// LogDev = 4
	  outportb(sioIndexPort, 0xf1);
	  tmp = inportb(sioDataPort);
	  print(" [f1] = %02x", tmp);
	  tmp |= 4;
	  outportb(sioDataPort, tmp);
	  _SioExitPnp();
	}
}
#endif // TEST_MODE

#ifdef GIGABYTE_H77
// для TI8728:
void	setCS0(void){
int	tmp;
	  _SioEnterPnp();
	  SioWriteData(7, 7);	// LogDev = 7
	  outportb(sioIndexPort, 0xef);
	  tmp = inportb(sioDataPort);
	  print(" [ef] = %02x", tmp);
	  tmp &= ~1;
	  outportb(sioDataPort, tmp);
	  _SioExitPnp();
}
void	setCS1(void){
int	tmp;
	  _SioEnterPnp();
	  SioWriteData(7, 7);	// LogDev = 7
	  outportb(sioIndexPort, 0xef);
	  tmp = inportb(sioDataPort);
	  print(" [ef] = %02x", tmp);
	  tmp |= 1;
	  outportb(sioIndexPort, tmp);
	  _SioExitPnp();
}

#endif // GIGABYTE_H77

//======================================================================================
VOID	ShowAllDevicesPath(  IN VOID  );


// выводим путь
static VOID	PrintDevicePath(EFI_DEVICE_PATH_PROTOCOL *pDpp){
CHAR16 *PathString;
  
	PathString = DevPathToString(pDpp, FALSE, TRUE);
	Print(L"PATH:%S\n", PathString);
}

// выводим путь и дополнительную информацию:
// - Handle, наличие BlockIo, LastBlock ...
// Handle	- хэндл, где установлен DevicePath
// pDpp		- хэндл на протокол DevicePath
static VOID	PrintDevicePath2(EFI_DEVICE_PATH_PROTOCOL *pDpp, EFI_HANDLE Handle){
CHAR16			*PathString;
EFI_STATUS		Status;
EFI_BLOCK_IO_PROTOCOL	*BlockIo;
  
	PathString = DevPathToString(pDpp, FALSE, TRUE);
	DEBUG((EFI_D_ERROR, "\r\n[%08x], PATH:%S  ", Handle, PathString));
	Status = gBS->HandleProtocol (
					Handle,
					&gEfiBlockIoProtocolGuid,
					(VOID *) &BlockIo);
	if (EFI_ERROR(Status)) 
	{
	  return;
	}
	DEBUG((EFI_D_ERROR, "	+ BlockIO"));
	DEBUG((EFI_D_ERROR, "\r\n BlockSize = %x, LastBlock = %x, LowestAlignedLba = %x, LogicalPartition = %d", 
			BlockIo->Media->BlockSize, BlockIo->Media->LastBlock, BlockIo->Media->LowestAlignedLba, BlockIo->Media->LogicalPartition));

}


static	VOID	ShowAllMyDevicesPath(void){
EFI_STATUS			Status;
EFI_HANDLE			*Buffer;
EFI_DEVICE_PATH_PROTOCOL	*DevicePath;
UINTN				HandleCount;
UINTN				LineCount = 0;
UINTN				i;

	Status = gBS->LocateHandleBuffer (ByProtocol,
					  &gEfiDevicePathProtocolGuid,
					  NULL,
					  &HandleCount,
					  &Buffer);

	if (EFI_ERROR(Status) || HandleCount == 0 || Buffer == NULL) 
	{
	  print("LocateHandleBuffer for DevicePathProtocol status = %x", Status);
	  return;
	}

	for(i = 0; i < HandleCount; i++) 
	{
	  Status = gBS->HandleProtocol (
					Buffer[i],
					&gEfiDevicePathProtocolGuid,
					(VOID *) &DevicePath);
	  if (EFI_ERROR(Status)) 
	  {
	    continue;
	  }
	  PrintDevicePath(DevicePath);
//	  PrintDevicePath2(DevicePath, Buffer[i]);
	  LineCount++;
	  if((LineCount & 0x7) == 0)
	  {
	    print("any key..");
	    getch(0);
	  }
	}

}

//======================================================================================
// верификация в размере региона:
int	compareSpiFlash(UINT8 *Buf, int regionAddress, UINT32 biosStart, int biosSize)
{
int	i;
int	chr;
int	res = TRUE;

	for(i = regionAddress; i < biosSize; i++)
	{
	  if(((UINT8*)biosStart)[i] != Buf[i])
	  {
	    res = FALSE;
	    print("Verification error: [%08x]: %02x -> %02x, 'x' - break", 
				i, Buf[i], ((UINT8*)biosStart)[i]);
	    chr = getch(0);
	    if(chr == 'x')
			break;
	  }

	}
	return	res;
}
//=================================================================================


#define	DXE_SERVICES_TABLE_GUID { 0x05AD34BA, 0x6F02, 0x4214, 0x95, 0x2E, 0x4D, 0xA0, 0x39, 0x8E, 0x2B, 0xB9}
#define	EFI_HOB_LIST_GUID       { 0x7739F24C, 0x93D7, 0x11D4, 0x9A, 0x3A, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}
#define EFI_MEMORY_TYPE_INFO_GUID	{ 0x4C19049F, 0x4137, 0x4DD3, 0x9C, 0x10, 0x8B, 0x97, 0xA8, 0x3F, 0xFD, 0xFA}
#define EFI_DEBUG_IMAGE_INFO_GUID	{ 0x49152E77, 0x1ADA, 0x4764, 0xB7, 0xA2, 0x7A, 0xFE, 0xFE, 0xD9, 0x5E, 0x8B}


#define EFI_ACPI_20_TABLE_GUID	{0x8868e871,0xe4f1,0x11d3,0xbc,0x22,0x0,0x80,0xc7,0x3c,0x88,0x81}
#define ACPI_TABLE_GUID		{0xeb9d2d30,0x2d88,0x11d3,0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d}
#define SAL_SYSTEM_TABLE_GUID	{0xeb9d2d32,0x2d88,0x11d3,0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d}
#define SMBIOS_TABLE_GUID	{0xeb9d2d31,0x2d88,0x11d3,0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d}
#define MPS_TABLE_GUID		{0xeb9d2d2f,0x2d88,0x11d3,0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d}
//
// ACPI 2.0 or newer tables should use EFI_ACPI_TABLE_GUID
//
#define EFI_ACPI_TABLE_GUID	{0x8868e871,0xe4f1,0x11d3,0xbc,0x22,0x0,0x80,0xc7,0x3c,0x88,0x81}
#define ACPI_10_TABLE_GUID	{0xeb9d2d30,0x2d88,0x11d3,0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d}

typedef	struct{
EFI_GUID	guid;
char		*tableName;
}	DEF_GUID_TABLE;

#define	DEF_GUID_NAME(name)\
	{name, #name}

DEF_GUID_TABLE	defGuidTable[] = {
	DEF_GUID_NAME(EFI_ACPI_20_TABLE_GUID),
	DEF_GUID_NAME(ACPI_TABLE_GUID),
	DEF_GUID_NAME(SAL_SYSTEM_TABLE_GUID),
	DEF_GUID_NAME(SMBIOS_TABLE_GUID),
	DEF_GUID_NAME(MPS_TABLE_GUID),
	DEF_GUID_NAME(DXE_SERVICES_TABLE_GUID),
	DEF_GUID_NAME(EFI_HOB_LIST_GUID),
	DEF_GUID_NAME(EFI_MEMORY_TYPE_INFO_GUID),
	DEF_GUID_NAME(EFI_DEBUG_IMAGE_INFO_GUID),
	};


int	guidCompare(EFI_GUID *guid1, EFI_GUID *guid2){
int	i;

	  for(i = 0; i < sizeof(EFI_GUID); i++)
	  {
	    if(((UINT8*)guid1)[i] != ((UINT8*)guid2)[i])
							break;
	  }
	  if(i >= sizeof(EFI_GUID))
	  {
	    return TRUE;
	  }
	return FALSE;
} 

int	findGuid(EFI_GUID *guid){
int	index;
int	flag = FALSE;

	for(index = 0; index < sizeof(defGuidTable) / sizeof(defGuidTable[0]); index++)
	{
	  if(guidCompare(guid, &defGuidTable[index].guid))
	  {
	    flag = TRUE;
	    break;
	  }
	}

	if(flag)
		return index;

	return -1;
}

void	printGuid(EFI_GUID *guid){
int	i;
	_print("  %08x", *(UINT32*)guid);
	_print("-%04x", ((UINT16*)guid)[2]);
	_print("-%04x", ((UINT16*)guid)[3]);
	for(i = 0; i < 8; i++)
		_print(" %02x", ((UINT8*)guid)[8 + i]);

}

// сохранение фрагмента памяти в файле:
enum {
	SUMODE_WRITE = 0,
	SUMODE_APPEND
};

//
EFI_STATUS	_SaveToUsb(char *fileName, UINT8 *buf, UINT32 len, int mode){
EFI_STATUS	Status = EFI_SUCCESS;
EFI_FILE_HANDLE File;
UINT8		*tmpBuf = gBuf;	
UINTN		BufferSize = 0;


	if(len > SIZE_MAX)
	{
	  print("***Error: len > SIZE_MAX: %d bytes", len);
	  return	EFI_NOT_FOUND;
	}

	// 1. создание файла:
  	sprintf(gFileOut, "%s%s", gFs, fileName);  
	if(mode == SUMODE_APPEND)
	{
UINTN	Position;
	  File = LibFsOpenFile(gFileOut, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
	  if (NULL == File) 
	  {
	    print("***Error: can not create file %s", gFileOut);
	    return	EFI_NOT_FOUND;
	  }
	  Position = LibFsSizeFile(File);
	  LibFsSetPosition(File, Position);
	}
	else
	{
	  File = LibFsCreateFile(gFileOut);
	  if (NULL == File) 
	  {
	    print("***Error: can not create file %s", gFileOut);
	    return	EFI_NOT_FOUND;
	  }
	}

	print("\r\nRead data to file %s...", gFileOut);

	// 2. копируем таблицу во временный буфер:
	CopyMem(tmpBuf, buf, len);

	// 3. записываем буфер  в файл:
	BufferSize = len;
	Status = LibFsWriteFile(File, &BufferSize, tmpBuf);

	LibFsCloseFile(File);


	if(!EFI_ERROR(Status))
	{
	  print("Save data to file %s, size = %lx, - OK", gFileOut, len);
	}
	else
	{
	  print("Save data to file %s: ERROR"
		"\r\n writed %lx bytes, status = %x", 
				gFileOut, BufferSize, Status);
	}
	return Status;
}

EFI_STATUS	SaveToUsb(char *fileName, UINT8 *buf, UINT32 len){

	return _SaveToUsb(fileName, buf, len, SUMODE_WRITE);
}

void	dispTableInfo(void){
int	i;
int	index;
EFI_CONFIGURATION_TABLE  *ConfigurationTable = gST->ConfigurationTable;
int	indexAcpiTab = -1;
int	indexSmbiosTab = -1;
EFI_GUID	*guid;
EFI_GUID	acpiGuid = ACPI_TABLE_GUID;
EFI_GUID	acpi20Guid = EFI_ACPI_20_TABLE_GUID;
EFI_GUID	smBiosGuid = SMBIOS_TABLE_GUID;
int	chr;


	print("NumberOfTableEntries = %d", gST->NumberOfTableEntries);
	print("Entries:");
	for(i = 0; i < gST->NumberOfTableEntries; i++)
	{
	  print("[%d]", i);
	  guid = &ConfigurationTable[i].VendorGuid;
	  printGuid(guid);
	  index = findGuid(guid);
	  if(index >= 0)
	  {
	    _print("  %s", defGuidTable[index].tableName);
	    if((indexAcpiTab < 0)
			&& guidCompare(guid, &acpiGuid))
	    {
	       indexAcpiTab = i;
	    }
	    if(guidCompare(guid, &acpi20Guid))
	    {
	       indexAcpiTab = i;
	    }
	    if(guidCompare(guid, &smBiosGuid))
	    {
	       indexSmbiosTab = i;
	    }
	  }
	}
	print("s - read SMBIOS");
	print("a - read ACPI tables");
	print("i - SaGlobalNvs, igdOpRegion, VBT");
	print ("any key - next table info");

	chr = getch(0);
/*
	if((chr == 'd') && (indexSmbiosTab >= 0))
	{
	  print("Delete Smbios record? <y>");
	  chr = getch(0);
	  if(chr == 'y')
	  {
	    // изменяем значение GUID'a в записи SMBIOS_TAB:
	    *(WORD*)&ConfigurationTable[indexSmbiosTab].VendorGuid = 0;
	  }

	  goto tab_fin;
	}
*/
	if(chr == 's') 
	{ // чтение SMBIOS:
enum	{VER_2, VER_3};
UINT8	*smbiosTable;
int	smbiosVersionM;
UINT32	smbiosVersion;
UINT8	*smbiosBase;
UINT8	*tmpTab;
UINT32	smbiosEnd;
UINT32	smbiosLength;
UINT8	structType;
UINT8	structLength;
UINT16	structHandle;
int	numOfTab = 0;

	  if(indexSmbiosTab < 0)
	  {
	    print("SMBIOS not found");
	    goto	tab_fin;
	  }
	  smbiosTable = (UINT8*)ConfigurationTable[indexSmbiosTab].VendorTable;
	  print("SMBIOS addr = %08x", smbiosTable);

	  smbiosVersionM = VER_2;
	  if(CompareMem(smbiosTable, "_SM3_", 5) == 0)
	  {
	    smbiosVersionM = VER_3;
	  }

	  if(smbiosVersionM == VER_3)
	  {
	    smbiosVersion = (*(UINT32*)(smbiosTable + 7)) & 0xffffff;
	    smbiosBase = (UINT8*)(UINTN)(*(UINT64*)(smbiosTable + 0x10));
	    smbiosLength = *(UINT32*)(smbiosTable + 0x0c);
	  }
	  else
	  {
	    smbiosVersion = (*(UINT32*)(smbiosTable + 6)) & 0xffff;
	    smbiosBase = (UINT8*)(UINTN)(*(UINT32*)(smbiosTable + 0x18));
	    smbiosLength = (UINT32)*(UINT16*)(smbiosTable + 0x16);
	  }
	  print("  ver %d.%d.%d, base = %08x, length = %x", smbiosVersion & 0xff,
							   (smbiosVersion >> 8) & 0xff,
							   (smbiosVersion >> 16) & 0xff,
							   smbiosBase,
							   smbiosLength
	  );
	  if(smbiosVersionM == VER_2)
	  { 
	    print("  Numer of Structures: %d", *(UINT16*)(smbiosTable + 0x1c));
	  }

	  smbiosEnd = (UINT32)(UINTN)smbiosBase + smbiosLength;
	  tmpTab = smbiosBase;
	  while((UINT32)(UINTN)tmpTab < smbiosEnd)
	  {
	    structType = tmpTab[0];
	    structLength = tmpTab[1];
	    structHandle = *(UINT16*)(tmpTab + 2);
	    tmpTab += structLength;
	    while((UINT32)(UINTN)tmpTab < smbiosEnd)
	    {
	      if(*tmpTab == 0)
	      {
	        tmpTab++;
		if(*tmpTab == 0)
		{
		  tmpTab++;
		  break;
		}
	      }
	      tmpTab++;
	    }
	    print("  [%d] %04x size = %d", structType, structHandle, structLength);
	    numOfTab++;
	    if((numOfTab & 0xf) == 0)
					getch(0);
	  }

	  SaveToUsb(FILE_SMBIOS_HEADER, smbiosTable, 0x20);
	  SaveToUsb(FILE_SMBIOS_TABLE, smbiosBase, smbiosLength);

	  print ("any key..");
	  getch(0);
	  dumpBuf(smbiosTable);

	  goto	tab_fin;

	}

	if(chr == 'a')
	{ // считывание ACPI таблиц:
UINT8		*acpiTable;
UINT32		*rsdt;
UINT64		*xsdt;
UINT32		*fadt = NULL;
UINT32		*dsdt;
UINT32		*tmp_tab;
UINT32		len;
UINT32		tmp_len;
UINT32		addr_min = 0xffffffff;
UINT32		addr_max = 0;
UINT32		i;
char		tmpStr[256];

#define	SET_MIN(arg)\
	if((UINT32)(UINTN)arg < addr_min)\
	{\
	  addr_min = (UINT32)(UINTN)arg;\
	}
#define	SET_MAX(arg)\
	if((UINT32)(UINTN)arg > addr_max)\
	{\
	  addr_max = (UINT32)(UINTN)arg;\
	}

	  if(indexAcpiTab < 0)
	  {
  	    print("index ACPI tables not found");
	    goto tab_fin;
	  }
	  if(gFlagUsb == FALSE)
	  {
  	    print("USB drive not found");
	    goto tab_fin;
	  }

	  // анализируем адреса таблиц:
	  acpiTable = (UINT8*)ConfigurationTable[indexAcpiTab].VendorTable;
	  SET_MIN(acpiTable);
	  SET_MAX(acpiTable + 0x100);

	  // RSDT:
	  rsdt = (UINT32*)(UINTN)*(UINT32*)(acpiTable + 16);
	  len =  rsdt[1];
	  SET_MIN(rsdt);
	  SET_MAX(rsdt + len / 4);

	  for(i = 0; i < (len - 36) / 4; i++)
	  {
	    tmp_tab = (UINT32*)(UINTN)rsdt[9 + i];
	    print("[%d] %08x", i, tmp_tab);
	    *(UINT32*)tmpStr = tmp_tab[0];
	    tmpStr[4] = 0;
	    _print("    \"%s\"", tmpStr);
	    if(*(UINT32*)tmpStr == *(UINT32*)"FACP")
	    {
	      fadt = tmp_tab;
	    }
	    tmp_len = tmp_tab[1];
	    SET_MIN(tmp_tab);
	    SET_MAX(tmp_tab + tmp_len / 4);
	  }
	  if(fadt)
	  {
	    dsdt = (UINT32*)(UINTN)fadt[10];
	    len =  dsdt[1];
	    SET_MIN(dsdt);
	    SET_MAX(dsdt + len / 4);
	  }

	  // XSDT:
	  xsdt = (UINT64*)(UINTN)*(UINT64*)(acpiTable + 24);
	  len = ((UINT32*)xsdt)[1];
	  SET_MIN(xsdt);
	  SET_MAX((UINT32)(UINTN)xsdt + len);

	  xsdt = (UINT64*)(((UINTN)xsdt) + 36);

	  for(i = 0; i < (len - 36) / 8; i++)
	  {
	    tmp_tab = (UINT32*)(UINTN)xsdt[i];
	    print("[%d] %08x", i, tmp_tab);
	    *(UINT32*)tmpStr = tmp_tab[0];
	    tmpStr[4] = 0;
	    _print("    \"%s\"", tmpStr);
	    if(*(UINT32*)tmpStr == *(UINT32*)"FACP")
	    {
	      fadt = tmp_tab;
	    }
	    tmp_len = tmp_tab[1];
	    SET_MIN(tmp_tab);
	    SET_MAX(tmp_tab + tmp_len / 4);
	  }
	  if(fadt)
	  {
	    dsdt = (UINT32*)(UINTN)fadt[10];
	    len =  dsdt[1];
	    SET_MIN(dsdt);
	    SET_MAX(dsdt + len / 4);
	  }
	  else
	  {
	    print("error: FADT not found!");
	  }
	  print("\r\n addr_min = %08x", addr_min);
	  print("\r\n addr_max = %08x\n", addr_max);

	  if((addr_max - addr_min) > 0x80000)
	  {
	    print("size of file too much: %08x", addr_max - addr_min);
	    goto fin_acpi_save;
	  }

	  // записываем фрагмент памяти в файл:
  	  SaveToUsb(FILE_ACPI_TABLE, (UINT8*)(UINTN)addr_min, (addr_max - addr_min + 0xff) & ~0xff);

	  CopyMem(tmpStr, "ACPIDUMP", 8);
	  *(UINT32*)(tmpStr + 8) = addr_min;
	  *(UINT32*)(tmpStr + 12) = (UINT32)(UINTN)acpiTable;
  	  _SaveToUsb(FILE_ACPI_TABLE, tmpStr, 16, SUMODE_APPEND);


fin_acpi_save:
	  print ("any key..");
	  getch(0);
	  goto	tab_fin;
	}

	if(chr == 'i')
	{ // SaGlobalNvs, IgdOpRegion, VBT
EFI_STATUS				Status = EFI_SUCCESS;
IGD_OPREGION_PROTOCOL			*IgdOpRegion;
EFI_GLOBAL_NVS_AREA_PROTOCOL		*GlobalNvsArea;
HASWELL_SYSTEM_AGENT_GLOBAL_NVS_AREA_PROTOCOL	*SaGlobalNvsArea = NULL;	// Haswell
SYSTEM_AGENT_GLOBAL_NVS_AREA_PROTOCOL	*SaGlobalNvsArea2 = NULL;		// Skylake
PCH_NVS_AREA_PROTOCOL			*PchNvsArea;				// Skulake
DXE_PLATFORM_SA_POLICY_PROTOCOL		*DxePlatformSaPolicy;
PLATFORM_GOP_POLICY_PROTOCOL		*PlatformGopPolicy;
UINT8					tmpStr[32];
UINT8					*tmpBuf = (UINT8*)(UINTN)(((UINT32)(UINTN)gBuf + 0xf) & ~0xf);	


	  // EfiGlobalNvs:
	  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
	  print("Locate GlobalNvsAreaProtocol: status = %x", Status);
	  if(!EFI_ERROR(Status) && (GlobalNvsArea->Area != NULL))
	  {
	    print("Revision = 0x%x", GlobalNvsArea->Area->Revision);					// 0x1
	    print("_DOSDisplaySupportFlag = 0x%x", GlobalNvsArea->Area->_DOSDisplaySupportFlag);		// 0x0
	    print("EcAvailable = 0x%x", GlobalNvsArea->Area->EcAvailable);				// 0x0
	    print("GlobalInterruptModeFlag = 0x%x", GlobalNvsArea->Area->GlobalInterruptModeFlag);	// 0x0
	    print("PlatformFlavor = 0x%x", GlobalNvsArea->Area->PlatformFlavor);				// 0x2
	    
	    CopyMem(tmpBuf, (UINT8*)GlobalNvsArea->Area, 0x600);
	    dumpBuf(tmpBuf);
  	    print ("any key..");
	    getch(0);
	    dumpBuf(tmpBuf + 0x100);
  	    print ("any key..");
	    getch(0);
	    dumpBuf(tmpBuf + 0x200);
  	    print ("any key..");
	    getch(0);
	    SaveToUsb(FILE_GNVS_DATA, (UINT8*)tmpBuf, 0x600);
	  }

	  // SaGlobalNvs:
	  if(pchNum == PCH10)
	  {
	    Status = gBS->LocateProtocol (
                  &gSaGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &SaGlobalNvsArea2
                  );
	  }
	  else
	  {
	    Status = gBS->LocateProtocol (
                  &gSaGlobalNvsAreaProtocolHaswellGuid,
                  NULL,
                  (VOID **) &SaGlobalNvsArea
                  );
	  }
	  print("Locate SaGlobalNvsAreaProtocol: status = %x", Status);
	  CopyMem(tmpStr, "SaGlobalNvs", 12);
	  if(!EFI_ERROR(Status) && (SaGlobalNvsArea->Area != NULL))
	  {
	    if(pchNum != PCH10)
	      {
	        ((UINT32*)tmpStr)[3] = (UINT32)(UINTN)SaGlobalNvsArea->Area;
		print("GfxTurboIMON = 0x%x", SaGlobalNvsArea->Area->GfxTurboIMON);				// 0x1f

		print("IgdState = 0x%x", SaGlobalNvsArea->Area->IgdState);					// 0x1
		print("CurrentDeviceList = 0x%x", SaGlobalNvsArea->Area->CurrentDeviceList);			// 0xf
		print("PreviousDeviceList = 0x%x", SaGlobalNvsArea->Area->PreviousDeviceList);			// 0xf
		print("NumberOfValidDeviceId = 0x%x", SaGlobalNvsArea->Area->NumberOfValidDeviceId);		// 0x8

		print("DeviceId1 = 0x%x", SaGlobalNvsArea->Area->DeviceId1);					// 0x100
		print("DeviceId2 = 0x%x", SaGlobalNvsArea->Area->DeviceId2);					// 0x400
		print("DeviceId3 = 0x%x", SaGlobalNvsArea->Area->DeviceId3);					// 0x300
		print("DeviceId4 = 0x%x", SaGlobalNvsArea->Area->DeviceId4);					// 0x301
		print("DeviceId5 = 0x%x", SaGlobalNvsArea->Area->DeviceId5);					// 0x302
		print("DeviceId6 = 0x%x", SaGlobalNvsArea->Area->DeviceId6);					// 0x303
		print("DeviceId7 = 0x%x", SaGlobalNvsArea->Area->DeviceId7);					// 0x304
		print("DeviceId8 = 0x%x", SaGlobalNvsArea->Area->DeviceId8);					// 0x305

  	        print ("any key..");
	        getch(0);

		print("BacklightControlSupport = 0x%x", SaGlobalNvsArea->Area->BacklightControlSupport);		// 0x2

		print("BrightnessPercentage = 0x%x", SaGlobalNvsArea->Area->BrightnessPercentage);		// 0x64
		
		print("IgdPanelType = 0x%x", SaGlobalNvsArea->Area->IgdPanelType);				// 0xf
		print("IgdPanelScaling = 0x%x", SaGlobalNvsArea->Area->IgdPanelScaling);				// 0
		print("IgdBlcConfig = 0x%x", SaGlobalNvsArea->Area->IgdBlcConfig);				// 0
		
		
		
		print("IgdBiaConfig = 0x%x", SaGlobalNvsArea->Area->IgdBiaConfig);				// 0x6	
		print("IgdSscConfig = 0x%x", SaGlobalNvsArea->Area->IgdSscConfig);				// 0x1

		print("IgdPowerConservation = 0x%x", SaGlobalNvsArea->Area->IgdPowerConservation);		// 0
		

		print("IgdDvmtMemSize = 0x%x", SaGlobalNvsArea->Area->IgdDvmtMemSize);				// 0x3

		print("IgdHpllVco = 0x%x", SaGlobalNvsArea->Area->IgdHpllVco);					// 0
		print("IgdSciSmiMode = 0x%x", SaGlobalNvsArea->Area->IgdSciSmiMode);				// 0
		

		print("LidState = 0x%x", SaGlobalNvsArea->Area->LidState);					// 0x1
		print("PackageCstateLimit = 0x%x", SaGlobalNvsArea->Area->PackageCstateLimit);			// 0x5
		print("Peg0LtrEnable = 0x%x", SaGlobalNvsArea->Area->Peg0LtrEnable);				// 0x1
		print("Peg0ObffEnable = 0x%x", SaGlobalNvsArea->Area->Peg0ObffEnable);				// 0x1
		print("Peg2LtrEnable = 0x%x", SaGlobalNvsArea->Area->Peg1LtrEnable);				// 0x1	
		print("Peg2ObffEnable = 0x%x", SaGlobalNvsArea->Area->Peg1ObffEnable);				// 0x1
		print("Peg2LtrEnable = 0x%x", SaGlobalNvsArea->Area->Peg2LtrEnable);				// 0x1
		print("Peg2ObffEnable = 0x%x", SaGlobalNvsArea->Area->Peg2ObffEnable);				// 0x1

		print("PegLtrMaxSnoopLatency = 0x%x", SaGlobalNvsArea->Area->PegLtrMaxSnoopLatency);		// 0x846
		print("PegLtrMaxNoSnoopLatency = 0x%x", SaGlobalNvsArea->Area->PegLtrMaxNoSnoopLatency);		// 0x846

		print("Peg0PowerDownUnusedBundles = 0x%x", SaGlobalNvsArea->Area->Peg0PowerDownUnusedBundles);	// 0xff
		print("Peg1PowerDownUnusedBundles = 0x%x", SaGlobalNvsArea->Area->Peg1PowerDownUnusedBundles);	// 0xff
		print("Peg2PowerDownUnusedBundles = 0x%x", SaGlobalNvsArea->Area->Peg2PowerDownUnusedBundles);  // 0xff
	      } // if(pchNum != PCH10)
	      else
	      {
	        ((UINT32*)tmpStr)[3] = (UINT32)(UINTN)SaGlobalNvsArea2->Area;
		print("IgdOpRegionAddress = 0x%x", SaGlobalNvsArea2->Area->IgdOpRegionAddress);			// 
		print("IgdState = 0x%x", SaGlobalNvsArea2->Area->IgdState);					// 
	      }

  	    print ("Save SaGlobalNvs? <y>");
	    chr = getch(0);
	    if(chr != 'y')
		goto	SA_info_next0;
	    SaveToUsb(FILE_SA_DATA, tmpStr, 0x10);
	    if(pchNum != PCH10)
		    _SaveToUsb(FILE_SA_DATA, (UINT8*)SaGlobalNvsArea->Area, sizeof(HASWELL_SYSTEM_AGENT_GLOBAL_NVS_AREA), SUMODE_APPEND);
	    else
		    _SaveToUsb(FILE_SA_DATA, (UINT8*)SaGlobalNvsArea2->Area, sizeof(SYSTEM_AGENT_GLOBAL_NVS_AREA), SUMODE_APPEND);

	  } //  SaGlobalNvs
SA_info_next0:

	  // PchNvs:
	  if(pchNum != PCH10)
			goto	SA_info_next1;

	  Status = gBS->LocateProtocol (
                  &gPchNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &PchNvsArea
                  );
	  print("Locate PchNvsAreaProtocol: status = %x", Status);
	  if(!EFI_ERROR(Status) && (PchNvsArea->Area != NULL))
	  {

		print("RcRevision = 0x%x", PchNvsArea->Area->RcRevision);       			// 0x
		print("PchSeries = 0x%x", PchNvsArea->Area->PchSeries);				// 0x
		print("XHPC = 0x%x", PchNvsArea->Area->XHPC);					// 0x
		print("XRPC = 0x%x", PchNvsArea->Area->XRPC);					// 0x
		print("XSPC = 0x%x", PchNvsArea->Area->XSPC);					// 0x
		print("XSPA = 0x%x", PchNvsArea->Area->XSPA);					// 0x
		print("HPTB = 0x%x", PchNvsArea->Area->HPTB);					// 0x
		print("HPTE = 0x%x", PchNvsArea->Area->HPTE);					// 0x

  	    print ("Save PchNvs? <y>");
	    chr = getch(0);
	    if(chr != 'y')
		goto	SA_info_next1;

	    CopyMem(tmpStr, "PchNvs     ", 12);
	    ((UINT32*)tmpStr)[3] = (UINT32)(UINTN)PchNvsArea->Area;
	    SaveToUsb(FILE_PCH_DATA, tmpStr, 0x10);
	    _SaveToUsb(FILE_PCH_DATA, (UINT8*)PchNvsArea->Area, sizeof(PCH_NVS_AREA), SUMODE_APPEND);
	  } //  PchNvs



SA_info_next1:
	  // DxePlatformSaPolicy
	  Status = gBS->LocateProtocol (
                  &gDxePlatformSaPolicyGuid,
                  NULL,
                  (VOID **) &DxePlatformSaPolicy
                  );
	  print("\r\nLocate DxePlatformSaPolicy: status = %x", Status);
	  if(!EFI_ERROR(Status))
	  {
	    print("Revision = 0x%x",DxePlatformSaPolicy->Revision);
	    print("Vtd = %08x",DxePlatformSaPolicy->Vtd);
	    print("PcieConfig = %08x",DxePlatformSaPolicy->PcieConfig);
	    print("MemoryConfig = %08x",DxePlatformSaPolicy->MemoryConfig);
	    print("IgdConfig = %08x",DxePlatformSaPolicy->IgdConfig);
	    print("MiscConfig = %08x",DxePlatformSaPolicy->MiscConfig);
	    print("VbiosConfig = %08x",DxePlatformSaPolicy->VbiosConfig);
/*
	    print("RenderStandby = 0x%x", DxePlatformSaPolicy->IgdConfig->RenderStandby);
	    print("DeepRenderStandby = 0x%x", DxePlatformSaPolicy->IgdConfig->DeepRenderStandby);
	    print("VbtAddress = 0x%x", DxePlatformSaPolicy->IgdConfig->VbtAddress);
	    print("Size = 0x%x", DxePlatformSaPolicy->IgdConfig->Size);
	    print("CdClk = 0x%x", DxePlatformSaPolicy->IgdConfig->CdClk);
	    print("CdClkVar = 0x%x", DxePlatformSaPolicy->IgdConfig->CdClkVar);
	    print("PlatformConfig = 0x%x", DxePlatformSaPolicy->IgdConfig->PlatformConfig);
	    print("IuerStatusVal = 0x%x", DxePlatformSaPolicy->IgdConfig->IuerStatusVal);
	    DxePlatformSaPolicy->IgdConfig->GopVersion[0xf] = 0;
	    Print(L"\r\nGopVersion = \"%s\"", DxePlatformSaPolicy->IgdConfig->GopVersion);
//	    dumpBuf2(DxePlatformSaPolicy->IgdConfig->GopVersion, 0x20);
	    print("\r\nLoadVbios = 0x%x",DxePlatformSaPolicy->VbiosConfig->LoadVbios);
	    print("ExecuteVbios = 0x%x",DxePlatformSaPolicy->VbiosConfig->ExecuteVbios);
	    print("VbiosSource = 0x%x",DxePlatformSaPolicy->VbiosConfig->VbiosSource);
*/	    

	    dumpBuf2(DxePlatformSaPolicy->IgdConfig, 0x80);

  	    print ("\r\nSave SaPolicy? <y>");
	    chr = getch(0);
	    if(chr != 'y')
		goto	SA_info_next2;
	    CopyMem(tmpStr, "SaPolicy    ", 12);
	    ((UINT32*)tmpStr)[3] = (UINT32)(UINTN)DxePlatformSaPolicy;
	    _SaveToUsb(FILE_SA_DATA, tmpStr, 0x10, SUMODE_APPEND);
	    _SaveToUsb(FILE_SA_DATA, (UINT8*)DxePlatformSaPolicy, 0xff0, SUMODE_APPEND);
	  }




SA_info_next2:
	  // IgdOpRegon:
	  Status = gBS->LocateProtocol (
                  &gIgdOpRegionProtocolGuid,
                  NULL,
                  (VOID **) &IgdOpRegion
                  );
	  print("\r\nLocate IgdOpRegionProtocol: status = %x", Status);
	  if(!EFI_ERROR(Status))
	  {
OPREGION_HEADER	Header;
	    _print(" - OK");
	    print("OpRegion = %lx, OpRegion->VBT = %lx", IgdOpRegion->OpRegion, &IgdOpRegion->OpRegion->VBT);
	    print("VbtData = %02x %02x %02x %02x", 
			IgdOpRegion->OpRegion->VBT.GVD1[0],
			IgdOpRegion->OpRegion->VBT.GVD1[1],
			IgdOpRegion->OpRegion->VBT.GVD1[2],
			IgdOpRegion->OpRegion->VBT.GVD1[3]
			);

	    CopyMem(&Header, &IgdOpRegion->OpRegion->Header, sizeof(Header));
	    Header.SIGN[0xf] = 0;
	    print("OpRegion signature = \"%s\"", Header.SIGN);
	    print("OpRegion size = %x", Header.SIZE);
	    print("OpRegion version = %08x", Header.OVER);
	    Header.SVER[0x1f] = 0;
	    print("BIOS version = \"%s\"", Header.SVER);
	    Header.VVER[0xf] = 0;
	    print("VideoBios version = \"%s\"", Header.VVER);
	    Header.GVER[0xf] = 0;
	    print("Graphic driver = \"%s\"", Header.GVER);
	    print("MBOX = %x", Header.MBOX);
	    print("Driver model = %d", Header.DMOD);
	    print("Platform Capabilities = %d", Header.PCON);
	    Header.DVER[0xf] = 0;
	    Print(L"GOP version = \"%s\"", Header.DVER);
/*
  	    print ("Correct MBOX? <y>");
	    chr = getch(0);
	    if(chr == 'y')
	    {
	      IgdOpRegion->OpRegion->Header.MBOX = 0xf;
	    }
*/
/*
  	    print ("any key..");
	    getch(0);
  	    print ("Dump MBOX1:");
	    dumpBuf((UINT8*)&IgdOpRegion->OpRegion->MBox1);
	    getch(0);

  	    print ("Dump MBOX2:");
	    dumpBuf((UINT8*)&IgdOpRegion->OpRegion->MBox2);
	    getch(0);

  	    print ("Dump MBOX3:");
	    dumpBuf((UINT8*)&IgdOpRegion->OpRegion->MBox3);
	    getch(0);

  	    print ("Dump MBOX5:");
	    dumpBuf(((UINT8*)IgdOpRegion->OpRegion) + 0x1c00);
	    getch(0);
*/


	  // IgdOpRegon->VBT:
  	    print ("Save VBT tables? <y>");
	    chr = getch(0);
	    if(chr != 'y')
			goto tab_fin;

	    if(*(DWORD*)&IgdOpRegion->OpRegion->VBT == *(DWORD*)"$VBT")
	    {
    	       SaveToUsb(FILE_VBT, IgdOpRegion->OpRegion->VBT.GVD1, 0x1200);
	    }
	  }

	  
{// считываем VBT из VideoBios (0xc0000):
DWORD	addr;
	    for(addr = 0xc0000; addr < 0xd0000; addr += 4)
	    { // ищем область VBT внутри видеобиоса:
	      if(*(DWORD*)(UINTN)addr == *(DWORD*)"$VBT")
	      {
    	       SaveToUsb(FILE_VBIOS_VBT, (UINT8*)(UINTN)addr, 0x1200);
	       break;
	      }
	    }
	    if(addr >= 0xd0000)
	    {
	      print("table VBT  not found in VideoBIOS area");
	    }
}

// считываем VBT из PlatformGopPolicy:
{
EFI_PHYSICAL_ADDRESS	VbtAddress;
UINT32			VbtSize;
DWORD			addr;

	  Status = gBS->LocateProtocol (
                  &gPlatformGopPolicyProtocolGuid,
                  NULL,
                  (VOID **) &PlatformGopPolicy
                  );
	  print("locate PlatformGopPolicy, Status = %x", Status);
	  if(!EFI_ERROR(Status))
	  {
	    Status = PlatformGopPolicy->GetVbtData(&VbtAddress, &VbtSize);
	    print("GetVbtData, Status = %x", Status);
	    if(!EFI_ERROR(Status))
	    {
	      print("VbtAddress = %lx, VbtSize = %lx", VbtAddress, VbtSize);
	      addr = (DWORD)VbtAddress;
	      if(*(DWORD*)(UINTN)addr == *(DWORD*)"$VBT")
	      {
    	        SaveToUsb(FILE_GOP_VBT, (UINT8*)(UINTN)addr, VbtSize);
	      }

	    }

	  }
}
	  print ("any key..");
	  getch(0);
	  goto	tab_fin;
	} // if(chr == 'i')


	if(indexAcpiTab < 0)
			  goto tab_fin;
{
char	tmpStr[256];
UINT8	*acpiTable;
UINT8	*rsdt;
UINT8	*xsdt;
UINT8	*rsdt_fadt = NULL;
UINT8	*xsdt_fadt = NULL;
UINT8	*rsdt_dsdt = NULL;
UINT8	*xsdt_dsdt = NULL;
int	len;

	print("ACPI table index = %d", indexAcpiTab);
	acpiTable = (UINT8*)ConfigurationTable[indexAcpiTab].VendorTable;
	print("ACPI table address: %08x", acpiTable);
	CopyMem(tmpStr, acpiTable + 9, 6);
	tmpStr[6] = 0;
	print("ACPI tab OEM ID: \"%s\"", tmpStr);
	print("Revision: %d", acpiTable[15]);
	print("RSDT address: %08x", *(UINT32*)(acpiTable + 16));
	print("XSDT address: %08lx", *(UINTN*)(acpiTable + 24));

	rsdt = (UINT8*)(UINTN)*(UINT32*)(acpiTable + 16);
	xsdt = (UINT8*)*(UINTN*)(acpiTable + 24);

	CopyMem(tmpStr, rsdt, 4);
	tmpStr[4] = 0;
	print("\r\ntable RSDT: (signature \"%s\")", tmpStr);
	if(*(UINT32*)tmpStr != *(UINT32*)"RSDT")
					goto	rsdt_fin;
	len = *(UINT32*)(rsdt + 4);
	print("Length: %d bytes", len);
	print("Revision: %d", rsdt[8]);
	CopyMem(tmpStr, rsdt + 10, 6);
	tmpStr[6] = 0;
	print("OEM ID: \"%s\"", tmpStr);
	CopyMem(tmpStr, rsdt + 16, 8);
	tmpStr[8] = 0;
	print("OEM table ID: \"%s\"", tmpStr);
	print("OEM Revision: %08x", *(UINT32*)(rsdt + 24));
	CopyMem(tmpStr, rsdt + 28, 4);
	tmpStr[4] = 0;
	print("Creator ID: \"%s\"", tmpStr);
	print("Creator Revision: %08x", *(UINT32*)(rsdt + 32));
	for(i = 0; i < (len - 36) / 4; i++)
	{
	   print("[%d] %08x", i, ((UINT32*)(rsdt + 36))[i]);
	   *(UINT32*)tmpStr = *(UINT32*)(UINTN)((UINT32*)(rsdt + 36))[i];
	   tmpStr[4] = 0;
	   _print("    \"%s\"", tmpStr);
	   if(*(UINT32*)tmpStr == *(UINT32*)"FACP")
	   {
	     rsdt_fadt = (UINT8*)(UINTN)((UINT32*)(rsdt + 36))[i];
	   }
	}
rsdt_fin:
	print ("any key..");
	getch(0);

	CopyMem(tmpStr, xsdt, 4);
	tmpStr[4] = 0;
	print("\r\ntable XSDT: (signature \"%s\")", tmpStr);
	if(*(UINT32*)tmpStr != *(UINT32*)"XSDT")
					goto	xsdt_fin;
	len = *(UINT32*)(xsdt + 4);
	print("Length: %d bytes", len);
	print("Revision: %d", xsdt[8]);
	CopyMem(tmpStr, xsdt + 10, 6);
	tmpStr[6] = 0;
	print("OEM ID: \"%s\"", tmpStr);
	CopyMem(tmpStr, xsdt + 16, 8);
	tmpStr[8] = 0;
	print("OEM table ID: \"%s\"", tmpStr);
	print("OEM Revision: %08x", *(UINT32*)(xsdt + 24));
	CopyMem(tmpStr, xsdt + 28, 4);
	tmpStr[4] = 0;
	print("Creator ID: \"%s\"", tmpStr);
	print("Creator Revision: %08x", *(UINT32*)(xsdt + 32));
	for(i = 0; i < (len - 36) / 8; i++)
	{
	   print("[%d] %08x", i, ((UINTN*)(xsdt + 36))[i]);
	   *(UINT32*)tmpStr = *(UINT32*)((UINTN*)(xsdt + 36))[i];
	   tmpStr[4] = 0;
	   _print("    \"%s\"", tmpStr);
	   if(*(UINT32*)tmpStr == *(UINT32*)"FACP")
	   {
	     xsdt_fadt = (UINT8*)((UINTN*)(xsdt + 36))[i];
	   }

	}
xsdt_fin:
	print ("any key..");
	getch(0);


	if(rsdt_fadt == NULL)
				goto	fadt1_fin;
	CopyMem(tmpStr, rsdt_fadt, 4);
	tmpStr[4] = 0;
	print("\r\ntable FADT: (signature \"%s\")", tmpStr);
	len = *(UINT32*)(rsdt_fadt + 4);
	print("Length: %d bytes", len);
	print("FADT Major version: %d", rsdt_fadt[8]);
	CopyMem(tmpStr, rsdt_fadt + 10, 6);
	tmpStr[6] = 0;
	print("OEM ID: \"%s\"", tmpStr);
	CopyMem(tmpStr, rsdt_fadt + 16, 8);
	tmpStr[8] = 0;
	print("OEM table ID: \"%s\"", tmpStr);
	print("OEM Revision: %08x", *(UINT32*)(rsdt_fadt + 24));
	CopyMem(tmpStr, rsdt_fadt + 28, 4);
	tmpStr[4] = 0;
	print("Creator ID: \"%s\"", tmpStr);
	print("Creator Revision: %08x", *(UINT32*)(rsdt_fadt + 32));

	print("FACS address: %08x", *(UINT32*)(rsdt_fadt + 36));
	print("DSDT address: %08x", *(UINT32*)(rsdt_fadt + 40));
	rsdt_dsdt = (UINT8*)(UINTN)*(UINT32*)(rsdt_fadt + 40);

fadt1_fin:
	print ("any key..");
	getch(0);

	if(xsdt_fadt == NULL)
				goto	fadt2_fin;
	CopyMem(tmpStr, xsdt_fadt, 4);
	tmpStr[4] = 0;
	print("\r\ntable FADT: (signature \"%s\")", tmpStr);
	len = *(UINT32*)(xsdt_fadt + 4);
	print("Length: %d bytes", len);
	print("FADT Major version: %d", xsdt_fadt[8]);
	CopyMem(tmpStr, xsdt_fadt + 10, 6);
	tmpStr[6] = 0;
	print("OEM ID: \"%s\"", tmpStr);
	CopyMem(tmpStr, xsdt_fadt + 16, 8);
	tmpStr[8] = 0;
	print("OEM table ID: \"%s\"", tmpStr);
	print("OEM Revision: %08x", *(UINT32*)(xsdt_fadt + 24));
	CopyMem(tmpStr, xsdt_fadt + 28, 4);
	tmpStr[4] = 0;
	print("Creator ID: \"%s\"", tmpStr);
	print("Creator Revision: %08x", *(UINT32*)(xsdt_fadt + 32));

	print("FACS address: %08x", *(UINT32*)(xsdt_fadt + 36));
	print("DSDT address: %08x", *(UINT32*)(xsdt_fadt + 40));
	if(*(UINT32*)(xsdt_fadt + 40) != *(UINT32*)(rsdt_fadt + 40))
	{
	   xsdt_dsdt = (UINT8*)(UINTN)*(UINT32*)(xsdt_fadt + 40);
	}

fadt2_fin:
	print ("any key..");
	getch(0);

	if(rsdt_dsdt == NULL)
				goto	dsdt1_fin;
	CopyMem(tmpStr, rsdt_dsdt, 4);
	tmpStr[4] = 0;
	print("\r\ntable DSDT: (signature \"%s\")", tmpStr);
	len = *(UINT32*)(rsdt_dsdt + 4);
	print("Length: %d bytes", len);
	print("Revision: %d", rsdt_dsdt[8]);
	CopyMem(tmpStr, rsdt_dsdt + 10, 6);
	tmpStr[6] = 0;
	print("OEM ID: \"%s\"", tmpStr);
	CopyMem(tmpStr, rsdt_dsdt + 16, 8);
	tmpStr[8] = 0;
	print("OEM table ID: \"%s\"", tmpStr);
	print("OEM Revision: %08x", *(UINT32*)(rsdt_dsdt + 24));
	CopyMem(tmpStr, rsdt_dsdt + 28, 4);
	tmpStr[4] = 0;
	print("Creator ID: \"%s\"", tmpStr);
	print("Creator Revision: %08x", *(UINT32*)(rsdt_dsdt + 32));

dsdt1_fin:
	print ("any key..");
	getch(0);

	if(xsdt_dsdt == NULL)
				goto	dsdt2_fin;
	CopyMem(tmpStr, xsdt_dsdt, 4);
	tmpStr[4] = 0;
	print("\r\ntable DSDT: (signature \"%s\")", tmpStr);
	len = *(UINT32*)(xsdt_dsdt + 4);
	print("Length: %d bytes", len);
	print("Revision: %d", xsdt_dsdt[8]);
	CopyMem(tmpStr, xsdt_dsdt + 10, 6);
	tmpStr[6] = 0;
	print("OEM ID: \"%s\"", tmpStr);
	CopyMem(tmpStr, xsdt_dsdt + 16, 8);
	tmpStr[8] = 0;
	print("OEM table ID: \"%s\"", tmpStr);
	print("OEM Revision: %08x", *(UINT32*)(xsdt_dsdt + 24));
	CopyMem(tmpStr, xsdt_dsdt + 28, 4);
	tmpStr[4] = 0;
	print("Creator ID: \"%s\"", tmpStr);
	print("Creator Revision: %08x", *(UINT32*)(xsdt_dsdt + 32));

}
dsdt2_fin:
tab_fin:
	print("end of tables");
	getch(0);
	print(" ");

}

//=============================================================================================
#ifdef	GRAPHICS_MODE_INFO
	

void	dispGraphicModeInfo(void){
EFI_STATUS				Status;
EFI_GRAPHICS_OUTPUT_PROTOCOL		*gGop = NULL;
EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE	*gMode;
EFI_GRAPHICS_OUTPUT_MODE_INFORMATION	*gInfo;

EFI_UGA_DRAW_PROTOCOL			*UgaDraw;
EFI_EDID_DISCOVERED_PROTOCOL		*EdidDiscovered;
EFI_EDID_ACTIVE_PROTOCOL		*EdidActive;
EFI_EDID_OVERRIDE_PROTOCOL		*EdidOverride;
GOP_DISPLAY_BRIGHTNESS_PROTOCOL		*GopDisplayBrightness;

UINT8					*tmpBuf = NULL;

	// временный буфер под данные из протоколов:
	tmpBuf = AllocateZeroPool(0x1000);
	if(tmpBuf)
		SetMem(tmpBuf, 0x1000, 0x55);

	Status = gBS->LocateProtocol (
                    &gEfiGraphicsOutputProtocolGuid, 
		    NULL,
                    (VOID**)&gGop
                    );

	// GraphicsOutput:
	if (EFI_ERROR(Status)) {
		print ("Locate GraphicsOutputProtocol: Status = %x", Status);
		Status = gBS->LocateProtocol (
                    &gEfiUgaDrawProtocolGuid,
		    NULL,
                    (VOID **) &UgaDraw
                    );
		if (EFI_ERROR (Status)) {
			print ("Locate EfiUgaDrawProtocol: Status = %x", Status);
			goto	fin;
		}
		print ("Locate EfiUgaDrawProtocol: OK");

	} else {
		gMode = gGop->Mode;
		gInfo = gMode->Info;
		print("MaxMode = %d, currentMode = %d", gMode->MaxMode, gMode->Mode);
		print("BufferBase = %lx, BufferSize = %x", gMode->FrameBufferBase, gMode->FrameBufferSize);
		print("INFO: vers.%x, Hor = %d, Vert = %d, ", gInfo->Version, gInfo->HorizontalResolution, gInfo->VerticalResolution);
	}
	print ("any key..");
	getch(0);


	// EdidDiscovered:
	Status = gBS->LocateProtocol (
                    &gEfiEdidDiscoveredProtocolGuid, 
		    NULL,
                    (VOID**)&EdidDiscovered
                    );
	if (EFI_ERROR(Status)) {
		print ("Locate EfiEdidDiscoveredProtocol: Status = %x", Status);
	}
	else {
		print ("Locate EfiEdidDiscoveredProtocol: OK");
		print ("SizeOfEdid = %lx, Edid = %lx", EdidDiscovered->SizeOfEdid, EdidDiscovered->Edid);
		if(EdidDiscovered->SizeOfEdid)
		{ // содержимое Edid:
		  dumpBuf2(EdidDiscovered->Edid, 0x80);
		  if(tmpBuf)
			CopyMem(tmpBuf, EdidDiscovered->Edid, 0x80);
		}
		print ("--------------------------------");
	}

	// EdidActive:
	Status = gBS->LocateProtocol (
		    &gEfiEdidActiveProtocolGuid,
		    NULL,
                    (VOID**)&EdidActive
                    );
	if (EFI_ERROR(Status)) {
		print ("Locate EfiEdidActiveProtocol: Status = %x", Status);
	}
	else {
		print ("Locate EfiEdidActiveProtocol: OK");
		print ("SizeOfEdid = %lx, Edid = %lx", EdidActive->SizeOfEdid, EdidActive->Edid);
		if(EdidDiscovered->SizeOfEdid)
		{ // содержимое Edid:
		  dumpBuf2(EdidDiscovered->Edid, 0x80);
		  if(tmpBuf)
			CopyMem(tmpBuf + 0x80, EdidDiscovered->Edid, 0x80);
		}
		print ("--------------------------------");
	}

	// записываем данные Edid в файл:
	if(tmpBuf)
	{
    	  SaveToUsb(FILE_EDID, tmpBuf, 0x100);
	}

	// Edidoverride:
	Status = gBS->LocateProtocol (
		    &gEfiEdidOverrideProtocolGuid,
		    NULL,
                    (VOID**)&EdidOverride
                    );
	if (EFI_ERROR(Status)) {
		print ("Locate EfiEdidOverrideProtocol: Status = %x", Status);
	}
	else {
		print ("Locate EfiEdidOverrideProtocol: OK");
	}

	// GopDisplayBrightness:
	Status = gBS->LocateProtocol (
		    &gGopDisplayBrightnessProtocolGuid,
		    NULL,
                    (VOID**)&GopDisplayBrightness
                    );
	if (EFI_ERROR(Status)) {
		print ("Locate GopDisplayBrightnessProtocol: Status = %x", Status);
	}
	else {
		print ("Locate GopDisplayBrightnessProtocol: OK");
	}


fin:
	if(tmpBuf)
		FreePool(tmpBuf);
	print ("any key..");
	getch(0);

}

// проверка, какой из драйверов подключен к VGA-устройству
VOID	testDriverBinding(VOID){
EFI_STATUS                     	Status;
EFI_HANDLE			VgaDeviceHandle;
UINTN				DriverBindingHandleCount;
EFI_HANDLE     			*DriverBindingHandleBuffer;
EFI_DRIVER_BINDING_PROTOCOL	*DriverBinding;
UINTN				index;
EFI_COMPONENT_NAME2_PROTOCOL	*EfiComponentName2Protocol;
CHAR16				*DriverName;
EFI_LEGACY_BIOS_PLATFORM_PROTOCOL	*LegacyBiosPlatform;
UINTN				HandleCount = 0;
EFI_HANDLE			*HandleBuffer = NULL;

	Status = gBS->LocateProtocol(&gEfiLegacyBiosPlatformProtocolGuid, NULL, &LegacyBiosPlatform);
	if(EFI_ERROR(Status))
	{
	  Print(L"\r\n%a.%d: locate LegacyBiosPlatformProtocol error\n", __FUNCTION__, __LINE__);
	  goto fin;
	}

	// запрашиваем хендл на VGA:
	Status = LegacyBiosPlatform->GetPlatformHandle (
                                          LegacyBiosPlatform,
                                          EfiGetPlatformVgaHandle,
                                          0,
                                          &HandleBuffer,
                                          &HandleCount,
                                          NULL
                                          );
	if(EFI_ERROR(Status))
	{
	  Print(L"\r\n%a.%d: GetPlatformHandle error\n", __FUNCTION__, __LINE__);
	  VgaDeviceHandle = 0;
//	  goto fin;
	}
	else
	{
	  Print(L"\r\n%a.%d: GetPlatformHandle, count = %d \n", __FUNCTION__, __LINE__, HandleCount);

	  // здесь должен быть хендл на VGA:
	  VgaDeviceHandle = HandleBuffer[0];
	  Print(L"%a.%d: HandleBuffer[0] = %lx\n", __FUNCTION__, __LINE__, HandleBuffer[0]);
	}

	// запрашиваем все Binding-протоколы, зарегистрированные в ядре:
	Status = gBS->LocateHandleBuffer (
             ByProtocol,
             &gEfiDriverBindingProtocolGuid,
             NULL,
             &DriverBindingHandleCount,
             &DriverBindingHandleBuffer
             );
	Print(L"%a.%d: Locate EfiDriverBindingProtocol's, Count = %d, Status = %x\n", __FUNCTION__, __LINE__, DriverBindingHandleCount, Status);
	if(EFI_ERROR(Status))
			goto fin;

	// для каждого протокола запрашиваем информацию по драйверу:
	for(index = 0; index < DriverBindingHandleCount; index++)
	{
	  // получаем собственно хэндл протокола:
	  Status = gBS->HandleProtocol (
                  DriverBindingHandleBuffer[index],
                  &gEfiDriverBindingProtocolGuid,
                  (VOID **) &DriverBinding
                  );
	  if(EFI_ERROR(Status))
			continue;

	  // для протокола получаем имя:
	  Status = gBS->HandleProtocol (
                DriverBinding->DriverBindingHandle,
                &gEfiComponentName2ProtocolGuid,
                (VOID **) &EfiComponentName2Protocol
                );
          if (!EFI_ERROR (Status) && EfiComponentName2Protocol != NULL) {
	  // имя существует:
		EfiComponentName2Protocol->GetDriverName(
					EfiComponentName2Protocol,
					EfiComponentName2Protocol->SupportedLanguages,
					&DriverName);
		if(StrStr(DriverName, L"GOP") || StrStr(DriverName, L"BIOS[INT10]") || StrStr(DriverName, L"NVIDIA"))
		{ // нас интересуют только два драйвера:
		  Print (L"%a.%d: Driver \"%S\" found \n", __FUNCTION__, __LINE__, DriverName);
		  if(VgaDeviceHandle == 0)
					continue;

		  // пытаемся запустить BindingSupported на устройстве DeviceHandle и смотрим, что получится:
		  Status = DriverBinding->Supported(
                                  DriverBinding,
                                  VgaDeviceHandle,
                                  NULL
                                  );
		  Print (L"   for Controller = %lx, DriverBinding->Supported: status = %x\n", VgaDeviceHandle, Status);
		  if (Status == EFI_ACCESS_DENIED)
			Print(L" *** EFI_ACCESS_DENIED\n");
		  if (Status == EFI_ALREADY_STARTED)
			Print(L" *** EFI_ALREADY_STARTED\n");

		  if(!EFI_ERROR(Status))
		  {
		    // пытаемся запустить BindingStart на устройстве DeviceHandle и смотрим, что получится:
		    Status = DriverBinding->Start(
                                  DriverBinding,
                                  VgaDeviceHandle,
                                  NULL
                                  );
		    Print(L"   for Controller = %lx, DriverBinding->Start: status = %x\n", VgaDeviceHandle, Status);
		    if (Status == EFI_ACCESS_DENIED)
			Print(L" *** EFI_ACCESS_DENIED\n");
		    if (Status == EFI_ALREADY_STARTED)
			Print(L" *** EFI_ALREADY_STARTED\n");
		  }

		} // if(StrStr(DriverName, L"GOP"..
	  } // if (!EFI_ERROR (Status) && EfiComponentName2Protocol != NULL) {
	} // for


fin:
	Print(L"%a.%d: fin", __FUNCTION__, __LINE__);

}
#endif




//=============================================================================================

//
//	Отображение конфигурационного пространства PCI-устройств
//

// дамп 256 байт памяти (выравнивается на границу 16 байт)
void	memDump(UINT8 *memBase0){
int	i, j;
UINT8	*memBase = (UINT8*)((UINTN)memBase0 & ~0xf);
UINT8	tmp;
UINT8	tmpBuf[20];

	print(" %lx:", (UINTN)memBase); 
	for(i = 0; i < 256; i++)
	{
	  if((i & 0xf) == 0)
		print(" %02x:", i);
	  if((i & 0x3) == 0)
		  _print("  %02x", memBase[i]);
	  else
		  _print(" %02x", memBase[i]);
	  if((i & 0xf) == 0xf)
	  { // текст:
	    _print("  ");
	    for(j = 0; j < 16; j++)
	    {
	      tmp = memBase[(i & ~0xf) + j] ;
	      if((tmp > 0x20) && (tmp < 0x80))
	      {
	        tmpBuf[j] = tmp;
	      }
	      else
	      {
	        tmpBuf[j] = '.';
	      }
	    }
	    tmpBuf[16] = 0;
	    _print("%s", tmpBuf);
	  }
	}
}

int	pciBus = 0;
int	pciDevice = 0;
int	pciFunction = 0;

void	dumpPciConfig(void){
UINT32	pciBase;
int	*var;
int	tmp;
int	chr;

loop:
	print("\r\n B%d:D%d:F%d dump:", pciBus, pciDevice, pciFunction);
	pciBase = mPciBase + pciBus * 0x100000 + pciDevice * 0x8000 + pciFunction * 0x1000;
	memDump((UINT8*)(UINTN)pciBase);

loop_kb:
	print("\r\n <b> - bus, <d> - device, <f> - function, <q> - quit");
	chr = getch(0);

	if((chr & ~0x20) == 'Q')
				return;
	switch(chr & ~0x20){
	case 'B':
			var = &pciBus;
			print("Bus = ");
			break;
	case 'D':
			var = &pciDevice;
			print("Device = ");
			break;
	case 'F':
			var = &pciFunction;
			print("Function = ");
			break;
	default:
			var = NULL;
	}
	if(var == NULL)
			goto	loop;
	// первая цифра:
	chr = getch(0);
	if((chr < '0') || (chr > '9'))
				goto	loop_kb;
	_print("%c", chr);
	tmp = chr - '0';

	// вторая цифра:
	chr = getch(0);
	if(chr == 0xd)
			goto	done;
	if((chr < '0') || (chr > '9'))
				goto	loop_kb;
	_print("%c", chr);
	tmp *= 10;
	tmp += chr - '0';

done:
	if(var == &pciDevice)
	{
	  if(tmp > 31)
		goto	err_too_much;
	}
	if(var == &pciFunction)
	{
	  if(tmp > 7)
		goto	err_too_much;
	}
	*var = tmp;
	goto	loop;

err_too_much:
	print("value too much");
	goto	loop_kb;

}

//========================================================================================
// отображаем память  и порты:
//
enum {
DMODE_MEM = 0,
DMODE_IO,
};


void	ioDump(UINT32 ioBase0){
int	i;
UINT32	ioBase = ioBase0 & ~0xffff000f;
UINT8	tmp_buf[256];

	print(" %x:", (UINTN)ioBase); 
	for(i = 0; i < 64; i++)
	{
	  ((UINT32*)tmp_buf)[i] = inportd(ioBase + i * 4);
	}

	for(i = 0; i < 256; i++)
	{
	  if((i & 0xf) == 0)
		print(" %02x:", i);
	  if((i & 0x3) == 0)
		  _print("  %02x", tmp_buf[i]);
	  else
		  _print(" %02x", tmp_buf[i]);
	}
}

int	isHex(int chr){

	if((chr >= '0') && (chr <= '9'))
					return TRUE;
	chr &= ~0x20;
	if((chr >= 'A') && (chr <= 'F'))
					return TRUE;
	return FALSE;
}

int	isDec(int chr){

	if((chr >= '0') && (chr <= '9'))
					return TRUE;
	return FALSE;
}

int	asciiToHex(int chr){

	if((chr >= '0') && (chr <= '9'))
					return chr - '0';

	chr &= ~0x20;
	if((chr >= 'A') && (chr <= 'F'))
					return (chr - 'A' + 10);

	
	return -1;
}


#define	PCIMMIO_ADDR(Bus, Device, Function)	(mPciBase + Bus * 0x100000 + Device * 0x8000 + Function * 0x1000)

#define	SAVE_TO_FILE() \
	size = strlen(tmpBuf);\
	Status = LibFsWriteFile(File, &size, tmpBuf);\
	if(EFI_ERROR(Status))\
			goto	done;
//--------------------------------------------------------------------------------------



int	Gpio1PortAddress[] = {0x10, 0xd0, 0xd4};
#define	GPIO1_CONFIG_SIZE	(sizeof(Gpio1PortAddress) / sizeof(int))
#define	GPIO1_PAD_SIZE		(24 + 24)
#define	GPIO1_BASE		0xFDAF0000

int	Gpio2PortAddress[] = {0x10, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4};
#define	GPIO2_CONFIG_SIZE	(sizeof(Gpio2PortAddress) / sizeof(int))
#define	GPIO2_PAD_SIZE  	(24 + 24 + 13 + 24 + 24 + 24)
#define	GPIO2_BASE		0xFDAE0000

int	Gpio3PortAddress[] = {0x10, 0xd0};
#define	GPIO3_CONFIG_SIZE	(sizeof(Gpio3PortAddress) / sizeof(int))
#define	GPIO3_PAD_SIZE  	12
#define	GPIO3_BASE		0xFDAD0000

int	Gpio4PortAddress[] = {0x10, 0xd0};
#define	GPIO4_CONFIG_SIZE	(sizeof(Gpio4PortAddress) / sizeof(int))
#define	GPIO4_PAD_SIZE  	11
#define	GPIO4_BASE		0xFDAC0000


#define	PORT_ADDRESS(name)	name, sizeof(name)/sizeof(int)

typedef	struct {
int	*ports;		// массив конфигурационных регистров
int	portSize;	// длина массива
int	padSize;	// количество PAD-регистров
} GPIO_GRAB;

GPIO_GRAB	GpioGrab[] = {
				{PORT_ADDRESS(Gpio1PortAddress), GPIO1_PAD_SIZE},
				{PORT_ADDRESS(Gpio2PortAddress), GPIO2_PAD_SIZE},
				{PORT_ADDRESS(Gpio3PortAddress), GPIO3_PAD_SIZE},
				{PORT_ADDRESS(Gpio4PortAddress), GPIO4_PAD_SIZE},
				};

char	*configNames[] = {"GpioConfigData1",
			  "GpioConfigData2",
                          "GpioConfigData3",
                          "GpioConfigData4"};

char	*padNames[] = {"GpioPadData1",
		       "GpioPadData2",
		       "GpioPadData3",
		       "GpioPadData4"};

DWORD	baseReg[] = {GPIO1_BASE,
		     GPIO2_BASE,
		     GPIO3_BASE,
		     GPIO4_BASE};

//--------------------------------------------------------
// считывание состояния GPIO-регистров из Skylake:
EFI_STATUS readSkyLakeGpio(EFI_FILE_HANDLE File){

EFI_STATUS	Status;
int		group;
int		*tmpPorts;
DWORD		tmp_port;
DWORD		tmp_data;
int		i;
int		num_group;
char		tmpBuf[1024];
UINTN		size;
UINT32		PwrmBase;
UINT32		GpioCfg;

	if(pchNum != PCH10)
        {
          print(" is not SkyLake platform");
          return EFI_NOT_FOUND;
        }

	PwrmBase = *(UINT32*)(UINTN) (PCIMMIO_ADDR(0, 31, 2) + 0x48);	// R_PCH_PMC_PWRM_BASE
	GpioCfg = *(UINT32*)(UINTN)(PwrmBase + 0x120);			// R_PCH_PWRM_GPIO_CFG
        sprintf(tmpBuf, "\r\nUINT32 GpioCfg = 0x%08x;\r\n", GpioCfg);
        SAVE_TO_FILE();

	num_group = sizeof(GpioGrab) / sizeof(GPIO_GRAB);
	for(group = 0; group < num_group; group++)
        {
          // конфигурационные регистры:
          tmpPorts = GpioGrab[group].ports;
          sprintf(tmpBuf, "\r\nRAW_DATA %s[%d] = {",
          		configNames[group], GpioGrab[group].portSize);
          for(i = 0; i < GpioGrab[group].portSize; i++)
          {
            tmp_port = baseReg[group] + tmpPorts[i];
            tmp_data = *((DWORD*)tmp_port);
            sprintf(tmpBuf + strlen(tmpBuf), "\r\n\t\t{0x%08x, 0x%08x},", tmp_port, tmp_data);
          }
          sprintf(tmpBuf + strlen(tmpBuf), "\r\n\t\t};");
	  SAVE_TO_FILE();

          // регистры PAD'ов:
          sprintf(tmpBuf, "\r\nRAW_DATA %s[%d] = {",
          		padNames[group], GpioGrab[group].padSize * 2);
	  SAVE_TO_FILE();
          for(i = 0; i < GpioGrab[group].padSize * 2; i++)
          {
            tmp_port = baseReg[group] + 0x400 + i * 4;
            tmp_data = *((DWORD*)tmp_port);
            sprintf(tmpBuf, "\r\n\t\t{0x%08x, 0x%08x},", tmp_port, tmp_data);
	    SAVE_TO_FILE();
          }
          sprintf(tmpBuf, "\r\n\t\t};");
	  SAVE_TO_FILE();

        }

        sprintf(tmpBuf, "\r\nRAW_DATA_LIST GpioDataList[%d] = {", num_group * 2);
	for(group = 0; group < num_group; group++)
        {
          sprintf(tmpBuf + strlen(tmpBuf), "\r\n\t\t{%3d, %s},",
          		GpioGrab[group].portSize, configNames[group]);
          sprintf(tmpBuf + strlen(tmpBuf), "\r\n\t\t{%3d, %s},",
          		GpioGrab[group].padSize * 2, padNames[group]);
	}
        sprintf(tmpBuf + strlen(tmpBuf), "\r\n\r\n\t};");
	sprintf(tmpBuf + strlen(tmpBuf), "\r\n\r\nUINTN	GpioDataListSize = sizeof (GpioDataList) / sizeof (RAW_DATA_LIST);\r\n");
	SAVE_TO_FILE();

done:
	return Status;
}


void	  saveGpioPirq(){
EFI_STATUS	Status;
EFI_FILE_HANDLE File;
int		i;
char		tmpBuf[1024];
UINTN		size;
UINT32		pciBase;
UINT32		GpioBase;
UINT32		PirqRoute[2];
UINT8		tmp8;
UINT32		tmp32;

	// 1. создание файла:
  	sprintf(gFileOut, "%s%s", gFs, FILE_GPIO_PIRQ);  

	File = LibFsCreateFile(gFileOut);
	if (NULL == File) {
		print("***Error: can not create file %s", gFileOut);
		return;
	}

	print("\r\nCopy data to file %s...", gFileOut);

	// 2. записываем в файл:
/*
//===================================================================================
//  table PIRQx -> IRQy
//	PIRQ:	         A     B     C     D     E     F     G     H	
UINT8 PirqTable[8] = {   11, 0x80,    7,   15,    5,   10,    3,   14};		
*/
  	sprintf(tmpBuf, "%s", "\r\n//===================================================================================");  
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n//  table PIRQx -> IRQy");  
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n//	PIRQ:	         A     B     C     D     E     F     G     H");  
	SAVE_TO_FILE();

	pciBase = mPciBase + 31 * 0x8000 + 0 * 0x1000;		// d31:f0
	PirqRoute[0] = inmemd(pciBase + 0x60);
	PirqRoute[1] = inmemd(pciBase + 0x68);
  	sprintf(tmpBuf, "%s", "\r\nUINT8 PirqTable[8] = {");  
	for(i = 0; i < 8; i++)
	{
	  tmp8 = ((UINT8*)PirqRoute)[i];
	  if(tmp8 < 0x80)
  		sprintf(tmpBuf + strlen(tmpBuf), " %4d,", tmp8);  
	  else
  		sprintf(tmpBuf + strlen(tmpBuf), " 0x%02x,", tmp8);  
	}
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "};\r\n\r\n");  
	SAVE_TO_FILE();

/*
//===================================================================================
// for PlatfromInit PEI:
//
UINT32	IrqRouteTable[IRQ_ROUTE_SIZE] = {
  // RCBA + [3100...317c]
  0x03243200,  0x00000000,  0x00014321,  0x43214321,		// 3100
  0x00000001,  0x00004321,  0x00000001,  0x00000000,		// 3110
  0x00000000,  0x00002321,  0x00000001,  0x00000000,		// 3120
  0x00000000,  0x00000000,  0x00000000,  0x00000000,		// 3130

  0x00000230,  0x32102037,  0x00003216,  0x00003254,		// 3140
  0x00007654,  0x00003210,  0x00000000,  0x00001230,		// 3150
  0x00003215,  0x00000000,  0x00000000,  0x00000000,		// 3160
  0x00000000,  0x00000000,  0x00000000,  0x00000000,		// 3170
};
*/
  	sprintf(tmpBuf, "%s", "\r\n//===================================================================================");  
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n// for PlatfromInit PEI:");  
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n//\r\nUINT32	IrqRouteTable[IRQ_ROUTE_SIZE] = {");  
	SAVE_TO_FILE();

  	sprintf(tmpBuf, "%s", "\r\n  // RCBA + [3100...317c]");  
	for(i = 0; i < 32; i++)
	{
	  tmp32 = *(UINT32*)(0xfed1c000 + 0x3100 + i * 4);
	  if((i & 3) == 0)
		sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n  ");  
	  sprintf(tmpBuf + strlen(tmpBuf), "0x%08x, ", tmp32);  
	  if((i & 3) == 3)
		sprintf(tmpBuf + strlen(tmpBuf), "		// %04x", 0x3100 + (i & ~3) * 4);  
	}
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n};\r\n\r\n");  
	SAVE_TO_FILE();

/*
UINT32	PirqRoute[2] = {0x0f07800b,		// 11, --, 07, 15
			0x0e030a05		// 05, 10, 03, 0e
			};			// PIRQ[n]_ROUTs /d31:f0:60, d31:f0:68/
*/
  	sprintf(tmpBuf, "%s", "\r\nUINT32	PirqRoute[2] = {");  
	sprintf(tmpBuf + strlen(tmpBuf), "0x%08x,", PirqRoute[0]);  
	sprintf(tmpBuf + strlen(tmpBuf), "		// %d, ", PirqRoute[0] & 0xff);  
	sprintf(tmpBuf + strlen(tmpBuf), "%d, ", (PirqRoute[0] >> 8) & 0xff);  
	sprintf(tmpBuf + strlen(tmpBuf), "%d, ", (PirqRoute[0] >> 16)& 0xff);  
	sprintf(tmpBuf + strlen(tmpBuf), "%d ", (PirqRoute[0] >> 24)& 0xff);  
	SAVE_TO_FILE();

  	sprintf(tmpBuf, "%s", "\r\n			");  
	sprintf(tmpBuf + strlen(tmpBuf), "0x%08x,", PirqRoute[1]);  
	sprintf(tmpBuf + strlen(tmpBuf), "		// %d, ", PirqRoute[1] & 0xff);  
	sprintf(tmpBuf + strlen(tmpBuf), "%d, ", (PirqRoute[1] >> 8) & 0xff);  
	sprintf(tmpBuf + strlen(tmpBuf), "%d, ", (PirqRoute[1] >> 16)& 0xff);  
	sprintf(tmpBuf + strlen(tmpBuf), "%d ", (PirqRoute[1] >> 24)& 0xff);  
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n			};			// PIRQ[n]_ROUTs /d31:f0:60, d31:f0:68/\r\n\r\n");  
	SAVE_TO_FILE();

/*
//===================================================================================
// for PlatfromInit DXE:
//
*/
  	sprintf(tmpBuf, "%s", "\r\n//===================================================================================");  
  	sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n// for PlatfromInit DXE:\r\n//");  
	SAVE_TO_FILE();

	if(pchNum == PCH10)
        { // SkyLake:
/*
RAW_DATA GpioConfigData1[3] = {

		...

                {0xFDAC0454, 0x00000077},
                };
RAW_DATA_LIST GpioDataList[8] = {
                {  3, GpioConfigData1},
                { 96, GpioPadData1},
                {  7, GpioConfigData2},
                {266, GpioPadData2},
                {  2, GpioConfigData3},
                { 24, GpioPadData3},
                {  2, GpioConfigData4},
                { 22, GpioPadData4},
                };

*/
	  Status = readSkyLakeGpio(File);
        }
	else
	{ // Haswell
/*
UINT32 mGpioData[GPIO_DATA_SIZE] =  { 
  0xbffff1ff,
  0xbeffffff,
  0x00000000,
  0x8ed46ebe,
  ...
  };     
*/
	  GpioBase = inmemd(pciBase + 0x48) & ~0xffff0007;

  	  sprintf(tmpBuf, "%s", "\r\nUINT32 mGpioData[GPIO_DATA_SIZE] =  { ");  
	  for(i = 0; i < 32; i++)
	  {
	    tmp32 = inportd(GpioBase + i * 4);
	    sprintf(tmpBuf + strlen(tmpBuf), "\r\n  0x%08x,", tmp32);  
	  }
  	  sprintf(tmpBuf + strlen(tmpBuf), "%s", "\r\n};\r\n\r\n");  
	  SAVE_TO_FILE();
	}

	LibFsCloseFile(File);

done:
	if(!EFI_ERROR(Status))
	{
	  print("Save Data to file %s OK", gFileOut);
	}
	else
	{
	  print("Save ACPI tables to file %s: ERROR"
		"\r\n writed %lx bytes, status = %x", 
				gFileOut, size, Status);
	}

	getch(0);


}


// ввод шестнадцатеричного числа:
enum {
	DATA8 = 0,
	DATA16,
	DATA32
	};

int	getData(char *txt, int mode, UINT32 *addr){
UINT32	tmp;
int	chr;


	print(txt);
	tmp  = 0;
			
	// ввод цифр:
loop_kb_digit:
	chr = getch(0);
	if(chr == 0xd)
			goto done;
	if(!isHex(chr))
			return FALSE;
	_print("%c", chr);
	tmp <<= 4;
	tmp += asciiToHex(chr);
	goto	loop_kb_digit;

	// проверка корректности значения:
done:
	if((mode == DATA8) && (tmp >= 0x100))
	{
	    print("value too much for data8");
	    return FALSE;
	}
	if((mode == DATA16) && (tmp >= 0x10000))
	{
	    print("value too much for data16");
	    return FALSE;
	}
	*addr = tmp;
	return TRUE;
}

void	dumpSystem(void){

int	mode = DMODE_MEM;
char	*modeName[2] = {"memory", "io port"};
UINT32	modeMask[2] = {0xffffffff, 0xffff};
UINT32	memAddress = 0;
UINT32	ioAddress = 0;
UINT32	*addr = &memAddress;
UINT32	res;
int	chr;


	print("\r\n <f> - save GPIO and PIRQ to USB-file");

loop:
	if(mode == DMODE_MEM)
	{
	  addr = &memAddress;
	}
	else
	{
	  addr = &ioAddress;
	}
	print("\r\n mode %s, address = %x", modeName[mode], (*addr) & modeMask[mode]);
	if(mode == DMODE_MEM)
	{
	  memDump((UINT8*)memAddress);
	}
	else
	{
	  ioDump(ioAddress);
	}

loop_kb:
	print("\r\n <m> - change mode, <a> - address, <u> - up, <d> - down, <q> - quit");
	print(" <F> - save GPIO/PIRQ to file, <E> - enable P2sb for SkyLake");
	print(" <W> - write to register or memory");
	chr = getch(0);

	if((chr & ~0x20) == 'Q')
				return;
	if((chr & ~0x20) == 'F')
	{
	  if(gFlagUsb == FALSE)
	  {
  	    print("*** USB drive not found ***");
	    goto loop;
	  }

	  saveGpioPirq();
	  goto loop;
	}
	if((chr & ~0x20) == 'M')
	{
	  if(mode == DMODE_MEM)
	  {
	    mode = DMODE_IO;
	    goto loop;
	  }	
	  else		
	  {
	    mode = DMODE_MEM;
	    goto loop;
	  }	
	}
	if((chr & ~0x20) == 'U')
	{
	  *addr -= 0x100;
	  *addr &= modeMask[mode];
	  goto loop;
	}
	if((chr & ~0x20) == 'D')
	{
	  *addr += 0x100;
	  *addr &= modeMask[mode];
	  goto loop;
	}

	if(chr == 'E')
	{ // включение функции B0:D31:F1
	  *(UINT8*)0xE00F90E1 = 0;
	    goto loop;
	}

	if(chr == 'W')
	{ // запись по адресу:
UINT32	writeAddress;
UINT32	writeData;
	  if(mode == DMODE_IO)
		res = getData("Address:", DATA16, &writeAddress);
	  else
		res = getData("Address:", DATA32, &writeAddress);
	  if(!res)
		goto	loop_kb;

	  res = getData("Data:", DATA32, &writeData);
	  if(!res)
		goto	loop_kb;
	  
	  if(mode == DMODE_IO)
		outportd(writeAddress, writeData);
	  else
		*(UINT32*)(UINTN) writeAddress = writeData;
	  print("write %x to [%x] OK", writeData, writeAddress);

	  goto loop;
	}

	if((chr & ~0x20) != 'A')
			goto	loop;

	// задание нового адреса:
	if(mode == DMODE_IO)
		res = getData("Address:", DATA16, addr);
	else
		res = getData("Address:", DATA32, addr);
	if(res)
		goto	loop;
	goto	loop_kb;

}

void	dumpSio(void){
int	i;
int	mode = DMODE_WINBOND;
int	indexBaseAddress = 0;
char	*modeName[2] = {"Winbond", "ITE"};
UINT8	baseAdresses[] = {0x2e, 0x4e};
UINT32	baseAddress = baseAdresses[indexBaseAddress];
int	Device = 0;	// номер устройства внутри SIO
UINT8	*tmpBuf = (UINT8*)(((UINTN)(gBuf + 0xf)) & ~0xf);
UINT32	tmp;
UINT8	tmpByte;
int	chr;



loop:
	print("\r\n mode = %s, baseAddr = %x, Device = %d", modeName[mode], baseAddress, Device);
	SioEnterPnp(mode, baseAddress);
	SioWriteData(0x7, (UINT8)Device);
	for(i = 0; i < 256; i++)
	{
	  if((i == 0x87) || (i == 0xaa))
				continue;
	  outportb(sioIndexPort, i);
	  tmpByte = (UINT8)inportb(sioDataPort);
	  tmpBuf[i] = tmpByte;
	}
	SioExitPnp(mode);
	dumpBuf(tmpBuf);

loop_kb:
	print("\r\n <m> - change mode, <a> - base address, <u> - up, <d> - down, <D> - device, <q> - quit");
	print("\r\n <f> - save SIO dump to USB-file");
	chr = getch(0);

	if((chr & ~0x20) == 'Q')
				return;

	if((chr & ~0x20) == 'F')
	{
	  if(gFlagUsb == FALSE)
	  {
  	    print("*** USB drive not found ***");
	    goto loop;
	  }

//	  saveSioDump();
{
int	tmpDevice;
#define	MAX_DEVICE_NUM	0x20

	  SioEnterPnp(mode, baseAddress);
	  sprintf(tmpBuf, "\r\nSIO Dump  (type = \"%s\", BaseAddr = %x)", modeName[mode], baseAddress);

	  for(tmpDevice = 0; tmpDevice < MAX_DEVICE_NUM; tmpDevice++)
	  {
	    sprintf(tmpBuf + strlen(tmpBuf), "\r\nDevice 0x%x", tmpDevice);
	    SioWriteData(0x7, (UINT8)tmpDevice);
	    for(i = 0; i < 256; i++)
	    {
	      if((i & 0xf) == 0)
	      {
	        sprintf(tmpBuf + strlen(tmpBuf), "\r\n%02x: ", i);
	      }
	      if((i & 0xf) == 0)
	      {
	        sprintf(tmpBuf + strlen(tmpBuf), " ");
	      }
	      if((i == 0x87) || (i == 0xaa))
	      {
	        sprintf(tmpBuf + strlen(tmpBuf), " **");
		continue;
	      }
	      outportb(sioIndexPort, i);
	      tmpByte = (UINT8)inportb(sioDataPort);
	      sprintf(tmpBuf + strlen(tmpBuf), " %02x", tmpByte);
	    }
	  }
	  SioExitPnp(mode);
    	  SaveToUsb(FILE_SIO_DUMP, tmpBuf, (UINT32)strlen(tmpBuf));
}

	  goto loop;
	} // 'F'

	if((chr & ~0x20) == 'M')
	{
	  if(mode == DMODE_WINBOND)
	  {
	    mode = DMODE_ITE;
	    goto loop;
	  }	
	  else		
	  {
	    mode = DMODE_WINBOND;
	    goto loop;
	  }	
	}
	if((chr & ~0x20) == 'A')
	{
	  indexBaseAddress ^= 1;
	  baseAddress = baseAdresses[indexBaseAddress];
	}

	if((chr & ~0x20) == 'U')
	{
	  Device++;
	  goto loop;
	}
	if((chr == 'd') && (Device > 0))
	{
	  Device--;
	  goto loop;
	}

	if(chr != 'D')
			goto	loop;

	// ввод номера устройства:
	print("Device (hex) = ");
	tmp  = 0;
			
	// ввод цифр:
loop_kb_digit:
	chr = getch(0);
	if(chr == 0xd)
			goto done;
	if(!isHex(chr))
			goto	loop_kb;
	_print("%c", chr);
	tmp <<= 4;
	tmp += asciiToHex(chr);
	goto	loop_kb_digit;

	// проверка корректности значения:
done:
	if(tmp >= 0x20)
	{
	  goto	err_too_much;
	}
	Device = tmp;
	goto	loop;

err_too_much:
	print("value too much for Device number");
	goto	loop_kb;

}


//==========================================================================================
#ifdef GET_MTRR_INFO

UINT64	readMsr(UINT32 msr);

VOID	dumpMtrr(VOID){
UINT64	tmp64_1;
UINT64	tmp64;
UINT64	base;
UINT64	mask;
UINT64	highMask;
UINT64	range;
int	i;

	print("MTRR:");

	tmp64 = readMsr(0xfe);
	print("MTRR_CAP: msr[FE] = %lx.%08lx,  number of var mtrr = %d", tmp64 >> 32, tmp64, tmp64 & 0xff);

	tmp64 = readMsr(0x2ff);
	print("DEF_TYPE: msr[2FF] = %lx.%08lx", tmp64 >> 32, tmp64);
	if(tmp64 & (1 << 11))
		_print("   default type = %d", tmp64 & 7);

	for(i = 0; i < 8; i++)
	{
	   tmp64_1 = readMsr(0x200 + i * 2);
	   tmp64 = readMsr(0x200 + i * 2 + 1);

	   print("[%x]: base = %lx.%08lx", 0x200 + i * 2, tmp64_1 >> 32, tmp64_1);
	   _print(" mask = %lx.%08lx", tmp64 >> 32, tmp64);

	   if(tmp64 & 0x800)
	   { // mtrr включен:
	     base = tmp64_1 & ~0xFFF;
	     mask = tmp64 & ~0xFFF;
	     base = base & mask;
	     highMask = ((UINT64)1 << 63);
	     while(highMask > 1)
	     {
	       if(highMask & mask)
				break;
	       highMask >>= 1;
	     }
	     highMask <<= 1;
	     highMask -= 1;
	     range = base + (~mask & highMask);

	     _print(" / %lx.%08lx...%lx.%lx type = %d/", base >> 32, base, range >> 32, range, tmp64_1 & 7);
	   }


	}

	getch(0);

{
EFI_STATUS	Status;
EFI_FILE_HANDLE File;
char		tmpBuf[1024];
UINTN		size;

	// 1. создание файла:
  	sprintf(gFileOut, "%s%s", gFs, FILE_MTRR);  

	File = LibFsCreateFile(gFileOut);
	if (NULL == File) {
		print("***Error: can not create file %s", gFileOut);
		return;
	}

	tmp64 = readMsr(0xfe);
	sprintf(tmpBuf, "\r\nMTRR_CAP: msr[FE] = %lx.%08lx,  number of var mtrr = %d", tmp64 >> 32, tmp64, tmp64 & 0xff);
	tmp64 = readMsr(0x2ff);
	sprintf(tmpBuf + strlen(tmpBuf), "\r\nDEF_TYPE: msr[2FF] = %lx.%08lx", tmp64 >> 32, tmp64);
	if(tmp64 & (1 << 11))
		sprintf(tmpBuf + strlen(tmpBuf), "   default type = %d", tmp64 & 7);
	SAVE_TO_FILE();

	for(i = 0; i < 8; i++)
	{
	   tmp64_1 = readMsr(0x200 + i * 2);
	   tmp64 = readMsr(0x200 + i * 2 + 1);

	   sprintf(tmpBuf, "\r\n[%x]: base = %lx.%08lx", 0x200 + i * 2, tmp64_1 >> 32, tmp64_1);
	   sprintf(tmpBuf + strlen(tmpBuf), " mask = %lx.%08lx", tmp64 >> 32, tmp64);

	   if(tmp64 & 0x800)
	   { // mtrr включен:
	     base = tmp64_1 & ~0xFFF;
	     mask = tmp64 & ~0xFFF;
	     base = base & mask;
	     highMask = ((UINT64)1 << 63);
	     while(highMask > 1)
	     {
	       if(highMask & mask)
				break;
	       highMask >>= 1;
	     }
	     highMask <<= 1;
	     highMask -= 1;
	     range = base + (~mask & highMask);

	     sprintf(tmpBuf + strlen(tmpBuf)," / %lx.%08lx...%lx.%lx type = %d/", base >> 32, base, range >> 32, range, tmp64_1 & 7);

	   }
	   SAVE_TO_FILE();
	}
done:
	LibFsCloseFile(File);
}



}

#endif	/* GET_MTRR_INFO*/


//==========================================================================================

void	dispOpcodeMenu(void){
#define	OPCODE_MENU_SIZE	8
UINT8	tmpBuf[OPCODE_MENU_SIZE];
int	i;
int	res;

	res = readOpcodeMenu(tmpBuf, OPCODE_MENU_SIZE);
	if(res == FALSE)
	{ // не у всех есть
	  return;
	}
        print("\r\n OpcodeMenu =");
	for(i = 0; i < OPCODE_MENU_SIZE; i++)
	{
          _print(" %02x", tmpBuf[i]);
	}
}

void	dispRegionsAddr(void){
DWORD	regionAddresses[4];
DWORD	regionAddr;
DWORD	base;
DWORD	end;
int	res;

	res = readRegionsAddr(regionAddresses, 4);
	if(res == FALSE)
	{ // не получилось
	  return;
	}

        print(" RCBA = %lx", getRcba());
        print(" spiMemBase = %lx", getSpiBaseMem());

        print(" Regions:");
	regionAddr = regionAddresses[0];
        base = (regionAddr & 0xfff) << 12;			// начальный адрес региона внутри микросхемы FLASH (смещение от начала микросхемы)
	end = (((regionAddr >> 16) & 0xfff) + 1) << 12;
        print(" Descriptor: %06x...%06x", base, end);

	regionAddr = regionAddresses[1];
        base = (regionAddr & 0xfff) << 12;			
	end = (((regionAddr >> 16) & 0xfff) + 1) << 12;
        print(" Bios      : %06x...%06x", base, end);

	regionAddr = regionAddresses[2];
        base = (regionAddr & 0xfff) << 12;			
	end = (((regionAddr >> 16) & 0xfff) + 1) << 12;
        print(" ME        : %06x...%06x", base, end);

	regionAddr = regionAddresses[3];
        base = (regionAddr & 0xfff) << 12;			
	end = (((regionAddr >> 16) & 0xfff) + 1) << 12;
        print(" GbE       : %06x...%06x", base, end);

}

//=============================================================================================
/*
	Тест заполнения хранилища
	дает возможность записать произвольное количество 
	переменных размером около 1 кб

*/

typedef struct {
  ///
  /// Variable Data Start Flag.
  ///
  UINT16      StartId;
  ///
  /// Variable State defined above.
  ///
  UINT8       State;
  UINT8       Reserved;
  ///
  /// Attributes of variable defined in UEFI specification.
  ///
  UINT32      Attributes;
  ///
  /// Size of variable null-terminated Unicode string name.
  ///
  UINT32      NameSize;
  ///
  /// Size of the variable data without this header.
  ///
  UINT32      DataSize;
  ///
  /// A unique identifier for the vendor that produces and consumes this varaible.
  ///
  EFI_GUID    VendorGuid;
} VARIABLE_HEADER;

#pragma pack()
#ifdef TEST_STORAGE_MODE
#define	TEST_VAR_NAME	L"TestVarName_XXX"
#define	TEST_VAR_SIZE	950		// сейчас общий размер (включая служебныю информацию)  ограничен значением 1024
#define	STORAGE_BASE	0xFFCF0000
#define	STORAGE_SIZE	0x10000

UINT16	testVarData[TEST_VAR_SIZE / 2];
GUID	gTestVarGuid = {0xb5a83069, 0xff5d, 0x4fed, 0xb7, 0x92, 0xce, 0x89, 0x9d, 0x30, 0x0f, 0x48};
GUID	gHistoryAddRecordGuid  = {0xa3101f51, 0x295f, 0x461c, 0x8e, 0x14, 0x16, 0x30, 0x59, 0xd0, 0xd5, 0x31};

UINT32		StorageBase = STORAGE_BASE;

typedef	struct	{
void	*HistoryAddRecord;
void	*GetCurrentUserId;
} HISTORY_ADD_RECORD;

#define HEVENT_HISTORY_SEVERITY_LVL_CHANGE  0x005B
#define SEVERITY_LVL_ALERT                  (1 << 1)

#define HISTORY_RECORD_FLAG_CLEAN_EN        (1 << 0)
#define HISTORY_RECORD_FLAG_RESULT_OK       (1 << 1)
#define HISTORY_RECORD_FLAG_NO_REREAD       (1 << 2)

/*
EFI_STATUS
HistoryAddRecord(
  IN UINT16 EventCode,
  IN UINT8 UserId,
  IN UINT8 Severity,
  IN UINT8 Flags
  );
*/

// ввод цифр:
int	inDigit(int *res){
int	tmp = 0;
int	chr;
loop_kb:
	chr = getch(0);
	if(chr == 0xd)
			goto done;
	if(!isDec(chr))
	{
	  print("\r\naborted");
	  return FALSE;
	}
	_print("%c", chr);
	tmp *= 10;
	tmp += (chr - '0');
	goto	loop_kb;

done:
	*res = tmp;

	return TRUE;
}


VOID	testStorage(void){
EFI_STATUS	Status = EFI_SUCCESS;
UINTN		Size;
int		i;
int		chr;
int		Count;
UINT8		*storagePnt;
HISTORY_ADD_RECORD		*HistoryAddRecord = NULL;

	for(i = 1; i < TEST_VAR_SIZE / 2; i++)
				testVarData[i] = (UINT16)i;

	Status = gBS->LocateProtocol (
                  &gHistoryAddRecordGuid,
                  NULL,
                  (VOID **) &HistoryAddRecord
                  );
	print("Locate HistoryAddRecord, status = %x", Status);
entry:	
	print("'w' - write some variables to storage, 'l' - list variables, 'b' - set storage base");
	print("'c' - write cycle of variables, 'a' - add record to archive, 'q' - quit");
	chr = getch(0);
	if(chr == 'q')
		return;

	if(chr == 'w')
		goto write;

	if(chr == 'c')
		goto write_cycle;

	if(chr == 'l')
		goto list;

	if(chr == 'b')
		goto base;

	if(chr == 'a')
		goto add;

	goto	entry;

// вставить запись(и) в архив:
add:
	// ввод количества записей:
	print("Write some records to archive");
	print("Num of records = ");
			
	// ввод цифр:
	if(!inDigit(&Count))
			goto	entry;
	print("write %d records ..", Count);

	if(HistoryAddRecord)
	{
	  for(i = 0; i < Count; i++)
	  {
	    Status = ((EFI_STATUS(*)(
				IN UINT16 EventCode,
				IN UINT8 UserId,
				IN UINT8 Severity,
				IN UINT8 Flags
		)) (UINTN)HistoryAddRecord->HistoryAddRecord)(
					HEVENT_HISTORY_SEVERITY_LVL_CHANGE, 
					((UINT8 (*)(VOID)) (UINTN)HistoryAddRecord->GetCurrentUserId)(),
					SEVERITY_LVL_ALERT, 
					HISTORY_RECORD_FLAG_RESULT_OK | HISTORY_RECORD_FLAG_NO_REREAD);
	    if(EFI_ERROR(Status))
	    {
	      print("*** write record to archive failed, status = %x", Status);
	      print("*** num records: %d from %d", i, Count);
	      break;
	    }
	  } // for
	  if(!EFI_ERROR(Status))
		_print(" OK");
	}
	goto	entry;


//	задаем базовый адрес хранилища:
base:
	goto	entry;

//	выводим каталог хранилища:
list:
{
VARIABLE_HEADER	*varHeader;
UINT16	*name;
UINT32	size;

	storagePnt = (UINT8*)(UINTN)(StorageBase + 0x64);
	print("  id  state  size       name");

	while(1)
	{
	  varHeader = (VARIABLE_HEADER*)storagePnt;
	  if(varHeader->StartId != 0x55aa)
					break;
	  size = sizeof(VARIABLE_HEADER) + varHeader->NameSize + varHeader->DataSize;
	  size = (size + 3) & ~3;
	  name = (UINT16*)(storagePnt + sizeof(VARIABLE_HEADER));
	  print(" %04x ", varHeader->StartId);
	  _print("  %02x ", varHeader->State);
	  _print("   %04x ", varHeader->DataSize);
	  if(varHeader->State == 0x3f)
			_print(" %ls ", name);
	  else
			_print("    <%ls> ", name);

	  storagePnt += size;
	}
	print("  Storage Size = %04x", (UINT32)(UINTN)storagePnt - StorageBase);
	
}
	goto	entry;

// записываем тестовые переменные в хранилище:
write:
{
CHAR16	testVarName[] = TEST_VAR_NAME;
char	_testVarName[32];
int	j;

	// ввод количества переменных:
	print("Write some test variables (size of each = 1kb)");
	print("Num of variables = ");
	Count = 0;
			
	// ввод цифр:
	if(!inDigit(&Count))
			goto	entry;
	print("write %d kbytes ..", Count);

	// записываем переменные:
        testVarData[0]  ^= 0x55AA;		// если не менять данные, новой записи не будет
	for(i = 0; i < Count; i++)
	{ 
	  Size = TEST_VAR_SIZE;
	  _sprintf(_testVarName, "TestVarName_%03d", i);		// имя очередной переменной
	  for(j = 0; j < 32; j++)
	  {
	    testVarName[j] = _testVarName[j];
	    if(_testVarName[j] == 0)
				break;
	  }
	  Status = mRuntimeServices->SetVariable(testVarName, 
				    &gVendorGuid,
				    (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
				    Size, 
				    testVarData);
	  if(EFI_ERROR(Status))
	  {
	    print("*** SetVariable error %x", Status);
	    print("%d kb from %d kb has writed", i, Count);
	    break;
	  }
	}
	if(!EFI_ERROR(Status))
			_print(" OK");
}
	goto	entry;


// записываем тестовые переменные в хранилище:
write_cycle:
{
#define	COUNT		30
#define	NUM_CYCLES	45

CHAR16	testVarName[] = TEST_VAR_NAME;
char	_testVarName[32];
int	j;
int	cycles;
int	cyclesCount= NUM_CYCLES;
int	flagBreak = FALSE;

	// ввод количества переменных:
	print("Num of cycles = ");
			
	// ввод цифр:
	if(!inDigit(&cyclesCount))
			goto	entry;
	print("Write %d test variables (size of each = 1kb) %d cycles\r\n", COUNT, cyclesCount);

	// записываем переменные:
	for(cycles = 0; cycles < cyclesCount; cycles++)
	{
          testVarData[0]  ^= 0x55AA;		// если не менять данные, новой записи не будет
	  for(i = 0; i < COUNT; i++)
	  { 
	    Size = TEST_VAR_SIZE;
	    _sprintf(_testVarName, "TestVarName_%03d", i);		// имя очередной переменной
	    for(j = 0; j < 32; j++)
	    {
	      testVarName[j] = _testVarName[j];
	      if(_testVarName[j] == 0)
				break;
	    }
	    Status = mRuntimeServices->SetVariable(testVarName, 
				    &gVendorGuid,
				    (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
				    Size, 
				    testVarData);
	    if(EFI_ERROR(Status))
	    {
	      print("*** SetVariable error %x", Status);
	      print("%d kb from %d kb has writed, cycles = %d", i, COUNT, cycles);
	      flagBreak = TRUE;
	      break;
	    }
	  } // for(i)
	  if(!EFI_ERROR(Status))
			_print(" %d OK", cycles);
	  if(flagBreak)
		break;
	} // for(cycles)
}
	goto	entry;

}

#endif // TEST_STORAGE_MODE

void	  testStall(){
int	stallCnt = 0;
int	chr;
UINTN	i;

	print("test Stall start:");
	while(1)
	{
//	  gBS->Stall(1000000);
	  for(i = 0; i < 1000000; i++)
	  {
	    gBS->Stall(1);
	  }
	  stallCnt++;
	  print("%d", stallCnt);
	  chr = getch(1);
	  if(chr != 0)
			break;
	}
}

//=============================================================================
//
//
//
void	dispFlashParm(void){
UINT32	jedec_id = 0;
int	biosCntl;
int	biosCntlSpi;
char	*endis[] = {"disabled", "enabled"};

	// читаем список опкодов контроллера SPI:
	dispOpcodeMenu();

	// читаем размещение регионов:
	dispRegionsAddr();

	// читаем идентификатор микросхемы:
	if(readJedecId(&jedec_id))
	{
	  print ("JEDEC ID = %06x", jedec_id);
	}
	else
	{
	  print ("read JEDEC ID failed");
	}

	// флаги BIOS-FLASH:
	biosCntl = readPci0(31, 0, 0xdc);

	if(pchNum == PCH10)
	{
	  biosCntlSpi = readPci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BC);
	  print ("BIOS_CNTL: [d31:f0] = %02x,  [d31:f5] = %04x", biosCntl, biosCntlSpi);
	}	
	else
	{
	  print ("BIOS_CNTL = %02x", biosCntl);
	}
	print ("BIOS Write Enable: %d", biosCntl & 1);
	print ("BIOS Lock Enable: %d", (biosCntl >> 1) & 1);
	print ("BIOS region SMM protection: %s\n", endis[(biosCntl >> 5) & 1]);
}


//=============================================================================================

EFI_STATUS 
EFIAPI 
BiosWriterEntryPoint ( 
IN EFI_HANDLE ImageHandle, 
IN EFI_SYSTEM_TABLE *SystemTable ) {

EFI_STATUS	Status = EFI_SUCCESS;
EFI_FILE_HANDLE File;

UINT16	*CfgStr= L"fs2=\"PciRoot(0x0)/Pci(0x1A,0x0)\""		// EHCI-2
		 L"fs4=\"PciRoot(0x0)/Pci(0x1D,0x0)\""		// EHCI-1
		 L"fs8=\"PciRoot(0x0)/Pci(0x14,0x0)\""		// XHCI
		 ;

EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;

UINTN	BufferSize = 0;

int	chr;

BIOS_REGION	pchRegion = {0x180000L, 0x680000L};
BIOS_REGION	fileRegion;
UINTN		fileSize;

UINT32	pchFlashSize;			// размер всей микросхемы FLASH, включая другие регионы
UINT32	pchFlashBase;			// базовый адрес начала микросхемы FLASH в 32-битном адресном пространстве памяти PC

int	flagPchDescriptorError = FALSE;

	// настройка скорости порта:
	setUsart(115200);
	// разрешения на вывод для BIOS-BIOS (?)
	setFlagUprintfAll();
	setFlagUprintf();

	TRACE(0);

	gST = SystemTable;
	gBS = SystemTable->BootServices;
	mBootServices = SystemTable->BootServices;
	mRuntimeServices = SystemTable->RuntimeServices;

	uprintf("\r\n ConOut = %x", gST->ConOut);
	TRACE(1);

	print ("utilty starts"); 
	TRACE(2);


	Status = setPciBase();
	if(Status == FALSE)
	{
	   print ("setPciBase error, pchId = %08x", pchId); 
	   goto fin;
	}
	print ("mPciBase = %x", mPciBase); 

	if(pchNum < PCH_MOBILE)
	{
	  print ("Intel Pch serie: %d", pchNum);
	}
	else
	{
	  print ("Intel Pch serie: %d Mobile", pchNum & 0xf);
	}
	print ("LPC bridge ID = %08x", readPci0(31, 0, 0));
	print ("Gbe ID = %08x", readPci0(25, 0, 0));

        initSpi();	// инициализируем работу с SPI

	dispFlashParm();
/*
	// читаем список опкодов контроллера SPI:
	dispOpcodeMenu();

	// читаем размещение регионов:
	dispRegionsAddr();

{ // читаем идентификатор микросхемы:
UINT32	jedec_id = 0;

	if(readJedecId(&jedec_id))
	{
	  print ("JEDEC ID = %06x", jedec_id);
	}
	else
	{
	  print ("read JEDEC ID failed");
	}
}
*/
{ //	параметры региона BIOS для существующей прошивки:
DWORD	biosRegionAddr;
	// параметры региона из PCH:
	biosRegionAddr = readBiosRegionAddr();
        pchRegion.address = (biosRegionAddr & 0xfff) << 12;			// начальный адрес региона BIOS внутри микросхемы FLASH (смещение от начала микросхемы)
        

	pchFlashSize = (((biosRegionAddr >> 16) & 0xfff) + 1) << 12;
	pchFlashBase = (UINT32)(0x100000000 - (UINTN)pchFlashSize);
	print("pchFlashSize = %lx, pchFlashBase = %lx ", pchFlashSize, pchFlashBase);
	if((pchFlashSize != SIZE_8M) && (pchFlashSize != SIZE_16M))
	{
	   print("\r\n pchFlashSize illegal!");
	   flagPchDescriptorError = TRUE;
	}

        pchRegion.size = (((biosRegionAddr >> 16) & 0xfff) + 1) << 12;		
	pchRegion.size -= pchRegion.address;					// размер региона BIOS
}        
/*
{ //	флаги BIOS-FLASH:
int	biosCntl = readPci0(31, 0, 0xdc);
int	biosCntlSpi;
char	*endis[] = {"disabled", "enabled"};


	if(pchNum == PCH10)
	{
	  biosCntlSpi = readPci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BC);
	  print ("BIOS_CNTL: [d31:f0] = %02x,  [d31:f5] = %04x", biosCntl, biosCntlSpi);
	}	
	else
	{
	  print ("BIOS_CNTL = %02x", biosCntl);
	}
	print ("BIOS Write Enable: %d", biosCntl & 1);
	print ("BIOS Lock Enable: %d", (biosCntl >> 1) & 1);
	print ("BIOS region SMM protection: %s", endis[(biosCntl >> 5) & 1]);
}
*/
	TRACE(3);


//	информация о пути загруженного образа:
{
	Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &ImageInfo);
	if(!EFI_ERROR(Status))
	{
	  if(ImageInfo->FilePath)
	  {
UINT16	*pnt16;
int	i;
	    pnt16 = (UINT16*)ImageInfo->FilePath;
	    if(pnt16[0] != 0x404)	
	    {
		print("\r\n devicePath != 0x0404");
		goto fin;
	    }

	    uprintf("\r\n devicePath dump:");
	    i = 0;
	    while(1)
	    {
	      if(pnt16[i] == 0)
				break;
	      if((i & 7) == 0)
			uprintf("\r\n");
	      uprintf(" %04x", pnt16[i]);
	      i++;
	    }

	    uprintf("\r\n devicePath = %ls\r\n", ((UINT16*)ImageInfo->FilePath) + 1);
	    // преобразуем путь к файлу в UINT8:
	    strcpy(fileImg, FS2);		// имя носителя
	    str16to8(fileImg + strlen(FS2), pnt16 + 3);

	  }
	  else
	    {
		print("\r\n devicePath = NULL");
		goto fin;
	    }
	  } // else(ImageInfo->FilePath)
	else
	{
		print("\r\n ImageInfo not found");
		goto fin;
	} // else(!EFI_ERROR(Status))
	uprintf("\r\n");
}

	// инициализация библиотеки FsUtilsLib:
	// 1. место под параметры:
	if (AllocFsDescTable(10) == -1) {
		print("***Error: unable memory allocation");
		goto fin;
	}
	// 2. собственно инициализация:
	InitFsDescTable(CfgStr);

	// 3. выделение памяти:
	gBuf = AllocateZeroPool(SIZE_MAX);
	if(gBuf == NULL)
	{
		print("***Error: unable  allocate memory: %x bytes", SIZE_MAX);
		goto fin;
	}

	// посмотрим существующие пути:
	ShowAllDevicesPath();
//	ShowAllMyDevicesPath();

	// находим активный носитель:
{
int	i = 0;
	// перебираем предполагаемые пути к активному носителю:
	while(1)
	{
	  if(fDev[i] == NULL)
	  {
		print("***Error: active drive is not found");
		getch(0);
		goto usb_path_fin;
	  }
	  strcpy(gFs, fDev[i]);
	  memcpy(fileImg, fDev[i], strlen(fDev[i]));
	  File = LibFsOpenFile(fileImg, EFI_FILE_MODE_READ, 0);
	  if(File)
	  { // нашли:
	     break;
	  }
			
	  i++;
	}

	LibFsCloseFile(File);
	print("active fs = %s", gFs);
	sprintf(fileIn, "%s%s", gFs, FILE_IN);  
	sprintf(fileOut, "%s%s", gFs, FILE_OUT);  
	gFlagUsb = TRUE;


usb_path_fin:
	uprintf("\r\n");
}
#ifdef GET_MTRR_INFO
	// читаем MTRR:
	dumpMtrr();
#endif	/*GET_MTRR_INFO*/

menu:
	if(gFlagUsb)
	{
	  print("'r' - read current BIOS");
	  print("'w' - rewrite BIOS with file %s", fileIn);
	}
#ifdef TEST_MODE
	print("'t' - test");
#endif // TEST_MODE
#ifdef GRAPHICS_MODE_INFO
	print("'g' - display graphics mode info");
#endif // GRAPHICS_MODE_INFO
	
	print("'p' - display PCI configuration memory");
	print("'s' - display memory and io ports");
	print("'o' - display SIO registers");
	print("'i' - display configuration table info");
	print("'d' - display all devices path");
	print("'f' - display BIOS-FLASH parameters");
	print("'U' - set/clear mode Uprintf");
	print("'L' - test Stall");
#ifdef TEST_STORAGE_MODE
	print("'V' - test variables");
#endif
	print("'x' - exit");

loop_kb:
	chr = getch(0);

	if(((chr & ~0x20) == 'R') && gFlagUsb)

			goto	read;
#ifdef TEST_MODE
	if((chr & ~0x20) == 'T')
	{
		Test((UINT8*)(UINTN)pchFlashBase);
		goto	menu;
	}
#endif // TEST_MODE

	if((chr & ~0x20) == 'I')
	{
		dispTableInfo();
		goto	menu;
	}

	if((chr & ~0x20) == 'F')
	{
		dispFlashParm();
		goto	menu;
	}

	if(chr == 'U')
	{
	  if(gFlagUprintf)
	  {
	    gFlagUprintf = FALSE;
	    print("Uprintf off");
	  }
	  else
	  {
	    gFlagUprintf = TRUE;
	    print("Uprintf on");
	  }
	  goto	menu;
	}

	if(chr == 'L')
	{
	  testStall();
	  goto menu;
	}


#ifdef TEST_STORAGE_MODE
	if(chr == 'V')
	{
	  testStorage();
	  goto	menu;
	}
#endif // TEST_STORAGE_MODE


#ifdef GRAPHICS_MODE_INFO
	if((chr & ~0x20) == 'G')
	{

		dispGraphicModeInfo();
		testDriverBinding();
		goto	menu;
	}
#endif // GRAPHICS_MODE_INFO

	if((chr & ~0x20) == 'P')
	{
		dumpPciConfig();
		goto	menu;
	}

	if((chr & ~0x20) == 'S')
	{
		dumpSystem();
		goto	menu;
	}

	if((chr & ~0x20) == 'O')
	{
		dumpSio();
		goto	menu;
	}

	if((chr & ~0x20) == 'D')
	{
		ShowAllMyDevicesPath();
		goto	menu;
	}


	if(((chr & ~0x20) == 'W') && gFlagUsb)
			goto	write;
	if((chr & ~0x20) == 'X')
			goto	fin2;

	_print(" <%02x>", chr);
	goto	loop_kb;

//	---------------------------------------------------------------------
//
//				ЧТЕНИЕ
//
// считывание текущей прошивки и запись в файл BIOS\BiosDump.bin:
read:
{
UINT8	*tmpBuf = gBuf;	
UINT32	tmpBiosSize = pchFlashSize;	

	if(pchFlashSize > SIZE_MAX)
	{
		print("***Error: pchFlashSize too much");
		goto fin;
	}

	// 1. создание файла:
	File = LibFsCreateFile(fileOut);
	if (NULL == File) {
		print("***Error: can not create file %s", fileOut);
		goto fin;
	}

	print("\r\nRead current BIOS to file %s...", fileOut);

	CopyMem(tmpBuf, (UINT8*)(UINTN)pchFlashBase, pchFlashSize);

	// запись в файл по фрагментам размером BUFFER_SIZE
	while(tmpBiosSize)
	{
	  BufferSize = BUFFER_SIZE;	
	  Status = LibFsWriteFile(File, &BufferSize, tmpBuf);
	  if(EFI_ERROR(Status))
				break;
	  tmpBiosSize -= BUFFER_SIZE;
	  tmpBuf += BUFFER_SIZE;
	  Print(L"\r\nRemain %d bytes...", tmpBiosSize);
	}

	LibFsCloseFile(File);


	if(!EFI_ERROR(Status))
	{
	  print("Save current BIOS in file %s, size = %lx, - OK", fileOut, pchFlashSize);
	}
	else
	{
	  print("Save current Bios in file %s: ERROR"
		"\r\n writed %lx bytes, status = %x", 
				fileOut, pchFlashSize - tmpBiosSize, Status);
	}

	goto	menu;
}

//	---------------------------------------------------------------------
//
//				ЗАПИСЬ
//
write:
	print("Rewrite current firmware with file %s? <y>", fileIn);

	chr = getch(0);
	if((chr & ~0x20) != 'Y')
			goto	next_write;
  
  
	// открываем файл:
	File = LibFsOpenFile(fileIn, EFI_FILE_MODE_READ, 0);
	if (NULL == File) {
		print("***Error: file %s not found", fileIn);
		goto	fin;
	}

	// запись в BIOS-FLASH:
	if(flagPchDescriptorError)
	{
          print("BIOS region size"    	" illegal");
          goto fin;
	  
	}
	// параметры региона из файла:
{
EFI_STATUS	status;
DWORD		fileRegionAddr;
UINTN		ReadSize = 4;

	LibFsSetPosition(File,  BIOS_REGION_OFFSET);
	status = LibFsReadFile(File, &ReadSize, &fileRegionAddr);
	if(EFI_ERROR(Status))
	{
	  print("***Error: read file %s error", fileIn);
	  goto	fin;
	}

        fileRegion.address = (fileRegionAddr & 0xfff) << 12;
        fileRegion.size = (((fileRegionAddr >> 16) & 0xfff) + 1) << 12;
	fileRegion.size -= fileRegion.address;

}

/*Отключение проверки в специфичном для платформы(GBYTEH77) случае, когда
в дескрипторе SPI, BIOS Region пересекается с другими регионами*/
#ifndef GBYTEH77

        if(fileRegion.address != pchRegion.address)
        { // не совпал адрес региона:
          print("BIOS region addresses"
          	" in the file and in the chipset are not identical");
          print("fileRegion.address = %lx", fileRegion.address);
          print("pchRegion.address = %lx", pchRegion.address);
          goto fin;
        }

        if(fileRegion.size != pchRegion.size)
        { // не совпал размер региона:
          print("BIOS region sizes"
          	" in the file and in the chipset are not identical");
          print("fileRegion.size = %lx", fileRegion.size);
          print("pchRegion.size = %lx", pchRegion.size);
          goto fin;
        }

#endif//!GBYTEH77

	// размер файла:
	fileSize = LibFsSizeFile(File);
	if(fileSize != pchFlashSize)
	{
          print("size of the file"
          	" is not correspond to pch Flash size");
          print("pch Flash size = %lx", pchFlashSize);
          print("file size = %lx", fileSize);
          goto fin;

	}


        if((fileRegion.address + fileRegion.size) != fileSize)
        { // размер файла не соответствует размеру региона:
          print("size of the file"
          	" is not correspond to region size");
          print("region max address = %lx",
          		fileRegion.address + fileRegion.size - 1);
          print("file max address = %lx", fileSize - 1);
          goto fin;
        }

#ifndef GIGABYTE_H77
        print("\r\n it is about to write %lx bytes from file %s to flash begin at %lx",
        		fileRegion.size, fileIn, fileRegion.address);
	print("to start rewrite push 'W' (not 'w')", fileIn);

	chr = getch(0);
	if(chr != 'W')
			goto	next_write;
#endif

{
UINTN		ReadSize;
EFI_STATUS	status;

	// считываем файл в буфер целиком:  
	LibFsSetPosition(File, 0);		// в начало файла
	ReadSize = pchFlashSize;
	status = LibFsReadFile(File, &ReadSize, gBuf);
	if(EFI_ERROR(status))
	  {
	    print("****Error: read input file error");
	    goto	fin;
	  }
	LibFsCloseFile(File);
}

 	if(writeSpiFlash(gBuf, pchRegion.address, pchRegion.size, pchFlashBase) == FALSE)
        {
          print("****Error: writeSpiFlash error");
	  goto	fin;
        }

 	if(compareSpiFlash(gBuf, pchRegion.address, pchFlashBase, pchFlashSize) == FALSE)
        {
	  goto	fin;
        }



#ifdef GIGABYTE_H77

	setCS1();	// вторая микросхема
	// заполняем буфер кодом 0xff, вторая прошивка работать не будет - главное, чтоб не мешала.
{
UINT32	i;
	for(i = pchRegion.address; i < pchFlashSize; i++)
	{
	   gBuf[i] = 0xff;
	}
}	
	// стираем вторую прошивку:
 	if(writeSpiFlash(gBuf, pchRegion.address, pchRegion.size, pchFlashBase) == FALSE)
        {
          print("****Error: writeSpiFlash error");
	  setCS0();	// основная микросхема
	  goto	fin;
        }
	// проверяем:
 	if(compareSpiFlash(gBuf, pchRegion.address, pchFlashBase, pchFlashSize) == FALSE)
        {
	  setCS0();	// основная микросхема
	  goto	fin;
        }

	setCS0();	// основная микросхема

#endif // GIGABYTE_H77

	// рестарт:
	print("Rewrite BIOS with file %s Done", fileIn);
	print("to restart PC push any button");

	getch(0);
{
UINT32	PmBase;

	if(pchNum == PCH10)
		PmBase = readPci0(PCI_DEVICE_NUMBER_PCH_PMC, PCI_FUNCTION_NUMBER_PCH_PMC, R_PCH_PMC_ACPI_BASE) & ~0xf;		// Skylake
	else
		PmBase = readPci0(31, 0, 0x40) & ~0xf;

	// блокируем SMI:
	print("\r\n SMI_EN = %08x", inportd((UINTN) (PmBase + R_PCH_SMI_EN)));
	outportd((UINTN) (PmBase + R_PCH_SMI_EN), 0);	
	print("\r\n SMI_EN = %08x", inportd((UINTN) (PmBase + R_PCH_SMI_EN)));

	outport(PmBase, 0x0900);		// clear PWRBTN__STS and PWRBTNOR_STS
        outportd(PmBase + 4, 0x3c00);
        while(1)
			continue;
}

next_write:
	print("Done");
	

//	---------------------------------------------------------------------
//
//				ВЫХОД
//

fin:
	getch(1000000);
fin2:


	if (gBuf != NULL) {
		FreePool(gBuf);
	}

	return EFI_SUCCESS;
}


//=======================================================================
// зачем-то нужно для LibC:	

int	main(void){
	return 0;

}


//========================================================================
//
//		from MdePkg\Library\UefiApplicationEntryPoint:
//
EFI_STATUS
EFIAPI
_ModuleEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                 Status;
  Status = BiosWriterEntryPoint ( ImageHandle, SystemTable );
  return Status;
}


