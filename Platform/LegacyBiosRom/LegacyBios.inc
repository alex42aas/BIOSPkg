;  ///
;  /// Causes the Compatibility16 code to do any internal initialization required.
;  /// Input:
;  ///   AX = Compatibility16InitializeYourself
;  ///   ES:BX = Pointer to EFI_TO_COMPATIBILITY16_INIT_TABLE
;  /// Return:
;  ///   AX = Return Status codes
;  ///
%define Legacy16InitializeYourself 0x0000
  
;  ///
;  /// Causes the Compatibility16 BIOS to perform any drive number translations to match the boot sequence.
;  /// Input:
;  ///   AX = Compatibility16UpdateBbs
;  ///   ES:BX = Pointer to EFI_TO_COMPATIBILITY16_BOOT_TABLE
;  /// Return:
;  ///   AX = Returned status codes
;  ///
%define Legacy16UpdateBbs 0x0001
  
;  ///
;  /// Allows the Compatibility16 code to perform any final actions before booting. The Compatibility16
;  /// code is read/write.
;  /// Input:
;  ///   AX = Compatibility16PrepareToBoot
;  ///   ES:BX = Pointer to EFI_TO_COMPATIBILITY16_BOOT_TABLE structure  
;  /// Return:
;  ///   AX = Returned status codes
;  ///
%define Legacy16PrepareToBoot 0x0002
  
;  ///
;  /// Causes the Compatibility16 BIOS to boot. The Compatibility16 code is Read/Only.
;  /// Input:
;  ///   AX = Compatibility16Boot
;  /// Output:
;  ///   AX = Returned status codes
;  ///
%define Legacy16Boot 0x0003
  
;  ///
;  /// Allows the Compatibility16 code to get the last device from which a boot was attempted. This is
;  /// stored in CMOS and is the priority number of the last attempted boot device.
;  /// Input:
;  ///   AX = Compatibility16RetrieveLastBootDevice
;  /// Output:
;  ///   AX = Returned status codes
;  ///   BX = Priority number of the boot device.
;  ///
%define Legacy16RetrieveLastBootDevice 0x0004
  
;  ///
;  /// Allows the Compatibility16 code rehook INT13, INT18, and/or INT19 after dispatching a legacy OpROM.
;  /// Input:
;  ///   AX = Compatibility16DispatchOprom
;  ///   ES:BX = Pointer to EFI_DISPATCH_OPROM_TABLE
;  /// Output:
;  ///   AX = Returned status codes
;  ///   BX = Number of non-BBS-compliant devices found. Equals 0 if BBS compliant.
;  ///
%define Legacy16DispatchOprom 0x0005
  
;  ///
;  /// Finds a free area in the 0xFxxxx or 0xExxxx region of the specified length and returns the address
;  /// of that region.
;  /// Input:
;  ///   AX = Compatibility16GetTableAddress
;  ///   BX = Allocation region
;  ///       00 = Allocate from either 0xE0000 or 0xF0000 64 KB blocks.
;  ///       Bit 0 = 1 Allocate from 0xF0000 64 KB block
;  ///       Bit 1 = 1 Allocate from 0xE0000 64 KB block
;  ///   CX = Requested length in bytes.
;  ///   DX = Required address alignment. Bit mapped. First non-zero bit from the right is the alignment.
;  /// Output:
;  ///   AX = Returned status codes
;  ///   DS:BX = Address of the region
;  ///
%define Legacy16GetTableAddress 0x0006
  
;  ///
;  /// Enables the EfiCompatibility module to do any nonstandard processing of keyboard LEDs or state.
;  /// Input:
;  ///   AX = Compatibility16SetKeyboardLeds
;  ///   CL = LED status.
;  ///     Bit 0  Scroll Lock 0 = Off
;  ///     Bit 1  NumLock
;  ///     Bit 2  Caps Lock
;  /// Output:
;  ///     AX = Returned status codes
;  ///
%define Legacy16SetKeyboardLeds 0x0007
  
;  ///
;  /// Enables the EfiCompatibility module to install an interrupt handler for PCI mass media devices that
;  /// do not have an OpROM associated with them. An example is SATA.
;  /// Input:
;  ///   AX = Compatibility16InstallPciHandler
;  ///   ES:BX = Pointer to EFI_LEGACY_INSTALL_PCI_HANDLER structure
;  /// Output:
;  ///   AX = Returned status codes
;  ///
%define Legacy16InstallPciHandler 0x0008


;;;;;;;;;;	DEBUG	ONLY
;  ///
;  /// Input:
;  ///   AX = Compatibility16EnableIrq2
;  /// Output:
;  ///   AX = Returned status codes
;  ///
%define Legacy16EnableIrq2 0x0009
;;;;;;;;;;	DEBUG	ONLY


; BBS defines

; priorities
%define BBS_DO_NOT_BOOT_FROM 	0xFFFC
%define BBS_LOWEST_PRIORITY 	0xFFFD
%define BBS_UNPRIORITIZED_ENTRY 0xFFFE
%define BBS_IGNORE_ENTRY		0xFFFF

; types
%define BBS_FLOPPY 				0x01
%define BBS_HARDDISK 			0x02
%define BBS_CDROM 				0x03
%define BBS_PCMCIA 				0x04
%define BBS_USB 				0x05
%define BBS_EMBED_NETWORK 		0x06
%define BBS_BEV_DEVICE 			0x80
%define BBS_UNKNOWN 			0xff

; defines for AMT support
AMT_DEVICE_PRESENT	equ 0x80
BOOT_FROM_AMT				equ 0x40

; Defines from TinyBios

dma0		equ	00h			;Primary 8237 DMA controller
pic0		equ	20h			;primary 8259 interrupt controller
timer		equ	40h			;8254 timer
port61		equ	61h			;miscellaneous control
kb_dat		equ	60h			;keyboard data port
kb_stat		equ	64h			;keyboard status / command port
cm_idx		equ	70h			;CMOS index port
cm_dat		equ	71h			;CMOS data port
post		equ	80h			;POST code port
fd_page		equ	81h			;DMA page register 1 (floppy)
port92		equ	92h			;Fast A20 gate / reset
pic1		equ	0a0h		;secondary 8259 interrupt controller
dma1		equ	0c0h		;Secondary 8237 DMA controller
iowait		equ	0ebh		;I/O wait register
npx			equ	0f0h		;x87 coprocessor

crtc		equ	03b4h		;CRTC I/O port (monochrome)
crtc_cga	equ	03d4h		;CRTC I/O port (color)
fdc			equ	03f0h		;primary floppy controller
fdc_ctrl	equ	fdc+2		;control register (drive select, motor)
fdc_stat	equ	fdc+4		;FDC status
fdc_data	equ	fdc+5		;FDC data
fdc_rate	equ	fdc+7		;disk rate
fdc_chg		equ	fdc+7		;disk change status (bit 7)
picedge0	equ	04d0h		;primary interrupts edge/level mode
picedge1	equ	04d1h		;secondary interrupts edge/level mode
pci_ad		equ	0cf8h		;PCI address register (32 bit)
pci_dat		equ	0cfch		;PCI data register (32 bit)
			;This is hardwired for $xxfc in
			;several places, look before you
			;change
;pci_datl	equ	low(pci_dat)
pci_datl	equ	0fch
	;
	; Interrupt vectors
	;
vec00		equ	0
vec08		equ	08h*4		;low hardware interrupts
vec10		equ	10h*4		;BIOS interrupts
vec13		equ	13h*4		;disk interrupt
vec40		equ	40h*4		;vector to floppy interrupt
vec41		equ	41h*4		;vector to drive 80 parameters
vec46		equ	46h*4		;vector to drive 81 parameters
vec70		equ	70h*4		;high hardware interrupts
	;
	; BIOS data area
	;
	; Most BIOSes access this as segment 0040, but I prefer 0000.
	; The only place where this can cause problems is the keyboard
	; buffer, which is offset to segment 0040.
	;
m_fdbase	equ	0078h	;INT1E = disk base

	; we use IRQ area instead of extended BIOS data area for the
	; PS/2 mouse variables. This can be moved anywhere.

m_msbase	equ	0200h		;base address
m_msvec		equ	m_msbase	;pointer to call-back routine
m_msflag	equ	m_msbase+4	;mouse flags
								;bit 7 = 1: command pending
								;bit 6 = 1: FE request resend
								;bit 5 = 1: FA command acknowledge
								;bit 4 = 1: FC error condition
								;bit 3 = 1: call back installed
								;bit 2 = 1: initialized
								;bit 1..0 : buffer index
m_msbuf		equ	m_msbase+5	;mouse buffer (3 bytes - 1)
m_msend		equ	m_msbase+7

m_rsio		equ	0400h		;serial I/O ports
m_lpio		equ	0408h		;parallel I/O ports
m_ebda		equ	040Eh		; Segment address of EBDA
m_devflg	equ	0410h		;device flags (word)
m_lomem		equ	0413h		;low memory size (word) in 1KB units
m_kbf		equ	0417h		;keyboard flag
m_kbf1		equ	0418h		;keyboard flag 2
m_kbnum		equ	0419h		;Alt-xxx numeric entry
m_kbhead	equ	041ah		;^buffer head
m_kbtail	equ	041ch		;^buffer tail
m_kbbuf		equ	041eh		;keyboard buffer
m_kbbuf9	equ	043eh		;end of keyboard buffer
m_fdrecal	equ	043eh		;floppy recal status
m_fdmot		equ	043fh		;floppy motor status
m_fdcnt		equ	0440h		;floppy motor timer
m_fdstat	equ	0441h		;last floppy status
m_fdfile	equ	0442h		;floppy status / command file
m_vmode		equ	0449h		;video mode
m_vcol		equ	044ah		;video columns (word)
m_vpgsz		equ	044ch		;video page size (word)
m_vpage		equ	044eh		;video page start address (word)
m_vcolrow	equ	0450h		;video column, row (8 pages)
m_vcursor	equ	0460h		;video cursor end, start line (word)
m_vpgno		equ	0462h		;video page number
m_vcrt		equ	0463h		;video CRTC I/O (word)
m_vmsel		equ	0465h		;video mode select register
			;used for key buffer, option CONSOLE
m_vpal		equ	0466h		;video CGA palette register
			;used for serial redirect flag, option CONSOLE.
m_ioofs		equ	0467h		;option ROM offset
m_ioseg		equ	0469h		;option ROM segment
m_timer		equ	046ch		;timer ticks since midnight (4 bytes)
m_timofl	equ	0470h		;1 equ timer overflow
m_brkflg	equ	0471h		;keyboard break flag
m_rstflg	equ	0472h		;reset flag, $1234 if Ctrl-Alt-Del
m_hdstat	equ	0474h		;HDD status last command
m_hdcnt		equ	0475h		;number of HDD drives
m_lptime	equ	0478h		;parallel port time-out (4 bytes)
m_rstime	equ	047ch		;serial port time-out (4 bytes)
m_kbstart	equ	0480h		;^buffer start
m_kbend		equ	0482h		;^buffer end
m_vrow		equ	0484h		;video row count - 1
m_hdst		equ	048ch		;HDD status register
m_hderr		equ	048dh		;HDD error register
m_hdflag	equ	048eh		;HDD interrupt flag
;m_fdinfo	equ	048fh		;floppy info
m_console	equ	048fh		;serial port console flag
m_fdmed0	equ	0490h		;floppy A: media status
;m_fdmed1	equ	0491h		;floppy B: media status
m_conkey	equ	0492h		;console key buffer
;m_fdmed0a	equ	0492h		;floppy A: initial media status
;m_fdmed1a 	equ	0493h		;floppy B: initial media status
m_fdtrk0	equ	0494h		;floppy A: current track
;m_fdtrk1	equ	0495h		;floppy B: current track
m_kbf3		equ	0496h		;keyboard status
m_kbf2		equ	0497h		;keyboard status

m_Int13Private	equ	04b0h		; (struct INT13_PRIVATE*)

m_prtsc		equ	0500h		;print screen status

m_misc_tbl	equ	0534h		; custom BDA field - 2 bytes contains offset to free unallocated memory for GetTableAddress. 

m_usb_key		equ	04E4h		; byte contains USB keyboard make-code. if 0, no key currently pressed
m_usb_delay	equ	04E5h		; byte contains remain timer ticks to next repeat

gop_enable  equ 04e6h   ; byte contains flag if GOP enabled or not

EventNotifyFlag	equ	0536h

ebda_seg	equ	9FC0h

	;
	; Temporary data area - cleared before boot
	;
tmp_diag	equ	04b0h		;diagnostic hex dump display (16)

tmp_losz	equ	0504h		;low top of memory (32 bit)
tmp_hisz	equ	0508h		;high top of memory (32 bit)
tmp_kbd		equ	0510h		;keyboard response, AA = ok (8)
tmp_kbc		equ	0511h		;keyboard controller, FF = not present
tmp_npx		equ	0514h		;control word, for init
tmp_kbfail 	equ	0518h		;1 if keyboard failure
tmp_rtc		equ	0519h		;1 if RTC battery failure or bad time /date
tmp_ss		equ	051ah		;RTC second for run check
tmp_mm		equ	051bh		;RTC minute for run check
tmp_timer	equ	051ch		;Timer (16 bit) for timer run check
tmp_tim		equ	051eh		;1 if timer / RTC update failure

tmp_pci		equ	0520h		;workspace for PCI BIOS

tmp_stack	equ	1000h		;POST stack

tmp_buf		equ	1000h		;temporary buffer, used for HDD init
	;
	; RTC / CMOS
	;
cm_nmi		equ	80h			;NMI disable index
cm_ss		equ	0			;RTC seconds
cm_ssa		equ	1			;RTC seconds alarm
cm_mm		equ	2			;RTC minutes
cm_mma		equ	3			;RTC minutes alarm
cm_hh		equ	4			;RTC hours
cm_hha		equ	5			;RTC hours alarm
cm_day		equ	6			;RTC day of week (cleared, not used)
cm_dd		equ	7			;RTC day
cm_mo		equ	8			;RTC month
cm_yy		equ	9			;RTC year
cm_a		equ	0ah			;RTC status, divider
cm_b		equ	0bh			;RTC mode, interrupt control
cm_c		equ	0ch			;RTC interrupt status
cm_d		equ	0dh			;RTC battery status
cm_dia		equ	0eh			;CMOS diagnostic byte (power fail flag)
cm_shut		equ	0fh			;CMOS shutdown register
cm_test		equ	2fh			;CMOS test register (legacy: checksum)
cm_exl		equ	30h			;CMOS extended memory size, low
cm_exh		equ	31h			;CMOS extended memory size, high
cm_cent		equ	32h			;CMOS century
cm_meml		equ	15h			;CMOS base memory, low
cm_memh		equ	16h			;CMOS base memory, high
cm_exl2		equ	17h			;CMOS extended memory size, low
cm_exh2		equ	18h			;CMOS extended memory size, high
	;
	; Miscellaneous
	;
eoi			equ	20h			;PIC: End of Interrupt
bofs		equ	0400h		;BIOS segment offset
	;
	; TTY equates
	;
bell		equ	7			;bell
bs			equ	8			;backspace
lf			equ	10			;line feed
cr			equ	13			;carriage return
vidseg		equ	0b000h		;video segment for monochrome
vid_fill	equ	0720h		;fill pattern, space
	;
	; PUSHA stack frame
	;
_ds			equ	18			;pushed before
_es			equ	16			;pushed before
_ax			equ	14
_al			equ	_ax
_ah			equ	_ax+1
_bx			equ	8
_bl			equ	_bx
_bh			equ	_bx+1
_cx			equ	12
_cl			equ	_cx
_ch			equ	_cx+1
_dx			equ	10
_dl			equ	_dx
_dh			equ	_dx+1
_si			equ	2
_di			equ	0
_bp			equ	4
_flags		equ	6
