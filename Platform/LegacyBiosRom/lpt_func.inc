	;
	; AH=00: output character
	;
lp_out:
	mov		al,bl					;output character
	out		dx,al					;[DX+0]
	inc		dx
	xor		si,si
lp_wait:
	in		al,dx					;[DX+1] get status
	mov		ah,al
	and		al,al					;busy ?
	js		lp_ok					;:no
	dec		si
	jnz		lp_wait
	dec		cl
	jnz		lp_wait
	or		ah,1					;time-out
	and		ah,0f9h
	jmp		short lp_out2			;flip bits, exit

lp_ok:
	inc		dx
	mov		al,0dh					;activate strobe
	out		dx,al					;[DX+2]
	out		iowait,ax
lp_in2:
	mov		al,0ch					;deactivate strobe
	out		dx,al					;[DX+2]
	dec		dx
	dec		dx
	out		iowait,ax
	;
	; get printer status
	;
lp_stat:
	inc		dx						;get status
	in		al,dx					;[DX+1]
	and		al,0f8h
	mov		ah,al
lp_out2:
	mov		al,bl					;restore AL
	xor		ah,048h
	jmp		lp_exit

	;
	; initialize printer
	;
lp_init:
	mov		al,8					;reset printer
	inc		dx
	inc		dx
	out		dx,al					;[DX+2]
	mov		cx,5000					;wait a bit
lp_in1:
	loop	lp_in1
	jmp		lp_in2
