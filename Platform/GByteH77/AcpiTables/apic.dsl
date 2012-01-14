/*
 * Intel ACPI Component Architecture
 * AML Disassembler version 20120816-32 [Aug 16 2012]
 * Copyright (c) 2000 - 2012 Intel Corporation
 * 
 * Disassembly of apic.bin, Mon Feb 13 13:06:47 2017
 *
 * ACPI Data Table [APIC]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "APIC"    [Multiple APIC Description Table (MADT)]
[004h 0004   4]                 Table Length : 00000072
[008h 0008   1]                     Revision : 03
[009h 0009   1]                     Checksum : 40
[00Ah 0010   6]                       Oem ID : "ALASKA"
[010h 0016   8]                 Oem Table ID : "A M I"
[018h 0024   4]                 Oem Revision : 01072009
[01Ch 0028   4]              Asl Compiler ID : "AMI "
[020h 0032   4]        Asl Compiler Revision : 00010013

[024h 0036   4]           Local Apic Address : FEE00000
[028h 0040   4]        Flags (decoded below) : 00000001
                         PC-AT Compatibility : 1

[02Ch 0044   1]                Subtable Type : 00 [Processor Local APIC]
[02Dh 0045   1]                       Length : 08
[02Eh 0046   1]                 Processor ID : 01
[02Fh 0047   1]                Local Apic ID : 00
[030h 0048   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1

[034h 0052   1]                Subtable Type : 00 [Processor Local APIC]
[035h 0053   1]                       Length : 08
[036h 0054   1]                 Processor ID : 02
[037h 0055   1]                Local Apic ID : 02
[038h 0056   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1

[03Ch 0060   1]                Subtable Type : 00 [Processor Local APIC]
[03Dh 0061   1]                       Length : 08
[03Eh 0062   1]                 Processor ID : 03
[03Fh 0063   1]                Local Apic ID : 04
[040h 0064   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1

[044h 0068   1]                Subtable Type : 00 [Processor Local APIC]
[045h 0069   1]                       Length : 08
[046h 0070   1]                 Processor ID : 04
[047h 0071   1]                Local Apic ID : 06
[048h 0072   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1

[04Ch 0076   1]                Subtable Type : 01 [I/O APIC]
[04Dh 0077   1]                       Length : 0C
[04Eh 0078   1]                  I/O Apic ID : 02
[04Fh 0079   1]                     Reserved : 00
[050h 0080   4]                      Address : FEC00000
[054h 0084   4]                    Interrupt : 00000000

[058h 0088   1]                Subtable Type : 02 [Interrupt Source Override]
[059h 0089   1]                       Length : 0A
[05Ah 0090   1]                          Bus : 00
[05Bh 0091   1]                       Source : 00
[05Ch 0092   4]                    Interrupt : 00000002
[060h 0096   2]        Flags (decoded below) : 0000
                                    Polarity : 0
                                Trigger Mode : 0

[062h 0098   1]                Subtable Type : 02 [Interrupt Source Override]
[063h 0099   1]                       Length : 0A
[064h 0100   1]                          Bus : 00
[065h 0101   1]                       Source : 09
[066h 0102   4]                    Interrupt : 00000009
[06Ah 0106   2]        Flags (decoded below) : 000D
                                    Polarity : 1
                                Trigger Mode : 3

[06Ch 0108   1]                Subtable Type : 04 [Local APIC NMI]
[06Dh 0109   1]                       Length : 06
[06Eh 0110   1]                 Processor ID : FF
[06Fh 0111   2]        Flags (decoded below) : 0005
                                    Polarity : 1
                                Trigger Mode : 1
[071h 0113   1]         Interrupt Input LINT : 01

Raw Table Data: Length 114 (0x72)

  0000: 41 50 49 43 72 00 00 00 03 40 41 4C 41 53 4B 41  APICr....@ALASKA
  0010: 41 20 4D 20 49 00 00 00 09 20 07 01 41 4D 49 20  A M I.... ..AMI 
  0020: 13 00 01 00 00 00 E0 FE 01 00 00 00 00 08 01 00  ................
  0030: 01 00 00 00 00 08 02 02 01 00 00 00 00 08 03 04  ................
  0040: 01 00 00 00 00 08 04 06 01 00 00 00 01 0C 02 00  ................
  0050: 00 00 C0 FE 00 00 00 00 02 0A 00 00 02 00 00 00  ................
  0060: 00 00 02 0A 00 09 09 00 00 00 0D 00 04 06 FF 05  ................
  0070: 00 01                                            ..
