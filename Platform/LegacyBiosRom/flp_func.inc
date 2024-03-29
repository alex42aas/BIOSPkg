	;
	; Illegal command
	;
fd_badcmd:
	mov		byte [m_fdstat],1		;illegal command
	;
	; AH=01: get status
	;
fd_status:
fd_exit:
	mov		al,[m_fdstat]			;get error code
fd_exit1:
	mov		[ss:bp+_ah],al				;return in AH
	mov		[ss:bp+_al],al				;return in AL
	and		al,al					;error ?
	jz		fd_exit2

	or		byte [ss:bp+18h],1			;yes: set carry
fd_exit2:
	popa							;restore registers
	pop		es
	pop		ds
	iret							;return from interrupt

	;
	; AH=00: recalibrate floppy drives
	;
fd_rst:

	les		si,[m_fdbase]

	in		al,pic0+1				;enable floppy interrupt
	and		al,0bfh
	out		iowait,al
	out		pic0+1,al
	
	mov		dx,fdc_rate				;set 500 kb/s data rate
	mov		al,0
	out		dx,al
	
	mov		dx,fdc_ctrl
	mov		word [m_fdstat],0		;clear error status
	mov		byte [m_fdcnt],0ffh		;prevent motor off event
	mov		al,[m_fdmot]			;digital output register
	rol		al,4
	and		al,11110011b
	or		al,8					;enable interrupts
	out		dx,al					;reset FDC
	and		byte [m_fdrecal],70h	;clear interrupt and recal flags
	mov		cx,100					;wait a bit
fd_rstw:
	out		iowait,ax
	loop	fd_rstw
	xor		al,4					;end reset pulse
	out		dx,al
	call	fd_wait					;wait for interrupt
	jb		fd_rst1					;:bad controller

	mov		bl,0c0h					;expected result
fd_rst0:
	mov		al,8					;sense interrupt
	call	fd_cmd					;send command
	jb		fd_rst1					;:error
	call	fd_res					;get results
	jb		fd_rst1					;:error
	cmp		byte [m_fdfile],bl		;drive ready changed state ?
	jnz		fd_rst1					;no: error
	inc		bl
	cmp		bl,0c4h					;done all drives ?
	jb		fd_rst0
	;
	; FDC specify command
	;
	mov		al,3					;specify command
	call	fd_cmd					;send to FDC
	jb		fd_spec9
	mov 	al,[es:si]				;m_fdbase -> step rate, head unload
	call	fd_cmd					;send to FDC
	jb		fd_spec9
	mov 	al,[es:si+1]			;m_fdbase -> motor on time, DMA mode
	call	fd_cmd

fd_spec9:
	mov 	al,[es:si+2]			;m_fdbase -> motor count
	mov		[m_fdcnt],al			;restore motor timer
	jmp		fd_exit

fd_rst1:
	or		byte [m_fdstat],20h		;bad controller
fd_rst2:
	jmp		fd_exit

	;
	; AH=02: read
	;
fd_read:
	call	fd_dma					;convert address ES:BX -> CH:DX
	mov		al,46h					;DMA mode: read	
fd_read2:
	call	fd_dma2					;complete DMA initialization
	jb		fd_rw90					;:error
	call	fd_seek					;turn on motor, seek to track
	jb		fd_rw90					;:error
fd_read3:
	mov		al,0e6h					;FDC read command
	;
	; execute read / write command
	;
fd_rw1:
	call	fd_cmd					;command
	jb		fd_rw9					;:error
	mov		al,0					;drive 0
	test	byte [ss:bp+_dh],1			;other head ?
	jz		fd_rw2
	or		al,4					;set head bit	
fd_rw2:	
	call	fd_cmd
	jb		fd_rw9
	mov		al,[ss:bp+_ch]				;track number
	call	fd_cmd
	jb		fd_rw9
	mov		al,[ss:bp+_dh]				;head number
	call	fd_cmd
	jb		fd_rw9
	mov		al,[ss:bp+_cl]				;sector number
	call	fd_cmd
	jb		fd_rw9
	mov 	al,[es:si+3]			;m_fdbase -> bytes / sector
	call	fd_cmd
	jb		fd_rw9
	mov		al,[es:si+4]			;m_fdbase -> end of track sector
	call	fd_cmd
	jb		fd_rw9
	mov 	al,[es:si+5]			;m_fdbase -> gap length
	call	fd_cmd
	jb		fd_rw9
	mov 	al,[es:si+6]			;m_fdbase -> data length
fd_rw3:
	call	fd_cmd
	jb		fd_rw9
	call	fd_wait					;wait for interrupt
	jb		fd_rw9
	call	fd_res					;get results
	jb		fd_rw9
	cmp		byte [ss:bp+_al],3			;write ?
	jz		fd_rw4
	cmp		byte [ss:bp+_al],5			;format ?
	jnz		fd_rw5
fd_rw4:
	mov		bx,1					;wait 1 ms after write -> write gate
	call	cs_waitbx				;delay (prevent immediate step away)
fd_rw5:
	test	byte [m_fdfile],0c0h	;any error ?
	jz		fd_rw99					;:no
	call	fd_error				;translate error
fd_rw9:
	cmp		byte [ss:bp+_ah],5			;format ?
	jz		fd_rw99
	mov		byte [ss:bp+_al],0			;clear sector count
fd_rw99:
	mov 	al,[es:si+2]			;m_fdbase -> motor count
	mov		[m_fdcnt],al
	jmp		fd_exit

	;
	; AH=03: write sectors
	;
fd_write:
	call	fd_dma					;convert address ES:BX -> CH:DX
	mov		al,4ah					;DMA mode: write
	call	fd_dma2					;complete DMA initialization
	jb		fd_rw90					;:error
	call	fd_seek					;turn on motor, seek to track
	jb		fd_rw90					;:error
	call	cs_waitbx				;wait BX milliseconds	
	mov		al,0c5h					;write command
	jmp		fd_rw1
	
fd_rw90:
	jmp		fd_rw9

	;
	; AH=04: verify
	;	
fd_verify:
	call	fd_dma					;convert address ES:BX -> CH:DX
	xor		dx,dx					;clear offset -> no DMA errors
	mov		al,42h					;DMA mode: verify
	jmp		fd_read2				;rest like read

 	;
  	; AH=05: format track
  	;
fd_format:
	call	fd_dma					;convert address ES:BX -> CH:DX
	mov		ax,0200h+4ah			;sectors * 4 shift, DMA mode write
	call	fd_dma2					;complete DMA initialization
	jb		fd_rw90					;:error
	call	fd_seek					;turn on motor, seek to track
	jb		fd_rw90					;:error
	call	cs_waitbx				;wait BX milliseconds	

	mov		al,3					;specify command
	call	fd_cmd					;send to FDC
	jb		fd_rw90
	mov 	al,[es:si]				;m_fdbase -> step rate, head unload
	call	fd_cmd					;send to FDC
	jb		fd_rw90
	mov 	al,[es:si+1]			;m_fdbase -> motor on time, DMA mode
	call	fd_cmd
	jb		fd_rw90

	mov		al,4dh					;format command
	call	fd_cmd
	jb		fd_rw90
	mov		al,[ss:bp+_dh]				;head
	shl		al,2
	call	fd_cmd
	jb		fd_rw90
	mov 	al,[es:si+3]			;bytes per sector
	call	fd_cmd
	jb		fd_rw90
	mov 	al,[es:si+4]			;sectors per track
	call	fd_cmd
	jb		fd_rw90
	mov 	al,[es:si+7]			;gap length
	call	fd_cmd
	jb		fd_rw90
	mov 	al,[es:si+8]			;fill byte
	jmp		fd_rw3					;continue as read / write

	;
	; AH=08: read drive parameters
	;
fd_drvprm:
	xor		bx,bx					;for non-existent drive
	xor		cx,cx
	xor		dx,dx
	xor		si,si
	xor		di,di
	mov		[m_fdstat],bl			;clear status
	mov		[ss:bp+_ax],bx				;clear AX
	
	cmp		byte [m_fdmed0],0		;drive present ?
	jz		fd_drvp9				;:no
	
	mov		si,cs					;segment -> ES
	mov		di,fd_ptab
	mov		bx,4					;drive type = 1.44 MB
	mov		cx,79*256+18			;80 tracks, 18 sectors per track
	mov		dx,1*256+1				;1 head, 1 drive

fd_drvp9:
	mov		[ss:bp+_bx],bx
	mov		[ss:bp+_cx],cx
	mov		[ss:bp+_dx],dx
	mov		[ss:bp+_es],si
	mov		[ss:bp+_di],di
	jmp		fd_exit					;return ok status

	;
	; AH=15: get drive type
	;
fd_gettyp:
	mov		al,0					;drive not present
	mov		byte [m_fdstat],al		;clear status
	cmp		dl,0					;drive 0 ?
	jnz		fd_gett2
	cmp		byte [m_fdmed0],0		;drive present ?
	jz		fd_gett2				;:no
	mov		al,2					;change line available
fd_gett2:
	mov		[ss:bp+_ah],al				;return in AH
	jmp		fd_exit2				;don't set CY

	;
	; AH=16: get disk change status
	;
fd_dskchg:
	les		si,[m_fdbase]			;^disk parameters
	mov		al,80h					;drive not ready (no drive)
	cmp		dl,0					;drive 0 ?
	jnz		fd_dsk2
	cmp		byte [m_fdmed0],0		;drive present ?
	jz		fd_dsk2					;:no
	call	fd_motor				;turn on motor
	call	cs_waitbx				;wait for motor startup time
	mov		byte [m_fdcnt],36		;restore motor timer
	mov		dx,fdc_chg				;read disk change line
	in		al,dx
	and		al,80h
	jz		fd_dsk2					;:not active, AL = 0
	mov		al,6					;disk change active
fd_dsk2:
	mov		[m_fdstat],al
	jmp		fd_exit1				;set CY if not 0

	;
	; AH=17: set drive type for format
	;
	; since we only support 1.44 MB, basically a no-op
	;
fd_fmttyp:
	cmp		al,5
	jae		fd_fmtt2
	cmp		al,0
	jnz		fd_dskchg
fd_fmtt2:
	jmp		fd_badcmd

	;
	; AH=18: set media type for format
	;
fd_medtyp:
	mov		al,80h						;drive not ready (no drive)
	cmp		dl,0						;drive 0 ?
	jnz		fd_medt9
	cmp		byte [m_fdmed0],0			;drive present ?
	jz		fd_medt9					;:no
	mov		al,0ch						;invalid format
	cmp		ch,79						;80 tracks ?
	jnz		fd_medt9					;:no, error
	cmp		cl,18						;18 sectors ?
	jnz		fd_medt9					;:no, error
	mov		[ss:bp+_es],cs					;return ^disk parameters
	mov		word [ss:bp+_di],fd_ptab
	mov		al,0						;ok status
fd_medt9:
	mov	[m_fdstat],al					;set status
	jmp	fd_exit1						;set CY if not 0

	;
	; initialize DMA controller
	;
	; ES    = buffer segment
	; BX    = buffer offset
	; ES:BX -> physical address CH:DX, init ES:SI, AH
fd_dma:	 
	mov		dx,es						;buffer segment
	rol		dx,4
	mov		ch,dl						;high 4 bits
	and		dl,0f0h						;mask off low bits
	add		dx,bx						;add buffer offset
	adc		ch,0
	les		si,[m_fdbase]				;load ^ floppy parameters
	mov 	ah,[es:si+3]				;get shift count
	add		ah,7						;shift count + 7 (128 byte base)
	ret
	;
	; second half of DMA setup
	;
	; AL    = DMA mode
	; AH    = shift count (for sector -> byte calculation)
	; DX    = address (low 16 bits)
	; CH    = address (high 4 bits)
	;
fd_dma2:
	mov		cl,ah						;shift count
	mov		bl,[ss:bp+_al]					;number of sectors
	mov		bh,0
	shl		bx,cl						;-> byte count
	dec		bx							;-1 for DMA

	add		bx,dx						;test for DMA overflow
	jb		fd_dma9						;:error
	sub		bx,dx						;restore count

fd_dma3:
	cli									;critical section

	push	ax
	mov		al,6						;disable DRQ2
	out		dma0+10,al
	out		iowait,ax
	pop		ax

	out		dma0+12,al					;dummy access -> reset hi/lo FF
	out		iowait,ax

	out		dma0+11,al					;write DMA mode
	out		iowait,ax

	mov		al,dl						;low address
	out		dma0+4,al
	out		iowait,ax

	mov		al,dh						;high address
	out		dma0+4,al

	mov		al,ch						;DMA page register
	and		al,15
	out		fd_page,al

	mov		al,bl						;low count
	out		dma0+5,al
	out		iowait,ax

	mov		al,bh						;high count
	out		dma0+5,al
	out		iowait,ax

	mov		al,2						;enable DRQ2
	out		dma0+10,al

	sti									;end of critical section
	clc
	ret
	
fd_dma9:
	mov		byte [m_fdstat],9			;DMA overflow error
	ret

	;
	; turn on motor, seek to track if needed
	;
	; -> settling time in ms in BX
	;
fd_seek:
	call	fd_motor					;turn on motor

	; check and reset disk change
	
	mov		dx,fdc_chg					;disk change line active ?
	in		al,dx
	and		al,80h
	jz		fd_seek1x					;:no
	call	fd_seek1z					;recalibrate
	cmp		byte [ss:bp+_ch],0				;destination = track 0 ?
	jnz		fd_seek1b
	inc		byte [ss:bp+_ch]
	call	fd_seek2					;seek to track 1
	dec		byte [ss:bp+_ch]
fd_seek1b:
	call	fd_seek2					;seek to track 0 / wanted track
	mov		dx,fdc_chg					;disk change line still active ?
	in		al,dx
	and		al,80h
	jz		fd_seek1c					;no: ok
	mov		byte [m_fdstat],80h			;set time-out error
fd_seek1c:
	ret

	; recalibrate if required

fd_seek1x:
	test	byte [m_fdrecal],1			;need recalibration ?
	jnz		fd_seek2					;:no
fd_seek1z:
	mov		al,7						;recalibrate
	call	fd_cmd
	jb		fd_seek9
	mov		al,0						;drive 0
	call	fd_cmd
	jb		fd_seek9
	call	fd_wsk						;wait for seek complete
	jb		fd_seek9
	or		byte [m_fdrecal],1			;set bit - done
	mov		byte [m_fdtrk0],0			;set current track
	or		bx,bx						;motor settling time ?
	jnz		fd_seek2					;yes, bigger than head settling time
	mov 	bl,[es:si+9]				;m_fdbase -> head settling time

	; seek to track if required
	
fd_seek2:
	mov		al,[ss:bp+_ch]					;destination track
	cmp		al,[m_fdtrk0]				;= current track ?
	jz		fd_seek9					;:done (implied clc)
	mov		al,0fh						;seek command
	call	fd_cmd
	jb		fd_seek9
	mov		al,0						;drive number
	call	fd_cmd
	jb		fd_seek9
	mov		al,[ss:bp+_ch]					;destination track	
	call	fd_cmd
	jb		fd_seek9
	mov		[m_fdtrk0],al				;set new position
	call	fd_wsk						;wait for seek complete
	or		bx,bx						;motor settling time ?
	jnz		fd_seek9					;yes, bigger than head settling time
	mov 	bl,[es:si+9]				;m_fdbase -> head settling time

fd_seek9:
	ret

	;
	; turn on motor
	;
	; out: BX = turn-on delay in ms
	;
fd_motor:
	xor		bx,bx						;no settling time required
	
	; turn on motor if required
	
	mov		byte [m_fdcnt],255			;prevent motor shutdown
	mov		byte [m_fdstat],0			;clear error code
	test	byte [m_fdmot],1			;motor running ?
	jnz		fd_mot1						;:yes
	or		byte [m_fdmot],1			;set motor flag
	mov		dx,fdc_ctrl					;turn on motor
	mov		al,00011100b				;drive 0, DMA enable, not reset
	out		dx,al
	mov 	bl,[es:si+10]				;m_fdbase -> motor start time
	shl		bx,7						;x 128 -> value in ms
fd_mot1:
	ret

	;
	; wait for seek to complete, get status
	;
fd_wsk:
	call	fd_wait						;wait for interrupt
	jb		fd_wsk8						;:error
fd_wsk2:
	mov		al,8						;sense interrupt
	call	fd_cmd
	jb		fd_wsk8
	call	fd_res						;get results
	jb		fd_wsk8
	mov		al,[m_fdfile]
	and		al,01100000b				;error, seek end bits
	cmp		al,00100000b				;no error, seek end ?
	jz		fd_wsk9						;:ok	
fd_wsk8:
	or		byte [m_fdstat],40h			;set seek error bit
	stc
fd_wsk9:
	ret

	;
	; Did we get interrupt ? CY set if time-out
	;
fd_wait:
	mov		byte [m_fdcnt],0ffh		;keep motor running
	push	cx
	mov		cx,[m_timer]			;start time
	add		cx,19					;time-out 1 second
fd_wait2:
	test	byte [m_fdrecal],80h	;did we get interrupt ?
	jnz		fd_wait3				;:yes
	cmp		cx,[m_timer]
	js		fd_wait4				;:time-out
	hlt								;wait for next interrupt
	jmp		fd_wait2				;look again
fd_wait3:
	pop		cx
	and		byte [m_fdrecal],7Fh	;clear interrupt flag
	ret

fd_wait4:
	or		byte [m_fdstat],80h		;set time-out status
	stc
	pop		cx
	ret

	;
	; write command byte AL to FDC
	;
fd_cmd:
	mov		ah,al					;save data
	mov		dx,fdc_stat
	xor		cx,cx
fd_cmd1:
	in		al,dx					;read status
	and		al,0c0h					;RQM / DIO
	xor		al,80h					;RQM set, DIO clear = write to FDC
	jz		fd_cmd2					;:ok
	loop	fd_cmd1					;keep trying
	or 		byte [m_fdstat],80h		;time out
	stc
	ret

fd_cmd2:
	mov		al,ah					;restore data
	inc		dx
	out		dx,al					;write data to fdc_data
	ret								;(carry clear)

	;
	; get results from FDC
	;
fd_res:
	mov		dx,fdc_stat
	xor		cx,cx
	mov		di,m_fdfile				;destination pointer
fd_res1:
	xor		cx,cx					;clear time-out
fd_res2:
	out		iowait,ax
	in		al,dx					;read status
	cmp		al,0c0h					;FDC ready for data read ?
	jnb		fd_res3					;:yes
	loop	fd_res2
fd_res8:
	or		byte [m_fdstat],80h		;time out
	stc
	ret
	
fd_res3:
	out		iowait,ax
	inc		dx
	in		al,dx					;read fdc_data
	dec		dx
	mov		[di],al					;store result
	inc		di
	out		iowait,ax				;give FDC some time to update status
	out		iowait,ax
	out		iowait,ax
	out		iowait,ax
	cmp		di,m_fdfile+7			;max bytes ?
	jz		fd_res9					;:done
	;
	; wait for next byte
	;
fd_res4:
	in		al,dx					;get controller status
	cmp		al,0c0h
	jae		fd_res3					;:ready with data
	and		al,0f0h					;ready for next command ?
	cmp		al,80h
	jz		fd_res9					;:yes, done
	loop	fd_res4					;keep trying
	jmp		fd_res8					;time-out
	
fd_res9:
	clc
	ret

	;
	; translate error code
	;
fd_error:
	mov		al,20h					;(FDC error)
	test	byte [m_fdfile],80h		;invalid command ?
	jnz		fd_err9					;:yes
	mov		al,[m_fdfile+1]			;ST1 status
	and		al,10110111b			;mask off unused status bits
	mov		bx,fd_errtab+9			;^error table
	stc								;ensure termination
fd_err1:
	dec		bx
	rcr		al,1					;test bit
	jnb		fd_err1					;try next
	mov 	al,[cs:bx]				;get correct error code
fd_err9:
	or		[m_fdstat],al
	ret
