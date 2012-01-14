	;
	; AH=00: get key from buffer
	;
kb_get:
	call	kb_getch				;get character from buffer
	call	kb_xlat					;translate extended characters
	jb		kb_get					;:extended character, try again
	jmp		kb_exit

	;
	; AH=01: check if key available
	;
kb_check0:
	call	kb_getch				;skip extended character
kb_check:
	call	kb_chk					;check for character
	jz		kb_exitst				;:nothing available
	call	kb_xlat					;check if extended character
	jb		kb_check0				;:extended character, skip
kb_extst2:
	inc	dx							;clear Z flag
kb_exitst:
	pop		ds						;exit, flags modified
	pop		dx
	pop		cx
	pop		bx
	retf	2

	;
	; AH=10: extended read
	;
kb_extrd:
	call	kb_getch				;get character
	call	kb_exlat				;convert extended codes
	jmp		kb_exit

	;
	; AH=11: extended status
	;
kb_extst:
	call	kb_chk					;check status
	jz		kb_exitst				;:nothing, return Z flag
	call	kb_exlat
	jmp		kb_extst2				;return result

	;
	; AH=12: extended shift status
	;
kb_extsh:
	xor		ah,ah
	mov		al,[m_kbf1]				;system request shift
	test	al,kb_fsys				;system request ?
	jz		kb_extsh2
	mov		ah,80h					;yes: set bit 7
kb_extsh2:
	and		al,01110011b
	or		ah,al
	mov		al,[m_kbf3]				;right control and alt keys
	and		al,00001100b
	or		ah,al

	;
	; AH=02: return current shift status
	;
kb_shift:
	mov		al,[m_kbf]				;get shift status
	jmp		kb_exit

	;
	; AH=03: set key repetition rate
	;
kb_rate:
	cmp		al,5					;correct command ?
	jnz		kb_exit
	cmp		bl,31					;test rate
	ja		kb_exit
	cmp		bh,3					;test delay
	ja		kb_exit
	push	ax						;save AX
	shl		bh,5
	mov		al,0f3h					;set repeat rate / delay command
	call	kb_send					;send to keyboard
	mov		al,bl					;combine delay, rate
	add		al,bh
	call	kb_send
	pop		ax						;restore AX
	jmp		kb_exit

	;
	; AH=05: place scan code in buffer
	;
kb_write:
	mov		al,1					;error status
	cli								;prevent conflict
	mov		bx,[m_kbtail]			;^kb buffer
	inc		bx						;increment
	inc		bx
	cmp		bx,[m_kbend]		  	;at end ?
	jnz		kb_write2
	mov		bx,[m_kbstart]			;yes: go to start
kb_write2:
	cmp		bx,[m_kbhead]			;buffer full ?
	jz		kb_write3				;:yes
	xchg	bx,[m_kbtail]			;update tail, get old value
	dec		ax						;clear AL
	mov		[bx+bofs],cx			;store scan code
kb_write3:
	sti								;end of critical section
	jmp		kb_exit

	;
	; get scan code from buffer
	;
kb_getch0:
	sti								;reenable interrupts
	hlt								;wait for next event
kb_getch:
	cli								;critical section
	mov		bx,[m_kbhead]			;^head of buffer
	cmp		bx,[m_kbtail]			;= tail of buffer ?
	jz		kb_getch0				;yes: wait
	mov		ax,[bx+bofs]			;get scan code
	inc		bx						;increment pointer
	inc		bx
	cmp		bx,[m_kbend]  			;at end ?
	jnz		kb_getch2
	mov		bx,[m_kbstart]			;yes: go to start
kb_getch2:
	mov		[m_kbhead],bx			;update pointer
	sti								;end of critical section
	ret

	;
	; check if there is anything in buffer (Z set if not)
	;
kb_chk:
	cli								;critical section
	mov		bx,[m_kbhead]			;^head of buffer
	cmp		bx,[m_kbtail]			;= tail of buffer ?
	sti								;end of critical section
	mov		ax,[bx+bofs]			;get scan code
	ret

	;
	; check if extended character, set C if yes
	;
kb_xlat:
	cmp		ah,84h					;extended ?
	jbe		kb_xlat83
	cmp		ah,0e0h
	jnz		kb_stc1					;:bad
	mov		ah,1ch					;keypad Enter fixed code
	cmp		al,13					;keypad Enter ?
	jz		kb_xlatok				;:yes
	cmp		al,10					;keypad ^Enter
	jz		kb_xlatok				;:yes
	mov		ah,35h					;keypad /
	jmp		short kb_xlatok
	
kb_xlat83:
	cmp		ax,00e0h				;extension ?
	jz		kb_xlatok
	cmp		ax,00f0h
	jz		kb_xlatok
	cmp		al,0f0h					;fill-in ?
	jz		kb_stc1
	cmp		al,0e0h
	jnz		kb_xlatok
	mov		al,0
kb_xlatok:
	clc								;ok to use
	ret
	
kb_stc1:
	stc								;extended code - bad
	ret

	;
	; translate extended characters
	;
kb_exlat:
	cmp		al,0f0h					;special ?
	jnz		kb_exlat2
	or		ah,ah					;0: more special
	jz		kb_exlat2
	mov		al,0
kb_exlat2:
	ret
