	;
	; I/O ports
	;
pci_ver		equ	0210h		;PCI version reported
bios3seg	equ	000fh		;$f000 / 4096
	;
	; 32 bit pusha stack frame
	;
_eax		equ	1ch
_ebx		equ	10h
_ecx		equ	18h
_edx		equ	14h
_ebp		equ	08h
_esi		equ	04h
_edi		equ	00h
_efl		equ	24h			;flags

	;
	; PCI configuration space structure
	;
p_id		equ	0			;vendor, device ID
p_cmd		equ	4			;command register
p_stat		equ	6			;status register
p_class		equ	8			;class code, revision ID
p_linesz	equ	12			;cache line size
p_lat		equ	13			;latency timer
p_hedt		equ	14			;header type
p_bist		equ	15			;built-in self test
p_base		equ	10h			;base address registers
pb_bus		equ	18h			;bridge: primary bus number
pb_bus2		equ	19h			;bridge: secondary bus number
pb_bus3		equ	1ah			;bridge: subordinate bus number
pb_lat2		equ	1bh			;bridge: secondary latency timer
pb_io		equ	1ch			;bridge: I/O limit
pb_stat		equ	1eh			;bridge: secondary status
pb_mem		equ	20h			;bridge: memory base low
pb_memp		equ	24h			;bridge: prefetchable memory base
p_cis		equ	28h			;end of base address registers
pb_mem2		equ	28h			;bridge: memory base high
pb_memp2	equ	2ch			;bridge: prefetchable memory base high
p_rom		equ	30h			;expansion rom base
pb_io2		equ	34h			;bridge: I/O base high
pb_rom		equ	38h			;bridge: ROM
p_line		equ	3ch			;IRQ assigned to function
p_pin		equ	3dh			;0 = not used, 1=A, 2=B, 3=C, 4=D
p_mingnt	equ	3eh			;minimum grant time
pb_ctl		equ	3eh			;bridge: control
p_maxlat	equ	3fh			;maximum latency

p_int:		resw	2		;PCI interrupt lines, LSB = INTA,
							;MSB = INTD
p_irqpt:	resw	1		;pointer to IRQ table entry
p_mem:		resw	1		;regular memory (64K steps)
p_memlim:	resw	1		;+ memory limit
p_memp:		resw	1		;prefetchable memory (64K steps)
p_memplim: 	resw	1		;+ prefetchable memory limit
p_io:		resw	1		;I/O address	(16 byte steps)
p_iolim:	resw	1		;+ I/O limit
p_memr:		resw	1		;memory below 1MB (segment value)
p_memrlim: 	resw	1		;+ limit
p_capa		resw	1		;low: 0 if back to back mode supported
							;high: 0 for fast, 2 for medium, 4/6
							;slow devsel
p_bus:		resb	1		;current bus
p_lastbus: 	resb	1		;+ last bus number

	;
	; dispatch table
	;
pci_vec:
	dw		(pci_badc)		;00: bad command
	dw		(pci_pres)		;01: PCI BIOS present
	dw		(pci_find)		;02: find PCI device
	dw		(pci_class)		;03: find PCI class code
	dw		(pci_badc)		;04: invalid
	dw		(pci_badc)		;05: invalid
	dw		(pci_spec)		;06: generate special cycle
	dw		(pci_badc)		;07: invalid
	dw		(pci_readb)		;08: read config byte
	dw		(pci_readw)		;09: read config word
	dw		(pci_readd)		;0A: read config dword
	dw		(pci_writb)		;0B: write config byte
	dw		(pci_writw)		;0C: write config word
	dw		(pci_writd)		;0D: write config dword
	dw		(pci_badc)		;0E: get IRQ routing options not implemented
	dw		(pci_badc)		;0F: set PCI IRQ not implemented

