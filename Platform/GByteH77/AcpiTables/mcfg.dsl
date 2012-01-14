/*
 * Intel ACPI Component Architecture
 * AML Disassembler version 20120816-32 [Aug 16 2012]
 * Copyright (c) 2000 - 2012 Intel Corporation
 * 
 * Disassembly of mcfg.bin, Mon Feb 13 13:06:47 2017
 *
 * ACPI Data Table [MCFG]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "MCFG"    [Memory Mapped Configuration table]
[004h 0004   4]                 Table Length : 0000003C
[008h 0008   1]                     Revision : 01
[009h 0009   1]                     Checksum : 6D
[00Ah 0010   6]                       Oem ID : ""
[010h 0016   8]                 Oem Table ID : ""
[018h 0024   4]                 Oem Revision : 01072009
[01Ch 0028   4]              Asl Compiler ID : "MSFT"
[020h 0032   4]        Asl Compiler Revision : 00000097

[024h 0036   8]                     Reserved : 0000000000000000

[02Ch 0044   8]                 Base Address : 00000000F8000000
[034h 0052   2]         Segment Group Number : 0000
[036h 0054   1]             Start Bus Number : 00
[037h 0055   1]               End Bus Number : 3F
[038h 0056   4]                     Reserved : 00000000

Raw Table Data: Length 60 (0x3C)

  0000: 4D 43 46 47 3C 00 00 00 01 6D 00 00 00 00 00 00  MCFG<....m......
  0010: 00 00 00 00 00 00 00 00 09 20 07 01 4D 53 46 54  ......... ..MSFT
  0020: 97 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F8  ................
  0030: 00 00 00 00 00 00 00 3F 00 00 00 00              .......?....
