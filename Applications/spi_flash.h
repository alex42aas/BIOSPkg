/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef	SPI_FLASH_H
#define SPI_FLASH_H

typedef UINT8	BYTE;
typedef UINT16	WORD;
typedef UINT32	DWORD;

enum 
{
	PCH_UDF	=0,
	PCH5 = 5,
	PCH6,
	PCH7,
	PCH8,
	PCH10 = 0xa,
	PCH_MOBILE = 0x10,
	PCH8M = 0x18		
};

//==================================================================================
//	смещения для series 6-7-8
#define	BFPR_OFFSET	0x00	// регистр для Primary Region
#define	HSFS_OFFSET	0x04	// статус транзакции (8 bits)
#define	FLADDR_OFFSET	0x08	// регистр адреса SPI (24 bits)
#define	FLDATA_OFFSET	0x10	// регистр данных SPI (32  bits)

#define FREG0		0x54	// границы дескриптора
#define FREG1		0x58	// границы региона BIOS
#define FREG2		0x5c	// границы региона ME
#define FREG3		0x60	// границы региона Gbe

#define	SSFS_OFFSET	0x90	// статус транзакции (8 bits)
#define	SSFS_SCIP	(1 << 0)
#define	SSFS_DONE	(1 << 2)
#define	SSFC_OFFSET	0x91	// командный регистр транзакции (24 bits)
#define	PREOP_OFFSET	0x94	// коды префиксов
#define	OPMENU_OFFSET	0x98	// Opcode Menu

#define FDOC_OFFSET	0xb0	// командный регистр доступа к Flash Descriptor
#define	FDOD_OFFSET	0xb4	// регистр данных доступа к Flash Descriptor


// команды SPI-FLASH:
#define	SPI_READ		0x03
#define	SPI_READ_STATUS		0x05
#define	SPI_READ_STATUS2	0x35

#define	SPI_WRITE_ENABLE	0x06
#define	SPI_WRITE_DISABLE	0x04
#define	SPI_PAGE_PROGRAM	0x02
#define	SPI_WRITE_STATUS	0x01

#define	SPI_SECTOR_ERASE	0x20	// 4kb erase

#define SPI_JEDEC_ID		0x9f

//==================================================================================


extern	DWORD	mPciBase;
extern  int	pchNum;
extern  int	pchId;


DWORD	inmemd(DWORD addr);

int	setPciBase(void);
DWORD	getRcba(void);
DWORD	readPci0(int dev, int func, int offset);

void	initSpi(void);
DWORD	getSpiBaseMem(void);
DWORD	readBiosRegionAddr(void);

int	executeOpcode(int prefix_index, int opcode_index, int data_len);

int	readFlashStatus(void);
int	readJedecId(UINT32 *jedec_id);

int	_writeSector(DWORD addr, BYTE *buf, int len, DWORD biosStart);
int	writeSpiFlash(BYTE *buf, DWORD startAddr, DWORD size, DWORD biosStart);

int	readOpcodeMenu(UINT8 *buf, int len);
int	readRegionsAddr(UINT32 *regionAddr, int len);

#endif	/* SPI_FLASH_H	*/
