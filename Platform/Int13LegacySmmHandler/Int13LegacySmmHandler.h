/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _INT13_LEGACY_SMM_HANDLER_
#define _INT13_LEGACY_SMM_HANDLER_

#include <Library/FsUtils.h> 
#include <Protocol/LegacyBios.h>
#include <Protocol/SmmSwDispatch.h>
#include <Protocol/BlockIo.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Mbr.h>
#include <IndustryStandard/ElTorito.h>
#include <Protocol/Timer.h>


#define APM_CNT_DEBUG_TTO_CMD       0xAA		// вывод в консоль
#define APM_CNT_INT13_CMD           0xAB		// замена INT13

#define INT13_PRIVATE_ADDR          0x4B0
#define BOOT_SECTOR_OUTPUT_ADDR     0x7C00  


#define PRIMARY_HD_IDENT       0x80
#define PRIMARY_CDROM_IDENT	   0xF0

#define DISK_RET_SUCCESS       0x00
#define DISK_RET_EPARAM        0x01
#define DISK_RET_EADDRNOTFOUND 0x02
#define DISK_RET_EWRITEPROTECT 0x03
#define DISK_RET_ECHANGED      0x06
#define DISK_RET_EBOUNDARY     0x09
#define DISK_RET_EBADTRACK     0x0c
#define DISK_RET_ECONTROLLER   0x20
#define DISK_RET_ETIMEOUT      0x80
#define DISK_RET_ENOTLOCKED    0xb0
#define DISK_RET_ELOCKED       0xb1
#define DISK_RET_ENOTREMOVABLE 0xb2
#define DISK_RET_ETOOMANYLOCKS 0xb4
#define DISK_RET_EMEDIA        0xC0
#define DISK_RET_ENOTREADY     0xAA


#define SEGMENT_TO_LINEAR(segment, base) ((segment*16)+base)

#define DISABLE_TPL_NOTIFICATION BIT8
#define ENABLE_TPL_NOTIFICATION BIT9

#define MAX_HDD_SUPPORTED 16 

//Загрузка с cd/dvd, вызывается из Int19 
#define CD_DVD_BOOT_EVENT 0xFFFF
//TODO: возможно добавить значение для загрузки с жесткого диска (всегда читается первый сектор, можно перетащить параметры из асма в си). 

#pragma pack(1)

typedef struct {
	UINT8 lo;
	UINT8 hi;
} _REG;

typedef union {
  _REG reg;
  UINT16 val;
} COMMON_REG;

typedef	struct {
  COMMON_REG	flags;
  COMMON_REG	ax;
  COMMON_REG	bx;
  COMMON_REG	cx;
  COMMON_REG	dx;
  COMMON_REG	si;
  COMMON_REG	di;
  COMMON_REG	bp;
  COMMON_REG	sp;
  COMMON_REG	ds;
  COMMON_REG	es;
  COMMON_REG	ss;
}	SAVE_REGS;

typedef	struct{
  UINT32		BbsEntry;
  UINT16		BbsDeviceType;
  SAVE_REGS		SaveRegs;
}	INT13_PRIVATE;


typedef struct {
  UINT8  DapSize; //size of this struct (10h)
  UINT8  Reversed; 
  UINT16 NumberOfSectors;
  UINT16 BufferOffset;
  UINT16 BufferSegment;
  UINT64 FirstSector; 
  UINT64 FlatAddress; //64-bit flat address of transfer buffer (optional, if OutputBufferOffset = 0xFFFF and OutputBufferSegment = 0xFFFF);
} DAP;



typedef struct {
	UINT8 Bus;
	UINT8 Slot; 
	UINT8 Function;
	UINT8 Reversed;
	UINT32 Reversed1;
} __PCI;

typedef struct {
	UINT16 Base;
	UINT16 Reversed;
	UINT32 Reversed1;
} __ISA;

typedef union {
  __PCI pci;
  __ISA isa;
} INTERFACE_PATH;


typedef struct {
  UINT16 BufferSize; //1Eh
  UINT16 InformationFlags;
  UINT32 PhysicalCylinderCount;
  UINT32 PhysicalHeadsCount;
  UINT32 PhysicalSectorPerTrackCount;
  UINT64 SectorCount;
  UINT16 BytesPerSector;
  UINT32 PointerToEDDConfig; //optional, set to NULL


  UINT16 DevicePathPresent; //0BEDDh
  UINT8  DevicePathLength; //36
  UINT8  Reversed1; //0
  UINT16 Reversed2; //0
  UINT32 Signature; //0x00494350 PCI
  UINT64 InterfaceType; //0x00415451 ATA

  INTERFACE_PATH Interfacepath;

  UINT64 DevicePath; //not implemented yet

  UINT8 Reversed3;

  UINT8 CheckSum; //0


} DRIVE_PARAMETERS;


typedef struct {
	UINT8 PacketSize;
	UINT8 BootMediaType;
	UINT8 DriveNumber;
	UINT8 ControllerIndex;
	UINT32 LbaOfDiskEmulated;
	UINT16 DeviceSpecification;
	UINT16 UserBufferSegment;
	UINT16 LoadSegment;
	UINT16 SectorCount;
	UINT8 AH_08_CilynderCount;
	UINT8 AH_08_SectorCount;
	UINT8 AH_08_HeadCount;
} CDROM_SPEC_PACKET;

#pragma pack()


EFI_STATUS
SwDispatchInt13Hahdler (
  IN EFI_HANDLE    DispatchHandle,
  IN EFI_SMM_SW_DISPATCH_CONTEXT   * DispatchContext
    );


//TODO: Add (Head, Cylynded, Sector) -> LBA 
//Now: LBA = Sector - 1;
EFI_STATUS ReadSector(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN UINTN Count, IN UINT8 Cilynder, IN UINT8 Head, IN UINT64 Sector, IN UINT32 OutputAddress);
EFI_STATUS ExtPerformSectorOperation(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN DAP* Descriptor, BOOLEAN isRead);
EFI_STATUS GetDriveParameters(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN UINT32 OutputAddr); 
EFI_STATUS CdRomFillSpecPacket(IN UINTN OutputAddress);

EFI_BLOCK_IO_PROTOCOL* getPreferredBlockIo(UINT8 DriveNumber);


EFI_STATUS ReadBlocksFromLba(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN UINTN Count, IN UINTN Start, IN UINTN OutputAddress);
EFI_STATUS WriteBlocksFromLba(IN EFI_BLOCK_IO_PROTOCOL* blockIo, IN UINTN Count, IN UINTN Start, IN UINTN OutputAddress);

EFI_STATUS PerformCdBoot(IN EFI_BLOCK_IO_PROTOCOL* cdBlockIo);

EFI_STATUS InitInt13LegacySmmHandler();
EFI_STATUS FindSataBlockIo(UINT32 Port, UINT32 Device, UINT32 Function, UINT16 BbsDeviceType);


#endif //_INT13_LEGACY_SMM_HANDLER_