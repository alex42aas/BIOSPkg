/*
 * Intel ACPI Component Architecture
 * AML Disassembler version 20120816-32 [Aug 16 2012]
 * Copyright (c) 2000 - 2012 Intel Corporation
 * 
 * Disassembly of bgrt.bin, Mon Feb 13 13:06:47 2017
 *
 * ACPI Data Table [BGRT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "BGRT"    [Boot Graphics Resource Table]
[004h 0004   4]                 Table Length : 00000038
[008h 0008   1]                     Revision : 00
[009h 0009   1]                     Checksum : 12
[00Ah 0010   6]                       Oem ID : "ALASKA"
[010h 0016   8]                 Oem Table ID : "A M I"
[018h 0024   4]                 Oem Revision : 01072009
[01Ch 0028   4]              Asl Compiler ID : "AMI "
[020h 0032   4]        Asl Compiler Revision : 00010013

[024h 0036   2]                      Version : 0001
[026h 0038   1]                       Status : 01
[027h 0039   1]                   Image Type : 00
[028h 0040   8]                Image Address : 00000000AE5E0018
[030h 0048   4]                Image OffsetX : 00000060
[034h 0052   4]                Image OffsetY : 00000100

Raw Table Data: Length 56 (0x38)

  0000: 42 47 52 54 38 00 00 00 00 12 41 4C 41 53 4B 41  BGRT8.....ALASKA
  0010: 41 20 4D 20 49 00 00 00 09 20 07 01 41 4D 49 20  A M I.... ..AMI 
  0020: 13 00 01 00 01 00 01 00 18 00 5E AE 00 00 00 00  ..........^.....
  0030: 60 00 00 00 00 01 00 00                          `.......
