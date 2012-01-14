/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>
#include "spi_flash.h"
#include "BiosWriter.h"
#include <Library/FsUtils.h>
#include <Register/PchRegsSpi.h>
#include <Register/PchRegsPmc.h>
#include <Protocol/Spi.h>

typedef UINT8	BYTE;
typedef UINT16	WORD;
typedef UINT32	DWORD;

void	wait_mcs(int delay){

	Stall(delay);
}


BYTE	inmemb(DWORD addr){
BYTE volatile 	res;

	res = *(BYTE*)(UINTN)(addr);	
	return res;
}


#define	outmemb(addr, data)	*(BYTE*)(UINTN)(addr) = (BYTE)(data)

WORD	inmem(DWORD addr){
WORD volatile 	res;

	res = *(WORD*)(UINTN)(addr);	
	return res;
}
#define	outmem(addr, data)	*(WORD*)(UINTN)(addr) = (WORD)(data)

DWORD	inmemd(DWORD addr){
DWORD volatile 	res;

	res = *(DWORD*)(UINTN)(addr);	
	return res;
}
#define	outmemd(addr, data)	*(DWORD*)(UINTN)(addr) = (DWORD)(data)
//----------------------------------------------------------------------


DWORD	mPciBase = 0xf8000000L;
int	pchNum;
int	pchId;


#define	PCI_ADDR	0xcf8
#define	PCI_DATA	0xcfc

//------------------------------------------------------------------------------
// считываем идентификатор чипсета (D31:f0.DID):
//
//	
//
int	readDID(void){
int	address;
DWORD	id;

	address = (0 << 16)		// bus
        	  | (31 << 11)		// device
		  | (0 << 8);		// func
        address |= 0x80000000L;

	outportd(PCI_ADDR, address);
        id = inportd(PCI_DATA);
	switch(id >> 24)
	{
	case 0x1c:
		pchNum = PCH6;
		break;
	case 0x1e:
		pchNum = PCH7;
		break;
	case 0x3b:
		pchNum = PCH5;
		break;
	case 0x8c:
		pchNum = PCH8;
		break;
	case 0xa1:
		pchNum = PCH10;
		break;
	case 0x9c:
		pchNum = PCH8M;
		break;
	default:
		pchNum = PCH_UDF;

	} // switch

	pchId = id;

	if(pchNum == PCH_UDF)
				return FALSE;
	return TRUE;

}

//------------------------------------------------------------------------------
// считываем базовый адрес отображения конфигурационного пространства PCI в память:
//
//	
//
int	setPciBase(void){
int	address;

	if(readDID() == FALSE)
			return FALSE;

	if(pchNum == PCH5)
	{ // пока так, непонятно, где искать:
	  mPciBase = 0xe0000000;
	  return TRUE;
	}

	address = (0 << 16)		// bus
        	  | (0 << 11)		// device
		  | (0 << 8);		// func
        address |= 0x80000000L;
	address += 0x60;		// смещение PCIMEM_BASE

	outportd(PCI_ADDR, address);
        mPciBase = inportd(PCI_DATA) & ~0xfff;

	return TRUE;
}


DWORD	readPci0(int dev, int func, int offset){

	return inmemd(mPciBase
        		+ ((DWORD)dev << 15)
                        + ((DWORD)func << 12)
                        + offset);
}

void	writePci0(int dev, int func, int offset, DWORD data){
DWORD	addr;

	addr = mPciBase  + ((DWORD)dev << 15)
                        + ((DWORD)func << 12)
                        + offset;
	outmemd(addr, data);
}

DWORD	getRcba(void){
	return (readPci0(31, 0, 0xf0) & ~0x3fff);
}

EFI_STATUS
EFIAPI
PchAcpiBaseGet  (
  OUT UINT16                            *Address
  ){
  if(pchNum == PCH10)
	*Address = (UINT16)(readPci0(PCI_DEVICE_NUMBER_PCH_PMC, PCI_FUNCTION_NUMBER_PCH_PMC, R_PCH_PMC_ACPI_BASE) & ~0xf);
  else
	*Address = (UINT16)(readPci0(31, 0, 0x40) & ~0xf);

  return EFI_SUCCESS;
}


//------------------------------------------------------------------------------
//  запись в BIOS-FLASH через SPI-контроллер чипсета:
//

//------------------------------------------------------------------------------
//
//			Skylake
//
typedef struct {
  UINT32                Signature;
  EFI_HANDLE            Handle;
  PCH_SPI_PROTOCOL      SpiProtocol;
  UINT16                PchAcpiBase;
  UINTN                 PchSpiBase;
  UINT8                 ReadPermission;
  UINT8                 WritePermission;
  UINT32                SfdpVscc0Value;
  UINT32                SfdpVscc1Value;
  UINT16                PchStrapBaseAddr;
  UINT16                PchStrapSize;
  UINT16                CpuStrapBaseAddr;
  UINT16                CpuStrapSize;
  UINT8                 NumberOfComponents;
  UINT32                Component1StartAddr;
  UINT32                TotalFlashSize;
} SPI_INSTANCE;

EFI_STATUS
SpiProtocolConstructor (
  IN     SPI_INSTANCE       *SpiInstance
  );

///
/// Global variables
///
SPI_INSTANCE                  *mSpiInstance;
PCH_SPI_PROTOCOL	      *mSpiProtocol;	
///
/// PchSpiBar0PhysicalAddr keeps the reserved MMIO range assiged to SPI from PEI.
/// It won't be updated no matter the SPI MMIO is reallocated by BIOS PCI enum.
/// And it's used to override the SPI BAR0 register in runtime environment,
///
UINT32                        mPchSpiBar0PhysicalAddr;


UINTN
AcquireSpiBar0 (
  IN  SPI_INSTANCE                *SpiInstance
  )
{
  return getSpiBaseMem();
}

VOID
ReleaseSpiBar0 (
  IN     SPI_INSTANCE       *SpiInstance
  )
{
}
/**
  This function is a hook for Spi to disable BIOS Write Protect

  @retval EFI_SUCCESS             The protocol instance was properly initialized
  @retval EFI_ACCESS_DENIED       The BIOS Region can only be updated in SMM phase

**/
EFI_STATUS
EFIAPI
DisableBiosWriteProtect (
  VOID
  )
{
  if(readPci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BC) & B_PCH_SPI_BC_EISS)
			return EFI_ACCESS_DENIED;
  ///
  /// Enable the access to the BIOS space for both read and write cycles
  ///
  writePci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BC, 
			readPci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BC) | B_PCH_SPI_BC_WPD);
  return EFI_SUCCESS;
}

/**
  This function is a hook for Spi to enable BIOS Write Protect


**/
VOID
EFIAPI
EnableBiosWriteProtect (
  VOID
  )
{
  ///
  /// Disable the access to the BIOS space for write cycles
  ///
  writePci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BC, 
			readPci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BC) & ~B_PCH_SPI_BC_WPD);
}

UINTN
MmPciBase (
  IN UINT32                       Bus,
  IN UINT32                       Device,
  IN UINT32                       Function
  )
{
//  ASSERT ((Bus <= 0xFF) && (Device <= 0x1F) && (Function <= 0x7));

  return ((UINTN)mPciBase + (UINTN) (Bus << 20) + (UINTN) (Device << 15) + (UINTN) (Function << 12));
}


int	SkylakeSpiInit(void){
EFI_STATUS	Status;

  mSpiInstance = AllocateRuntimeZeroPool (sizeof (SPI_INSTANCE));
  if (mSpiInstance == NULL) {
    uprintf("\r\n%s.%d: AllocateRuntimeZeroPool error\n", __FUNCTION__, __LINE__);
    return FALSE;
  }

  ///
  /// Initialize the SPI protocol instance
  ///
  Status = SpiProtocolConstructor (mSpiInstance);
  if (EFI_ERROR (Status)) {
    uprintf("\r\n%s.%d: SpiProtocolConstructor error\n", __FUNCTION__, __LINE__);
    return FALSE;
  }

  mSpiProtocol = &mSpiInstance->SpiProtocol;

  return TRUE;
}

//
//			end of Skylake
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// базовый адрес контроллера SPI:
UINT32	mSpiBaseMem = 0;

DWORD	getSpiBaseMem(void){
DWORD	baseMem;

	if(pchNum == PCH10)
	{
	  baseMem = readPci0(PCI_DEVICE_NUMBER_PCH_SPI, PCI_FUNCTION_NUMBER_PCH_SPI, R_PCH_SPI_BAR0) & ~0xf;
	}
	else
	{
	  baseMem = getRcba() + 0x3800;
	}

	return baseMem;
}

// инициализация библиотек:
void	initSpi(void){
EFI_STATUS	Status;

  mSpiBaseMem = getSpiBaseMem();
  uprintf("\r\nmSpiBaseMem = %x\n", mSpiBaseMem);

  if(pchNum == PCH10)
  {
    Status = SkylakeSpiInit();
    if(EFI_ERROR(Status))
		uprintf("\r\nSkylakeSpiInit error\n");
  }
}	




//------------------------------------------------------------------------------
// считываем адрес региона БИОС:
DWORD	readBiosRegionAddr(void){

	return        inmemd(mSpiBaseMem + FREG1);
}

//------------------------------------------------------------------------------
// ждем окончания выполнения SPI-цикла
//
int	IsCycleDone(void){
UINT8	tmp;

	tmp = inmemb(mSpiBaseMem + SSFS_OFFSET);
	if(tmp & SSFS_SCIP)
		return FALSE;
	if(tmp & SSFS_DONE)	
			return TRUE;
	return FALSE;
}


//------------------------------------------------------------------------------
// читаем регистр статуса микросхемы SPI-FLASH (не контроллера)
int	readFlashStatus(void){
int	i;
int	tmp;
int	status0;
DWORD	tmp_timer0;

int	index_read_status0 = -1;

	if(!mSpiBaseMem)
	{
         uprintf("\r\n mSpiBaseMem not set");
	  return -1;
	}

	if(pchNum == PCH10)
	{
EFI_STATUS	Status;
	  status0 = 0;
	  Status = mSpiProtocol->FlashReadStatus(mSpiProtocol, 1, (UINT8*)&status0);
	  if(EFI_ERROR(Status))
	  {
            uprintf("\r\n%s.%d: for Skylake not supported yet", __FUNCTION__, __LINE__);
	    return -1;
	  }
	  return status0;
	}

	// 1. ищем коды в OpcodeMenu:
        for(i = 0; i < 8; i++)
          {
            if( inmemb(mSpiBaseMem + OPMENU_OFFSET + i) == SPI_READ_STATUS)
              {
                index_read_status0 = i;	// READ_STATUS
              }
          }

        if(index_read_status0 < 0)
          {
            uprintf("\r\n code SPI_READ_STATUS0 not found");
            goto	err;
          }


	// проверяем готовность:
        if( (tmp = inmemb(mSpiBaseMem + SSFS_OFFSET)) &  SSFS_SCIP)
          {
            uprintf("\r\n SPI not ready before read status:"
            				" status = %02x", tmp);
            goto	err;
          }

        // запись в командный регистр
        // по адресу SSFS, т.е. одновременно
        // в командный регистр и регистр статуса:
        // 0xfc403004
        outmemd(mSpiBaseMem + SSFS_OFFSET,
            	(0x1fL << 27)   		// reserved
        	+ (1L << 22) 			// наличие данных
                + (0L << 16)			// (количество байт - 1)
                + ((DWORD)index_read_status0 << 12)   	// индекс команды
                + (1L << 3)			// сбросить ошибку
                + (1L << 2));			// сбросить Complete

        outmemd(mSpiBaseMem + SSFS_OFFSET,
        	inmemd(mSpiBaseMem + SSFS_OFFSET) | (1 << 9));	// Go!

        // ждем окончания транзакции:
        tmp_timer0 = 0;
        while(1)
          {
            if( IsCycleDone() )
		              break;
            wait_mcs(100);
            tmp_timer0++;
            if(tmp_timer0 > 100)
              {
                uprintf("\r\n SPI not ready after read status"
		                	": status = %02x", tmp);
                goto	err;
              }
          } // while

        status0 = (BYTE) inmemd(mSpiBaseMem + FLDATA_OFFSET);

        return status0;

err:
	return -1;
}

//--------------------------------------------------------------------
//  читаем код JEDEC микросхемы SPI-FLASH
//
int	readJedecId(UINT32 *jedec_id){
int	i;
int	tmp;
DWORD	tmp_timer0;
int	index_jedec = -1;

	if(!mSpiBaseMem)
	{
          uprintf("\r\n mSpiBaseMem not set");
	  return FALSE;
	}

	if(pchNum == PCH10)
	{
EFI_STATUS	Status;
	  Status = mSpiProtocol->FlashReadJedecId(mSpiProtocol, 0, 3, (UINT8*)jedec_id);
	  if(EFI_ERROR(Status))
	  {
            uprintf("\r\n%s.%d: for Skylake not supported yet", __FUNCTION__, __LINE__);
	    return FALSE;
	  }

	  return TRUE;
	}

	// 1. ищем коды в OpcodeMenu:
        for(i = 0; i < 8; i++)
          {
            if( inmemb(mSpiBaseMem + OPMENU_OFFSET + i) == SPI_JEDEC_ID)
              {
                index_jedec = i;	// READ_STATUS
              }
          }

        if(index_jedec < 0)
          {
            uprintf("\r\n code SPI_JEDEC_ID not found");
            goto	err;
          }


	// проверяем готовность:
        if( (tmp = inmemb(mSpiBaseMem + SSFS_OFFSET)) & SSFS_SCIP)
          {
            uprintf("\r\n SPI not ready before read JEDEC ID:"
            				" status = %02x", tmp);
            goto	err;
          }

        // запись в командный регистр
        // по адресу SSFS, т.е. одновременно
        // в командный регистр и регистр статуса:
        // 0xfc403004
        outmemd(mSpiBaseMem + SSFS_OFFSET,
            	(0x1fL << 27)   		// reserved
        	+ (1L << 22) 			// наличие данных
                + (2L << 16)			// (количество байт - 1)
                + ((DWORD)index_jedec << 12)   	// индекс команды
                + (1L << 3)			// сбросить ошибку
                + (1L << 2));			// сбросить Complete

        outmemd(mSpiBaseMem + SSFS_OFFSET,
        	inmemd(mSpiBaseMem + SSFS_OFFSET) | (1 << 9));	// Go!

        // ждем окончания транзакции:
        tmp_timer0 = 0;
        while(1)
          {
            if( IsCycleDone() )
	    {
		break;
	    }
            wait_mcs(100);
            tmp_timer0++;
            if(tmp_timer0 > 100)
              {
                uprintf("\r\n SPI not ready after read JEDEC ID"
		                	": status = %02x", tmp);
                goto	err;
              }
          } // while

        *jedec_id = inmemd(mSpiBaseMem + FLDATA_OFFSET) & 0xffffff;
        return TRUE;

err:
	return FALSE;
}

//-------------------------------------------------------------------------------
//  выполняем операцию обмена с заданными префиксом, опкодом 
//  и количеством байтов данных
int	executeOpcode(int prefix_index, int opcode_index, int data_len){
int	status;
int	flag_data;
int	flag_prefix = 0;
DWORD	tmp_timer0;
DWORD	data;

	if(!mSpiBaseMem)
	{
          uprintf("\r\n mSpiBaseMem not set");
	  return FALSE;
	}

	if(pchNum == PCH10)
	{
            uprintf("\r\n%s.%d: for Skylake not supported yet", __FUNCTION__, __LINE__);
	    return FALSE;
	}

	if(data_len)
          {
            flag_data = 1;
          }
        else
          {
            flag_data = 0;
            data_len = 1;
          }

	if(prefix_index >= 0)
          {
            flag_prefix = 1;
          }
        else
          {
            prefix_index = 0;
          }

	// проверка состояния:
        if( (status = inmemb(mSpiBaseMem + SSFS_OFFSET) ) & SSFS_SCIP)
          {
            uprintf("\r\n SPI not ready before transaction:"
            		" opcode = %02x status = %02x",
                        ((BYTE*)(UINTN)mSpiBaseMem)[OPMENU_OFFSET + opcode_index],
                        status);
            goto	err;
          }

        // запись в командный регистр
        // по адресу SSFS, т.е. одновременно
        // в командный регистр и регистр статуса:
        //	0xfc43040c
        data =
            	(0x1fL << 27)   			// reserved
        	+ ((DWORD)flag_data << 22) 		// наличие данных
                + (((DWORD)data_len - 1) << 16)		// (количество байт - 1)
                + ((DWORD)opcode_index << 12)		// индекс команды
                + ((DWORD)prefix_index << 11)         	// индекс префикса
                + ((DWORD)flag_prefix << 10)		// наличие префикса
                + (1 << 3)				// сбросить ошибку
                + (1 << 2);				// сбросить Complete
        outmemd(mSpiBaseMem + SSFS_OFFSET, data);

        // проверяем, что Done сброшен:
        while(inmemb(mSpiBaseMem + SSFS_OFFSET) & SSFS_DONE)	// Done off ?
        {
          continue;
        }

	// запускаем операцию обмена:
        data = inmemd(mSpiBaseMem + SSFS_OFFSET) | (1 << 9);
        outmemd(mSpiBaseMem + SSFS_OFFSET, data);		// Go!

        // ждем окончания транзакции:
        tmp_timer0 = 0;
        while(1)
          {
            if( IsCycleDone() )
				break;
            wait_mcs(100);
            tmp_timer0++;
            if(tmp_timer0 > 4000)
              { // > 400 ms:
                uprintf("\r\n SPI not ready after transaction:"
                	" opcode = %02x  status = %02x",
                        inmemb(mSpiBaseMem + OPMENU_OFFSET + opcode_index),
			status);
                goto	err;
              }
          } // while
	return TRUE;

err:
	return FALSE;
}




//==============================================================================

#define	SECTOR_SIZE		0x1000
#define SECTOR_SIZE_NAME	"4K"


int	_writeSectorSkylake(DWORD addr, BYTE *buf, int len, DWORD biosStart){
EFI_STATUS	Status;
DWORD		tmp_addr = addr & ~0xff000000;
int		i;

	if(!mSpiBaseMem)
	{
          uprintf("\r\n mSpiBaseMem not set");
	  return FALSE;
	}

	// 1. стирание блока:
	// 1.1. проверка на чистоту:
        for(i = 0; i < (SECTOR_SIZE / 4); i++)
	{
	  if(((DWORD*)(UINTN)(biosStart + tmp_addr))[i] != 0xffffffff)
									break;
	}
	if(i >= SECTOR_SIZE / 4)
	{ // сектор чист:
	  goto	write;
	}
	// 1.2. стирание:
	Status = mSpiProtocol->FlashErase (mSpiProtocol, FlashRegionAll, tmp_addr, len);
	if(EFI_ERROR(Status))
	{
          uprintf("\r\n_writeSector: ***Error: can not erase FLASH");
          Print(L"\r\n_writeSector: ***Error: can not erase FLASH");
          goto	err;
	}

write:
	// 2. запись сектора:
	// 2.1. проверка на необходимость записи:
        for(i = 0; i < (SECTOR_SIZE / 4); i++)
	{
	  if(((DWORD*)buf)[i] != 0xffffffff)
						break;
	}
	if(i >= SECTOR_SIZE / 4)
	{ // писать не надо:
	  goto verify;
	}

	Status = mSpiProtocol->FlashWrite (mSpiProtocol, FlashRegionAll, tmp_addr, len, buf);
	if(EFI_ERROR(Status))
	{
          uprintf("\r\n_writeSector: ***Error: can not write FLASH");
          Print(L"\r\n_writeSector: ***Error: can not write FLASH");
          goto	err;
	}

verify:
	// 3. верификация сектора:
        for(i = 0; i < (len / 4); i++)
        {
          if ( ((DWORD*)buf)[i] != ((DWORD*)(UINTN)(biosStart + addr))[i])
	  {
              uprintf("\r\n_writeSector: ***Error: verification %lx: %x -> %x",
				addr + i * 4, ((DWORD*)buf)[i], ((DWORD*)(UINTN)(biosStart + addr))[i]);
	      goto err;
	   }
        }

	return TRUE;

err:
        return FALSE;
}


int	_writeSector(DWORD addr, BYTE *buf, int len, DWORD biosStart){
DWORD	tmp_addr = addr & ~0xff000000;
int	i;
int	j;
int	index_erase = -1;
int	index_write = -1;
int	index_prefix = -1;
int	index_read_status = -1;
int	BiosWriteEnable = (int)readPci0(31, 0, 0xdc) & 1;	// BIOS_CNTL.BIOSWE
DWORD	data;

	if(!mSpiBaseMem)
	{
          uprintf("\r\n mSpiBaseMem not set");
	  return FALSE;
	}

	// 0. проверка параметров:
	if(tmp_addr & (SECTOR_SIZE - 1))
        { // адрес не выровнен:
          uprintf("\r\n_writeSector: ***Error: start addr = %x is not aligned at %s",
            		tmp_addr, SECTOR_SIZE_NAME);
          goto err;
        }

	if(len > SECTOR_SIZE)
        { // размер превышает размер сектора:
          uprintf("\r\n***Error: size = %x too much (> %s)",
            		len, SECTOR_SIZE_NAME);
          goto err;
        }

	if(pchNum == PCH10)
	{ // Skylake:
            return _writeSectorSkylake(addr, buf, len, biosStart);
	}

	// 1. находим коды инструкций в OpcodeMenu:
        for(i = 0; i < 8; i++)
          {
            if( inmemb(mSpiBaseMem + OPMENU_OFFSET + i) == SPI_SECTOR_ERASE)
              {
                index_erase = i;	//  индекс SPI_SECTOR_ERASE
              }
            if( inmemb(mSpiBaseMem + OPMENU_OFFSET + i) == SPI_PAGE_PROGRAM)
              {
                index_write = i;	//  индекс PAGE_PROGRAM
              }
            if( inmemb(mSpiBaseMem + OPMENU_OFFSET + i) == SPI_READ_STATUS)
              {
                index_read_status = i;	//  индекс READ_STATUS
              }
          }
	// 2. находим префиксы в PREOP:
        for(i = 0; i < 2; i++)
          {
            if( inmemb(mSpiBaseMem + PREOP_OFFSET + i) == SPI_WRITE_ENABLE)
              {
                index_prefix = i;	//  индекс префикса WRITE_ENABLE
              }
          }

        if(index_erase < 0)
          {
            uprintf("\r\n_writeSector: code SPI_SECTOR_ERASE not found");
            goto	err;
          }
        if(index_write < 0)
          {
            uprintf("\r\n_writeSector: code SPI_PAGE_PROGRAM not found");
            goto	err;
          }
        if(index_read_status < 0)
          {
            uprintf("\r\n_writeSector: code SPI_READ_STATUS not found");
            goto	err;
          }
        if(index_prefix < 0)
          {
            uprintf("\r\n_writeSector: code SPI_PREFIX not found");
            goto	err;
          }

	// разрешение на запись в BIOS:
        data = readPci0(31, 0, 0xdc) | 1;
	writePci0(31, 0, 0xdc, data);

	// 3. стирание блока:
	// 3.1. проверка на чистоту:
{
int	erase_cnt = 0;
erase:
        for(i = 0; i < (SECTOR_SIZE / 4); i++)
	{
	  if(((DWORD*)(UINTN)(biosStart + tmp_addr))[i] != 0xffffffff)
									break;
	}
	if(i >= SECTOR_SIZE / 4)
	{ // сектор чист:
	  goto	write;
	}
	// 3.2. стирание:
	// 3.2.1. записываем адрес:
	outmemd(mSpiBaseMem + FLADDR_OFFSET, tmp_addr);
        // 3.2.2. выполняем SPI-обмен:
	if(executeOpcode(index_prefix, index_erase, 0) == FALSE)
        {
           uprintf("\r\n_writeSector: ***Error: SPI executeOpcode error");
           goto	err;
        }
	erase_cnt++;
	if(erase_cnt < 3)
			goto	erase;	// проверить результат

        uprintf("\r\n_writeSector: ***Error: can not erase FLASH");
        Print(L"\r\n_writeSector: ***Error: can not erase FLASH");
        goto	err;

}

write:
	// 4. запись сектора страницами по 64 байта:
        for(i = 0; i < len; i += 64, tmp_addr += 64)
        {
	  // 4.1. проверка, что есть отличные от 0xff данные:
          for(j = 0; j < 16; j++)
          {
            if (((DWORD*)(buf + i))[j] != 0xff)
						break;
          }
	  if(j >= 16)
	  { // все данные == 0xff - переходим к следующей странице:
		continue;
	  }

          // 4.2. записываем адрес:
          outmemd(mSpiBaseMem + FLADDR_OFFSET, tmp_addr);

          // 4.3. записываем данные:
          for(j = 0; j < 16; j++)
          {
            outmemd(mSpiBaseMem + FLDATA_OFFSET + j * 4, ((DWORD*)(buf + i))[j]);
          }

          // 4.4. выполняем SPI-обмен:
	  if(executeOpcode(index_prefix, index_write, 64) == FALSE)
          {
            uprintf("\r\n_writeSector: ***Error: SPI executeOpcode error");
            goto	err;
          }
	  
	}

	// 5. верификация сектора:
        for(i = 0; i < (len / 4); i++)
        {
          if ( ((DWORD*)buf)[i] != ((DWORD*)(UINTN)(biosStart + addr))[i])
	  {
              uprintf("\r\n_writeSector: ***Error: verification %lx: %x -> %x",
				addr + i * 4, ((DWORD*)buf)[i], ((DWORD*)(UINTN)(biosStart + addr))[i]);
	      goto err;
	   }
        }

	if(BiosWriteEnable == 0)
		writePci0(31, 0, 0xdc, readPci0(31, 0, 0xdc) & ~1);

	return TRUE;

err:
	if(BiosWriteEnable == 0)
		writePci0(31, 0, 0xdc, readPci0(31, 0, 0xdc) & ~1);
        return FALSE;
}

//-------------------------------------------------------------------------------
// записываем файл блоками по SECTOR_SIZE:
BYTE	byteBuf[SECTOR_SIZE];

int	getch(IN int timeOut);

int	writeSpiFlash(BYTE *buf, DWORD startAddr, DWORD size, DWORD biosStart){
DWORD	actAddr = startAddr;
int	res;

	if(!mSpiBaseMem)
	{
          uprintf("\r\n mSpiBaseMem not set");
	  return FALSE;
	}

	if(startAddr & (SECTOR_SIZE - 1))
        { // адрес не выровнен:
          uprintf("\r\n***Error: start addr = %x is not aligned at %s",
            		startAddr, SECTOR_SIZE_NAME);
          goto error;
        }

	if(size & (SECTOR_SIZE - 1))
        { // размер не выровнен:
          uprintf("\r\n***Error: size = %x is not aligned at %s",
            		size, SECTOR_SIZE_NAME);
          goto error;
        }

        uprintf("\r\nwriteSpiFlash:  d31:f0:dc = %08lx",
	        				readPci0(31, 0, 0xdc));

	// устанавливаем смещение в источнике:
	buf += startAddr;

	while(size){
UINTN		ReadSize;
int		chr;

	chr = getch(1);
	 if((chr == 0x1b) || ((chr & ~0x20) == 'X'))
	 {
	   uprintf("\r\n aborted");
	   break;
	 }
	 if((actAddr & 0xffff) == 0)
	 {
	   Print(L"\r\n write %lx..", actAddr);
	   uprintf("\r\n write %lx..", actAddr);
	 }
	  // чтение сектора из источника по текущему указателю:
	  ReadSize = SECTOR_SIZE;
	  CopyMem(byteBuf, buf, ReadSize);

	  res = _writeSector(actAddr, byteBuf, SECTOR_SIZE, biosStart);
          if(res == FALSE)
          { // ошибка записи:
            goto error;
          }

          actAddr += SECTOR_SIZE;
	  buf += SECTOR_SIZE;
          size -= SECTOR_SIZE;
        }
	Print(L"\r\n ..Done");

	return TRUE;

error:
	return FALSE;
}

int	readOpcodeMenu(UINT8 *buf, int len){
int	i;
	if(!mSpiBaseMem)
	{
          uprintf("\r\n mSpiBaseMem not set");
	  return FALSE;
	}

	for(i = 0; i < len; i++)
	{
          buf[i] = *(UINT8*)(UINTN)(mSpiBaseMem + OPMENU_OFFSET + i);
	}
	return TRUE;
}

int	readRegionsAddr(DWORD *regionAddr, int len){
int	i;
int	Offsets[] = {FREG0, FREG1, FREG2, FREG3};

	if(!mSpiBaseMem)
	{
         uprintf("\r\n mSpiBaseMem not set");
	  return FALSE;
	}

	if(len > (sizeof(Offsets) / sizeof(int)))
		len = sizeof(Offsets) / sizeof(int);

	for(i = 0; i < len; i++)
	{
	  regionAddr[i] = inmemd(mSpiBaseMem + Offsets[i]);
	}

	return TRUE;
}

