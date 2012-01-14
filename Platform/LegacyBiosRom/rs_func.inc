	;
	; AH=0: initialize serial port
	;
rs_init:
	mov		al,bl				;AH=0; get baud rate
	shr		al,4				;-> table index
	and		al,1110b
	cmp		dx,CONSOLE			;console ?
	jz		rs_stat				;if yes, don't let DOS change the parameters
	mov		si,ax
	add		dl,3
	mov		al,80h				;base+3 DLAB=1: access baud rate register
	out		dx,al
	dec		dx
	dec		dx
	mov		ax,word [cs:si+rs_baud]	;get baudrate
	out		dx,al					;base+1 baudrate MSB
	mov		al,ah
	dec		dx
	out		dx,al					;base+0 baudrate LSB
	add		dl,3
	mov		al,bl
	and		al,1fh					;set parameters
	out		dx,al					;base+3
	dec		dx
	dec		dx
	in		al,dx					;base+1
	and		al,1 					;don't disable interrupt on console
	cmp		dx,CONSOLE+1
	jz		rs_init1
	mov		al,0					;disable interrupts
rs_init1:
	out		dx,al					;base+1
	dec		dx						;base+0

	;
	; AH=3: get status
	;
rs_stat:
	add		dl,5
	in		al,dx
	mov		ah,al
	inc		dx
	in		al,dx
	jmp		rs_exit

	;
	; AH=1: transmit character
	;
rs_xmit:
	mov		al,3					;modem control: set DTR, RTS
	add		dl,4
	out		dx,al
	inc		dx            		   	;modem status: wait for DSR, CTS
	inc		dx
	mov		bh,30h	
	call	rs_wait
	jnz		rs_xmtime				;:time-out
	dec		dx
	mov		bh,20h					;line status: wait for xmit ready
	call	rs_wait
	jnz		rs_xmtime
	sub		dl,5
	mov		al,bl					;character to send
	out		dx,al					;send it
	jmp		rs_exit
	;
	; time-out error
	;
rs_xmtime:
	mov		al,bl					;restore character
rs_rxtime:
	or		ah,80h					;set time-out flag
	jmp		rs_exit

	;
	; AH=2: receive character
	;
rs_recv:
	mov		al,1					;modem control: set DTR
	add		dl,4
	out		dx,al
	inc		dx						;modem status: wait for DSR
	inc		dx
	mov		bh,20h
	call	rs_wait
	jnz		rs_rxtime				;:time-out
	dec		dx
	mov		bh,1					;line status: wait for receive data
	call	rs_wait
	jnz		rs_rxtime				;:time-out
	and		ah,1eh					;mask status
	and		dl,0f8h					;receive register
	in		al,dx					;read character
	jmp		rs_exit					;exit

	;
	; serial port wait
	;
rs_wait:
	mov		ch,cl					;copy time-out value
	xor		si,si					;clear counter
rs_wait2:
	in		al,dx
	mov		ah,al					;save status
	and		al,bh					;check status
	cmp		al,bh					;set ?
	jz		rs_wait9				;:yes
	dec		si
	jnz		rs_wait2				;:try again
	dec		ch
	jnz		rs_wait2
	or		bh,bh					;time-out
rs_wait9:
	ret
