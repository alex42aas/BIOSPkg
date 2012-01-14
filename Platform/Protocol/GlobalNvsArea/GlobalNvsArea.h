/** @file
  Definition of the global NVS area protocol.  This protocol
  publishes the address and format of a global ACPI NVS buffer used as a communications
  buffer between SMM code and ASL code.
  The format is derived from the ACPI reference code, version 0.95.

  Note:  Data structures defined in this protocol are not naturally aligned.

@copyright
  Copyright (c) 2011 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
**/
#ifndef _GLOBAL_NVS_AREA_H_
#define _GLOBAL_NVS_AREA_H_

///
/// Includes
///
#define GLOBAL_NVS_DEVICE_ENABLE  1
#define GLOBAL_NVS_DEVICE_DISABLE 0

///
/// Forward reference for pure ANSI compatability
///
//EFI_FORWARD_DECLARATION (EFI_GLOBAL_NVS_AREA_PROTOCOL);

///
/// Global NVS Area Protocol GUID
///
#define EFI_GLOBAL_NVS_AREA_PROTOCOL_GUID \
  { \
    0x74e1e48, 0x8132, 0x47a1, 0x8c, 0x2c, 0x3f, 0x14, 0xad, 0x9a, 0x66, 0xdc \
  }
///
/// Revision id - Added TPM related fields
///
#define GLOBAL_NVS_AREA_REVISION_1  1
///
/// Extern the GUID for protocol users.
///
extern EFI_GUID gEfiGlobalNvsAreaProtocolGuid;

///
/// Global NVS Area definition
///
#pragma pack(1)
typedef struct {
  ///
  /// Miscellaneous Dynamic Values, the definitions below need to be matched
  /// GNVS definitions in Platform.ASL
  ///
  UINT16 OperatingSystem;  ///< 00
  UINT8  SmiFunction;      ///< 02   SMI function call via IO Trap
  UINT8  SmiParameter0;    ///< 03
  UINT8  SmiParameter1;    ///< 04
  UINT8  SciFunction;      ///< 05   SCI function call via _L00
  UINT8  SciParameter0;    ///< 06
  UINT8  SciParameter1;    ///< 07
  UINT8  GlobalLock;       ///< 08   Global lock function call
  UINT8  LockParameter0;   ///< 09
  UINT8  LockParameter1;   ///< 10
  UINT32 Port80DebugValue; ///< 11
  UINT8  PowerState;       ///< 15   AC = 1
  UINT8  DebugState;       ///< 16
  ///
  /// Thermal Policy Values
  ///
  UINT8 Reserved;                           ///< 17
  UINT8 Ac1TripPoint;                       ///< 18
  UINT8 Ac0TripPoint;                       ///< 19
  UINT8 PassiveThermalTripPoint;            ///< 20
  UINT8 PassiveTc1Value;                    ///< 21
  UINT8 PassiveTc2Value;                    ///< 22
  UINT8 PassiveTspValue;                    ///< 23
  UINT8 CriticalThermalTripPoint;           ///< 24
  UINT8 EnableDigitalThermalSensor;         ///< 25   DTS Function enable
  UINT8 BspDigitalThermalSensorTemperature; ///< 26   Temperature of BSP
  UINT8 ApDigitalThermalSensorTemperature;  ///< 27   Temperature of AP
  UINT8 DigitalThermalSensorSmiFunction;    ///< 28   SMI function call via DTS IO Trap
  ///
  /// Battery Support Values
  ///
  UINT8 reserved2;
  UINT8 NumberOfBatteries; ///< 30
  UINT8 BatteryCapacity0;  ///< 31   Battery 0 Stored Capacity
  UINT8 BatteryCapacity1;  ///< 32   Battery 1 Stored Capacity
  UINT8 BatteryCapacity2;  ///< 33   Battery 2 Stored Capacity
  UINT8 BatteryStatus0;    ///< 34   Battery 0 Stored Status
  UINT8 BatteryStatus1;    ///< 35   Battery 1 Stored Status
  UINT8 BatteryStatus2;    ///< 36   Battery 2 Stored Status
  /// NOTE: Do NOT Change the Offset of Revision Field
  ///
  UINT8 Revision;     ///< 37   Revision of the structure EFI_GLOBAL_NVS_AREA
  UINT8 Reserved3[2]; ///< 38:39
  ///
  /// Processor Configuration Values
  ///
  UINT8  ApicEnable;      ///< 40   APIC Enabled by SBIOS (APIC Enabled = 1)
  UINT8  ThreadCount;     ///< 41   Number of Enabled Threads
  UINT8  CurentPdcState0; ///< 42   PDC settings, Processor 0
  UINT8  CurentPdcState1; ///< 43   PDC settings, Processor 1
  UINT8  MaximumPpcState; ///< 44   Maximum PPC state
  UINT32 PpmFlags;        ///< 45:48 PPM configuration flags, same as CFGD
  UINT8  C6C7Latency;     ///< 49 C6/C7 Entry/Exit latency
  ///
  /// SIO Configuration Values
  ///
  UINT8 DockedSioPresent; ///< 50   Dock SIO Present
  UINT8 DockComA;         ///< 51     COM A Port
  UINT8 DockComB;         ///< 52     COM B Port
  UINT8 DockLpt;          ///< 53     LPT Port
  UINT8 DockFdc;          ///< 54     FDC Port
  UINT8 OnboardCom;       ///< 55   Onboard COM Port
  UINT8 OnboardComCir;    ///< 56   Onboard COM CIR Port
  UINT8 SMSC1007;         ///< 57
  UINT8 WPCN381U;         ///< 58
  UINT8 SMSC1000;         ///< 59
  /*
  ///
  /// Internal Graphics Device Values
  ///
  UINT8  IgdState;              ///< 60   IGD State (Primary Display = 1)
  UINT8  DisplayToggleList;     ///< 61   Display Toggle List Selection
  UINT8  CurrentDeviceList;     ///< 62   Current Attached Device List
  UINT8  PreviousDeviceList;    ///< 63   Previous Attached Device List
  UINT16 CurrentDisplayState;   ///< 64   Current Display State
  UINT16 NextDisplayState;      ///< 66   Next Display State
  UINT16 SetDisplayState;       ///< 68   Set Display State
  UINT8  NumberOfValidDeviceId; ///< 70   Number of Valid Device IDs
  UINT32 DeviceId1;             ///< 71   Device ID 1
  UINT32 DeviceId2;             ///< 75   Device ID 2
  UINT32 DeviceId3;             ///< 79   Device ID 3
  UINT32 DeviceId4;             ///< 83   Device ID 4
  UINT32 DeviceId5;             ///< 87   Device ID 5
  UINT32 AKsv0;                 ///< 91:94 First four bytes of AKSV (manufacturing mode)
  UINT8  AKsv1;                 ///< 95    Fifth byte of AKSV (manufacturing mode
  UINT8  Reserved6[7];          ///< 96:102
  ///
  /// Backlight Control Values
  ///
  UINT8 BacklightControlSupport; ///< 103  Backlight Control Support
  UINT8 BrightnessPercentage;    ///< 104  Brightness Level Percentage
  ///
  /// Ambient Light Sensor Values
  ///
  UINT8 AlsEnable;           ///< 105  Ambient Light Sensor Enable
  UINT8 AlsAdjustmentFactor; ///< 106  Ambient Light Adjusment Factor
  UINT8 LuxLowValue;         ///< 107  LUX Low Value
  UINT8 LuxHighValue;        ///< 108  LUX High Value
  UINT8 Reserved7[1];        ///< 109
  ///
  /// Extended Mobile Access Values
  ///
  UINT8  EmaEnable;    ///< 110  EMA Enable
  UINT16 EmaPointer;   ///< 111  EMA Pointer
  UINT16 EmaLength;    ///< 113  EMA Length
  UINT8  Reserved8[1]; ///< 115
  ///
  /// Mobile East Fork Values
  ///
  UINT8 MefEnable; ///< 116 Mobile East Fork Enable
  ///
  /// PCIe Dock Status
  ///
  UINT8 PcieDockStatus; ///< 117 PCIe Dock Status
  UINT8 Reserved9[4];   ///< 118:121
  ///
  /// TPM Registers
  ///
  UINT8  MorData;       ///< 122 Memory Overwrite Request Data
  UINT8  TcgParamter;   ///< 123 Used for save the Mor and/or physical presence paramter
  UINT32 PPResponse;    ///< 124 Physical Presence request operation response
  UINT8  PPRequest;     ///< 128 Physical Presence request operation
  UINT8  LastPPRequest; ///< 129 Last Physical Presence request operation
  ///
  /// SATA Values
  ///
  UINT8 GtfTaskFileBufferPort0[7]; ///< 130  GTF Task File Buffer for Port 0
  UINT8 GtfTaskFileBufferPort2[7]; ///< 137  GTF Task File Buffer for Port 2
  UINT8 IdeMode;                   ///< 144  IDE Mode (Compatible\Enhanced)
  UINT8 GtfTaskFileBufferPort1[7]; ///< 145:151 GTF Task File Buffer for Port 1
  ///
  /// Board Id
  /// This field is for the ASL code to know whether this board is Matanzas, or Oakmont, etc
  ///
  UINT16 BoardId;              ///< 152  Board Id
  UINT8  PlatformId;           ///< 154  Platform Id
  UINT8  Reserved10[7];        ///< 155:161
  UINT64 BootTimeLogAddress;   ///< 162:169 Boot Time Log Table Address
  UINT32 IgdOpRegionAddress;   ///< 170  IGD OpRegion Starting Address
  UINT8  IgdBootType;          ///< 174  IGD Boot Type CMOS option
  UINT8  IgdPanelType;         ///< 175  IGD Panel Type CMOs option
  UINT8  Reserved1;            ///< 176  Reserved
  UINT8  Reserved2;            ///< 177  Reserved
  UINT8  IgdPanelScaling;      ///< 178  IGD Panel Scaling
  UINT8  IgdBlcConfig;         ///< 179  IGD BLC Configuration
  UINT8  IgdBiaConfig;         ///< 180  IGD BIA Configuration
  UINT8  IgdSscConfig;         ///< 181  IGD SSC Configuration
  UINT8  Igd409;               ///< 182  IGD 0409 Modified Settings Flag
  UINT8  Igd509;               ///< 183  IGD 0509 Modified Settings Flag
  UINT8  Igd609;               ///< 184  IGD 0609 Modified Settings Flag
  UINT8  Igd709;               ///< 185  IGD 0709 Modified Settings Flag
  UINT8  IgdPowerConservation; ///< 186  IGD Power Conservation Feature Flag
  UINT8  IgdDvmtMemSize;       ///< 187  IGD DVMT Memory Size
  UINT8  IgdFunc1Enable;       ///< 188  IGD Function 1 Enable
  UINT8  IgdHpllVco;           ///< 189  HPLL VCO
  UINT32 NextStateDid1;        ///< 190  Next state DID1 for _DGS
  UINT32 NextStateDid2;        ///< 194  Next state DID2 for _DGS
  UINT32 NextStateDid3;        ///< 198  Next state DID3 for _DGS
  UINT32 NextStateDid4;        ///< 202  Next state DID4 for _DGS
  UINT32 NextStateDid5;        ///< 206  Next state DID5 for _DGS
  UINT32 NextStateDid6;        ///< 210  Next state DID6 for _DGS
  UINT32 NextStateDid7;        ///< 214  Next state DID7 for _DGS
  UINT32 NextStateDid8;        ///< 218  Next state DID8 for _DGS
  UINT8  IgdSciSmiMode;        ///< 222  GMCH SMI/SCI mode (0=SCI)
  UINT8  IgdPAVP;              ///< 223  IGD PAVP data
  UINT8  Reserved13;           ///< 224  Reserved
  UINT8  PcieOSCControl;       ///< 225  PCIE OSC Control
  UINT8  NativePCIESupport;    ///< 226  Native PCI Express Support
  ///
  /// USB Sideband Deferring Support
  ///
  UINT8  HostAlertVector1;           ///< 227  GPE vector used for HOST_ALERT#1
  UINT8  HostAlertVector2;           ///< 228  GPE vector used for HOST_ALERT#2
  UINT8  Reserved11[7];              ///< 229
  UINT8  EcAvailable;                ///< 236 Embedded Controller Availability Flag.
  UINT8  Reserved12[19];             ///< 237
  UINT32 NvIgOpRegionAddress;        ///< 256 NVIG support
  UINT32 NvHmOpRegionAddress;        ///< 260 NVHM support
  UINT32 ApXmOpRegionAddress;        ///< 264 AMDA support
  UINT32 DeviceId6;                  ///< 268   Device ID 6
  UINT32 DeviceId7;                  ///< 272   Device ID 7
  UINT32 DeviceId8;                  ///< 276   Device ID 8
  UINT32 EndpointBaseAddress;        ///< 280 PEG Endpoint PCIe Base Address
  UINT32 CapStrPresence;             ///< 284 PEG Endpoint Capability Structure Presence
  UINT32 EndpointPcieCapBaseAddress; ///< 288 PEG Endpoint PCIe Capability Structure Base Address
  UINT32 EndpointVcCapBaseAddress;   ///< 292 PEG Endpoint Virtual Channel Capability Structure Base Address
  UINT32 XPcieCfgBaseAddress;        ///< 296 Any Device's PCIe Config Space Base Address
  UINT32 OccupiedBuses1;             ///< 300 Occupied Buses from 0 to 31
  UINT32 OccupiedBuses2;             ///< 304 Occupied Buses from 32 to 63
  UINT32 OccupiedBuses3;             ///< 308 Occupied Buses from 64 to 95
  UINT32 OccupiedBuses4;             ///< 312 Occupied Buses from 96 to 127
  UINT32 OccupiedBuses5;             ///< 316 Occupied Buses from 128 to 159
  UINT32 OccupiedBuses6;             ///< 320 Occupied Buses from 160 to 191
  UINT32 OccupiedBuses7;             ///< 324 Occupied Buses from 192 to 223
  UINT32 OccupiedBuses8;             ///< 328 Occupied Buses from 224 to 255
  UINT8  Reserved14[11];             ///< reserve offsets 332 - 342
  ///
  /// Thermal
  ///
  UINT8  ActiveThermalTripPointMCH;   ///< 343 Active Trip Point for MCH
  UINT8  PassiveThermalTripPointMCH;  ///< 344 Passive Trip Point for MCH
  UINT8  ActiveThermalTripPointTMEM;  ///< 345 Active Trip Point for TMEM
  UINT8  PassiveThermalTripPointTMEM; ///< 346 Passive Trip Point for TMEM
  UINT32 PlatformCpuId;               ///< 347   Device ID 8
  UINT32 TBARB;                       ///< 351 Thermal BAR for BIOS, TBAB in GloblNvs.asl
  UINT32 TBARBH;                      ///< 355 Thermal BAR for BIOS, TBAH in GloblNvs.asl
  UINT8  RunTimeInterface;            ///< 359 Run Time Interface for Intelligent Power Savings
  UINT8  TsOnDimmEnabled;             ///< 360 TS-on-DIMM is chosen in SETUP and present on the DIMM
  UINT8  ActiveThermalTripPointPCH;   ///< 361 Active Trip Point for PCH
  UINT8  PassiveThermalTripPointPCH;  ///< 362 Passive Trip Point for PCH
  ///
  /// Board info
  ///
  UINT8 PlatformFlavor; ///< 363 Platform Flavor
  UINT8 BoardRev;       ///< 364 Board Rev
  ///
  /// HG Board Info
  ///
  UINT8  SgMode;           ///< 365 SG Mode (0=Disabled, 1=SG Muxed, 2=SG Muxless, 3=DGPU Only)
  UINT8  SgFeatureList;    ///< 366 SG Feature list
  UINT8  SgDgpuPwrOK;      ///< 367 dGPU PWROK GPIO assigned
  UINT8  SgDgpuHoldRst;    ///< 368 dGPU HLD RST GPIO assigned
  UINT8  SgDgpuDisplaySel; ///< 369 dGPU Display Select GPIO assigned
  UINT8  SgDgpuEdidSel;    ///< 370 dGPU EDID Select GPIO assigned
  UINT8  SgDgpuPwmSel;     ///< 371 dGPU PWM Select GPIO assigned
  UINT8  SgDgpuPwrEnable;  ///< 372 dGPU PWR Enable GPIO assigned
  UINT8  SgDgpuPrsnt;      ///< 373 dGPU Present Detect GPIO assigned
  UINT32 SgMuxDid1;        ///< 374 DID1 Mux Setting
  UINT32 SgMuxDid2;        ///< 378 DID2 Mux Setting
  UINT32 SgMuxDid3;        ///< 382 DID3 Mux Setting
  UINT32 SgMuxDid4;        ///< 386 DID4 Mux Setting
  UINT32 SgMuxDid5;        ///< 390 DID5 Mux Setting
  UINT32 SgMuxDid6;        ///< 394 DID6 Mux Setting
  UINT32 SgMuxDid7;        ///< 398 DID7 Mux Setting
  UINT32 SgMuxDid8;        ///< 402 DID8 Mux Setting
  UINT16 GpioBaseAddress;  ///< 406 GPIO Base Address
  UINT8  SgGPIOSupport;    ///< 408 SG GPIO
  UINT8  Reserved15[4];    ///< 409:412
  /// Active LFP Value
  ///
  UINT8 ActiveLFP; ///< 413 Active LFP
  /// Graphics Turbo IMON Value
  ///
  UINT8 GfxTurboIMON; ///< 414 IMON Current Value
  
  /// Package temperature
  ///
  UINT8 PackageDTSTemperature;              ///< 415 Package temperature
  UINT8 IsPackageTempMSRAvailable;          ///< 416 Package Temperature MSR available
  UINT8 PeciAccessMethod;                   ///< 417 PECI Access Method (Direct I/O or ACPI)
  UINT8 Ac0FanSpeed;                        ///< 418 _AC0 Fan Speed
  UINT8 Ac1FanSpeed;                        ///< 419 _AC1 Fan Speed
  UINT8 Ap2DigitalThermalSensorTemperature; ///< 420   Temperature of the second AP
  UINT8 Ap3DigitalThermalSensorTemperature; ///< 421   Temperature of the third AP
  UINT8 Reserved16[4];                      ///< 422:425

  
  UINT8 LtrEnable[8];             // 426:433 Latency Tolerance Reporting Control
  UINT8 ObffEnable[8];            // 466:473 Optimized Buffer Flush and Fill


  UINT8 XhciMode;                     // 484 xHCI mode
  /// XTU 3.0 Specification
  ///
  UINT32 XTUBaseAddress;              // 488 XTU Base Address
  UINT32 XTUSize;                     // 492 XTU Entries Size
  UINT32 XMPBaseAddress;              // 496 XTU Base Address
  UINT8  DDRReferenceFreq;            // 500 DDR Reference Frequency

  UINT8 Rtd3Support;                 // 496 Runtime D3 support.
  UINT8 Rtd3P0dl;                    // 497 User selctable Delay for Device D0 transition.
  UINT8 Rtd3P3dl;                    // 498 User selctable Delay for Device D0 transition.
  UINT32 DeviceId9;                  // 500    Device ID 9
  UINT32 DeviceId10;                 // 504   Device ID 10
  UINT32 DeviceId11;                 // 508   Device ID 11
  */
  UINT8	reserve4[122 - 60];
  UINT8 _DOSDisplaySupportFlag;			// 122 _DOS Display Support Flag.
  UINT8 EcAvailable;				// 123 Embedded Controller Availability Flag.
  UINT8 GlobalInterruptModeFlag;		// 124 Global IOAPIC/8259 Interrupt Mode Flag.
			
  UINT8	reserve41[143 - 125];
  UINT8 ActiveThermalTripPointSA;		// 143
  UINT8 PassiveThermalTripPointSA;		// 144
  UINT8 ActiveThermalTripPointTMEM;		// 145
  UINT8 PassiveThermalTripPointTMEM;		// 146
/*
PNHM, 32,     //   (147) CPUID Feature Information [EAX]
  TBAB, 32,     //   (151) Thermal Base Low Address for BIOS
  TBAH, 32,     //   (155) Thermal Base High Address for BIOS
  RTIP, 8,      //   (159) Run Time Interface for Intelligent Power Savings
  TSOD, 8,      //   (160) TS-on-DIMM is chosen in SETUP and present on the DIMM
*/

  UINT8	reserve5[151 - 147];
  UINT32	TBARB;				// 151 Thermal Base Low Address for BIOS
  UINT32	TBARBH;				// 155 Thermal Base High Address for BIOS


  UINT8	reserve51[161 - 159];
  UINT8 ActiveThermalTripPointPCH;		// 161
  UINT8 PassiveThermalTripPointPCH;		// 162
  UINT8 PlatformFlavor;				// 163 Platform Flavor


  /// Package temperature
  ///
  /*
  UINT8 PackageDTSTemperature;              ///< 415 Package temperature
  UINT8 IsPackageTempMSRAvailable;          ///< 416 Package Temperature MSR available
  UINT8 PeciAccessMethod;                   ///< 417 PECI Access Method (Direct I/O or ACPI)
  UINT8 Ac0FanSpeed;                        ///< 418 _AC0 Fan Speed
  UINT8 Ac1FanSpeed;                        ///< 419 _AC1 Fan Speed
  UINT8 Ap2DigitalThermalSensorTemperature; ///< 420   Temperature of the second AP
  UINT8 Ap3DigitalThermalSensorTemperature; ///< 421   Temperature of the third AP
  */
  UINT8	reserve6;
  UINT8 PackageDTSTemperature;          // 165 Package temperature
  UINT8 IsPackageTempMSRAvailable;      // 166 Package Temperature MSR available
  UINT8 PeciAccessMethod;               // 167 PECI Access Method (Direct I/O or ACPI)
  UINT8 Ac0FanSpeed;			// 168 _AC0 Fan Speed
  UINT8 Ac1FanSpeed;			// 169 _AC1 Fan Speed
  UINT8 Ap2DigitalThermalSensorTemperature; // 170   Temperature of the second AP
  UINT8 Ap3DigitalThermalSensorTemperature; // 171   Temperature of the third AP


  //
  // DPTF Devices and trip points
  //
  UINT8	reserve7[209 - 172];
  UINT8	 EnableDptf;			// 209

  UINT8	 EnableSaDevice;		// 210
  UINT8	 CriticalThermalTripPointSA;	// 211
  UINT8	 HotThermalTripPointSA;		// 212

  UINT8	 EnablePchDevice;		// 213
  UINT8	 CriticalThermalTripPointPCH;	// 214
  UINT8	 HotThermalTripPointPCH;	// 215

    //
  // DPTF Policies
  //
  UINT8	 EnableCtdpPolicy;		// 216
  UINT8	 EnableLpmPolicy;		// 217
  UINT8	 CurrentLowPowerMode;		// 218 for LPM
  UINT8	 EnableCurrentExecutionUnit;	// 219
  UINT16 TargetGfxFreq;			// 220

  //
  // DPPM Devices and trip points
  //
  UINT8	 EnableMemoryDevice;		// 222
  UINT8	 CriticalThermalTripPointTMEM;	// 223
  UINT8	 HotThermalTripPointTMEM;	// 224

  UINT8	 EnableFan1Device;		// 225
  UINT8	 EnableFan2Device;		// 226
  
  UINT8	EnableAmbientDevice;		// 227
  UINT8	ActiveThermalTripPointAmbient;	// 228
  UINT8	PassiveThermalTripPointAmbient;	// 229
  UINT8	CriticalThermalTripPointAmbient;// 230
  UINT8	HotThermalTripPointAmbient;	// 231

  UINT8	EnableSkinDevice;		// 232
  UINT8	ActiveThermalTripPointSkin;	// 233
  UINT8	PassiveThermalTripPointSkin;	// 234
  UINT8	CriticalThermalTripPointSkin;	// 235
  UINT8	HotThermalTripPointSkin;	// 236

  UINT8	EnableExhaustFanDevice;			// 237
  UINT8	ActiveThermalTripPointExhaustFan;	// 238
  UINT8	PassiveThermalTripPointExhaustFan;	// 239
  UINT8	CriticalThermalTripPointExhaustFan;	// 240
  UINT8	HotThermalTripPointExhaustFan;		// 241

  UINT8	EnableVRDevice;			// 242
  UINT8	ActiveThermalTripPointVR;	// 243
  UINT8	PassiveThermalTripPointVR;	// 244
  UINT8	CriticalThermalTripPointVR;	// 245
  UINT8	HotThermalTripPointVR;		// 246

  //
  // DPPM Policies
  //
  UINT8	EnableActivePolicy;		// 247
  UINT8	EnablePassivePolicy;		// 248
  UINT8	EnableCriticalPolicy;		// 249
  UINT8	EnableCoolingModePolicy;	// 250
  UINT8	TrtRevision;			// 251

  //
  // CLPO (Current Logical Processor Off lining Setting)
  //
  UINT8	LPOEnable;			// 252
  UINT8	LPOStartPState;			// 253
  UINT8	LPOStepSize;			// 254
  UINT8	LPOPowerControlSetting;		// 255
  UINT8	LPOPerformanceControlSetting;	// 256

  //
  // Miscellaneous DPTF
  //
  UINT32	PpccStepSize;		// 257
  UINT8	EnableDisplayParticipant;	// 261

  ///
  /// PFAT
  ///
  UINT64 PfatMemAddress;                // 262 PFAT Memory Address for Tool Interface
  UINT8  PfatMemSize;                   // 270 PFAT Memory Size for Tool Interface
  UINT16 PfatIoTrapAddress;             // 271 IoTrap Address for Tool Interface

  UINT8	reserve8[286 - 273];
  UINT8  ConfigTdpBios;			// 286 enable/disable BIOS only version of Config TDP

  UINT8	reserve9[519 - 287];
  UINT8  EnablePowerDevice;		// 519
  UINT8  EnablePowerPolicy;		// 520
//  XHPR, 8,      //   (521) RTD3 USB Power Resource config


  UINT8	reserve10[639 - 521];
  UINT8  PL1LimitCS;			// 639
  UINT16 PL1LimitCSValue;		// 640

  UINT8	reserve11[690 - 642];
  
} EFI_GLOBAL_NVS_AREA;
#pragma pack()
///
/// Global NVS Area Protocol
///
struct _EFI_GLOBAL_NVS_AREA_PROTOCOL {
  EFI_GLOBAL_NVS_AREA *Area;
};


typedef	struct _EFI_GLOBAL_NVS_AREA_PROTOCOL	EFI_GLOBAL_NVS_AREA_PROTOCOL;
#endif
