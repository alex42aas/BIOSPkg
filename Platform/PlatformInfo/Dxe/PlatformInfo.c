/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "EdkIIGlueDxe.h"
#include "Common\EdkIIGlueDependencies.h"
#include "CpuFuncs.h"

#include <CommonAddresses.h>
#include <PlatformDataLib.h>

#include "Protocol/PlatformInfo.h"

#include <Protocol/DiskInfo/DiskInfo.h>


//HOB_SAVE_MEMORY_DATA
#include <MemInfoHob.h>
#include <Protocol/MemInfo/MemInfo.h>


#include <RedirectDebugLib.h>

#define LOG DEBUG


//=============================================================================

#define CPUID_SIGNATURE                 0x0

//=============================================================================

EFI_GUID gMemRestoreDataGuid   = EFI_MEMORY_RESTORE_DATA_GUID;
EFI_GUID gEfiDataHubProtocolGuid = EFI_DATA_HUB_PROTOCOL_GUID;
EFI_GUID gEfiPlatformInfoProtocolGuid = EFI_PLATFORM_INFO_PROTOCOL_GUID; 
EFI_GUID gEfiBlockIoProtocolGuid = EFI_BLOCK_IO_PROTOCOL_GUID; 
EFI_GUID gEfiDiskInfoProtocolGuid = EFI_DISK_INFO_PROTOCOL_GUID; 

/** Указатель  */
EFI_BOOT_SERVICES  *gBS;

//=============================================================================

static const char *GetProcessorFamilyDescription(UINT32 code)
{
  static const char *processor_family[] =
  {
    "Unknown processor",                                           /* 0x00 */
    "",
    "",
    "8086",
    "80286",
    "i386",
    "i486",
    "8087",
    "80287",
    "80387",
    "80487",
    "Pentium",
    "Pentium Pro",
    "Pentium II",
    "Pentium MMX",
    "Celeron",
    "Pentium II Xeon",
    "Pentium III",
    "M1",
    "M2",
    "Celeron M",                                           /* 0x14 */
    "Pentium 4 HT",
    "",
    "",
    "Duron",
    "K5",
    "K6",
    "K6-2",
    "K6-3",
    "Athlon",
    "AMD29000",
    "K6-2+",
    "Power PC",
    "Power PC 601",
    "Power PC 603",
    "Power PC 603+",
    "Power PC 604",
    "Power PC 620",
    "Power PC x704",
    "Power PC 750",
    "Core Duo",                                           /* 0x28 */
    "Core Duo mobile",
    "Core Solo mobile",
    "Atom",
    "Core M",
    "",
    "",
    "",
    "Alpha",                                      /* 0x30 */
    "Alpha 21064",
    "Alpha 21066",
    "Alpha 21164",
    "Alpha 21164PC",
    "Alpha 21164a",
    "Alpha 21264",
    "Alpha 21364",
    "Turion II Ultra Dual-Core Mobile M",         /* 0x38 */
    "Turion II Dual-Core Mobile M",
    "Athlon II Dual-Core M",
    "Opteron 6100",
    "Opteron 4100",
    "Opteron 6200",
    "Opteron 4200",
    "FX",
    "MIPS",                                       /* 0x40 */
    "MIPS R4000",
    "MIPS R4200",
    "MIPS R4400",
    "MIPS R4600",
    "MIPS R10000",
    "C-Series",                                   /* 0x46 */
    "E-Series",
    "A-Series",
    "G-Series",
    "Z-Series",
    "R-Series",
    "Opteron 4300",
    "Opteron 6300",
    "Opteron 3300",
    "FirePro",
    "SPARC",
    "SuperSPARC",
    "MicroSPARC II",
    "MicroSPARC IIep",
    "UltraSPARC",
    "UltraSPARC II",
    "UltraSPARC IIi",
    "UltraSPARC III",
    "UltraSPARC IIIi",
    "",                                           /* 0x59 */
    "",
    "",
    "",
    "",
    "",
    "",                                           /* 0x5F */
    "68040",
    "68xxx",
    "68000",
    "68010",
    "68020",
    "68030",
    "Athlon X4",                                  /* 0x66 */
    "Opteron X1000",
    "Opteron X2000 APU",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                                           /* 0x6F */
    "Hobbit",
    "",                                           /* 0x71 */
    "",
    "",
    "",
    "",
    "",
    "",                                           /* 0x77 */
    "Crusoe TM5000",
    "Crusoe TM3000",
    "Efficeon TM8000",                                           /* 0x7A */
    "",
    "",
    "",
    "",
    "",                                           /* 0x7F */
    "Weitek",
    "",                                           /* 0x81 */
    "Itanium",
    "Athlon 64",
    "Opteron",
    "Sempron",                                           /* 0x85 */
    "Turion 64 Mobile",
    "Dual-Core Opteron",
    "Athlon 64 X2 Dual-Core",
    "Turion 64 X2 Mobile",
    "Quad-Core Opteron",
    "3rd-generation Opteron",
    "Phenom FX Quad-Core",
    "Phenom X4 Quad-Core",
    "Phenom X2 Dual-Core",
    "Athlon X2 Dual-Core",                                           /* 0x8F */
    "PA-RISC",
    "PA-RISC 8500",
    "PA-RISC 8000",
    "PA-RISC 7300LC",
    "PA-RISC 7200",
    "PA-RISC 7100LC",
    "PA-RISC 7100",
    "",                                           /* 0x97 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                                           /* 0x9F */
    "V30",
    "Xeon 3200 Quad-Core",                        /* 0xA1 */
    "Xeon 3000 Dual-Core",
    "Xeon 5300 Quad-Core",
    "Xeon 5100 Dual-Core",
    "Xeon 5000 Dual-Core",
    "Xeon LV Dual-Core",
    "Xeon ULV Dual-Core",
    "Xeon 7100 Dual-Core",
    "Xeon 5400 Quad-Core",
    "Xeon Quad-Core",
    "Xeon 5200 Dual-Core",
    "Xeon 7200 Dual-Core",
    "Xeon 7300 Quad-Core",
    "Xeon 7400 Quad-Core",
    "Xeon 7400 Multi-Core",                       /* 0xAF */
    "Pentium III Xeon",
    "Pentium III Speedstep",
    "Pentium 4",
    "Xeon",
    "AS400",
    "Xeon MP",
    "Athlon XP",
    "Athlon MP",
    "Itanium 2",
    "Pentium M",
    "Celeron D",                                           /* 0xBA */
    "Pentium D",
    "Pentium Extreme Edition",
    "Core Solo",
    "",
    "Core 2 Duo",
    "Core 2 Solo",
    "Core 2 Extreme",
    "Core 2 Quad",
    "Core 2 Extreme mobile",
    "Core 2 Duo mobile",
    "Core 2 Solo mobile",
    "Core i7",
    "Dual-Core Celeron",                                           /* 0xC7 */
    "IBM390",
    "G4",
    "G5",
    "ESA/390 G6",                                           /* 0xCB */
    "z/Architecture base",
    "Core i5",
    "Core i3",
    "",
    "",
    "",
    "VIA C7-M",
    "VIA C7-D",
    "VIA C7",
    "VIA Eden",
    "Multi-Core Xeon",
    "Dual-Core Xeon 3xxx",
    "Quad-Core Xeon 3xxx",
    "VIA Nano",
    "Dual-Core Xeon 5xxx",
    "Quad-Core Xeon 5xxx",	/* 0xDB */
    "",
    "Dual-Core Xeon 7xxx",
    "Quad-Core Xeon 7xxx",
    "Multi-Core Xeon 7xxx",
    "Multi-Core Xeon 3400",
    "",
    "",
    "",
    "Opteron 3000",
    "AMD Sempron II",
    "Embedded Opteron Quad-Core",
    "Phenom Triple-Core",
    "Turion Ultra Dual-Core Mobile",
    "Turion Dual-Core Mobile",
    "Athlon Dual-Core",
    "Sempron SI",
    "Phenom II",
    "Athlon II",
    "Six-Core Opteron",
    "Sempron M",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                                           /* 0xF9 */
    "i860",
    "i960",
    "",                                           /* 0xFC */
    "",
    "",
    ""                                            /* 0xFF */
  };

  if (code <= 0xFF)
    return processor_family[code];

  switch (code)
  {
    case 0x104: return "SH-3";
    case 0x105: return "SH-4";
    case 0x118: return "ARM";
    case 0x119: return "StrongARM";
    case 0x12C: return "6x86";
    case 0x12D: return "MediaGX";
    case 0x12E: return "MII";
    case 0x140: return "WinChip";
    case 0x15E: return "DSP";
    case 0x1F4: return "Video Processor";
    default: return "";
  }
}

static const char *GetProcessorManufacturerDescription(UINT16 code)
{
	//if (value == "AuthenticAMD")
	//	value = "Advanced Micro Devices [AMD]";
	//if (value == "GenuineIntel")
	//	value = "Intel Corp.";
	
	return NULL;
}

//=============================================================================
BOOLEAN
IsIntelProcessor(void)
{
	EFI_CPUID_REGISTER Reg;

	AsmCpuid (CPUID_SIGNATURE, &Reg.RegEax, &Reg.RegEbx, &Reg.RegEcx, &Reg.RegEdx);

	return ((Reg.RegEbx == 'uneG') && (Reg.RegEdx == 'Ieni') && (Reg.RegEcx == 'letn'));
}

//=============================================================================
BOOLEAN
IsAmdProcessor(void)
{
	EFI_CPUID_REGISTER Reg;

	AsmCpuid (CPUID_SIGNATURE, &Reg.RegEax, &Reg.RegEbx, &Reg.RegEcx, &Reg.RegEdx);

	return ((Reg.RegEbx == 'htuA') && (Reg.RegEdx == 'itne') && (Reg.RegEcx == 'DMAc'));
}

//=============================================================================
const UINT8* GetProcessorManufacturerDescriptionInternal(void)
{
	if (IsIntelProcessor ())
	{
		return "Intel(R)";
	}
	else if (IsAmdProcessor())
	{
		return "AMD(R)";
	}
	else
	{
		return "OEM";
	}
}

//=============================================================================
EFI_STATUS
FindProcessorInfo(OUT PSProcessorInfo pProcessorInfo)
{
	EFI_STATUS efiStatus;
	UINT64 nCounter = 0;
	EFI_DATA_HUB_PROTOCOL* DataHub = NULL;
	CHAR8 szProcessorDescr[MAX_NAME_LENGTH];

	EFI_DATA_RECORD_HEADER* pRecordHeader;
	EFI_CPU_DATA_RECORD* pUsefulData;
	CHAR8 szTempCpuIdString[MAX_NAME_LENGTH];

	/** Хранит номер команды cpuid для получения описания процессора */
	UINT32 nCpuIdCmd;
	EFI_CPUID_REGISTER CpuIdRegisters;

	UINT32 nTempProcessorFamily = 0;
	
	LOG((EFI_D_INFO, "\r\nFindProcessorInfo: entry"));
	
	pProcessorInfo->_nVersion = PLATFORM_INFO_VERSION_1_0;
	
	efiStatus = gBS->LocateProtocol (&gEfiDataHubProtocolGuid, NULL, (VOID **) &DataHub);
	if (EFI_ERROR(efiStatus))
	{
		DEBUG((EFI_D_ERROR, "\r\nFindProcessorInfo: LocateProtocol failed 0x%08x", efiStatus));
		return efiStatus;
	}
	
	ASSERT(0x0100 == EFI_DATA_RECORD_HEADER_VERSION);
	
	while (1)
	{
		efiStatus = DataHub->GetNextRecord(DataHub, &nCounter, NULL, &pRecordHeader);
		if (EFI_ERROR(efiStatus))
		{
			LOG((EFI_D_INFO, "\r\nFindProcessorInfo: GetNextRecord failed 0x%08x", efiStatus));
			break;
		}
		
		if (!nCounter)
		{
			LOG((EFI_D_INFO, "\r\nFindProcessorInfo: all records were processed"));
			break;
		}
		
		if (!CompareGuid(&pRecordHeader->DataRecordGuid, &gEfiProcessorSubClassGuid) ||			
			!CompareGuid(&pRecordHeader->ProducerName, &gEfiProcessorProducerGuid))
		{
			continue;
		}
	
		pUsefulData = (EFI_CPU_DATA_RECORD*)(((UINT8*)pRecordHeader) + pRecordHeader->HeaderSize);
		
		switch (pUsefulData->DataRecordHeader.RecordType)
		{
		case ProcessorFamilyRecordType:
			nTempProcessorFamily = pUsefulData->VariableRecord.ProcessorFamily;
			LOG((EFI_D_INFO, "\r\nProcessor family=0x%08x", nTempProcessorFamily));
			break;
		case ProcessorCoreFrequencyRecordType:
			LOG((EFI_D_INFO, "\r\nProcessor freq=%d * 10^%d Hz", 
				pUsefulData->VariableRecord.ProcessorCoreFrequency.Value,
				pUsefulData->VariableRecord.ProcessorCoreFrequency.Exponent));
			pProcessorInfo->_nCpuFreqInMHz = (UINT32)(((UINT64)pUsefulData->VariableRecord.ProcessorCoreFrequency.Value));
			LOG((EFI_D_INFO, "\r\nProcessor freq=%d MHz", pProcessorInfo->_nCpuFreqInMHz));
			break;
		//case ProcessorMaxFsbFrequencyRecordType:
		//	LOG("\r\nFSB max freq=%d * 10^%d Hz", 
		//		pUsefulData->VariableRecord.ProcessorMaxFsbFrequency.Value,
		//		pUsefulData->VariableRecord.ProcessorMaxFsbFrequency.Exponent);
		//	break;
		case ProcessorCoreCountRecordType:
			LOG((EFI_D_INFO, "\r\nProcessor number of cores = %d", 
				pUsefulData->VariableRecord.ProcessorCoreCount));
			pProcessorInfo->_nCoresCount = pUsefulData->VariableRecord.ProcessorCoreCount;
			break;
		case ProcessorThreadCountRecordType:
			LOG((EFI_D_INFO, "\r\nProcessor number of threads = %d", 
				pUsefulData->VariableRecord.ProcessorThreadCount));
			pProcessorInfo->_nThreadsCount = pUsefulData->VariableRecord.ProcessorThreadCount;
			break;
		case ProcessorIdRecordType:
			LOG((EFI_D_INFO, "\r\nProcessor signature = 0x%08x", 
				pUsefulData->VariableRecord.ProcessorId.Signature));
			break;
			
		//case ProcessorManufacturerRecordType:
		//	nSaveManufacturerId = pUsefulData->VariableRecord.ProcessorManufacturer;
		//	break;
		}
	}

	//
	//	Формирование текстового описания процессора
	//
	
#if 1
	//
	// Более перспективный способ получения информации о процессоре
	//
#define CPUID_BRAND_STRING_CMD_FIRST 0x80000002
#define CPUID_BRAND_STRING_CMD_LAST  0x80000004

	strcpy(szProcessorDescr, "");

	for (nCpuIdCmd = CPUID_BRAND_STRING_CMD_FIRST; nCpuIdCmd <= CPUID_BRAND_STRING_CMD_LAST; nCpuIdCmd++)
	{
		AsmCpuid (nCpuIdCmd, &CpuIdRegisters.RegEax, &CpuIdRegisters.RegEbx, &CpuIdRegisters.RegEcx, &CpuIdRegisters.RegEdx);

		_sprintf(szTempCpuIdString, "%c%c%c%c", 
			CpuIdRegisters.RegEax & 0xFF,
			(CpuIdRegisters.RegEax >> 8) & 0xFF,
			(CpuIdRegisters.RegEax >> 16) & 0xFF,
			(CpuIdRegisters.RegEax >> 24) & 0xFF);
		strcat(szProcessorDescr, szTempCpuIdString);
		_sprintf(szTempCpuIdString, "%c%c%c%c", 
			CpuIdRegisters.RegEbx & 0xFF,
			(CpuIdRegisters.RegEbx >> 8) & 0xFF,
			(CpuIdRegisters.RegEbx >> 16) & 0xFF,
			(CpuIdRegisters.RegEbx >> 24) & 0xFF);
		strcat(szProcessorDescr, szTempCpuIdString);
		_sprintf(szTempCpuIdString, "%c%c%c%c", 
			CpuIdRegisters.RegEcx & 0xFF,
			(CpuIdRegisters.RegEcx >> 8) & 0xFF,
			(CpuIdRegisters.RegEcx >> 16) & 0xFF,
			(CpuIdRegisters.RegEcx >> 24) & 0xFF);
		strcat(szProcessorDescr, szTempCpuIdString);
		_sprintf(szTempCpuIdString, "%c%c%c%c", 
			CpuIdRegisters.RegEdx & 0xFF,
			(CpuIdRegisters.RegEdx >> 8) & 0xFF,
			(CpuIdRegisters.RegEdx >> 16) & 0xFF,
			(CpuIdRegisters.RegEdx >> 24) & 0xFF);
		strcat(szProcessorDescr, szTempCpuIdString);
	}
#else
	_sprintf(
		szProcessorDescr, 
		"%s %s", 
		GetProcessorManufacturerDescriptionInternal(), 
		GetProcessorFamilyDescription(nTempProcessorFamily));
#endif
		
	LOG((EFI_D_INFO, "\r\n%s", szProcessorDescr));
	
	AsciiStrToUnicodeStr(szProcessorDescr, pProcessorInfo->_wszCpuModel);
	
	return EFI_SUCCESS;
}

//=============================================================================
EFI_STATUS
FindMemoryInfo(PSMemoryInfo pMemInfo)
{
	EFI_PEI_HOB_POINTERS  Hob;
	UINT32                nHobSize;
	HOB_SAVE_MEMORY_DATA* pData = NULL;
	UINT64                nMemSize;
	UINT32                i;
	UINT32                j;
	

	pMemInfo->_nVersion = PLATFORM_INFO_VERSION_1_0;
	
	Hob.Raw = GetHobList();
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: entry, GetHobList = %llx", (UINT32)(UINTN)Hob.Raw));

	if (Hob.Raw == NULL)
	{
		DEBUG((EFI_D_ERROR, "\r\nFindPlatformInfo: GetHobList fail"));
		return EFI_INVALID_PARAMETER;
	}

	while (!END_OF_HOB_LIST (Hob))
	{	
		if (Hob.Header->HobType != GET_HOB_TYPE(Hob))
		{
			Hob.Raw = GET_NEXT_HOB (Hob);
			continue;
		}
	
		if (!CompareGuid(&gMemRestoreDataGuid, &Hob.Guid->Name))
		{
			Hob.Raw = GET_NEXT_HOB (Hob);
			continue;
		}
		
		LOG((EFI_D_INFO, "\r\nFindPlatformInfo: Hob GUID match"));
		
		nHobSize = Hob.Header->HobLength;
		pData = (HOB_SAVE_MEMORY_DATA*)Hob.Raw;
		
		break;
	}
	
	if (!pData)
	{
		// HOB не найден
		DEBUG((EFI_D_ERROR, "\r\nFindPlatformInfo: exit - HOB was not found"));
		return EFI_NOT_FOUND;
	}
	
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: correct data"));
	
	nMemSize = 0;	
	for (i = 0; i < MAX_CONTROLLERS; i++)
	{
		for (j = 0; j < MAX_CHANNEL; j++)
		{
			nMemSize += pData->MrcData.SysOut.Outputs.Controller[i].Channel[j].Capacity;
			LOG((EFI_D_INFO, "\r\nFindPlatformInfo: mem capasity =%d", pData->MrcData.SysOut.Outputs.Controller[i].Channel[j].Capacity));
		}
	}		
		
	pMemInfo->_nTotalMemoryInMb = (UINT32)(nMemSize);
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: nMemSize=%d", pMemInfo->_nTotalMemoryInMb));	

	
	pMemInfo->_nMemFreqInMHz = (UINT32)pData->MrcData.SysOut.Outputs.Frequency;
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: nMemFreqInMHz=%d", pMemInfo->_nMemFreqInMHz));	
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: nMemFreqMax=%d", pData->MrcData.SysOut.Outputs.FreqMax));	
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: nMemClock=%d", pData->MrcData.SysOut.Outputs.MemoryClock));	
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: nMemClockMax=%d", pData->MrcData.SysOut.Outputs.MemoryClockMax));	
	

	pMemInfo->_nMemType = (EDdrType)pData->MrcData.SysOut.Outputs.DdrType;
	LOG((EFI_D_INFO, "\r\nFindPlatformInfo: nMemType=%d", pMemInfo->_nMemType));	
	
	LOG((EFI_D_INFO, "\r\n%a.%d: exit OK", __FUNCTION__, __LINE__));

	return EFI_SUCCESS;
}

//=============================================================================

EFI_STATUS	CreateMemInfoProtocol(VOID){
EFI_STATUS		Status;
EFI_PEI_HOB_POINTERS	Hob;
UINT32			nHobSize;
HOB_SAVE_MEMORY_DATA*	pData = NULL;
MEM_INFO_PROTOCOL	*pMemInfoProtocol;
MEMORY_INFO_DATA	*pMemoryInfoData;
EFI_HANDLE		Handle;
UINT64			nMemSize;
int			cntr;
int			chan;
int			dimm;
int			dimmIndex;
	

	Hob.Raw = GetHobList();
	LOG((EFI_D_INFO, "\r\n%a.%d: entry", __FUNCTION__, __LINE__));

	if (Hob.Raw == NULL)
	{
	  DEBUG((EFI_D_ERROR, "\r\n%a.%d: GetHobList fail", __FUNCTION__, __LINE__));
	  return EFI_INVALID_PARAMETER;
	}

	while (!END_OF_HOB_LIST (Hob))
	{	
	  if (Hob.Header->HobType != GET_HOB_TYPE(Hob))
	  {
	    Hob.Raw = GET_NEXT_HOB (Hob);
	    continue;
	  }
	
	  if (!CompareGuid(&gMemRestoreDataGuid, &Hob.Guid->Name))
	  {
	    Hob.Raw = GET_NEXT_HOB (Hob);
	    continue;
	  }
		
	  LOG((EFI_D_INFO, "\r\n%a.%d: Hob GUID match", __FUNCTION__, __LINE__));
		
	  nHobSize = Hob.Header->HobLength;
	  pData = (HOB_SAVE_MEMORY_DATA*)Hob.Raw;
		
	  break;
	}
	
	if (!pData)
	{
	  // HOB не найден
	  DEBUG((EFI_D_ERROR, "\r\n%a.%d: exit - HOB was not found", __FUNCTION__, __LINE__));
	  return EFI_NOT_FOUND;
	}

	pMemInfoProtocol = (MEM_INFO_PROTOCOL*)AllocatePool(sizeof(MEMORY_INFO_DATA));
	pMemoryInfoData = &pMemInfoProtocol->MemInfoData;
	if (!pMemoryInfoData)
	{
	  DEBUG ((EFI_D_ERROR, "\r\n%a.%d: AllocatePool failed", __FUNCTION__, __LINE__));
	  return EFI_OUT_OF_RESOURCES;
	}

	nMemSize = 0;	
	for (cntr = 0; cntr < MAX_CONTROLLERS; cntr++)
	{
	  for (chan = 0; chan < MAX_CHANNEL; chan++)
	  {
	    nMemSize += pData->MrcData.SysOut.Outputs.Controller[cntr].Channel[chan].Capacity;
	    LOG((EFI_D_INFO, "\r\n mem capasity = %d", pData->MrcData.SysOut.Outputs.Controller[cntr].Channel[chan].Capacity));
	    for(dimm = 0; dimm < MAX_DIMMS_IN_CHANNEL; dimm++)
	    { 
	      dimmIndex = cntr * MAX_CHANNEL * MAX_DIMMS_IN_CHANNEL + chan * MAX_DIMMS_IN_CHANNEL + dimm;
	      pMemoryInfoData->dimmSize[dimmIndex] = (UINT16) pData->MrcData.SysOut.Outputs.Controller[cntr].Channel[chan].Dimm[dimm].DimmCapacity;
	      pMemoryInfoData->DimmExist[dimmIndex] = (pData->MrcData.SysOut.Outputs.Controller[cntr].Channel[chan].Dimm[dimm].Status == DIMM_PRESENT);
	      pMemoryInfoData->RankInDimm[dimmIndex] = pData->MrcData.SysOut.Outputs.Controller[cntr].Channel[chan].Dimm[dimm].RankInDIMM;
	      LOG((EFI_D_INFO, "\r\n [dimm %d]: size = %d, exist = %d, rank = %d", 
						dimmIndex, pMemoryInfoData->dimmSize[dimmIndex], pMemoryInfoData->DimmExist[dimmIndex], pMemoryInfoData->RankInDimm[dimmIndex]));
	      pMemoryInfoData->DimmsSpdData[dimmIndex] = (UINT8*)&pData->MrcData.SysIn.Inputs.Controller[cntr].Channel[chan].Dimm[dimm].Spd;
	      LOG((EFI_D_INFO, " spd = %lx", pMemoryInfoData->DimmsSpdData[dimmIndex]));

	    }
	  }
	}		
		
	pMemoryInfoData->memSize = (UINT32)(nMemSize);
	LOG((EFI_D_INFO, "\r\n nMemSize = %d", pMemoryInfoData->memSize));	

	pMemoryInfoData->ddrFreq = (UINT16)pData->MrcData.SysOut.Outputs.Frequency;
	pMemoryInfoData->ddrFreqMax = (UINT16)pData->MrcData.SysOut.Outputs.FreqMax;
	LOG((EFI_D_INFO, "\r\n nMemFreqInMHz=%d", pMemoryInfoData->ddrFreq));	
	LOG((EFI_D_INFO, "\r\n nMemFreqMax=%d", pMemoryInfoData->ddrFreqMax));	

	pMemoryInfoData->EccSupport = (UINT16)pData->MrcData.SysOut.Outputs.EccSupport;
	LOG((EFI_D_INFO, "\r\n output->Ecc = %d", pData->MrcData.SysOut.Outputs.EccSupport));	

	Handle = NULL;
	Status = gBS->InstallMultipleProtocolInterfaces (
		&Handle,
		&gMemInfoProtocolGuid,
		pMemInfoProtocol,
		NULL
		);
	if (EFI_ERROR (Status)) 
	{
	  DEBUG ((EFI_D_ERROR, "\r\n%a.%d: install protocol failed", __FUNCTION__, __LINE__));
	  FreePool(pMemInfoProtocol);
	}

	return Status;
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
PlatformInfoEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
	EFI_STATUS                   Status = EFI_SUCCESS;
	EFI_PLATFORM_INFO_PROTOCOL*  pPlatformInfoProtocol;

	LOG((EFI_D_INFO, "\r\nPlatformInfoDxe: entry"));

	// setup gBs:
	UefiBootServicesTableLibConstructor(ImageHandle, SystemTable);
	ASSERT (gBS != NULL);
	DxeServicesTableLibConstructor(ImageHandle, SystemTable);
	ASSERT (gDS != NULL);
	Status = HobLibConstructor (ImageHandle, SystemTable);
	ASSERT(!EFI_ERROR(Status));
  
	pPlatformInfoProtocol = (EFI_PLATFORM_INFO_PROTOCOL*)AllocatePool(sizeof(*pPlatformInfoProtocol));
	if (!pPlatformInfoProtocol)
	{
	  DEBUG ((EFI_D_ERROR, "PlatformInfoEntryPoint: AllocatePool failed"));
	  return EFI_OUT_OF_RESOURCES;
	}
	memset(pPlatformInfoProtocol, 0x0, sizeof(*pPlatformInfoProtocol));
	pPlatformInfoProtocol->_Version = PLATFORM_INFO_VERSION_1_0;
	
	  
  	LOG((EFI_D_INFO, "\r\nPlatformInfoDxe: FindProcessorInfo..."));
  
	FindProcessorInfo(&pPlatformInfoProtocol->_PlatformInfo._ProcessorInfo);
	
	LOG((EFI_D_INFO, "\r\nPlatformInfoDxe: FindMemoryInfo..."));
	
	FindMemoryInfo(&pPlatformInfoProtocol->_PlatformInfo._MemoryInfo);

	CreateMemInfoProtocol();
  
	LOG((EFI_D_INFO, "\r\nPlatformInfoDxe: InstallProtocol..."));

	Status = gBS->InstallMultipleProtocolInterfaces (
		&ImageHandle,
		&gEfiPlatformInfoProtocolGuid,
		pPlatformInfoProtocol,
		NULL
		);
	if (EFI_ERROR (Status)) 
	{
	  DEBUG ((EFI_D_ERROR, "\r\n%a.%d: install protocol failed", __FUNCTION__, __LINE__));
	  FreePool(pPlatformInfoProtocol);
	}

	LOG((EFI_D_INFO, "\r\nPlatformInfoDxe exit "));
  
	return Status;
}
