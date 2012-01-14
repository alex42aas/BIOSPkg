## @file
# EFI/Framework Emulation Platform with UEFI HII interface supported.
#
# The Emulation Platform can be used to debug individual modules, prior to creating
#    a real platform. This also provides an example for how an DSC is created.
#
# Copyright (c) 2006 - 2012, Intel Corporation. All rights reserved.<BR>
#
#    This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = GBYTEH77
  PLATFORM_GUID                  = EB216561-961F-47EE-9EF9-CA426EF547C2		
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/Application
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = BIOSPkg/Application.fdf
  #
  # This flag is to control tool to generate PCD info for dynamic(ex) PCD,
  # then enable or disable PCD info feature. TRUE is enable, and FLASE is disable.
  # If the flag is absent, it will be same as FALSE.
  #
  PCD_INFO_GENERATION            = TRUE


  EDK_GLOBAL	PLATFORM_ROOT			= BIOSPkg\Platform\GByteH77

  EDK_GLOBAL	PROJECT_PCH_FAMILY 		= IntelPch
  EDK_GLOBAL	PROJECT_PCH 			= LynxPoint
  EDK_GLOBAL	CHIPSET_PCH_CONTROLLER 		= $(PROJECT_PCH) 
  EDK_GLOBAL	PROJECT_PCH_ROOT 		= BIOSPkg\Intel\$(CHIPSET_PCH_CONTROLLER)

  EDK_GLOBAL	PROJECT_CPU_ROOT 		= BIOSPkg\Intel\Cpu

  EDK_GLOBAL	PROJECT_ME_ROOT 		= BIOSPkg\Intel\Me

  EDK_GLOBAL	PROJECT_ACPI_ROOT 		= BIOSPkg\Intel\AcpiTables

  EDK_GLOBAL	PLATFORM_SI_PACKAGE		= SkyLakePkg\Intel
 
  EDK_GLOBAL	EDK_PREFIX	 		= EdkCompatibilityPkg\

  EDK_GLOBAL	PROJECT_FAMILY 			= IntelMpg
  EDK_GLOBAL	PROJECT_SA_FAMILY 		= IntelSa
  EDK_GLOBAL	PROJECT_SA_ROOT 		= BIOSPkg\Intel\SystemAgent
  EDK_GLOBAL	PROJECT_SA_MRC			= MemoryInit

  EDK_GLOBAL	GENACPITABLE 			= $(EFI_SOURCE)\$(PROJECT_SA_ROOT)\SampleCode\Tools\GENACPITABLE

  EDK_GLOBAL	PROJECT_PLATFORM		= BIOSPkg/Platform
  EDK_GLOBAL	PROJECT_PLATFORM2		= BIOSPkg/Platform/GByteH77
  EDK_GLOBAL	PROJECT_INTEL			= BIOSPkg/Intel



  #
  # Defines for default states.  These can be changed on the command line.
  # -D FLAG=VALUE
  #
  DEFINE SECURE_BOOT_ENABLE      = FALSE

  DEFINE MSFT_MACRO_VERSION	= /D EFI_SPECIFICATION_VERSION=0x0002000A /D PI_SPECIFICATION_VERSION=0x0001000A /D TIANO_RELEASE_VERSION=0x00080006

  DEFINE MSFT_MACRO_PCH		= /D TRAD_FLAG  /D PCD_EDKII_GLUE_PciExpressBaseAddress=0xF8000000 /D$(PROJECT_PCH) /D PROJECT_PCH_ROOT=$(PROJECT_PCH_ROOT) 

  DEFINE MSFT_MACRO_PROJECT	= /D PROJECT_ME_ROOT=$(PROJECT_ME_ROOT) /D PROJECT_CPU_ROOT=$(PROJECT_CPU_ROOT) /D PROJECT_FAMILY=$(PROJECT_FAMILY)
  DEFINE MSFT_MACRO_SA		= /DME_SUPPORT_FLAG /DULT_FLAG /DTRAD_FLAG /DPEG_FLAG /DDMI_FLAG /D PROJECT_SA_ROOT=$(PROJECT_SA_ROOT) /D PROJECT_SA_FAMILY=$(PROJECT_SA_FAMILY)
  DEFINE MSFT_MACRO_ACPI	= /D PROJECT_ACPI_ROOT=$(PROJECT_ACPI_ROOT)
  DEFINE MSFT_MACRO_PLATFORM    = /D GBYTEH77_BOARD /DAPPLICATION_ONLY		# platform define

!if $(TARGET) == RELEASE
  DEFINE MSFT_MACRO		= $(MSFT_MACRO_VERSION) $(MSFT_MACRO_PCH) $(MSFT_MACRO_PROJECT) $(MSFT_MACRO_SA) $(MSFT_MACRO_ACPI) $(MSFT_MACRO_PLATFORM)
!else
  DEFINE MSFT_MACRO		= $(MSFT_MACRO_VERSION) $(MSFT_MACRO_PCH) $(MSFT_MACRO_PROJECT) $(MSFT_MACRO_SA) $(MSFT_MACRO_ACPI) $(MSFT_MACRO_PLATFORM) /D EFI_DEBUG
!endif


  DEFINE	PROJECT_PLATFORM		= $(EFI_SOURCE)/BIOSPkg/Platform

  
################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this
#                              Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT              # The entry: 0|DEFAULT is reserved and always required.


################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# CommonPkg
#
[LibraryClasses]
  PlatformDebugLib|CommonPkg/Library/PlatformDebugLib/PlatformDebugLib.inf
  DebugLib|CommonPkg/Library/RedirectDebugLib/RedirectDebugLib.inf
  CommonUtilsLib|CommonPkg/Library/CommonUtilsLib/CommonUtilsLib.inf
  FsUtilsLib|CommonPkg/Library/FsUtilsLib/FsUtilsLib.inf
  XmlParserLib|CommonPkg/Library/XmlParserLib/XmlParserLib.inf
  BootMaintLib|CommonPkg/Library/BootMaintLib/BootMaintLib.inf
  DeviceManager|CommonPkg/Library/DeviceManager/DeviceManager.inf
  Md5Lib|CommonPkg/Library/Md5Lib/Md5Lib.inf
  MessagesLib|CommonPkg/Library/MessagesLib/MessagesLib.inf
  FileDescInitLib|CommonPkg/Library/StdLibSupport/FileDescInit.inf
  ShellLib|CommonPkg/Library/StdLibSupport/UefiShellLib.inf
  SortLib|CommonPkg/Library/StdLibSupport/BaseSortLib.inf
  PathLib|CommonPkg/Library/StdLibSupport/BasePathLib.inf
  CircBufLib|CommonPkg/Library/CircBufLib/CircBufLib.inf
  HttpParserLib|CommonPkg/Library/HttpParserLib/HttpParserLib.inf
  HttpQueryLib|CommonPkg/Library/HttpQueryLib/HttpQueryLib.inf
  RoXmlLib|CommonPkg/Library/RoXmlLib/RoXmlLib.inf
  FswLib|CommonPkg/Library/FswLib/FswLib.inf

  PchSpiCommonLib|$(PLATFORM_SI_PACKAGE)/Pch/LibraryPrivate/BasePchSpiCommonLib/BasePchSpiCommonLib2.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  ShellCEntryLib|CommonPkg/Library/StdLibSupport/UefiShellCEntryLib.inf
[LibraryClasses.common.DXE_DRIVER]
  ShellCEntryLib|CommonPkg/Library/StdLibSupport/UefiShellCEntryLib.inf
[LibraryClasses.common.UEFI_APPLICATION]
  ShellCEntryLib|CommonPkg/Library/StdLibSupport/UefiShellCEntryLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# CryptoPkg
#
[LibraryClasses]
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
[LibraryClasses.common.PEIM]
!if $(SECURE_BOOT_ENABLE) == TRUE  
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/PeiCryptLib.inf
!endif
[LibraryClasses.common.DXE_DRIVER]
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# BIOSPkg
#
[LibraryClasses]
  TimerLib|BIOSPkg/Platform/Library/AcpiTimerLib/AcpiTimerLib.inf
  PlatformBdsLib|BIOSPkg/Platform/Library/PlatformBdsLib/PlatformBdsLib.inf  
  PlatformGuidNamesLib|BIOSPkg/Platform/Library/PlatformGuidNamesLib/PlatformGuidNamesLib.inf
[LibraryClasses.common]
  PciPlatformLib|BIOSPkg/Platform/Library/PciPlatformLib/PciPlatformLib.inf 
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# IA32FamilyCpuPkg
#
[LibraryClasses]
  CpuConfigLib|IA32FamilyCpuPkg/Library/CpuConfigLib/CpuConfigLib.inf
  SmmCpuPlatformHookLib|IA32FamilyCpuPkg/Library/SmmCpuPlatformHookLibNull/SmmCpuPlatformHookLibNull.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#

  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# IntelFrameworkModulePkg
#
[LibraryClasses]
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  GenericBdsLib|IntelFrameworkModulePkg/Library/GenericBdsLib/GenericBdsLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# IntelFrameworkPkg
#
[LibraryClasses.common.DXE_SMM_DRIVER]
  DxeSmmDriverEntryPoint|IntelFrameworkPkg/Library/DxeSmmDriverEntryPoint/DxeSmmDriverEntryPoint.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# MdePkg
#
[LibraryClasses]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
#  DevicePathLib|MdePkg/Library/UefiDevicePathLibDevicePathProtocol/UefiDevicePathLibDevicePathProtocol.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  SmmLib|MdePkg/Library/SmmLibNull/SmmLibNull.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PostCodeLib|MdePkg/Library/BasePostCodeLibPort80/BasePostCodeLibPort80.inf	#++
[LibraryClasses.common.USER_DEFINED]
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
[LibraryClasses.common.PEIM, LibraryClasses.common.PEI_CORE]
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptPei/BaseMemoryLibOptPei.inf
  IoLib|MdePkg/Library/PeiIoLibCpuIo/PeiIoLibCpuIo.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf
[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
[LibraryClasses.common.PEIM]
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
[LibraryClasses.common]
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  SmbusLib|MdePkg/Library/DxeSmbusLib/DxeSmbusLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
[LibraryClasses.common.DXE_CORE]
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
[LibraryClasses.common.SMM_CORE]
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
[LibraryClasses.common.DXE_SMM_DRIVER]
  SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  MemoryAllocationLib|MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf
[LibraryClasses.common.UEFI_DRIVER]
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf 
[LibraryClasses.common.DXE_DRIVER]
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
[LibraryClasses.common.UEFI_APPLICATION]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# MdeModulePkg
#
[LibraryClasses]
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
  IpIoLib|MdeModulePkg/Library/DxeIpIoLib/DxeIpIoLib.inf
  UdpIoLib|MdeModulePkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
  DpcLib|MdeModulePkg/Library/DxeDpcLib/DxeDpcLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  DebugPrintErrorLevelLib|MdeModulePkg/Library/DxeDebugPrintErrorLevelLib/DxeDebugPrintErrorLevelLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
  LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
[LibraryClasses.common.USER_DEFINED]
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
[LibraryClasses.common.PEIM, LibraryClasses.common.PEI_CORE]
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
[LibraryClasses.common.PEI_CORE]
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
[LibraryClasses.common.PEIM]
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
[LibraryClasses.common]
  S3BootScriptLib|MdeModulePkg/Library/PiDxeS3BootScriptLib/DxeS3BootScriptLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
[LibraryClasses.common.DXE_CORE]
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
[LibraryClasses.common.SMM_CORE]
  MemoryAllocationLib|MdeModulePkg/Library/PiSmmCoreMemoryAllocationLib/PiSmmCoreMemoryAllocationLib.inf
  SmmServicesTableLib|MdeModulePkg/Library/PiSmmCoreSmmServicesTableLib/PiSmmCoreSmmServicesTableLib.inf
  SmmCorePlatformHookLib|MdeModulePkg/Library/SmmCorePlatformHookLibNull/SmmCorePlatformHookLibNull.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# ApplicationPkg
#
[LibraryClasses]
  ChipsetConfigLib|ApplicationPkg/Library/ChipsetConfigLib/ChipsetConfig.inf
  UserManagerLib|ApplicationPkg/Library/UserManagerLib/UserManagerLib.inf
  TokenViewerLib|ApplicationPkg/Library/TokenViewerLib/TokenViewerLib.inf
  NetSetupLib|ApplicationPkg/Library/NetSetupLib/NetSetup.inf
  CertViewerLib|ApplicationPkg/Library/CertViewerLib/CertViewerLib.inf
  ApplicationDescUtilsLib|ApplicationPkg/Library/ApplicationDescUtilsLib/ApplicationDescUtilsLib.inf
  CertStorageLib|ApplicationPkg/Library/CertStorageLib/CertStorageUtilsLib.inf
  LdapInterfaceLib|ApplicationPkg/Library/LdapInterfaceLib/LdapInterfaceLib.inf
  FwUpdateLib|ApplicationPkg/Library/FwUpdateLib/FwUpdateLib.inf  
  Base32Lib|ApplicationPkg/Library/Base32Lib/Base32Lib.inf
  Base64Lib|ApplicationPkg/Library/Base64Lib/Base64Lib.inf
  ExtHdrUtilsLib|ApplicationPkg/Library/ExtHdrUtilsLib/ExtHdrUtilsLib.inf
  ShaLib|ApplicationPkg/Library/ShaLib/ShaLib.inf
  GostHashLib|ApplicationPkg/Library/GostHashLib/GostHashLib.inf
  Crc32Lib|ApplicationPkg/Library/Crc32Lib/Crc32Lib.inf
  FeLib|ApplicationPkg/Library/FeLib/FeLib.inf
  VfrCommonLib|ApplicationPkg/Library/VfrCommonLib/VfrCommonLib.inf
!ifdef MB_LITE
  BootMngrLib|ApplicationPkg/Library/BootMngrLiteLib/BootMngrLib.inf
  VarStorageUtilsLib|ApplicationPkg/Library/VarStorageUtilsLiteLib/VarStorageUtilsLib.inf
!else
  BootMngrLib|ApplicationPkg/Library/BootMngrLib/BootMngrLib.inf
  VarStorageUtilsLib|ApplicationPkg/Library/VarStorageUtilsLib/VarStorageUtilsLib.inf
!endif # MB_LITE
[LibraryClasses.common.DXE_DRIVER]
  PciDevsMonitorLib|ApplicationPkg/Library/PciDevsMonitorLib/PciDevsMonitorLib.inf
  RevokeChkConfigLib|ApplicationPkg/Library/RevokeChkConfigLib/RevokeChkConfig.inf
  Pkcs11Lib|ApplicationPkg/Library/Pkcs11Lib/Pkcs11Lib.inf
  PciDevsDescLib|ApplicationPkg/Library/PciDevsDescLib/PciDevsDescLib.inf
  DiagnosticsConfigLib|ApplicationPkg/Library/DiagnosticsConfigLib/DiagnosticsConfig.inf
  CDPSupportLib|ApplicationPkg/Library/CDPSupportLib/CDPSupport.inf
  AuthModeConfig|ApplicationPkg/Library/AuthModeConfig/AuthModeConfig.inf
  FaultToleranceLib|ApplicationPkg/Library/FaultToleranceLib/FaultToleranceLib.inf
[LibraryClasses.common.UEFI_DRIVER]
  CcidLib|ApplicationPkg/Library/CcidLib/CcidLib.inf
[LibraryClasses.common.UEFI_APPLICATION]
  Pkcs11Lib|ApplicationPkg/Library/Pkcs11Lib/Pkcs11Lib.inf
  FaultToleranceLib|ApplicationPkg/Library/FaultToleranceLib/FaultToleranceLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# OpenLDAP
#
[LibraryClasses.common.DXE_DRIVER]
  LdapCommon|OpenLDAP/Library/LdapCommon/LdapCommon.inf
  Libldap|OpenLDAP/Library/libldap/libldap_dxe.inf
  Liblber|OpenLDAP/Library/liblber/liblber.inf
  Liblutil|OpenLDAP/Library/liblutil/liblutil.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#

  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# Replacement
#
[LibraryClasses]
  BaseLib|Replacement/MdePkg/Library/BaseLib/BaseLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# ShellPkg
#
[LibraryClasses]
  FileHandleLib|ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# UefiCpuPkg
#
[LibraryClasses]
  MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  LocalApicLib|UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLib.inf
  UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf
[LibraryClasses.common.PEIM, LibraryClasses.common.PEI_CORE]
  UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_PCH_ROOT)
#
[LibraryClasses.common.PEIM, LibraryClasses.common.PEI_CORE]
  PchPlatformLib|$(PROJECT_PCH_ROOT)/Library/PchPlatformLib/PchPlatformLib.inf
  PchPciExpressHelpersLib|$(PROJECT_PCH_ROOT)/Library/PchPciExpressHelpersLib/PchPciExpressHelpersLib.inf
  PchSmbusLibPei|$(PROJECT_PCH_ROOT)/Library/PchSmbusLib/Pei/PchSmbusLibPei.inf
  PchGuidLib|$(PROJECT_PCH_ROOT)/Guid/PchGuidLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
  
  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_PLATFORM2)
#
[LibraryClasses]
  PlatformDataLib|$(PROJECT_PLATFORM2)/PlatformDataLib/PlatformDataLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#



[BuildOptions]
 
  MSFT:*_*_IA32_CC_FLAGS    = /D EFI32 $(MSFT_MACRO)
  MSFT:*_*_IA32_ASM_FLAGS   = /DEFI32
  MSFT:*_*_IA32_VFRPP_FLAGS = /D EFI32 $(MSFT_MACRO)
  MSFT:*_*_IA32_APP_FLAGS   = /D EFI32 $(MSFT_MACRO)
  MSFT:*_*_IA32_PP_FLAGS    = /D EFI32 $(MSFT_MACRO)

  MSFT:*_*_X64_CC_FLAGS     = /D EFIX64 $(MSFT_MACRO)
  MSFT:*_*_X64_ASM_FLAGS    = /DEFIX64 /Fl
  MSFT:*_*_X64_VFRPP_FLAGS  = /D EFIX64 $(MSFT_MACRO)
  MSFT:*_*_X64_APP_FLAGS    = /D EFIX64 $(MSFT_MACRO)
  MSFT:*_*_X64_PP_FLAGS     = /D EFIX64 $(MSFT_MACRO)

  MSFT:*_*_IPF_CC_FLAGS     = /D EFI64 $(MSFT_MACRO)
  MSFT:*_*_IPF_ASM_FLAGS    =
  MSFT:*_*_IPF_VFRPP_FLAGS  = /D EFI64 $(MSFT_MACRO)
  MSFT:*_*_IPF_APP_FLAGS    = /D EFI64 $(MSFT_MACRO)
  MSFT:*_*_IPF_PP_FLAGS     = /D EFI64 $(MSFT_MACRO)



################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSupportUefiDecompress|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSwitchToLongMode|TRUE				# PEI-32 -> DXE-64
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplBuildPageTables|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreImageLoaderSearchTeSectionFirst|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableCollectStatistics|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdFrameworkCompatibilitySupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciBridgeIoAlignmentProbe|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE


  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutUgaSupport|FALSE

[PcdsFeatureFlag.x64]
  gEfiCommonPkgTokenSpaceGuid.UseGop|TRUE

  


[PcdsFixedAtBuild]

  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreMaxFvSupported|8
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreMaxPpiSupported|80

  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxSizeNonPopulateCapsule|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxSizePopulateCapsule|0x0
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000040
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x1f
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x0f
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE

  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreMaxPeimPerFv|64

  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLangCodes|"en;fr;ru;en-US;fr-FR;ru-RU;"

   gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBiosVideoCheckVgaEnable|FALSE
   gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBiosVideoCheckVbeEnable|TRUE

  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBiosVideoSetTextVgaModeEnable|TRUE




  gEfiPlatformPkgTokenSpaceGuid.BootableMassStorageController|0x00001f02		# Primary SATA controller

 
!if $(SECURE_BOOT_ENABLE) == TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000
!endif

!if $(SECURE_BOOT_ENABLE) == TRUE
  # override the default values from SecurityPkg to ensure images from all sources are verified in secure boot
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif

[PcdsFixedAtBuild.x64]
  gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE



################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsDynamicDefault.common.DEFAULT]

  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|25
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|80

  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|800
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|600

  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|800
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|600


[PcdsDynamicHii.common.DEFAULT]
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPlatformBootTimeOut|L"Timeout"|gEfiGlobalVariableGuid|0x0|10
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdHardwareErrorRecordLevel|L"HwErrRecSupport"|gEfiGlobalVariableGuid|0x0|1

  
###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################



#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# BIOSPkg
#

[Components.X64]
  BIOSPkg/Applications/BiosWriter.inf		# приложение: EFI-загрузчик 
  BIOSPkg/Applications/BiosWriterGByteH77.inf	# приложение: EFI-загрузчик 
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#






#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# MdeModulePkg
#
[Components.IA32]
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf

[Components.X64]
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


###################################################################################################
#
# BuildOptions Section - Define the module specific tool chain flags that should be used as
#                        the default flags for a module. These flags are appended to any 
#                        standard flags that are defined by the build process. They can be 
#                        applied for any modules or only those modules with the specific 
#                        module style (EDK or EDKII) specified in [Components] section.
#
###################################################################################################
[BuildOptions]
  DEBUG_*_*_DLINK_FLAGS = /EXPORT:InitializeDriver=$(IMAGE_ENTRY_POINT) /BASE:0x10000 /ALIGN:4096 /FILEALIGN:4096 /SUBSYSTEM:CONSOLE
#  RELEASE_*_*_DLINK_FLAGS = /ALIGN:4096 /FILEALIGN:4096

# Add override here, because default X64_CC_FLAGS add /X
#  DEBUG_*_X64_CC_FLAGS     == /nologo /c /WX /GS- /W4 /Gs32768 /D UNICODE /O1ib2s /GL /Gy /FIAutoGen.h /EHs-c- /GR- /GF /Zi /Gm 
#  RELEASE_*_X64_CC_FLAGS     == /nologo /c /WX /GS- /W4 /Gs32768 /D UNICODE /O1ib2s /GL /Gy /FIAutoGen.h /EHs-c- /GR- /GF 
#  NOOPT_*_X64_CC_FLAGS       == /nologo /c /WX /GS- /W4 /Gs32768 /D UNICODE /Gy /FIAutoGen.h /EHs-c- /GR- /GF /Zi /Gm /Od 

#############################################################################################################
# NOTE:
# The following [Libraries] section is for building EDK module under the EDKII tool chain.
# If you want build EDK module for Nt32 platform, please uncomment [Libraries] section and
# libraries used by that EDK module.
# Currently, Nt32 platform do not has any EDK style module
#
#

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# CommonPkg
#
[Libraries.IA32]
  CommonPkg/Library/PlatformDebugLib/PlatformDebugLib.inf
  CommonPkg/Library/RedirectDebugLib/EdkIIGluePeiRedirectDebugLib.inf
[Libraries.X64]
  CommonPkg/Library/PlatformDebugLib/PlatformDebugLib.inf
  CommonPkg/Library/RedirectDebugLib/EdkIIGlueDxeRedirectDebugLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# EdkCompatibilityPkg
#
[Libraries]
  !include EdkCompatibilityPkg\Sample\Platform\EdkLibAll.dsc
[Libraries.IA32]
  !include EdkCompatibilityPkg\Sample\Platform\EdkIIGlueLib32.dsc
[Libraries.X64]
  !include EdkCompatibilityPkg\Sample\Platform\EdkIIGlueLibAll.dsc
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# BIOSPkg
#
[Libraries.X64]
  BIOSPkg/Intel/AcpiTables/SampleCode/Library/PlatformAcpiLib/AcpiGnvsInitLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# MdePkg
#
[Libraries.IA32]
  MdePkg/Library/BaseLib/BaseLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_ACPI_ROOT)
#
[Libraries]
  $(PROJECT_ACPI_ROOT)\Protocol\AcpiProtocolLib.inf
  $(PROJECT_ACPI_ROOT)\Dptf\Guid\DptfGuidLib.inf
  $(PROJECT_ACPI_ROOT)\Cppc\Guid\CppcGuidLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#

  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_CPU_ROOT)
#
[Libraries.IA32]
  !include $(PROJECT_CPU_ROOT)\Include\IntelCpuPeiLib.dsc
[Libraries.X64]
  !include $(PROJECT_CPU_ROOT)\Include\IntelCpuDxeLib.dsc
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#

  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_ME_ROOT)
#
[Libraries.IA32]
  !include $(PROJECT_ME_ROOT)\Include\MePeiLib.dsc
[Libraries.X64]
  !include $(PROJECT_ME_ROOT)\Include\MeDxeLib.dsc
  !include $(PROJECT_ME_ROOT)\SampleCode\Include\MeDxeLibSampleCode.dsc
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#

  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_PCH_ROOT)
#
[Libraries.IA32]
  !include $(PROJECT_PCH_ROOT)\Include\IntelPchPeiLib.dsc
[Libraries.X64]
  !include $(PROJECT_PCH_ROOT)\Include\IntelPchDxeLib.dsc
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_PLATFORM)
#
[Libraries.IA32]
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/CpuPolicyUpdate/Pei/CpuPolicyUpdatePeiLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/PchPolicyUpdate/Pei/PchPolicyUpdatePeiLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/SaPolicyUpdate/Pei/SaPolicyUpdatePeiLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/MePolicyUpdate/Pei/MePolicyUpdatePeiLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/MePolicyUpdate/Pei/AmtPolicyUpdatePeiLib.inf
[Libraries.X64]
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/CpuPolicyUpdate/Dxe/CpuPolicyUpdateDxeLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/PchPolicyUpdate/Dxe/PchPolicyUpdateDxeLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/SaPolicyUpdate/Dxe/SaPolicyUpdateDxeLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/MePolicyUpdate/Dxe/MePolicyUpdateDxeLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/MePolicyUpdate/Dxe/AmtPolicyUpdateDxeLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/MePolicyUpdate/Dxe/AtPolicyUpdateDxeLib.inf
  $(PROJECT_PLATFORM)/Library/PolicyUpdate/AcpiPolicyUpdate/Dxe/AcpiPolicyUpdateDxeLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_PLATFORM2)
#
[Libraries.IA32]
  $(PROJECT_PLATFORM2)/PlatformDataLib/PlatformDataLib.inf
[Libraries.X64]
  $(PROJECT_PLATFORM2)/PlatformDataLib/PlatformDataLib.inf
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#

  
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#
# $(PROJECT_SA_ROOT)
#
[Libraries.IA32]
  !include $(PROJECT_SA_ROOT)\Include\IntelSaPeiLib.dsc
[Libraries.X64]
  !include $(PROJECT_SA_ROOT)\Include\IntelSaDxeLib.dsc
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#


!include StdLib/StdLib.inc
