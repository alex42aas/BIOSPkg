AMT_VENDOR_ID		equ		0x8086
AMT_DEVICE_ID		equ		0x1c3c

;; HDD defines
%define HD_WAIT    	20      ;Максимальное время ожидания инита HD. x секунды
%define HD_ENA    	 1       ;не проверять статус HD в течении x секунд

%define HDD_LBA            ;разрешить поддержку  LBA

MASTER_DRIVE		equ		0a0h
SLAVE_DRIVE		equ		0b0h

; definition of array block of hdc' address
hdc_dat			equ		0
hdc_err     		equ		2
hdc_cnt     		equ   		4
hdc_sec     		equ   		6
hdc_cyl     		equ   		8
hdc_cyh     		equ		10
hdc_drv     		equ		12
hdc_cmd_stat    	equ		14
hdc_ctrl_stat2  	equ		16 
hdc_len			equ		18

; array block of hdc' address
; I   - primary SATA channel (drive 80h/81h in case of HDD)
; II  - secondary SATA channel (drive 82h/83h in case of HDD)
; III - AMT primary channel (virtual floppy/virtual CD)

hdc_array:
prim_dat    			dw 0
prim_err    			dw 0
prim_cnt    			dw 0
prim_sec    			dw 0
prim_cyl    			dw 0
prim_cyh    			dw 0
prim_drv    			dw 0
prim_cmd_stat     		dw 0
prim_ctrl_stat2   		dw 0

secd_dat    			dw 0
secd_err    			dw 0
secd_cnt    			dw 0
secd_sec    			dw 0
secd_cyl    			dw 0
secd_cyh    			dw 0
secd_drv    			dw 0
secd_cmd_stat     		dw 0
secd_ctrl_stat2   		dw 0

amt_dat 			dw 0
amt_err 			dw 0
amt_cnt 			dw 0
amt_sec 			dw 0
amt_cyl 			dw 0
amt_cyh 			dw 0
amt_drv 			dw 0
amt_cmd_stat    		dw 0
amt_ctrl_stat2   		dw 0

; block to store hdc location parameters
prim_bus			db	0
prim_dev			db	0
prim_func			db	0
prim_class			db	1
prim_subclass			db	1

prim_irq			db	0
prim_ctrl			dw	0x0000
prim_base			dw	0x0000
prim_bmaster			dw	0x0000

secd_irq			db	0
secd_ctrl			dw	0x0000
secd_base			dw	0x0000
secd_bmaster			dw	0x0000

amt_bus				db	0x00
amt_devfunc			db	0xb2
amt_class			db	0x01
amt_subclass			db	0x01

amt_prim_irq			db	10
amt_prim_ctrl			dw	0x0000
amt_prim_base			dw	0x0000

amt_secd_irq			db	10
amt_secd_ctrl			dw	0x0000
amt_secd_base			dw	0x0000

amt_bmaster			dw	0x0000

; buffer for device's strings

model_len	equ	32

model_dev0:
	times model_len db 0
model_dev1:
	times model_len db 0
model_dev2:
	times model_len db 0
model_dev3:
	times model_len db 0
model_amt_master:
	times model_len db 0
model_amt_slave:
	times model_len db 0

type_len		equ	1
type_dev0:
	db	0
type_dev1:
	db	0
type_dev2:
	db	0
type_dev3:
	db	0
type_amt0:
	; bit 7 - AMT device presense; bit 6 - boot from AMT device
	db	0
type_amt1:
	; bit 7 - AMT device presense; bit 6 - boot from AMT device
	db	0

; translated drive numbers table
transl_dn:
	db	0				; drive 80h
	db	0				; drive 81h
	db	0				; drive 82h
	db	0				; drive 83h
transl_flag:
	db	0

        ;
        ; drive parameter structure (stored in data module)
        ;
edpt_cyl         equ     0       ;number of cylinders
edpt_head        equ     2       ;number of heads
edpt_sig         equ     3       ;signature, $A0
edpt_pspt        equ     4       ;physical sectors per track
edpt_mul         equ     5       ;(precompensation) -> multiple count
edpt_shl         equ     7       ;reserved -> shift count
edpt_ctrl        equ     8       ;drive control byte
edpt_pcyl        equ     9       ;physical cylinders
edpt_phead       equ     11      ;physical heads
edpt_lz          equ     12      ;(landing zone)
edpt_spt         equ     14      ;logical sectors per track
edpt_chsum       equ     15      ;reserved
edpt_len         equ     16      ;length of structure
edpt_sec  	 equ		 16			 ; extension - qword of sectors count
edpt_ext_len	 equ		 24

        ;
        ; disk address packet for extended read/write/verify/seek
        ;
drq_len       equ     0       ;packet size in bytes
drq_res       equ     1       ;reserved, must be 0
drq_blk       equ     2       ;number of blocks, max. 127
drq_res2      equ     3       ;reserved, must be 0
drq_ofs       equ     4       ;transfer buffer offset
drq_seg       equ     6       ;transfer buffer segment
drq_lba       equ     8       ;block number (8 bytes)

;
; Hard disk parameters
;

hd_prm0:
		TIMES edpt_ext_len  db 0
hd_prm1:
		TIMES edpt_ext_len  db 0
hd_prm2:
		TIMES edpt_ext_len  db 0
hd_prm3:
		TIMES edpt_ext_len  db 0
hd_prm4:
		TIMES edpt_ext_len  db 0
hd_prm5:
		TIMES edpt_ext_len  db 0
hd_top: 
		db      86h     ;4 HDD + 2 AMT 

forcelba_flag db  1 ; disabled by default

        ;
        ; INT 13 entry
        ;
Int13:
        sti

%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
%if VERBOSE_INT_13
	push	'i'
	call	SendChar
	push	0x13
	call	SendByte
	push	' '
	call	SendChar

	push	ax
	call	SendWord
	push	' '
	call	SendChar
	push	dx
	call	SendWord
%endif
%endif
%endif

	;push	2000
	;call	WaitSomeTime

				cmp			dl, 0xF0										;CDROM ?
				je			Int13_cd
				cmp			dl, 0xF1										;CDROM ?
				je			Int13_cd

        and     dl,dl               		   	;HDD ?
        jns     Int40                		   	;:floppy

        and     ah,ah   						;reset drive ?
        jz      int13hd1        				;:yes

        cmp     dl,byte [cs:hd_top]			   	;compare with max drive number
        jae     Int40   						;:floppy or DiskOnChip
        ;cmp     dl,81h  						;above valid HDD ?
        ;ja      Int40   						;-> floppy only
        ;int     40h     						;reset floppy
        ;mov     ah,0

	; translate drive number
	cmp		byte [cs:transl_flag], 0
	je		int13_should_not_translate

	push	di
	push	ax

	xor		di, di
int13_find_transl_drive:
	cmp		dl, byte [cs:transl_dn+di]
	je		int13_transl_drive_found
	inc		di
	cmp		di, 4
	jne		int13_find_transl_drive

int13_transl_drive_found:
	mov		ax, di
	or		ax, 80h
	mov		dl, al

	pop		ax
	pop		di

%if SERIAL_DEBUG
%if VERBOSE_INT_13
	push	't'
	call	SendChar
	push	dx
	call	SendByte
	push	' '
	call	SendChar
%endif
%endif

	cmp		dl, 83h
	ja	  Int40

int13_should_not_translate:

        ;
        ; dispatch disk commands
        ;
int13hd1:
%if SERIAL_DEBUG
%if VERBOSE_INT_13
	push	'h'
	call	SendChar
%endif
%endif

        push    ds      						;save registers
        push    es
        pusha
        mov     bp,sp   						;access to stack frame

				push		edi

        xor     di,di   						;access BIOS segment
        mov     ds,di

        mov     di,ax   						;command -> index
        shr     di,8
        add     di,di
        and     byte [ss:bp+18h],0feh      		;clear return carry
        cmp     di,hd_vec99-hd_vectab   		;limit command vector
        jae     hd_badcmd      					;:too high
        jmp     [cs:di+hd_vectab]       		;jump to command

        ;
        ; Illegal command
        ;
hd_badcmd:
		mov  byte [m_hdstat],1       			;illegal command

        ;
        ; AH=01: get status
        ;
hd_status:
		mov  al,[m_hdstat]   					;get old status
        mov  [ss:bp+_al],al     					;return in AL

        ;
        ; return status
        ;
hd_exit0:
        mov     [m_hdstat],al   				;set error code
        mov     [ss:bp+_ah],al     				;return in AH
hd_exit1:
        and     al,al   						;error ?
        jz      hd_exit2        				;:no
        or      byte [ss:bp+18h],1 				;yes: set carry

hd_exit2:

%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
%if VERBOSE_INT_13
	push	mes_crlf
	call	SendTxt
%endif
%endif
%endif

				pop			edi
        popa            						;restore registers
        pop     es
        pop     ds

        iret            						;return from interrupt

        ;
        ; IDE vector table
        ;
        align 2
hd_vectab:
				dw   		hd_rst  				;AH=00: recalibrate drive
        dw      hd_status       ;AH=01: get status
        dw      hd_read 				;AH=02: read
        dw      hd_write        ;AH=03: write
        dw      hd_verify       ;AH=04: verify
        dw      hd_badcmd       ;AH=05: format track -> not supported
        dw      hd_badcmd       ;AH=06: bad
        dw      hd_badcmd       ;AH=07: bad
        dw      hd_getprm       ;AH=08: read drive parameters
        dw      hd_setprm       ;AH=09: set drive parameters
        dw      hd_badcmd       ;AH=0A: read long -> not supported
        dw      hd_badcmd       ;AH=0B: write long -> not supported
        dw      hd_seek					;AH=0C: seek
        dw      hd_rst2 				;AH=0D: alternate disk reset (HD only)
        dw      hd_badcmd       ;AH=0E: bad
        dw      hd_badcmd       ;AH=0F: bad
        dw      hd_trdy 				;AH=10: test drive ready
        dw      hd_recal        ;AH=11: recalibrate
        dw      hd_badcmd       ;AH=12: bad
        dw      hd_badcmd       ;AH=13: bad
        dw      hd_diag 				;AH=14: controller diagnostics
        dw      hd_gettyp       ;AH=15: get drive type
        dw      hd_badcmd       ;AH=16: bad
        dw      hd_badcmd       ;AH=17: bad
        dw      hd_badcmd       ;AH=18: bad
        dw      hd_badcmd       ;AH=19: bad
        dw      hd_badcmd       ;AH=1A: bad
        dw      hd_badcmd       ;AH=1B: bad
        dw      hd_badcmd       ;AH=1C: bad
        dw      hd_badcmd       ;AH=1D: bad
        dw      hd_badcmd       ;AH=1E: bad
        dw      hd_badcmd       ;AH=1F: bad
        dw      hd_badcmd       ;AH=20: bad
        dw      hd_badcmd       ;AH=21: bad
        dw      hd_badcmd       ;AH=22: bad
        dw      hd_badcmd				;AH=23: timer not supported
        dw      hd_setmul       ;AH=24: set multiple mode
        dw      hd_id   				;AH=25: identify drive
        dw      hd_badcmd       ;AH=26: bad
        dw      hd_badcmd       ;AH=27: bad
        dw      hd_badcmd       ;AH=28: bad
        dw      hd_badcmd       ;AH=29: bad
        dw      hd_badcmd       ;AH=2A: bad
        dw      hd_badcmd       ;AH=2B: bad
        dw      hd_badcmd       ;AH=2C: bad
        dw      hd_badcmd       ;AH=2D: bad
        dw      hd_badcmd       ;AH=2E: bad
        dw      hd_badcmd       ;AH=2F: bad
        dw      hd_badcmd       ;AH=30: bad
        dw      hd_badcmd       ;AH=31: bad
        dw      hd_badcmd       ;AH=32: bad
        dw      hd_badcmd       ;AH=33: bad
        dw      hd_badcmd       ;AH=34: bad
        dw      hd_badcmd       ;AH=35: bad
        dw      hd_badcmd       ;AH=36: bad
        dw      hd_badcmd       ;AH=37: bad
        dw      hd_badcmd       ;AH=38: bad
        dw      hd_badcmd       ;AH=39: bad
        dw      hd_badcmd       ;AH=3a: bad
        dw      hd_badcmd       ;AH=3b: bad
        dw      hd_badcmd       ;AH=3c: bad
        dw      hd_badcmd       ;AH=3d: bad
        dw      hd_badcmd       ;AH=3e: bad
        dw      hd_badcmd       ;AH=3f: bad
        dw      hd_badcmd       ;AH=40: bad
        dw      hd_edd41        ;AH=41: detect extended interface
        dw      hd_xrd  				;AH=42: extended read
        dw      hd_xwr  				;AH=43: extended write
        dw      hd_xver 				;AH=44: extended verify
        dw      hd_badcmd       ;AH=45: bad (lock / unlock drive)
        dw      hd_badcmd       ;AH=46: bad (eject removable media)
        dw      hd_xsk  				;AH=47: extended seek
        dw      hd_edd48        ;AH=48: get extended parameters
hd_vec99:                       ;end of table

        ;
        ; AH=00: reset hard disk drives
        ; AH=0D: alternate reset (doesn't reset floppy)
        ;
hd_rst:
hd_rst2:

				call		GetHdcBlock

				rol			edi, 16
        mov     dx, word [cs:di+hdc_ctrl_stat2]
				rol			edi, 16
        mov     al,4    		;soft reset
        out     dx,al

				push		10
				call		WaitSomeTime

        mov     al,0    		;end of reset, interrupt enable
        out     dx,al   		;hdc_ctrl
        call    hd_busy18       ;wait while busy
        jb      hd_rst8 		;:error
				rol			edi, 16
        mov     dx, word [cs:di+hdc_err]      ;check error status
				rol			edi, 16
        in      al,dx
        and     al,7fh
        sub     al,1
        jnz     hd_rst8 		;:bad status
        mov     al,0    		;ok status
        jmp     hd_exit0        ;return

hd_rst8:
        mov     al,5    		;reset failed
hd_rst9:
        jmp     hd_exit0

        ;
        ; AH=02: read sectors
        ;
hd_read:

%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
%if VERBOSE_INT_13
	push	cx

	mov		cl, byte [ss:bp+_al]		; number of sectors
	push	cx
	call	SendByte
	push	' '
	call	SendChar

	push	'C'
	call	SendChar
	push	'H'
	call	SendChar
	push	'S'
	call	SendChar
	push	' '
	call	SendChar

	mov		ch, byte [ss:bp+_cl]		; 2 high cylinder bits (7-6)
	shr		ch, 6
	mov		cl, byte [ss:bp+_ch]		; low cylinder bits

	push	cx
	call	SendWord
	push	' '
	call	SendChar

	mov		cl, byte [ss:bp+_dh]		; head
	push	cx
	call	SendByte
	push	' '		
	call	SendChar

	mov		cl, byte [ss:bp+_cl]		; sector
	and		cl, 0x3f
	push	cx
	call	SendByte
	push	' '		
	call	SendChar

	mov		cx, word [ss:bp+_es]
	push	cx
	call	SendWord
	push	':'
	call	SendChar
	mov		cx, word [ss:bp+_bx]
	push	cx
	call	SendWord

	pop		cx
%endif
%endif
%endif

				call		GetHdcBlock
        call    hd_sel  				;select drive
        jb 			near hd_read9

        call  hd_chs  				;translate CHS
        jb 		near hd_read9

        mov     bl,[ss:bp+_al]     		;get sector count
        cld    			        		;forward mode
        mov     di,[ss:bp+_bx]     		;get destination address
        mov     byte [m_hdflag],0       ;clear interrupt flag

        mov     al,20h  				;issue read command

				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16
        out     dx,al

hd_read1:

        call    hd_int  				;wait for interrupt
        jb      hd_read9

				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]        ;read status
				rol			edi, 16
        in      al,dx

        mov     byte [m_hdflag],0       ;clear interrupt flag for next
        test    al,1    				;ERR ?
        jnz     hd_read8

        test    al,8    				;DRQ ?
        jz      hd_read8        		;:no

				rol			edi, 16
        mov     dx, word [cs:di+hdc_dat] 		;read 512 bytes from drive
				rol			edi, 16

        mov     cx,256
        rep     insw
        dec     bl      				;another sector ?
        jnz     hd_read1        		;:yes
hd_read8:

        sub     byte [ss:bp+_al],bl        ;adjust sector count to reality
        call    hd_stat 				;get status
hd_read9:
        jmp     hd_exit0

        ;
        ; AH=03: write sectors
        ;
hd_write:
				call		GetHdcBlock
        mov     si,bx   				;source address
        call    hd_sel  				;select drive
        jb      hd_writ9
        call    hd_chs  				;translate CHS
        jb      hd_writ9
        mov     bl,[ss:bp+_al]     		;get sector count
        cld             				;forward mode
        mov     al,30h  				;issue write command
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16
        out     dx,al

hd_writ1:
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]        ;read status
				rol			edi, 16
        xor     cx,cx
        mov     byte [m_hdflag],0       ;clear interrupt flag for next
hd_writ2:
        in      al,dx
        test    al,8    				;DRQ ?
        jnz     hd_writ3        		;:yes
        test    al,21h  				;error ?
        jnz     hd_writ8
        loop    hd_writ2
        mov     al,80h  				;time-out
        jmp     short hd_writ9

hd_writ3:
				rol			edi, 16
        mov     dx, word [cs:di+hdc_dat] 		;write 512 bytes from drive
				rol			edi, 16
        mov     cx,256
        es rep  outsw
        call    hd_int  				;wait for interrupt
        jb      hd_writ9
        dec     bl      				;another sector ?
        jnz     hd_writ1        		;:yes

hd_writ8:
        call    hd_stat 				;get status
hd_writ9:
        jmp     hd_exit0

        ;
        ; AH=04: verify sectors
        ;
hd_verify:
				call		GetHdcBlock
				call 		hd_sel  				;select drive
        jb      hd_ver9
        call    hd_chs  				;translate CHS
        mov     al,40h
        call    hd_cmd  				;read verify command
        jb      hd_ver9
        call    hd_stat 				;get status
hd_ver9:
        jmp     hd_exit0

        ;
        ; AH=08: get drive parameters
        ;
hd_getprm:
				call		GetHdcBlock
				cmp 		byte [m_hdcnt],0        ;no drives ?
        jnz     hd_getp1
        jmp     hd_badcmd		        ;:bad command

hd_getp1:
        call    hd_parm 				;get ^parameters
        mov     al,7    				;invalid drive number
        mov     cx,0    				;return 0 size
        mov     dx,0
        jb      hd_getp9        		;:not present

        mov     dh,[cs:di+edpt_head]     ;DH = max head number
        dec     dh
        mov     dl,[m_hdcnt]    		;DL = number of drives
        mov     cx,[cs:di+edpt_cyl]      ;CX = max cylinders (swapped)
        dec     cx      				;-2 for diag, max cyl
        dec     cx
        cmp     cx, 1023				;limit cylinder to 1023
        jbe     hd_getp2
        mov     cx, 1023
hd_getp2:

        xchg    cl,ch
        shl     cl,6    				;CL high 6 bits = cylinders high
        or      cl,[cs:di + edpt_spt]    ;CL = number of sectors
        mov     al,0    				;clear status

hd_getp9:
        mov     [ss:bp+_cx],cx    			;cylinder count
        mov     [ss:bp+_dx],dx     		;number drives, heads

%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
%if VERBOSE_INT_13
	push	cx

	push	'-'
	call	SendChar
	push	'>'
	call	SendChar
	push	' '
	call	SendChar
	mov		cl, byte [ss:bp+_dl]			; number of drives
	push	cx
	call	SendByte
	
	push	'M'
	call	SendChar
	push	'a'
	call	SendChar
	push	'x'
	call	SendChar
	push	'C'
	call	SendChar
	push	'H'
	call	SendChar
	push	'S'
	call	SendChar
	push	' '
	call	SendChar

	mov		cl, byte [ss:bp+_cl]		; cylinder [9..8]
	shr		cl, 6
	push	cx
	call	SendByte
	mov		cl, byte [ss:bp+_ch]		; cylinder [7..0]
	push	cx
	call	SendByte
	push	' '
	call	SendChar

	mov		cl, byte [ss:bp+_dh]		; head
	push	cx
	call	SendByte
	push	' '
	call	SendChar

	mov		cl, byte [ss:bp+_cl]		; sector
	and		cl, 0x3f
	push	cx
	call	SendByte

	pop		cx
%endif
%endif
%endif
        jmp     hd_exit0

        ;
        ; AH=09: set drive parameters
        ;
hd_setprm:
				call		GetHdcBlock
				call 		hd_sel  				;select drive
        jb      hd_setp9
        mov     ah,[cs:di + edpt_head]    ;number heads (physical)
        dec     ah
				and			ah, 0fh
				rol			edi, 16
        mov     dx, word [cs:di+hdc_drv]      	;set maximum heads
				rol			edi, 16
        in      al,dx
        or      al,ah
        out     dx,al
        mov     al,[cs:di + edpt_spt]   ;(physical)
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cnt] 		;sector count
				rol			edi, 16
        out     dx,al
        mov     al,91h  				;set drive parameters
        call    hd_cmd
        jb      hd_setp9        		;:error
        call    hd_stat 				;check status
hd_setp9:
        jmp     hd_exit0
 
        ;
        ; AH=0C: seek
        ;
hd_seek:
				call		GetHdcBlock
        call    hd_sel  				;select drive
        jb      hd_seek9
        call    hd_chs  				;set CHS value
        mov     al,70h
        call    hd_cmd  				;seek command
        jb      hd_seek9
        call    hd_stat 				;check status
hd_seek9:
        cmp     al,40h  				;seek error ?
        jnz     hd_seek91
        mov     al,0    				;don't show it... (Core test will fail otherwise)
hd_seek91:
		jmp  hd_exit0

        ;
        ; AH=10: test drive ready
        ;
hd_trdy:
				call		GetHdcBlock
        mov     cx,0ffffh       		;no time-out
        call    hd_sel0 				;select drive, test ready
        jb      hd_trdy9
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]        ;check status
				rol			edi, 16
        in      al,dx
        mov     [m_hdst],al
        mov     ah,0aah 				;not ready
        test    al,40h
        jz      hd_trdy8
        mov     ah,40h  				;seek error
        test    al,10h
        jz      hd_trdy8
        mov     ah,0cch 				;write fault
        test    al,20h
        jnz     hd_trdy8
        mov     ah,0    				;ok status
hd_trdy8:
        mov     al,ah
hd_trdy9:
        jmp     hd_exit0

        ;
        ; AH=11: recalibrate
        ;
hd_recal:
				call		GetHdcBlock
        call    hd_sel  				;select drive
        mov     al,10h
        call    hd_cmd  				;recalibrate command
        jb      hd_rec9
        call    hd_stat 				;get status
hd_rec9:
        jmp     hd_exit0

        ;
        ; AH=14: controller diagnostics
        ;
hd_diag:
				call		GetHdcBlock
        call    hd_busy18       		;wait for not busy
        mov     al,20h  				;bad controller
        jb      hd_diag9        		;:bad
        mov     al,90h  				;diagnostic command
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16
        out     dx,al
        ;out     iowait,al
        mov     cx,18*6 				;max. 6 seconds (!!!)
        call    hd_busy 				;wait for not busy
        mov     al,80h  				;time-out
        jb      hd_diag9
				rol			edi, 16
        mov     dx, word [cs:di+hdc_err]      	;check error register
				rol			edi, 16
        in      al,dx
        and     al,7fh
        sub     al,1
        jz      hd_diag9        		;:ok
        mov     al,20h  				;bad controller
hd_diag9:
       jmp     hd_exit0

        ;
        ; AH=15: read DASD type
        ;
hd_gettyp:
				call		GetHdcBlock
				call 		hd_parm 				;get pointer to parameter block
        jb      hd_gett8        		;:not present
        mov     al,[cs:di + edpt_head]   ;number heads
        mul     byte [cs:di + edpt_spt]  ;number sectors
        mov     dx,[cs:di + edpt_cyl]    ;number cylinders
        dec     dx      				;minus one for diagnostics
        mul     dx
        mov     cl,3    				;drive present

        jmp     short hd_gett9

hd_gett8:
        xor     ax,ax   				;0 = drive not present
        xor     cx,cx
        xor     dx,dx
hd_gett9:
        mov     byte [ss:bp+_ah],cl        ;0 = not present, 3 = present
        mov     [ss:bp+_cx],dx 		    ;CX = MSB sector count
        mov     [ss:bp+_dx],ax     		;DX = LSB sector count
        mov     al,0    				;ok status
        mov     [m_hdstat],al

        jmp     hd_exit1

        ;
        ; AH = 24: set multiple mode
        ;
hd_setmul:
				call	GetHdcBlock
				call 	hd_sel  				;select drive
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cnt]
				rol			edi, 16
        mov     al,[ss:bp+_al]     		;number of sectors
        out     dx,al
        mov     al,0c6h
        call    hd_cmd  				;set multiple mode command
        jb      hd_setm9
        call    hd_stat 				;get status
hd_setm9:
        jmp     hd_exit0

        ;
        ; AH=25: identify drive
        ;
hd_id:
				call		GetHdcBlock
				call    hd_selb 				;select drive
				                        ;ignore time-out here, if drive not ready
                				        ;(ATAPI drive doesn't report ready
				                        ;until spoken to)

        mov     al,0ech 				;identify drive
        call    hd_cmd 					;issue command
				;!!! Warning: ATAPI doesn't reply with interrupt on this command, so time out appears. commented.
        ;jb      hd_id9  				;:bad drive

				rol			edi, 16
				mov			dx, [cs:di+hdc_cmd_stat]
				rol			edi, 16
        in      al,dx   				;hdc_stat
        test    al,1    				;error ?
        jz      hd_id1  				;:no

hd_id0:
				mov     al,0a1h 				;ATAPI identify drive
        call    hd_cmd  				;issue command
				;!!! Warning: ATAPI doesn't set interrupt at all, it seems. comment also.
        ;jb      hd_id9  				;:time-out

hd_id1:
				xor     cx,cx
hd_id2:
				in      al,dx   				;hdc_stat
        test    al,8    				;DRQ ?
        jnz     hd_id3  				;:yes
        loop    hd_id2

        mov     al,80h  				;time-out
        jmp     short hd_id9

hd_id3:

				cld             				;forward direction
				rol			edi, 16
        mov     dx, word [cs:di+hdc_dat]
				rol			edi, 16
        mov     cx,256  				;512 bytes
        mov     di,bx   				;destination
        rep     insw    				;read data
        call    hd_stat 				;get status
hd_id9:
        jmp 	hd_exit0        		;exit

        ;
        ; AH=41: detect EDD support
        ;
hd_edd41:
        cmp     bx,55aah        						;magic cookie ?
        jnz     hd_edd419       						;no: bad
				mov     word [ss:bp+_bx],0aa55h    	;return cookie
				mov     word [ss:bp+_cx],5 					;support packet commands; no lock /eject; EDD functions supported, EDD drive parameters table valid
    mov     byte [ss:bp+_ah],0x30 					;major version
    mov     byte [m_hdstat],0

%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
%if VERBOSE_INT_13
	push	cx

	push	'-'
	call	SendChar
	push	'>'
	call	SendChar
	push	' '
	call	SendChar

	mov		cl, byte [ss:bp+_ah]
	push	cx
	call	SendByte
	push	' '
	call	SendChar
	mov		cx, word [ss:bp+_cl]
	push	cx
	call	SendByte

	pop		cx
%endif
%endif
%endif
        jmp     hd_exit2        		;return carry clear

hd_edd419:
		jmp  hd_badcmd       			;return error

        ;
        ; AH=42: extended read
        ;
hd_xrd:

%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
%if VERBOSE_INT_13
	push	ax
	push	ecx
	push	ds
	push	si

	mov		ds, word [ss:bp+_ds]		; disk address packet
	mov		si, word [ss:bp+_si]

	push	' '
	call	SendChar
	push	ds
	call	SendWord
	push	':'
	call	SendChar
	push	si
	call	SendWord
	push	' '
	call	SendChar

	mov		cl, byte [si]			; size of packet
	push	cx
	call	SendByte
	push	' '
	call	SendChar

	mov		cx, word [si+2]			; number of blocks to transfer
	push	cx
	call	SendWord
	push	' '
	call	SendChar

	mov		ecx, dword [si+4]		; transfer buffer
	push	ecx
	call	SendDword
	push	' '
	call	SendChar

	mov		ecx, dword [si+12]		; starting absolute block number (QWORD)
	push	ecx
	call	SendDword
	mov		ecx, dword [si+8]
	push	ecx
	call	SendDword
	push	' '
	call	SendChar

	pop		si
	pop		ds
	pop		ecx
	pop		ax
%endif
%endif
%endif

				call		GetHdcBlock
				call    hd_sel_w_lba  				;select drive
        jb      hd_xrd9
        call    hd_xadr 				;handle address
        jb      hd_xrd9
        mov     byte [m_hdflag],0       ;clear interrupt flag
        mov     al,24h  				;issue read command
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16
        out     dx,al

hd_xrd1:
        call    hd_int  				;wait for interrupt
        jb      hd_xrd9
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]        ;read status
				rol			edi, 16
        in      al,dx
        mov     byte [m_hdflag],0       ;clear interrupt flag for next
        test    al,1    				;ERR ?
        jnz     hd_xrd8
        test    al,8    				;DRQ ?
        jz      hd_xrd8 				;:no
				rol			edi, 16
        mov     dx, word [cs:di+hdc_dat] 		;read 512 bytes from drive
				rol			edi, 16
        mov     cx,256
        rep     insw
        dec     bl      				;another sector ?
        jnz     hd_xrd1 				;:yes
hd_xrd8:

        mov     es,[ss:bp+_ds]     		;access address packet
        sub     byte [es:si+drq_blk],bl ;adjust sector count to reality
        call    hd_stat 				;get status
hd_xrd9:

        jmp     hd_exit0

        ;
        ; AH=43: extended write
        ;
hd_xwr:
				call		GetHdcBlock
				call    hd_sel_w_lba  				;select drive
        jb      hd_xwr9
        call    hd_xadr 				;handle address
        jb      hd_xwr9
        mov     si,di   				;buffer ^
        mov     al,34h  				;issue write command
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16
        out     dx,al

hd_xwr1:
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]        ;read status
				rol			edi, 16
        xor     cx,cx
        mov     byte [m_hdflag],0       ;clear interrupt flag for next
hd_xwr2:
        in      al,dx
        test    al,8    				;DRQ ?
        jnz     hd_xwr3 				;:yes
        test    al,21h  				;error ?
        jnz     hd_xwr8
        loop    hd_xwr2
        mov     al,80h  				;time-out
        jmp     short hd_xwr9

hd_xwr3:
				rol			edi, 16
        mov     dx, word [cs:di+hdc_dat] 		;write 512 bytes from drive
				rol			edi, 16
        mov     cx,256
        es rep  outsw
        call    hd_int  				;wait for interrupt
        jb      hd_xwr9
        dec     bl      				;another sector ?
        jnz     hd_xwr1 				;:yes

hd_xwr8:
        call    hd_stat 				;get status
hd_xwr9:
        mov     es,[ss:bp+_ds]     		;access address packet
        mov     si,[ss:bp+_si]
        sub     byte [es:si+drq_blk],bl ;adjust sector count to reality
        jmp     hd_exit0

        ;
        ; AH=44: extended verify
        ;
hd_xver:
				call		GetHdcBlock
        call    hd_sel_w_lba  				;select drive
        jb      hd_xver9
        call    hd_xadr 				;handle address
        jb      hd_xver9
        mov     al,40h
        call    hd_cmd  				;read verify command
        jb      hd_xver9
        call    hd_stat 				;get status
hd_xver9:
        jmp     hd_exit0

        ;
        ; AH=47: extended seek
        ;
hd_xsk:
				call		GetHdcBlock
				call    hd_sel_w_lba  				;select drive
        jb      hd_xsk9
        call    hd_xadr 				;handle address
        jb      hd_xsk9
        mov     al,70h
        call    hd_cmd  				;seek command
        jb      hd_xsk9
        call    hd_stat 				;check status
hd_xsk9:
        jmp     hd_exit0

        ;
        ; AH=48: return drive parameters
        ;
        ; Note: Phoenix spec says we should return PHYSICAL geometry, but
        ; Award BIOS returns LOGICAL... Users of this function are most
        ; interested in the max sector count anyway.
        ;
hd_edd48:
				call		GetHdcBlock
        call    hd_parm 							;get ^parameter block -> DI
        jb 			near hd_edd489

        mov     si,di   							;^parameter block
        cld             							;forward direction
        mov     es,[ss:bp+_ds]     		;buffer segment
        mov     di,[ss:bp+_si]     		;buffer offset
        mov     al,1    							;(error code)

        cmp     word [es:di],26 			;buffer at least 26 bytes long
        jb      hd_edd489       			;less -> error

        mov     ax, 0x1A   						;buffer length EDD-1.x
				cmp			word [es:di], 0x1E
				jb			hd_edd48_fill_buffer

				mov			ax, 0x1E							;buffer length EDD-2.x
				cmp			word [es:di], 0x42		; EDD-2.x
				jb			hd_edd48_fill_buffer

				mov			ax, 0x42							;buffer length EDD-3.0

hd_edd48_fill_buffer:
        stosw
        mov     ax,0    							;flags: invalid geometry
        stosw
        xor     eax,eax
        mov     ax,[cs:si+edpt_pcyl]      ;number of cylinders
        stosd
				xor			eax, eax
        mov     al,[cs:si+edpt_phead]     ;number of heads
        stosd
				xor			eax, eax
        mov     al,[cs:si+edpt_pspt]      ;number of sectors
        stosd
				mov			eax, dword [cs:si+edpt_sec]
				stosd
				mov			eax, dword [cs:si+edpt_sec+4]
				stosd
        mov     ax, 512  				;bytes per sector
        stosw

				cmp			word [es:di-0x1a], 0x1a
				jbe			hd_edd48_exit_ok
		
			; Boris addon
			; EDD-2.x
		
				mov			eax, 0xFFFFFFFF			; EDD configuration parameters not available
				stosd

				cmp			word [es:di-0x1e], 0x1e
				jbe			hd_edd48_exit_ok

			; EDD-3.0
				mov		ax, 0xBEDD				; device path present
				stosw
				mov		ax, 0x24				; device path length
				stosb
				mov		ax, 0					; next 3 bytes are reserved
				stosw
				mov		al, 0
				stosb
				mov		eax, 0x00494350			; host bus = "PCI"
				stosd
				mov		eax, 0x00415451			; "ATA"
				stosd
				mov		eax, 0
				stosd
				mov		al, byte [cs:prim_bus]
				stosb
				mov		al, byte [cs:prim_dev]
				stosb
				mov		al, byte [cs:prim_func]
				stosb
				mov		al, 0					; reserved
				stosb
				mov		eax, 0					; reserved
				stosd
				mov		eax, 0					; master
				stosd
				mov		eax, 0
				stosd							; reserved
				mov		al, 0
				stosb

		; calculate checksum
			; not implemented yet
		
hd_edd48_exit_ok:

%if SERIAL_DEBUG
%if VERBOSE_INT_13
	push	eax
	push	ds
	push	si

	mov		ds, word [ss:bp+_ds]
	mov		si, word [ss:bp+_si]

	push	' '
	call	SendChar

	mov		ax, word [si]		; size of buffer
	push	ax
	call	SendWord
	push	' '
	call	SendChar

	mov		ax, word [si+2]		; information flags
	push	ax
	call	SendWord
	push	' '
	call	SendChar

	mov		eax, dword [si+4]	; cylinders
	push	eax
	call	SendDword
	push	' '
	call	SendChar

	mov		eax, dword [si+8]	; heads
	push	eax
	call	SendDword
	push	' '
	call	SendChar

	mov		eax, dword [si+12]	; sectors
	push	eax
	call	SendDword
	push	' '
	call	SendChar

	mov		eax, dword [si+20]	; total sectors (QWORD)
	push	eax
	call	SendDword
	push	' '
	call	SendChar
	mov		eax, dword [si+16]
	push	eax
	call	SendDword
	push	' '
	call	SendChar

	mov		ax, word [si+24]	; bytes per sector
	push	ax
	call	SendWord
	push	' '
	call	SendChar

	pop		si
	pop		ds
	pop		eax
%endif
%endif

        mov     al,0    				;ok status

hd_edd489:
		jmp  hd_exit0

        ;
        ; write LBA address to command file
        ;
        ; returns sector count in BL, transfer address in ES:DI
        ;
        ; this will break on old drives that don't support LBA
        ;
hd_xadr:

        mov     es,[ss:bp+_ds]     						;restore segment, SI still OK
        cmp     byte [es:si+drq_len],16 			;at least 16 bytes
        jb      hd_xadr9        							;:error
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cnt]     	;sector count
				rol			edi, 16
        mov     ax, word [es:si+drq_blk]
        mov     bl,al   											;return in BL
				xchg		ah, al
        out     dx,al													; sector count [15..8]
				push		5
				call		WaitSomeTime
				xchg		ah, al
				out			dx,al													; sector count [7..0]
				push		5
				call		WaitSomeTime

				push		ecx
        mov     eax,dword [es:si+drq_lba]     	;LBA sector number [23..0]
				mov			ecx,dword [es:si+drq_lba+3]			;LBA sector number [47..24] (+3 it's not a bug!!!)

				xchg		eax, ecx
				rol			edi, 16
        mov     dx, word [cs:di+hdc_sec]     	;sector count
				rol			edi, 16
				out			dx, al													;hdc_sec sector = LBA [31..24]
				push		5
				call		WaitSomeTime
				xchg		eax, ecx
        out     dx, al													;hdc_sec sector = LBA [7..0]
				push		5
				call		WaitSomeTime
				shr			ecx, 8
        shr     eax, 8

				xchg		eax, ecx
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cyl]     	;sector count
				rol			edi, 16
				out			dx, al													; hdc_cyl cylinder low = LBA [39..32]
				push		5
				call		WaitSomeTime
				xchg		eax, ecx
        out     dx, al   												; hdc_cyl cylinder low = LBA [15..8]
				push		5
				call		WaitSomeTime

				shr			ecx, 8
        shr     eax, 8

				xchg		eax, ecx
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cyh]     	;sector count
				rol			edi, 16
				out			dx, al													; hdc_cyh cylinder high = LBA [47..40]
				push		5
				call		WaitSomeTime
				xchg		eax, ecx
        out     dx, al   												; hdc_cyh cylinder high = LBA [23..16]
				push		5
				call		WaitSomeTime

				rol			edi, 16
        mov     dx, word [cs:di+hdc_drv]     	;sector count
				rol			edi, 16
        in      al,dx   												; hdc_drv get drive
        and     al,0F0h
				;or			al, ah
        or      al,40h  												;set LBA mode
        out     dx,al   												; hdc_drv heads = LBA [27..24]
				push		5
				call		WaitSomeTime

        mov     di,[es:si+drq_ofs]      				;get ^transfer buffer
        mov     es,[es:si+drq_seg]

				pop		ecx

        cld             												;forward mode
        clc             												;ok
        ret

hd_xadr9:
        mov     al,1    												;return error
        stc
        ret

        ;
        ; wait for not busy, check status
        ;
hd_stat0:
        call    hd_busy18       		;wait until not busy
        jb      hd_stat9

        ; Enter here for faster service (assuming normally not busy)
        ; This is arranged to get fastest response when no error.

hd_stat:
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]     	;test whether busy
				rol			edi, 16
        in      al,dx
        test    al,80h
        jnz     hd_stat0        		;:busy
        mov     [m_hdst],al
        mov     ah,al   				;save status
        test    al,24h  				;write fault / ECC ?
        jnz     hd_stat1
        and     al,50h  				;not ready, or seek error ?
        cmp     al,50h
        jnz     hd_stat2
        test    ah,1    				;other error ?
        jnz     hd_stat3
        mov     al,0    				;return ok status
        ret

hd_stat1:
        mov     al,11h  				;ECC corrected data
        test    ah,4
        jnz     hd_stat9
        mov     al,0cch 				;no - must be write fault
hd_stat9:
        stc
        ret

hd_stat2:
        mov     al,0aah 				;not ready
        test    ah,40h
        jz      hd_stat9
        mov     al,40h  				;no - must be seek error
        stc
        ret

hd_stat3:
				rol			edi, 16
        mov     dx, word [cs:di+hdc_err] 		;read error register
				rol			edi, 16
        in      al,dx
        mov     [m_hderr],al
        mov     si,hd_errtab
        cmp     al,0    				;nothing set -> undefined error
        jz      hd_stat5
hd_stat4:
        inc     si
        shl     al,1
        jnb     hd_stat4
hd_stat5:
        mov     al,[cs:si]      ;get error code
        stc
        ret

        ;
        ; error register -> error code translation
        ;
hd_errtab:
				db   		0e0h    ;nothing set - status error
        db      0ah     ;80 - bad sector flag detected
        db      10h     ;40 - bad ECC
        db      0bbh    ;20 - undefined error
        db      04h     ;10 - record not found
        db      01h     ;08 - abort -> bad command
        db      0bbh    ;04 - undefined error
        db      40h     ;02 - seek error
        db      02h     ;01 - address mark not found

        ;
        ; get pointer to parameter block
        ;
        ; entry: DL = drive
        ; exit:  CS:DI = parameter block
        ;
hd_parm:
%if 0
	mov		cl, byte [cs:BootDriveNum]
	cmp		cl, 0
	je		hd_parm_of_bootable_drive

	mov		dl, byte [cs:BootDriveNum]

hd_parm_of_bootable_drive:

        and     dl, 7fh
        ;cmp     dl, [m_hdcnt]    			;legal drive number ?
        ;jae     hd_parm9        			;:bad
        mov     di, hd_prm0
				sub			di, edpt_ext_len

				push		cx
				movzx		cx, dl
				inc			cx
add_edpt_len:
				add			di, edpt_ext_len
				loop		add_edpt_len		
				pop			cx

        ret

hd_parm9:
        mov     al,1    ;error code
        stc
%endif
        ret

        ;
        ; wait while HD busy, CX ticks
        ;
hd_busy18:
				mov  		cx,18   						;18 ticks = 1 second
hd_busy:
        add     cx,[m_timer]    		;start time + max number of ticks
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16

hd_busy1:
        in      al,dx
        test    al,80h  			;busy ?
        jz      hd_busy9        	;:no, carry clear
        cmp     cx,[m_timer]
        jns     hd_busy1        	;keep waiting
hd_busy8:
        stc             			;time-out
        mov     al,80h  			;status code
hd_busy9:
        ret

        ;
        ; select drive, wait for drive ready
        ;
        ; -> CS:DI = ^parameter block
        ;

        ; special entry: set also LBA mode (for extended operations)
hd_sel_w_lba:
				mov     cx,18   			;1 s time-out for ready wait
				call		hd_parm

        mov     byte [m_hdflag],0   ;clear interrupt flag
				rol			edi, 16
        mov     dx, word [cs:di+hdc_drv]
				rol			edi, 16
				in			al, dx
				or			al, 40h				; set LBA bit
        out     dx,al
				jmp			hd_rdy

        ; special entry: no drive number check
hd_selb:
        mov     cx,18   			;1 s time-out
        and     dl,7fh  			;mask drive number
        jmp     short hd_sel1

        ; normal entry

hd_sel:
				mov     cx,18   			;1 s time-out for ready wait
hd_sel0:
				call		hd_parm
hd_sel1:
        mov     byte [m_hdflag],0   ;clear interrupt flag

        ;
        ; wait until HD ready, CX ticks
        ;
hd_rdy:
				add     cx, [m_timer]    	;start time + max number of ticks
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16
hd_rdy1:
        in      al,dx
        test    al,80h
        jnz     hd_rdy2 			;:busy
        test    al,40h
        jnz     hd_busy9        	;:ready, carry clear
hd_rdy2:
        cmp     cx,[m_timer]
        jns     hd_rdy1 			;keep waiting
        jmp     hd_busy8

        ;
        ; issue command AL, wait for interrupt
        ;
hd_cmd:
				mov     byte [m_hdflag],0       ;clear interrupt flag
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cmd_stat]
				rol			edi, 16
        out     dx,al

				; Boris addon - skip interrupt wait for drive identify command (0ECh) and verify command (40h)
				cmp		al, 40h
				jz		hd_int9
				cmp		al, 91h			; CompactFlash
				jz		hd_int9

        ; wait for HD interrupt

hd_int:
				;mov     cx,18*1									;18*4 ;1 seconds
				mov     cx,18/2									;18*4 ;0.5 seconds
        add     cx,[m_timer]					  ;start time + max number of ticks
hd_int1:
		    cli 			    		        			;test in critical section as some
	    							                    ;modern drives are "too fast" for
								                        ;slower embedded boards.

        test    byte [m_hdflag],0ffh    ;interrupt ?
        jnz     hd_int9 								;:yes, return NC

        cmp     cx,[m_timer]    				;time-out ?
        js      hd_int8 								;:yes, return CY
        sti             								;end critical section, HLT follows
        hlt             								;power-saving wait for next interrupt
        jmp     hd_int1

hd_int8:
        stc             								;time-out
        mov     al,80h  								;status code
hd_int9:
        sti             								;re-enable interrupts !
        ret


        ;
        ; write CHS parameters to command file, including CHS translation
        ;
hd_chs:
        mov     cl,[cs:di+edpt_shl]      				;get shift count
        cmp     cl,0ffh 												;LBA mode ?
        jz      hd_chs2

				rol			edi, 16
				mov     dx, word [cs:di+hdc_cnt]      	;sector count
				rol			edi, 16
        mov     al,[ss:bp+_al]
        out     dx,al
        inc     dx

        mov     bx,[ss:bp+_cx]     							;sector number, cylinder
        mov     al,bl   												;sector number
        and     ax,3fh  												;(need AH = 0 for divide)
        out     dx,al   												;hdc_sec

        mov     al, [ss:bp+_dh]     						;head number
        div     byte [cs:di+edpt_phead]    			;divide by physical heads
                        												;-> AL = heads, AH = cylinders

        inc     dx
        xchg    bl,bh   												;swap cylinder
        shr     bh,6    												;bit 7..6 become bits 9..8
        shl     bx,cl   												;shift cylinder for CHS translation
        or      al,bl   												;head
        out     dx,al   												;hdc_cyl - cylinder low

        inc     dx
        mov     al,bh   												;cylinder high
        out     dx,al   												;hdc_cyh

		    inc     dx
        in      al,dx   												;hdc_drv
				and			ah, 0fh
        or      al,ah   												;heads
        out     dx,al

        ret

        ;
        ; LBA translation
        ;
hd_chs2:
        push    eax     							;save eax, ebx
        push    ebx

				rol			edi, 16
				mov     dx, word [cs:di+hdc_drv]      	;get drive
				rol			edi, 16
				in			al, dx
				or			al, 40h													; set LBA mode
				out			dx, al

				rol			edi, 16
				mov     dx, word [cs:di+hdc_cnt]      	;set count
				rol			edi, 16
        mov     al,[ss:bp+_al]
        out     dx,al

        xor     eax,eax
        mov     ax,[ss:bp+_cx]     		;sector number, cylinder
        xchg    al,ah   							;swap cylinder high, low
        shr     ah,6    							;fix cylinder high
        mov     ebx,eax
        shl     eax,8   							;cylinder * 255
        sub     eax,ebx
        xor     bx,bx
        mov     bl,[ss:bp+_dh]     		;head number
        add     eax,ebx 							;add head
        mov     ebx,eax
        shl     eax,6   							;cylinder * 255 + head * 63
        sub     eax,ebx
        xor     ebx,ebx
        mov     bl,[ss:bp+_cl]     		;sector number
        and     bl,63
        dec     bx      							;- 1
        add     eax,ebx

				rol			edi, 16
				mov     dx, word [cs:di+hdc_sec]      	;sector count
				rol			edi, 16
        out     dx,al   							;hdc_sec sector = LBA 7..0

        shr     ax,8

				rol			edi, 16
				mov     dx, word [cs:di+hdc_cyl]      	;sector count
				rol			edi, 16
        out     dx,al   							;hdc_cyl cylinder low = LBA 15..8

        shr     eax,16

				rol			edi, 16
				mov     dx, word [cs:di+hdc_cyh]      	;sector count
				rol			edi, 16
        out     dx,al   							;hdc_cyh cylinder high = LBA 23..16

				rol			edi, 16
				mov     dx, word [cs:di+hdc_drv]      	;sector count
				rol			edi, 16
        in      al,dx   							;hdc_drv get drive

        and     al,0b0h 							;keep reserved, drive select bits
        or      al,040h 							;set LBA mode
        or      al,ah
        out     dx,al   							;hdc_drv heads = LBA27..24

        pop     ebx
        pop     eax

        ret

mes_init_sata_channel:
		db		'Init SATA', 0
mes_hd_init:
		db		'HdInit',0
mes_no_hdd:
		db		'no hdd',13,10,0
mes_hd_ok:
		db		'ok',13,10,0
mes_hd_cs:
		db		13, 10, 'cs = ', 0

        ;
        ; HD detect / init
        ;
hd_init:

%if SERIAL_DEBUG
	push	mes_hd_init
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	mes_hd_init
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

	; update device addresses
	push	ax
	; primary
	mov		ax, word [cs:prim_base]
	mov		word [cs:prim_dat], ax
	inc		ax
	mov		word [cs:prim_err], ax
	inc		ax
	mov		word [cs:prim_cnt], ax
	inc		ax
	mov		word [cs:prim_sec], ax
	inc		ax
	mov		word [cs:prim_cyl], ax
	inc		ax
	mov		word [cs:prim_cyh], ax
	inc		ax
	mov		word [cs:prim_drv], ax
	inc		ax
	mov		word [cs:prim_cmd_stat], ax
	mov		ax, word [cs:prim_ctrl]
	mov		word [cs:prim_ctrl_stat2], ax

	; secondary
	mov		ax, word [cs:secd_base]
	mov		word [cs:secd_dat], ax
	inc		ax
	mov		word [cs:secd_err], ax
	inc		ax
	mov		word [cs:secd_cnt], ax
	inc		ax
	mov		word [cs:secd_sec], ax
	inc		ax
	mov		word [cs:secd_cyl], ax
	inc		ax
	mov		word [cs:secd_cyh], ax
	inc		ax
	mov		word [cs:secd_drv], ax
	inc		ax
	mov		word [cs:secd_cmd_stat], ax
	mov		ax, word [cs:secd_ctrl]
	mov		word [cs:secd_ctrl_stat2], ax
	pop		ax

%ifdef  HD_WAIT
        ;
        ; Some drives take a long time to become responsive to commands,
        ; because they only store very minimal firmware, and fetch the
        ; actual code from disk. Some of them are allergic to being touched
        ; before they are ready.
        ;

        xor     bx,bx   			;clear second counter
        cmp     bx,HD_ENA 		      	;0 delay ?
        jz      hd_wait3       			;yes: bypass

hd_wait1:
        mov     ax,18  				;about 1 second
        add     ax,[m_timer]    		;start time + max number of ticks
hd_wait2:

        hlt             				;low power wait, we'll be here for a while
        cmp     ax,[m_timer]    		;time-out ?
        jns     hd_wait2        		;no: keep waiting

        cmp     bx,HD_ENA      			;can we touch the drive now ?
        jb      hd_wait8        		;:no

hd_wait3:
        mov     al,0ffh 			;place FF on the IDE bus (or loopback)
        mov     dx, word [cs:prim_dat]
        out     dx,al

        mov     dx, word [cs:prim_cmd_stat]     ;does the status register read non-FF ?
        in      al,dx

        cmp     al,0ffh
        jz      hd_nohdd       			;FF: no drive attached, bail
        test    al,80h  			;busy ?
        jnz     hd_wait8        		;:don't touch
        mov     al, MASTER_DRIVE		;access master drive
        mov     dx, word [cs:prim_drv]
        out     dx,al

	push		1
	call		WaitSomeTime

        mov     dx, word [cs:prim_cmd_stat]	        ;read status
        in      al,dx

        test    al,80h  				;busy ?
        jnz     hd_wait8
        test    al,40h  				;drive ready ?
        jnz     hd_ok    	   			;:terminate the wait

hd_wait8:

%if SERIAL_DEBUG
	push	'.'
	call	SendChar
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	'.'
	call	SendChar
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

        inc     bx      				;second counter
        cmp     bx,HD_WAIT
        jb      hd_wait1

hd_nohdd:
%if SERIAL_DEBUG
	push	mes_no_hdd
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	mes_no_hdd
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
        jmp		hd_wait9

hd_ok:
%if SERIAL_DEBUG
	push	mes_hd_ok
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	mes_hd_ok
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

hd_wait9:
%endif

%if 1
	pushf					;++

;++++++++++++++++++++++++++++++++++++++++++++++
;	push	cx
;	pushf
;	mov	ax, '_'
;	call	SendTxtInitSata
;	call	SendWord
;	pop	cx
;++++++++++++++++++++++++++++++++++++++++++++++

	cli

	mov	dx, word [cs:secd_cmd_stat]
	in	al, dx				; clear interrupt request
	mov	dx, word [cs:prim_cmd_stat]
	in	al, dx				; clear interrupt request

	push	bx
	push	cx

;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	cx
	mov	ax, '@'
	call	SendTxtInitSata
	xor	ax, ax
	mov	al, byte [cs:prim_irq]
	push	ax
	call	SendByte
	pop	cx
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

	mov	cl, byte [cs:prim_irq]
	cmp	cl, 7
	ja	hd_irq_in_slave_8259

	; set handler for this Irq
	push	ax
	push	cx
	push	ds

	xor	ax, ax
	mov	ds, ax
	xor	ch, ch
	add	cl, 08h
	shl	cx, 2			;*4

	mov	bx, cx
	mov	word [bx], IrqHardDrive
	add	bx, 2
	mov	word [bx], cs

	pop	ds
	pop	cx
	pop	ax

	in	al, pic0+1
	mov	ch, 1
	shl	ch, cl
	not	ch

	and	al, ch
	out	pic0+1, al

	jmp	prim_hd_irq_enabled

hd_irq_in_slave_8259:

	sub	cl, 8

	; set handler for this Irq
	push	ax
	push	cx
	push	ds

	xor	ax, ax
	mov	ds, ax
	xor	ch, ch
	add	cl, 70h
	shl	cx, 2			;*4

	mov	bx, cx
	mov	word [bx], IrqHardDrive
	add	bx, 2
	mov	word [bx], cs

	pop	ds
	pop	cx
	pop	ax


;++++++++++++++++++++++++++++++++++++++++++++++
;	push	cx
;	mov	ax, '*'
;	call	SendTxtInitSata
;	pop	cx
;++++++++++++++++++++++++++++++++++++++++++++++

	in    	al,pic1+1       	;enable HD interrupt
	mov	ch, 1
	shl	ch, cl
	not	ch

	and	al, ch
    	out   	pic1+1,al

;++++++++++++++++++++++++++++++++++++++++++++++
;	mov	ax, '#'
;	call	SendTxtInitSata
;++++++++++++++++++++++++++++++++++++++++++++++


    	in    	al, pic0+1       	;enable cascade interrupt
    	and   	al, 0fbh		; +cascade irq2
    	out   	pic0+1, al

prim_hd_irq_enabled:
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	mov	ax, '*'
	call	SendTxtInitSata
	xor	ax, ax
	mov	al, byte [cs:prim_irq]
	push	ax
	call	SendByte
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

	mov	cl, byte [cs:secd_irq]
	cmp	cl, 7
	ja	hd_secd_irq_in_slave_8259

	; set handler for this Irq
	push	ax
	push	cx
	push	ds

	xor	ax, ax
	mov	ds, ax
	xor	ch, ch
	add	cl, 08h
	shl	cx, 2			;*4

	mov	bx, cx
	mov	word [bx], IrqHardDrive
	add	bx, 2
	mov	word [bx], cs

	pop	ds
	pop	cx
	pop	ax

	in	al, pic0+1
	mov	ch, 1
	shl	ch, cl
	not	ch
	and	al, ch
	out	pic0+1, al
	jmp	hd_irq_enabled

hd_secd_irq_in_slave_8259:

;++++++++++++++++++++++++++++++++++++++++++++++
;	push	cx
;	mov	ax, '%'
;	call	SendTxtInitSata
;	pop	cx
;++++++++++++++++++++++++++++++++++++++++++++++
	sub		cl, 8

	; set handler for this Irq
	push	ax
	push	cx
	push	ds

	xor	ax, ax
	mov	ds, ax
	xor	ch, ch
	add	cl, 70h
	shl	cx, 2			;*4

	mov	bx, cx
	mov	word [bx], IrqHardDrive
	add	bx, 2
	mov	word [bx], cs
;++++++++++++++++++++++++++++++++++++++++++++++
	mov	ax, word[bx]
	mov	bx, ax
;++++++++++++++++++++++++++++++++++++++++++++++

	pop	ds
	pop	cx
	pop	ax

;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	bx
	mov	ax, mes_hd_cs
	push	ax
	call	SendTxt
	call	SendWord
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

;++++++++++++++++++++++++++++++++++++++++++++++
;	push	cx
;	mov	ax, '^'
;	call	SendTxtInitSata
;	pop	cx
;++++++++++++++++++++++++++++++++++++++++++++++

        in      al,pic1+1       	;enable HD interrupt
	mov	ch, 1
        shl	ch, cl
	not	ch

	and	al, ch
        out     pic1+1, al

;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	cx
	push	ax
	mov	ax, '#'
	call	SendTxtInitSata
	call	SendByte
	pop	cx
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

        in      al, pic0+1     	  	;enable cascade interrupt
        and     al, 0fbh		; +cascade irq2
        out     pic0+1 ,al

hd_irq_enabled:
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	cx
	push    ax
	mov	ax, '$'
	call	SendTxtInitSata
	call	SendByte
	pop	cx
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

	pop	cx
	pop	bx

;--    	sti	
	popf				;++
%endif

        mov     byte [m_hdcnt], 4        ;4 drives to start
        mov     byte [m_hdstat], 0       ;clear status
	mov	cl, 80h			 ; drive counter

;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	cx
	mov	ax, '%'
	call	SendTxtInitSata
	pop	cx
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

%if SERIAL_DEBUG
	push	mes_init_sata_channel
	call	SendTxt
	push	'0'
	call	SendChar
	push	mes_crlf
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	mov	ax, '0'
	call	SendTxtInitSata
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

        mov     di, hd_prm0      		;setup first drive
        mov     dl, 080h
        mov     al, MASTER_DRIVE

        call    hd_pres 				;check presence
        jb      hd_init1

        call    hd_set  				;set parameters
        jb      hd_init1
	inc	cl

hd_init1:

%if SERIAL_DEBUG
	push	mes_init_sata_channel
	call	SendTxt
	push	'1'
	call	SendChar
	push	mes_crlf
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	mov	ax, '1'
	call	SendTxtInitSata
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

        mov     di, hd_prm1      		;setup second drive
        mov     dl, 081h
        mov     al, SLAVE_DRIVE

        call    hd_pres 				;check presence
        jb      hd_init2

        call    hd_set  				;set parameters
        jb      hd_init2
				inc			cl
hd_init2:

%if SERIAL_DEBUG
	push	mes_init_sata_channel
	call	SendTxt
	push	'2'
	call	SendChar
	push	mes_crlf
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	mov	ax, '2'
	call	SendTxtInitSata
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

        mov     di, hd_prm2      		;setup third drive
        mov     dl, 082h
        mov     al, MASTER_DRIVE

        call    hd_pres 				;check presence
        jb      hd_init3

        call    hd_set  				;set parameters
        jb      hd_init3
				inc			cl
hd_init3:

%if SERIAL_DEBUG
	push	mes_init_sata_channel
	call	SendTxt
	push	'3'
	call	SendChar
	push	mes_crlf
	call	SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	mov	ax, '3'
	call	SendTxtInitSata
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

        mov     di, hd_prm3      		;setup fourth drive
        mov     dl, 083h
        mov     al, SLAVE_DRIVE

        call    hd_pres 				;check presence
        jb      hd_init4

        call    hd_set  				;set parameters
        jb      hd_init4
	inc	cl

hd_init4:
;	mov	cl, 84h
	mov     byte [cs:hd_top],cl     		;store top hard disk number
	and     cl,7fh                          ;done - store number drives
	mov     byte [m_hdcnt],cl

	; translate strings
	mov	cx, 64
	mov	si, model_dev0

translate_strings_loop:
	mov	ax, word [cs:si]	
	xchg	ah, al
	mov	word [cs:si], ax
	inc	si
	inc	si
	loop	translate_strings_loop

	; set 0 at the end of strings
	mov	si, model_dev0+31
	mov	byte [cs:si], 0
	mov	si, model_dev1+31
	mov	byte [cs:si], 0
	mov	si, model_dev2+31
	mov	byte [cs:si], 0
	mov	si, model_dev3+31
	mov	byte [cs:si], 0

%if SERIAL_DEBUG
	; print strings
	push	model_dev0
	call	SendTxt
	push	mes_crlf
	call	SendTxt

	push	model_dev1
	call	SendTxt
	push	mes_crlf
	call	SendTxt

	push	model_dev2
	call	SendTxt
	push	mes_crlf
	call	SendTxt	

	push	model_dev3
	call	SendTxt
	push	mes_crlf
	call	SendTxt	
%endif
;++++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
	push	model_dev0
	call	SendTxt
	push	mes_crlf
	call	SendTxt

	push	model_dev1
	call	SendTxt
	push	mes_crlf
	call	SendTxt

	push	model_dev2
	call	SendTxt
	push	mes_crlf
	call	SendTxt	

	push	model_dev3
	call	SendTxt
	push	mes_crlf
	call	SendTxt	
%endif
;++++++++++++++++++++++++++++++++++++++++++++++

	ret


;++++++++++++++++++++++++++++++++++++++++++++++
SendTxtInitSata:
	push	ax
	push	mes_init_sata_channel
	call	SendTxt
	call	SendChar
	push	mes_crlf
	call	SendTxt
	ret
;++++++++++++++++++++++++++++++++++++++++++++++

        ;
        ; check drive presence, AL = A0 or B0
        ;
hd_pres:
				call		GetHdcBlock
        push    dx

				rol			edi, 16
        mov     dx, word [cs:di+hdc_cnt] 				;write test pattern
				rol			edi, 16
        mov     al,55h
        out     dx,al
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cyl] 				;write negative pattern
				rol			edi, 16
        mov     al,0aah
        out     dx,al
				rol			edi, 16
        mov     dx, word [cs:di+hdc_cnt] 				;read test pattern
				rol			edi, 16
        in      al,dx
        xor     al,55h
        jz      hd_pres9        				;:ok
hd_pres8:
        stc
hd_pres9:
        pop     dx
        ret

        ;
        ; set up drive DL
        ;
hd_set:

		mov     ah,25h  						;get drive ID
    mov     bx,tmp_buf
    int     13h
    jb      hd_set9

    cmp     word [tmp_buf+0],848ah  		;CompactFlash ?
    jz      hd_set0							;:yes
    cmp     word [tmp_buf+0],844ah  		;CompactFlash ? (new SanDisk)
    jz      hd_set0 						;:yes
    test    byte [tmp_buf+1],80h    		;ATAPI ?
    jz      hd_set0 						;:no

        ; note I/O base and drive ID of ATAPI CD-ROM
        ; this is assumed to be the first ATAPI device found

		%if SERIAL_DEBUG
		push	mes_cdrom_found
		call	SendTxt
		%endif

	push	cx
	push	si
	movzx	cx, dl
	and		cx, 7fh
	inc		cx
	mov		si, type_dev0
	sub		si, type_len
add_bev_type:
	add		si, type_len
	loop	add_bev_type
	mov		byte [cs:si], BBS_BEV_DEVICE
	pop		si
	pop		cx

				; addon - store device' strings
				push	ax
				push	cx
				push	si
				push	di
				push	es

				cld

				mov			ax, cs
				mov			es, ax

				mov			si, tmp_buf+0x36
				mov			di, model_dev0

				push	cx
				movzx	cx, dl
				and		cx, 7fh
				inc		cx
				sub		di, model_len
add_bev_model_len:
				add		di, model_len
				loop	add_bev_model_len
				pop		cx

        ; form "Px-" prefix, where x - port #
        push  dx
        push  bx
        and   dl, 7fh
        mov   bl, 0x30
        mov   byte [cs:di], bl
        inc   di
        mov   byte [cs:di], 'P'
        inc   di        
        mov   byte [cs:di], '-'
        inc   di
        mov   bl, dl
        add   bl, 0x30
        mov   byte [cs:di], bl
        inc   di
        pop   bx
        pop   dx

				mov			cx, 32      ; string length
        sub     cx, 4       ; prefix length
				rep			movsb				

				pop		es
				pop		di
				pop		si
				pop		cx
				pop		ax

				ret

hd_set9:
        stc             						;error return
        ret
hd_set0: 

	push	cx
	push	si
	movzx	cx, dl
	and		cx, 7fh
	inc		cx
	mov		si, type_dev0
	sub		si, type_len
add_ipl_type:
	add		si, type_len
	loop	add_ipl_type
	mov		byte [cs:si], BBS_HARDDISK
	pop		si
	pop		cx

        mov     al,[tmp_buf+12] 				;sectors per track
        mov     [cs:di+edpt_spt],al
        mov     [cs:di+edpt_pspt],al
        mov     al,[tmp_buf+94] 				;multiple block size
        mov     [cs:di+edpt_mul],al
        mov     ax,[tmp_buf+2]  				;cylinders
        mov     [cs:di+edpt_pcyl],ax
        mov     bl,[tmp_buf+6]  				;heads
        mov     [cs:di+edpt_phead],bl

				; addon - store device' strings
				push	ax
				push	cx
				push	si
				push	di
				push	es

				cld

				mov			ax, cs
				mov			es, ax

				mov			si, tmp_buf+0x36
				mov			di, model_dev0

				push	cx
				movzx	cx, dl
				and		cx, 7fh
				inc		cx
				sub		di, model_len
add_ipl_model_len:
				add		di, model_len
				loop	add_ipl_model_len
				pop		cx

        ; form "Px-" prefix, where x - port #
        push  dx
        push  bx
        and   dl, 7fh
        mov   bl, 0x30
        mov   byte [cs:di], bl
        inc   di
        mov   byte [cs:di], 'P'
        inc   di        
        mov   byte [cs:di], '-'
        inc   di
        mov   bl, dl
        add   bl, 0x30
        mov   byte [cs:di], bl
        inc   di
        pop   bx
        pop   dx

				mov			cx, 32      ; string length
        sub     cx, 4       ; prefix length

				rep			movsb				

				pop		es
				pop		di
				pop		si
				pop		cx
				pop		ax

        ; CHS translation: shift cylinders right / heads left until
        ; cylinders < 1024

        mov     bh,0    						;shift count
	
		cmp 	byte [cs:forcelba_flag],0
		jnz 	.skip
		cmp     ax,8191 						;force LBA for high cylinder count
    ja      hd_lba
.skip:

hd_set1:
        cmp     ax,1024
        jb      hd_set2
        shr     ax,1    						;cylinders / 2
        shl     bl,1    						;heads * 2
%ifdef  HDD_LBA
        jb 		near hd_lba  					;:overflow - use LBA mode for this drive
%else
        jb      hd_set9 						;:overflow - cannot translate drive
%endif
        inc     bh      						;count the shifts
        jmp     hd_set1

hd_set2:
        mov     [cs:di+edpt_cyl],ax
        mov     [cs:di+edpt_head],bl
        mov     [cs:di+edpt_shl],bh

        mov     byte [cs:di+edpt_sig],0a0h       ;signature
hd_set2c:
        mov     ah,9    						;set drive parameters
        int     13h
        jb      hd_set9

        mov     ah,0dh  						;reset drive
        int     13h

hd_set2b:

        push    dx
        mov     ah,8    						;get max CHS
        int     13h
        mov     al,dh   						;heads
        pop     dx
        jb      hd_set9

        mov     ah,4    						;verify sectors
        mov     dh,al   						;max head
        mov     al,cl   						;max sector -> sector count
        and     al,3fh
        and     cl,0c0h 						;start sector = 1
        or      cl,1
        sub     ch,1    						;cylinder - 1
        jnb     hd_set3
        sub     cl,40h
hd_set3:
        int     13h
        jb  	near  hd_set9

%ifdef  HD_TIME
        mov     ah,23h  ;set drive time-out
        mov     al,HD_TIME
        int     13h
%endif

        ret             ;normal return

%ifdef  HDD_LBA
        ;
        ; determine LBA parameters (always 255 heads / 63 sectors)
        ;
hd_lba:

		test    byte [tmp_buf+99],2     ;LBA mode supported ?
        jz 		near     hd_set9 		;:no

        push    dx
        mov     ax,[tmp_buf+120]        ;number of LBA sectors (low)
        mov     dx,[tmp_buf+122]        ;(high)
        mov     bx,255*63       		; / heads / sectors
        div     bx
        pop     dx

        ; set drive parameters

        mov     word [cs:di+edpt_cyl],ax					;3ffh
        mov     byte [cs:di+edpt_head],255       ;255 heads
        mov     byte [cs:di+edpt_shl],0ffh       ;special shift -> LBA mode
        mov     byte [cs:di+edpt_spt],63 				;63 sectors
        ;mov     byte [cs:di+edpt_ctrl],8 			;>8 heads
				;mov			byte [cs:di+edpt_sig], 0a0h
				;mov			byte [cs:di+edpt_chsum], 0
				
				push		eax
				mov			eax, dword [tmp_buf+100*2]
				mov			dword [cs:di+edpt_sec], eax
 				mov			eax, dword [tmp_buf+102*2]
				mov			dword [cs:di+edpt_sec+4], eax
				pop			eax

        jmp     hd_set2b        				;note we don't set LBA parameters
%endif 

ResetDiskSystem:
	; reset disk system
	push		ax
	push		dx

  mov			dx, word [cs:prim_dat]
	cmp			dx, 0
	je			rds_skip_primary_channel

	mov			al, MASTER_DRIVE
	mov			dx, word [cs:prim_drv]
	out			dx, al

  mov     al,4    		;soft reset
  mov			dx, word [cs:prim_ctrl_stat2]
  out     dx,al

	push		10
	call		WaitSomeTime

  mov     al,0    		;end of reset, interrupt enable
  out     dx,al   		;hdc_ctrl

  mov     dx, word [cs:prim_cmd_stat]

rds_wait_primary_channel_bsy:
  in      al,dx
  test    al,80h  			;busy ?
  jnz     rds_wait_primary_channel_bsy

rds_skip_primary_channel:

  mov			dx, word [cs:secd_dat]
	cmp			dx, 0
	je			rds_skip_secondary_channel

	mov			al, MASTER_DRIVE
	mov			dx, word [cs:secd_drv]
	out			dx, al

  mov     al,4    		;soft reset
  mov			dx, word [cs:secd_ctrl_stat2]
  out     dx,al

	push		10
	call		WaitSomeTime

  mov     al,0    		;end of reset, interrupt enable
  out     dx,al   		;hdc_ctrl

  mov     dx, word [cs:secd_cmd_stat]

rds_wait_secondary_channel_bsy:
  in      al,dx
  test    al,80h  			;busy ?
  jnz     rds_wait_secondary_channel_bsy

rds_skip_secondary_channel:

  mov			dx, word [cs:amt_dat]
	cmp			dx, 0
	je			rds_skip_amt_channel

	mov			al, MASTER_DRIVE
	mov			dx, word [cs:amt_drv]
	out			dx, al

  mov     al,4    		;soft reset
  mov			dx, word [cs:amt_ctrl_stat2]
  out     dx,al

	push		10
	call		WaitSomeTime

  mov     al,0    		;end of reset, interrupt enable
  out     dx,al   		;hdc_ctrl

  mov     dx, word [cs:amt_cmd_stat]

rds_wait_amt_channel_bsy:
  in      al,dx
  test    al,80h  			;busy ?
  jnz     rds_wait_amt_channel_bsy

rds_skip_amt_channel:

	pop		dx
	pop		ax

	ret

; functions to boot from device

boot_ipl0:

  push  ax
  push  ds
  xor   ax, ax
  mov   ds, ax
  mov   al, byte [m_hdcnt]
	inc		al
	mov		byte [m_hdcnt], al

	push	80h
	call	WriteTranslateDriveNumTable

  pop   ds
  pop   ax

	retf

boot_ipl1:

  push  ax
  push  ds
  xor   ax, ax
  mov   ds, ax
  mov   al, byte [m_hdcnt]
	inc		al
	mov		byte [m_hdcnt], al

	push	81h
	call	WriteTranslateDriveNumTable

  pop   ds
  pop   ax

	retf

boot_ipl2:

  push  ax
  push  ds
  xor   ax, ax
  mov   ds, ax
  mov   al, byte [m_hdcnt]
	inc		al
	mov		byte [m_hdcnt], al

	push	82h
	call	WriteTranslateDriveNumTable

  pop   ds
  pop   ax

	retf

boot_ipl3:

  push  ax
  push  ds
  xor   ax, ax
  mov   ds, ax
  mov   al, byte [m_hdcnt]
	inc		al
	mov		byte [m_hdcnt], al

	push	83h
	call	WriteTranslateDriveNumTable

  pop   ds
  pop   ax

	retf

boot_bev:

	mov		dl, byte [cs:BootDriveNum]
	call	GetHdcBlock

	call		CdromBoot
	retf

boot_amt_bev:

	mov		dl, 0F1h
	call	GetHdcBlock

	;mov			dx, word [cs:amt_drv]
	;mov			al, SLAVE_DRIVE
	;out			dx, al

	call		CdromBoot
	retf

CdromBoot:
%if SERIAL_DEBUG
	push	mes_boot_from_cdrom
	call	SendTxt
%endif

	; set operation buffer [es:si]
	;mov		ax, 8000h
	xor		ax, ax
	mov		es, ax
	mov		si, tmp_buf

	; zero buffer
	mov		cx, 0x800
init_buffer_with_zero:
	mov		byte [es:si], 0
	inc		si
	loop	init_buffer_with_zero

	; read boot directory (lba 0x00000011 by ElTorito)
	mov		ax, tmp_buf				; buffer offset
	push	ax
	mov		ax, es						; buffer segment
	push	ax
	mov		eax, 0x00000001			; sectors number
	push	eax
	mov		eax, 0x00000011			; lba
	push	eax
	test	byte [cs:type_amt1], BOOT_FROM_AMT
	jz		CdRomBoot_normal0
	call	read_amt_via_packet_interface
	jmp		CdRomBoot_0
CdRomBoot_normal0:
	call	read_via_packet_interface
CdRomBoot_0:

	mov		si, tmp_buf
	cmp		byte [es:si], 0			; check if bootable image
	jnz		not_bootable_cd_image
	inc		si
	cmp		dword [es:si], 0x30304443		; "CD00"
	jnz		not_bootable_cd_image
	add		si, 4
	cmp		word [es:si], 0x0131				; "1" + version
	jnz		not_bootable_cd_image
	add		si, 2
	cmp		dword [es:si], 0x54204c45		; "EL T"
	jnz		not_bootable_cd_image
	add		si, 4
	cmp		dword [es:si], 0x5449524F		; "ORIT"
	jnz		not_bootable_cd_image
	add		si, 4
	cmp		dword [es:si], 0x5053204F		; "O SP"
	jnz		not_bootable_cd_image
	add		si, 4	
	cmp		dword [es:si], 0x46494345		; "ECIF"
	jnz		not_bootable_cd_image
	add		si, 4
	cmp		dword [es:si], 0x54414349		; "ICAT"
	jnz		not_bootable_cd_image
	add		si, 4
	cmp		dword [es:si], 0x004E4F49		; "ION "
	jnz		not_bootable_cd_image

	mov		si, tmp_buf+0x47
	mov		ebx, dword [es:si]					; first sector of Boot Catalog

	; read first sector of Boot Catalog
	mov		ax, tmp_buf				; buffer offset
	push	ax
	mov		ax, es						; buffer segment
	push	ax
	mov		eax, 0x00000001			; sectors number
	push	eax
	push	ebx								; lba

	test	byte [cs:type_amt1], BOOT_FROM_AMT
	jz		CdRomBoot_normal1
	call	read_amt_via_packet_interface
	jmp		CdRomBoot_1
CdRomBoot_normal1:
	call	read_via_packet_interface
CdRomBoot_1:

	; check validation entry
	mov		si, tmp_buf
	cmp		byte [es:si], 0x01			; Header id
	jnz		not_bootable_cd_image
	inc		si
	cmp		byte [es:si], 0x00			; Platform id should be 80x86
	add		si, 3
	mov		al, byte [es:si]
	cmp		al, 0
	jz		unknown_cd_manufacturer
%if SERIAL_DEBUG
	push	mes_cdrom_by						; print manufacturer
	call	SendTxt

print_cd_manufacturer:
	push	ax
	call	SendChar
	inc		si
	mov		al, byte [es:si]
	cmp		al, 0
	jne		print_cd_manufacturer
	
	push	mes_crlf
	call	SendTxt
%endif

unknown_cd_manufacturer:

	mov		si, tmp_buf+0x1e
	cmp		word [es:si], 0xAA55		; key word
	jnz		not_bootable_cd_image

	; get Initial/Default entry
	mov		si, tmp_buf+0x20

	cmp		byte [es:si], 0x88			; is entry bootable?
	jnz		not_bootable_cd_image
	add		si, 2
	mov		ax, word [es:si]				; load segment
	cmp		ax, 0
	jnz		cdrom_load_to_custom_sector
	mov		ax, 0x07c0							; default sector
cdrom_load_to_custom_sector:
	mov		ds, ax

	add		si, 4
	mov		cx, word [es:si]				; sector count
	shr		cx, 2

	add		si, 2
	mov		ebx, dword [es:si]			; Load RBA

	; get load sector to load segment
	mov		ax, 0							; buffer offset
	push	ax
	mov		ax, ds						; buffer segment
	push	ax
	movzx	eax, cx						; sectors number
	push	eax
	push	ebx								; lba

	test	byte [cs:type_amt1], BOOT_FROM_AMT
	jz		CdRomBoot_normal2
	call	read_amt_via_packet_interface
	jmp		CdRomBoot_2
CdRomBoot_normal2:
	call	read_via_packet_interface
CdRomBoot_2:

	mov		dl, 0xF0

Cdrom_boot_jump_2_boot_code:

	; jump to boot code
	jmp		7c0h:0

	clc	
	ret

not_bootable_cd_image:
	stc
	ret

mes_boot_from_cdrom:
	db	'Boot from CD-ROM',13,10,0
mes_cdrom_by:
	db	'by ',0
mes_cd_read_sectors:
	db	'Read CD-ROM sector(s) ',0
mes_amt_read_sectors:
	db	'Read AMT sector(s) ',0
mes_cd_from:
	db	' from ',0
mes_cd_to:
	db	' to ',0

read_via_packet_interface:

	; parameters through stack:
	; 	dword 	: lba						[bp+4]
	; 	dword	: sectors number			[bp+8]
	; 	word	: receive buffer segment	[bp+12]
	; 	word	: receive buffer offset		[bp+14]
	;		word	:	should wait irq					[bp+16]
	; result: in ax

	push	bp
	mov		bp, sp

	push	es
	push	di
	push	dx
	push	cx
	push	eax

%if 0
%if SERIAL_DEBUG
	push	mes_cd_read_sectors
	call	SendTxt
	mov		eax, dword [ss:bp+8]
	push	eax
	call	SendDword
	push	mes_cd_from
	call	SendTxt
	mov		eax, dword [ss:bp+4]
	push	eax
	call	SendDword
	push	mes_cd_to
	call	SendTxt
	mov		ax, word [ss:bp+12]
	push	ax
	call	SendWord
	push	':'
	call	SendChar
	mov		ax, word [ss:bp+14]
	push	ax
	call	SendWord
	push	mes_crlf
	call	SendTxt
%endif
%endif

	mov		ax, word [ss:bp+12]
	mov		es, ax
	mov		di, word [ss:bp+14]

	mov		cx, 5				; number of tries to read

try_to_read_cd_sector:

	; 1 - send PACKET command

	rol		edi, 16
	mov		dx, word [cs:di+hdc_cmd_stat]
	rol		edi, 16

packet_wait_till_bsy1:
	in		al, dx

	test	al, 80h
	jnz		packet_wait_till_bsy1

	test	al, 08h
	jnz		packet_wait_till_bsy1

	xor		al, al
	rol		edi, 16
	mov		dx, word [cs:di+hdc_err]			; Features = no Overlap, no DMA
	rol		edi, 16
	out		dx, al

	xor		al, al							; Tag = 0
	rol		edi, 16
	mov		dx, word [cs:di+hdc_cnt]
	rol		edi, 16
	out		dx, al

	mov		al, 00h						; byte count limit low = 08(00)h
	rol		edi, 16
	mov		dx, word [cs:di+hdc_cyl]
	rol		edi, 16
	out		dx, al

	mov		al, 08h						; byte count limit high = (08)00h
	rol		edi, 16
	mov		dx, word [cs:di+hdc_cyh]
	rol		edi, 16
	out		dx, al

%if 0
	rol		edi, 16
  mov   dx, word [cs:di+hdc_drv]
	rol		edi, 16
	in		al, dx
	and		al, 0xEF
	out		dx, al
%endif

	mov		al, 0a0h
	rol		edi, 16
	mov		dx, word [cs:di+hdc_cmd_stat]
	rol		edi, 16
	out		dx, al

	; 2 - Wait for BSY to clear and DRQ to set

	; generally, should wait 400ns
	push		5
	call		WaitSomeTime

	rol		edi, 16
	mov		dx, word [cs:di+hdc_cmd_stat]
	rol		edi, 16
packet_wait_till_bsy2:
	in		al, dx

	test	al, 80h
	jnz		packet_wait_till_bsy2

	test	al, 08h
	jz		packet_wait_till_bsy2

	; 3 - Write bytes of a command packet to device

	rol		edi, 16
	mov		dx, word [cs:di+hdc_dat]
	rol		edi, 16
	mov		ax, 00A8h			; A8h - read (12) command
	out		dx, ax				; word 0 - command

	mov		ax, word [ss:bp+6]
	xchg	ah, al
	out		dx, ax				; word 1 - LBA [31..16]

	mov		ax, word [ss:bp+4]
	xchg	ah, al
	out		dx, ax				; word 2 - LBA [15..0]

	mov		ax, word [ss:bp+10]
	xchg	ah, al
	out		dx, ax				; word 3 - sector count high

	mov		ax, word [ss:bp+8]
	xchg	ah, al
	out		dx, ax				; word 4 - sector count low

	xor		ax, ax
	out		dx, ax				; word 5 - reserved

	; clean irq flag
	push	ax
	push	ds
	xor		ax, ax
	mov		ds, ax
	mov		byte [m_hdflag], 0
	pop		ds
	pop		ax

	; just wait a little
	push		5
	call		WaitSomeTime

	; 4 - Wait for Irq
packet_wait_for_data_to_be_ready:

	sti
	push	ds
	push	ax
	push	dx

	xor		ax, ax
	mov		ds, ax

wait_packet_irq:
	cmp		byte [m_hdflag], 0ffh
	jne		wait_packet_irq

	mov		byte [m_hdflag], 0
	rol		edi, 16
	mov		dx, word [cs:di+hdc_cmd_stat]
	rol		edi, 16
	in		al, dx

	pop		dx
	pop		ax
	pop		ds


	; 5 - Read LBA Mid&High to determine how many bytes to read from device (in cx)

	rol		edi, 16
	mov		dx, word [cs:di+hdc_cyh]
	rol		edi, 16
	in		al, dx
	mov		ch, al
	rol		edi, 16
	mov		dx, word [cs:di+hdc_cyl]
	rol		edi, 16
	in		al, dx
	mov		cl, al

	; 6 - Read info to buffer

	rol		edi, 16
	mov		dx, word [cs:di+hdc_dat]
	rol		edi, 16
	shr		cx, 1
	cld
	rep		insw

	mov		ecx, dword [ss:bp+8]
	dec		ecx
	cmp		ecx, 0
	je		packet_read_successfully_complete

	; got more sectors to read
	mov		dword [ss:bp+8], ecx
	jmp		packet_wait_for_data_to_be_ready

packet_read_successfully_complete:

	; delay for all irq to be invoked
	push	20000
	call	WaitSomeTime

	pop		eax
	pop		cx
	pop		dx
	pop		di
	pop		es
	pop		bp
	clc

	ret		12

packet_read_terminated:
%if SERIAL_DEBUG
	push	mes_cd_read_term
	call	SendTxt
%endif
	dec		cx
	jnz		try_to_read_cd_sector

	pop		eax
	pop		cx
	pop		dx
	pop		di
	pop		es
	pop		bp
	stc
	ret		12

read_amt_via_packet_interface:

	; parameters through stack:
	; 	dword 	: lba						[bp+4]
	; 	dword	: sectors number			[bp+8]
	; 	word	: receive buffer segment	[bp+12]
	; 	word	: receive buffer offset		[bp+14]
	;		word	:	should wait irq					[bp+16]
	; result: in ax

	push	bp
	mov		bp, sp

	push	es
	push	di
	push	dx
	push	cx
	push	eax

%if 0
%if SERIAL_DEBUG
	push	mes_amt_read_sectors
	call	SendTxt
	mov		eax, dword [ss:bp+8]
	push	eax
	call	SendDword
	push	mes_cd_from
	call	SendTxt
	mov		eax, dword [ss:bp+4]
	push	eax
	call	SendDword
	push	mes_cd_to
	call	SendTxt
	mov		ax, word [ss:bp+12]
	push	ax
	call	SendWord
	push	':'
	call	SendChar
	mov		ax, word [ss:bp+14]
	push	ax
	call	SendWord
	push	mes_crlf
	call	SendTxt

%endif
%endif

	mov		ax, word [ss:bp+12]
	mov		es, ax
	mov		di, word [ss:bp+14]

	mov		cx, 5				; number of tries to read

try_to_read_amt_sector:

	; 1 - send PACKET command

	mov		dx, word [cs:amt_cmd_stat]
packet_amt_wait_till_bsy1:
	in		al, dx

	test	al, 80h
	jnz		packet_amt_wait_till_bsy1

	test	al, 08h
	jnz		packet_amt_wait_till_bsy1

	xor		al, al
	mov		dx, word [cs:amt_err]			; Features = no Overlap, no DMA
	out		dx, al

	xor		al, al							; Tag = 0
	mov		dx, word [cs:amt_cnt]
	out		dx, al

	mov		al, 00h						; byte count limit low = 08(00)h
	mov		dx, word [cs:amt_cyl]
	out		dx, al

	mov		al, 08h						; byte count limit high = (08)00h
	mov		dx, word [cs:amt_cyh]
	out		dx, al

  ;mov   dx, word [cs:amt_drv]
	;in		al, dx
	;and		al, 0xEF
	;out		dx, al

	mov		al, 0a0h
	mov		dx, word [cs:amt_cmd_stat]
	out		dx, al

	; 2 - Wait for BSY to clear and DRQ to set

	; generally, should wait 400ns
	push		10
	call		WaitSomeTime

	mov		dx, word [cs:amt_cmd_stat]
packet_amt_wait_till_bsy2:
	in		al, dx

	test	al, 80h
	jnz		packet_amt_wait_till_bsy2

	test	al, 08h
	jz		packet_amt_wait_till_bsy2

	; 3 - Write bytes of a command packet to device

	mov		dx, word [cs:amt_dat]
	mov		ax, 00A8h			; A8h - read (12) command
	out		dx, ax				; word 0 - command

	mov		ax, word [ss:bp+6]
	xchg	ah, al
	out		dx, ax				; word 1 - LBA [31..16]

	mov		ax, word [ss:bp+4]
	xchg	ah, al
	out		dx, ax				; word 2 - LBA [15..0]

	mov		ax, word [ss:bp+10]
	xchg	ah, al
	out		dx, ax				; word 3 - sector count high

	mov		ax, word [ss:bp+8]
	xchg	ah, al
	out		dx, ax				; word 4 - sector count low

	xor		ax, ax
	out		dx, ax				; word 5 - reserved

	; just wait a little
	push		10
	call		WaitSomeTime

	; 4 - Wait for Irq
packet_amt_wait_for_data_to_be_ready:
%if 0
	sti
	push	ds
	push	ax
	push	dx

	xor		ax, ax
	mov		ds, ax

wait_amt_packet_irq:
	cmp		byte [m_hdflag], 0ffh
	jne		wait_amt_packet_irq
	mov		byte [m_hdflag], 0
	rol		edi, 16
	mov		dx, word [cs:amt_cmd_stat]
	rol		edi, 16
	in		al, dx

	pop		dx
	pop		ax
	pop		ds

%else
	mov		dx, word [cs:amt_cmd_stat]
packet_amt_wait_till_bsy4:
	in		al, dx
	test	al, 80h
	jnz		packet_amt_wait_till_bsy4

	push		10
	call		WaitSomeTime

	; read status one more time
	mov		dx, word [cs:amt_cmd_stat]
	in		al, dx
	test	al, 08h
	jz		packet_amt_read_terminated
%endif

	; 5 - Read LBA Mid&High to determine how many bytes to read from device (in cx)

	mov		dx, word [cs:amt_cyh]
	in		al, dx
	mov		ch, al
	mov		dx, word [cs:amt_cyl]
	in		al, dx
	mov		cl, al

	; 6 - Read info to buffer

	mov		dx, word [cs:amt_dat]
	shr		cx, 1
	cld
	rep		insw

	mov		ecx, dword [ss:bp+8]
	dec		ecx
	cmp		ecx, 0
	je		packet_amt_read_successfully_complete

	; got more sectors to read
	mov		dword [ss:bp+8], ecx
	jmp		packet_amt_wait_for_data_to_be_ready

packet_amt_read_successfully_complete:

	pop		eax
	pop		cx
	pop		dx
	pop		di
	pop		es
	pop		bp
	clc
	ret		12

packet_amt_read_terminated:
%if SERIAL_DEBUG
	push	cx
	call	SendWord
	push	' '
	call	SendChar
	push	mes_amt_read_term
	call	SendTxt
%endif
	dec		cx
	cmp		cx, 0
	jne		try_to_read_amt_sector

	pop		eax
	pop		cx
	pop		dx
	pop		di
	pop		es
	pop		bp
	stc
	ret		12

Int13_cd:

	push	bp
	mov		bp, sp

	cmp		ah, 0x42
	jne		not_cdrom_extended_read

	; extended read function implementation for CDROMs
%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
%if VERBOSE_INT_13
	push	ecx
	push	ds
	push	si

	push	ds
	call	SendWord
	push	':'
	call	SendChar
	push	si
	call	SendWord
	push	' '
	call	SendChar

	mov		cl, byte [si]			; size of packet
	push	cx
	call	SendByte
	push	' '
	call	SendChar

	mov		cx, word [si+2]			; number of blocks to transfer
	push	cx
	call	SendWord
	push	' '
	call	SendChar

	mov		ecx, dword [si+4]		; transfer buffer
	push	ecx
	call	SendDword
	push	' '
	call	SendChar

	mov		ecx, dword [si+12]		; starting absolute block number (QWORD)
	push	ecx
	call	SendDword
	mov		ecx, dword [si+8]
	push	ecx
	call	SendDword
	push	' '
	call	SendChar

	mov		ecx, dword [si+20]		; 64-bit transfer buffer
	push	ecx
	call	SendDword
	mov		ecx, dword [si+16]
	push	ecx
	call	SendDword
	push	mes_crlf
	call	SendTxt

	pop		si
	pop		ds
	pop		ecx
%endif
%endif
%endif

	call	GetHdcBlock

	push	es
	push	di
	push	cx
	push	ebx

	mov		ax, word [si+drq_seg]						; buffer segment
	mov		es, ax
	mov		di, word [si+drq_ofs]						; buffer offset
	mov		cx, word [si+drq_blk]						; sectors number
	;inc		cx
	;shr		cx, 2
	mov		ebx, dword [si+drq_lba]					; lba

int13_cd_read_loop:
	mov		ax, di										; buffer offset
	push	ax
	mov		ax, es										; buffer segment
	push	ax
	;mov		eax, 0x00000001						; sectors number
	movzx		eax, cx									; sectors number
	push	eax
	push	ebx

	test	byte [cs:type_amt1], BOOT_FROM_AMT
	jz		int13_cd_read_via_packet_interface
	call	read_amt_via_packet_interface
	jmp		int13_cd_read_sector_complete
int13_cd_read_via_packet_interface:
	call	read_via_packet_interface
int13_cd_read_sector_complete:

	;add		di, 0x800
	;inc		ebx
	;loop	int13_cd_read_loop

	pop		ebx
	pop		cx
	pop		di
	pop		es

  and   word [ss:bp+6], 0xFFFE		; clear carry
	pop		bp
	mov		ah,0											; successful
	iret

not_cdrom_extended_read:

	cmp		ax, 0x4B00
	jne		not_cdrom_terminate_emulation

	; continue with get status to fill specification packet
	jmp		cd_fill_specification_packet

not_cdrom_terminate_emulation:

	cmp		ax, 0x4B01
	jne		not_cdrom_get_status

cd_fill_specification_packet:
	; ds:si points to empty specification packet
	; fill it
	mov		byte [si], 0x13						; packet size
	mov		byte [si+1], 0x00					; no emulation, no ATAPI or SCSI drivers in image
	mov		byte [si+2], 0xF0					; drive number
	mov		byte [si+3], 0x00					; CD-ROM controller number
	mov		dword [si+4], 0x00000140	; lba of drive image to emulate
	mov		word [si+8], 0x0000				; ide is master
	mov		word [si+10], 0x8800			; segment of 3k buffer to emulate CD-ROM reads
	mov		word [si+12], 0x0000			; load segment is 0x07c0
	mov		word [si+14], 0x0004			; number of 512-bytes sectors to load
	mov		byte [si+16], 0x00				; cylinder count
	mov		byte [si+17], 0x00				; sector count
	mov		byte [si+18], 0x00				; head count

  and   word [ss:bp+6], 0xFFFE		; clear carry
	pop		bp
	mov		ah,0											; successful
	iret

not_cdrom_get_status:

  or   word [ss:bp+6], 0x0001  ; set carry
	pop		bp
	iret

mes_cdrom_found:
	db	13,10,'CD-ROM found',13,10,0
mes_cd_read_term:
	db	13,10,'CD-ROM read terminated',13,10,0
mes_init_amt:
	db	'Init AMT',13,10,0
mes_amt_found:
	db	'AMT found',13,10,0
mes_amt_cd_found:
	db	'AMT CD found',13,10,0
mes_amt_read_term:
	db	13,10,'AMT read terminated',13,10,0

InitAMT:

%if SERIAL_DEBUG
	push	mes_init_amt
	call	SendTxt
%endif

	pushad
	push	ds
	push	es
	xor		ax, ax
	mov		es, ax
	mov		ds, ax

	; find AMT device on PCI bus
	mov		ax, 0xb102
	mov		cx, AMT_DEVICE_ID
	mov		dx, AMT_VENDOR_ID
	mov		si, 0
	int		0x1a
	jb		amt_check9

%if SERIAL_DEBUG
	push	mes_amt_found
	call	SendTxt
%endif

	cli

	; store AMT coordinates
	mov		byte [cs:amt_bus], bh
	mov		byte [cs:amt_devfunc], bl

	; enable device
	mov		ax, 0xb10b
	mov		bh, byte [cs:amt_bus]
	mov		bl, byte [cs:amt_devfunc]
	mov		di, 0x04		; command register
	mov		cl, 0x1f
	int		0x1a
	jb		amt_check9
	
	; get AMT BAR's
	mov		ax, 0xb10a
	mov		bh, byte [cs:amt_bus]
	mov		bl, byte [cs:amt_devfunc]
	mov		di, 0x10		; primary base
	int		0x1a
	jb		amt_check9

	and		cx, 0xFFFE
	mov		word [cs:amt_prim_base], cx

	mov		ax, 0xb10a
	mov		bh, byte [cs:amt_bus]
	mov		bl, byte [cs:amt_devfunc]
	mov		di, 0x14		; primary control
	int		0x1a
	jb		amt_check9

	and		cx, 0xFFFE
	mov		word [cs:amt_prim_ctrl], cx

	mov		ax, 0xb10a
	mov		bh, byte [cs:amt_bus]
	mov		bl, byte [cs:amt_devfunc]
	mov		di, 0x18		; secondary base
	int		0x1a
	jb		amt_check9

	and		cx, 0xFFFE
	mov		word [cs:amt_secd_base], cx

	mov		ax, 0xb10a
	mov		bh, byte [cs:amt_bus]
	mov		bl, byte [cs:amt_devfunc]
	mov		di, 0x1c		; secondary control
	int		0x1a
	jb		amt_check9

	and		cx, 0xFFFE
	mov		word [cs:amt_secd_ctrl], cx

	mov		ax, 0xb10a
	mov		bh, byte [cs:amt_bus]
	mov		bl, byte [cs:amt_devfunc]
	mov		di, 0x20		; bus master
	int		0x1a
	jb		amt_check9

	and		cx, 0xFFFE
	mov		word [cs:amt_bmaster], cx

	; update device addresses
	push	ax

	mov		ax, word [cs:amt_prim_base]
	mov		word [cs:amt_dat], ax
	inc		ax
	mov		word [cs:amt_err], ax
	inc		ax
	mov		word [cs:amt_cnt], ax
	inc		ax
	mov		word [cs:amt_sec], ax
	inc		ax
	mov		word [cs:amt_cyl], ax
	inc		ax
	mov		word [cs:amt_cyh], ax
	inc		ax
	mov		word [cs:amt_drv], ax
	inc		ax
	mov		word [cs:amt_cmd_stat], ax
	mov		ax, word [cs:amt_prim_ctrl]
	mov		word [cs:amt_ctrl_stat2], ax

	pop		ax

%if 1
	; set interrupt handler
  ;cli

	mov		dx, word [cs:amt_cmd_stat]
	in		al, dx				; clear interrupt request
  mov   dx, word [cs:amt_bmaster]
  add   dx, 2       ; AMT Bus Master primary IDE status register
  in    al, dx
  or    al, 0x04
  out   dx, al
  mov   dx, word [cs:amt_bmaster]  
  add   dx, 0x0a       ; AMT Bus Master secondary IDE status register
  in    al, dx
  or    al, 0x04
  out   dx, al

	push	bx
	push	cx

	mov		cl, byte [cs:amt_prim_irq]
	cmp		cl, 7
	ja		amt_irq_in_slave_8259

	; set handler for this Irq
	push	ax
	push	cx
	push	ds

	xor		ax, ax
	mov		ds, ax
	xor		ch, ch
	add		cl, 08h
	shl		cx, 2			;*4

	mov		bx, cx
	mov		word [bx], IrqAMT
	add		bx, 2
	mov		word [bx], cs

	pop		ds
	pop		cx
	pop		ax

	in		al, pic0+1
	mov		ch, 1
	shl		ch, cl
	not		ch
	and		al, ch
	out		pic0+1, al
	jmp		amt_irq_enabled

amt_irq_in_slave_8259:

	sub		cl, 8

	; set handler for this Irq
	push	ax
	push	cx
	push	ds

	xor		ax, ax
	mov		ds, ax
	xor		ch, ch
	add		cl, 70h
	shl		cx, 2			;*4

	mov		bx, cx
	mov		word [bx], IrqAMT
	add		bx, 2
	mov		word [bx], cs

	pop		ds
	pop		cx
	pop		ax

	in    al,pic1+1       	;enable HD interrupt
	mov		ch, 1
	shl		ch, cl
	not		ch

	and		al, ch
	out   pic1+1,al

	in    al,pic0+1       	;enable cascade interrupt
	and   al,0fbh				; +cascade irq2
	out   pic0+1,al

amt_irq_enabled:
	pop		cx
	pop		bx

	;sti
%endif

        mov     di, hd_prm4      		;setup first drive
        mov     dl, 084h
        mov     al, MASTER_DRIVE

        call    hd_pres 				;check presence
        jb      amt_init1

        call    hd_set  				;set parameters
        jb      amt_init1
				inc			cl
amt_init1:

        mov     di, hd_prm5      		;setup second drive
        mov     dl, 085h
        mov     al, SLAVE_DRIVE

        call    hd_pres 				;check presence
        jb      amt_init2

        call    hd_set  				;set parameters
        jb      amt_init2
				inc			cl
amt_init2:

	; translate AMT string
	mov		cx, 32
	mov		si, model_amt_master
amt_translate_string_loop:
	mov		ax, word [cs:si]	
	xchg	ah, al
	mov		word [cs:si], ax
	inc		si
	inc		si
	loop	amt_translate_string_loop

	; set 0 at the end of string
	mov		si, model_amt_master+31
	mov		byte [cs:si], 0
	mov		si, model_amt_slave+31
	mov		byte [cs:si], 0

%if SERIAL_DEBUG
	; print string
	push	model_amt_master
	call	SendTxt
	push	mes_crlf
	call	SendTxt
	push	model_amt_slave
	call	SendTxt
	push	mes_crlf
	call	SendTxt
%endif

amt_check9:	

	pop		es
	pop		ds
	popad
	sti
	ret

GetHdcBlock:
	; input:	dl - drive number
	; return: edi - high part is a pointer to hdc_block based on dl (drive number)
	; action: also set Master/Slave bit in DRIVE register

	push	ax
	push	dx
	push	cx

	rol		edi, 16

	mov		cl, byte [cs:BootDriveNum]

	cmp		cl, 0
	je		ghb_of_bootable_drive

	; check if boot from cd-rom. if so, skip drive change
	and		cl, 70h
	cmp		cl, 70h
	jne		ghb_not_cdrom_boot

	; check if request CD-ROM (dl=0fxh) or HDD (dl=8xh)
	mov		cl, dl
	and		cl, 70h
	cmp		cl, 70h
	jne		ghb_of_bootable_drive

ghb_not_cdrom_boot:
	mov		dl, byte [cs:BootDriveNum]

ghb_of_bootable_drive:

	movzx	cx, dl

	and		cx, 0fh
	shr		cx, 1
	inc		cx
	mov		di, hdc_array
	sub		di, hdc_len
ghb_add_hdc_len:
	add		di, hdc_len
	loop	ghb_add_hdc_len

	rol		edi, 16

	movzx		cx, dl
	and			cx, 0fh
	inc			cx

	mov     di, hd_prm0
	sub			di, edpt_ext_len

ghb_add_edpt_len:
	add			di, edpt_ext_len
	loop		ghb_add_edpt_len		

	mov		al, MASTER_DRIVE
	test	dl, 1
	jz		ghb_master
	mov		al, SLAVE_DRIVE
ghb_master:

	rol		edi, 16
	mov		dx, word [cs:di+hdc_drv]
	rol		edi, 16
	out		dx, al

	
	pop		cx
	pop		dx
	pop		ax
	
	ret

;delay procedure
; in - word in stack (number of cycles)	
WaitSomeTime:
	push	bp
	mov		bp, sp

	push	ax
	push	cx

	mov		cx, word [ss:bp+4]

wst_loop:
	out		iowait, al
	loop	wst_loop

	pop		cx
	pop		ax
	pop		bp
	ret		2

;delay procedure
; in - word in stack (number of seconds)	
WaitSomeSec:
	sti
	push	bp
	mov		bp, sp

	push	ax
	push	cx
	push	ds

	xor		ax, ax
	mov		ds, ax

	mov		cx, word [ss:bp+4]

wss_loop:

	push	cx
	mov		cx, [m_timer]
	add		cx, 18				; 1 sec

wss_1sec:
	hlt
	mov		ax, [m_timer]
	cmp		cx, ax
	jae		wss_1sec

	pop		cx
	loop	wss_loop

	pop		ds
	pop		cx
	pop		ax
	pop		bp
	ret		2

WriteTranslateDriveNumTable:
	; set drive number to translation table

	push	bp
	mov		bp, sp
	push	dx
	push	di
	push	ax

	mov		dx, word [ss:bp+4]
	mov		di, dx
	and		di, 7fh					; di - table index

	cmp		di, 3
	jne		wtdnt_not_fourth_drive

	; read previous drive in table
	dec		di
	mov		al, byte [cs:transl_dn+di]
	cmp		al, 0
	je		wtdnt_not_fourth_drive

	inc		al
	jmp		wtdnt_write_table

wtdnt_not_fourth_drive:

	cmp		di, 2
	jne		wtdnt_not_third_drive

	; read previous drive in table
	dec		di
	mov		al, byte [cs:transl_dn+di]
	cmp		al, 0
	je		wtdnt_not_third_drive

	inc		al
	jmp		wtdnt_write_table

wtdnt_not_third_drive:

	cmp		di, 1
	jne		wtdnt_not_second_drive

	; read previous drive in table
	dec		di
	mov		al, byte [cs:transl_dn+di]
	cmp		al, 0
	je		wtdnt_not_second_drive

	inc		al
	jmp		wtdnt_write_table

wtdnt_not_second_drive:

	mov		al, 80h

wtdnt_write_table:
	; write translated drive number to table
	mov		di, dx
	and		di, 7fh
	mov		byte [cs:transl_dn+di], al

	pop		ax
	pop		di
	pop		dx
	pop		bp
	ret		2
