/*
 * Intel ACPI Component Architecture
 * AML Disassembler version 20120816-32 [Aug 16 2012]
 * Copyright (c) 2000 - 2012 Intel Corporation
 * 
 * Disassembly of dmar.bin, Mon Feb 13 13:06:47 2017
 *
 * ACPI Data Table [DMAR]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "DMAR"    [DMA Remapping table]
[004h 0004   4]                 Table Length : 000000B8
[008h 0008   1]                     Revision : 01
[009h 0009   1]                     Checksum : B3     /* Incorrect checksum, should be D7 */
[00Ah 0010   6]                       Oem ID : "INTEL "
[010h 0016   8]                 Oem Table ID : "HSW "
[018h 0024   4]                 Oem Revision : 00000001
[01Ch 0028   4]              Asl Compiler ID : "INTL"
[020h 0032   4]        Asl Compiler Revision : 00000001

[024h 0036   1]           Host Address Width : 26
[025h 0037   1]                        Flags : 01

[030h 0048   2]                Subtable Type : 0000 [Hardware Unit Definition]
[032h 0050   2]                       Length : 0018
[034h 0052   1]                        Flags : 00
[035h 0053   1]                     Reserved : 00
[036h 0054   2]           PCI Segment Number : 0000
[038h 0056   8]        Register Base Address : 00000000FED90000

[040h 0064   1]      Device Scope Entry Type : 01
[041h 0065   1]                 Entry Length : 08
[042h 0066   2]                     Reserved : 0000
[044h 0068   1]               Enumeration ID : 00
[045h 0069   1]               PCI Bus Number : 00
[046h 0070   2]                     PCI Path : 02,00

[048h 0072   2]                Subtable Type : 0000 [Hardware Unit Definition]
[04Ah 0074   2]                       Length : 0020
[04Ch 0076   1]                        Flags : 01
[04Dh 0077   1]                     Reserved : 00
[04Eh 0078   2]           PCI Segment Number : 0000
[050h 0080   8]        Register Base Address : 00000000FED91000

[058h 0088   1]      Device Scope Entry Type : 03
[059h 0089   1]                 Entry Length : 08
[05Ah 0090   2]                     Reserved : 0000
[05Ch 0092   1]               Enumeration ID : 02
[05Dh 0093   1]               PCI Bus Number : F0
[05Eh 0094   2]                     PCI Path : 1F,00

[060h 0096   1]      Device Scope Entry Type : 04
[061h 0097   1]                 Entry Length : 08
[062h 0098   2]                     Reserved : 0000
[064h 0100   1]               Enumeration ID : 00
[065h 0101   1]               PCI Bus Number : F0
[066h 0102   2]                     PCI Path : 0F,00

[068h 0104   2]                Subtable Type : 4341 [Unknown SubTable Type]
[06Ah 0106   2]                       Length : 4950

**** Unknown DMAR sub-table type 0x4341


Raw Table Data: Length 184 (0xB8)

  0000: 44 4D 41 52 B8 00 00 00 01 B3 49 4E 54 45 4C 20  DMAR......INTEL 
  0010: 48 53 57 20 00 00 00 00 01 00 00 00 49 4E 54 4C  HSW ........INTL
  0020: 01 00 00 00 26 01 00 00 00 00 00 00 00 00 00 00  ....&...........
  0030: 00 00 18 00 00 00 00 00 00 00 D9 FE 00 00 00 00  ................
  0040: 01 08 00 00 00 00 02 00 00 00 20 00 01 00 00 00  .......... .....
  0050: 00 10 D9 FE 00 00 00 00 03 08 00 00 02 F0 1F 00  ................
  0060: 04 08 00 00 00 F0 0F 00 41 43 50 49 44 55 4D 50  ........ACPIDUMP
  0070: 00 D0 82 BA 00 D0 82 BA 9B AD B4 D3 49 13 00 08  ............I...
  0080: 88 8A 7D 00 18 7E 7D 00 0A 00 00 00 C0 D0 E0 F0  ..}..~}.........
  0090: 7B 37 A0 63 0A 28 00 8C 90 B9 7E 00 80 BA 7E 00  {7.c.(....~...~.
  00A0: 58 B6 00 00 0A 30 0A 31 64 37 A0 63 0A 34 00 8C  X....0.1d7.c.4..
  00B0: A8 B9 7E 00 78 B9 7E 00                          ..~.x.~.
