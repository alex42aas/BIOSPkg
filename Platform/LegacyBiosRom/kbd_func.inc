	;
	; put keystroke in buffer
	;
putbuf:
 	mov 	si,[m_kbtail]
 	mov 	di,si
 	inc 	si
 	inc 	si
 	cmp 	si,[m_kbend]			;end of buffer ?
 	jnz 	pb1						;:no
 	mov 	si,[m_kbstart]			;:restart at beg
pb1: 
	cmp 	si,[m_kbhead]			;buffer full ?
 	jz 		pbovr					;:overrun;:yes
; 	mov 	[di.bofs],ax			;store keystroke
 	mov 	[di+bofs],ax			;store keystroke
 	mov 	[m_kbtail],si
 	clc		;ok
 	ret
pbovr:
 	stc								;overrun: return error
 	ret

	;
	; wait for AT kbd
	;
waitkbd: 
	push 	cx
 	xor 	cx,cx					;timeout
wk1: 
	out		iowait,ax
	in 		al,kb_stat				;read status port
 	test 	al,2					;input buffer full ?
 	loopnz 	wk1						;:yes
 	pop 	cx
 	ret

	;
	; disable AT kbd
	;
disakbd: 
	cli
 	call 	waitkbd
 	mov 	al,0adh					;disable
 	out 	kb_stat,al
 	sti
 	ret

	;
	; send command to AT kbd
	;
kb_send:
 	push 	ax						;save
 	push 	cx
 	mov 	ah,3					;3 retries
sk1:
 	cli
 	and 	byte [m_kbf2],kb_fled+kb_fcled+8	;clear error bits
 	push 	ax
 	call 	waitkbd
 	pop 	ax
 	out 	kb_dat,al				;store command
 	sti
 	mov 	cx,2000h				;wait
sk2:
 	test 	byte [m_kbf2],kb_fack+kb_fres
 	jnz 	sk4						;:response
 	out		iowait,ax
 	loop 	sk2	;wait
sk3: 
	dec 	ah
 	jnz 	sk1						;:another retry
 	or 		byte [m_kbf2],kb_ferr	;set error bit
 	jmp 	short sk9				;done

sk4:
 	test 	byte [m_kbf2],kb_fres	;resend flag ?
 	jnz 	sk3						;:retry
sk9:
 	cli
 	pop 	cx
 	pop 	ax
 	ret

readchar:
 	call 	disakbd
 	cli
 	call 	waitkbd
 	in 		al,kb_dat				;read scan code
 	sti
 	cmp 	al,0feh					;resend ?
 	jz 		rch3					;:yes
 	cmp 	al,0fah					;ack ?
 	jnz 	setled					;:no
 	mov 	al,kb_fack
 	jmp 	short rch4

rch3:
 	mov 	al,kb_fres
rch4:
 	cli
 	or 		byte [m_kbf2],al
 	pop 	bx
 	jmp 	done

setled:

	cli
	push	dx
	mov		dx,pic0
	call	setleds					;set mode LEDs
	pop		dx
 	sti
 	ret

	;
	; update LEDs
	;
setleds:
	push	ax
 	mov 	ah,[m_kbf]				;current mode flags
 	rol		ah,4					;-> low bits
 	mov 	al,[m_kbf2]				;current LED status
 	and		ax,0707h				;LED bits only
 	cmp 	ah,al					;same ?
 	jz 		setled9					;:done
 	test 	byte [m_kbf2],kb_fcled	;led update pending ?
 	jnz 	setled9					;:yes, don't reenter
 	or 		byte [m_kbf2],kb_fcled	;set update flag
 	mov 	al,eoi					;reset interrupt controller
	out 	dx,al					;(or iowait, depending on DX)
 	mov 	al,0edh					;set mode indicators
 	call 	kb_send					;send kbd command
 	test 	byte [m_kbf2],kb_ferr	;transmit error ?
 	jnz 	setled8					;:yes
 	mov 	al,ah					;send mode
 	call 	kb_send
 	test 	byte [m_kbf2],kb_ferr	;transmit error ?
 	jnz 	setled8					;:yes
 	and 	byte [m_kbf2],255-kb_fled	;set new state
 	or 		[m_kbf2],ah
setled8:
 	and 	byte [m_kbf2],3fh		;reset update flag
setled9:
	pop		ax
	ret

	;
	; invalid key: ignore
	;
kinval:
 	ret
	
	;
	; left shift
	;
kshlt:
 	mov 	al,kb_flsh
kshlt1:
 	test	byte [m_kbf3],kb_fe0	;did we get E0 prefix ?
	jnz		kshlt2					;yes: ignore (extended key)
	or 		[m_kbf],al				;set flag
 	and 	cl,cl					;break ?
 	jns 	kshlt2
 	xor 	[m_kbf],al				;:clear flag
kshlt2:
 	ret

	;
	; right shift
	;
kshrt:
 	mov 	al,kb_frsh
 	jmp 	kshlt1

	;
	; left control
	;
kctlt:
 	or 		byte [m_kbf1],kb_flct	;set flag
 	and 	cl,cl					;break ?
 	jns 	kctlt1
 	xor 	byte [m_kbf1],kb_flct	;:clear flag
kctlt1:
 	or 		byte [m_kbf],kb_fcsh	;set left & right flag
 	test 	byte [m_kbf1],kb_flct
 	jnz 	kctlt2 					;:ok
 	test 	byte [m_kbf3],kb_frct
 	jnz 	kctlt2					;:ok
 	xor 	byte [m_kbf],kb_fcsh	;clear control flag
 	ret
kctlt2: 
	pop 	ax						;don't clear hold flag
 	jmp 	i12

	;
	; right control
	;
kctrt:
 	test 	byte [m_kbf3],kb_fe0+kb_fe1	;no E0/E1: caps lock
 	jz 		kcaps
kctrt1:
 	or 		byte [m_kbf3],kb_frct	;set flag
 	and 	cl,cl					;break ?
 	jns 	kctlt1
 	xor 	byte [m_kbf3],kb_frct	;:clear flag
 	jmp 	kctlt1

	;
	; left alt
	;
kallt:
 	test 	byte [m_kbf3],kb_fe0	;E0: right alt
 	jnz 	kalrt
 	or 		byte [m_kbf1],kb_flal	;set flag
 	and 	cl,cl					;break ?
 	jns 	kallt1
 	xor 	byte [m_kbf1],kb_flal	;:clear flag
kallt1:
 	or 		byte [m_kbf],kb_fash	;set left & right flag
 	test 	byte [m_kbf1],kb_flal
 	jnz 	kallt2					;:ok
 	test 	byte [m_kbf3],kb_fral
 	jnz 	kallt2					;:ok
 	xor 	byte [m_kbf],kb_fash	;clear alt flag
 	xor 	ax,ax					;any char entered via alt ?
 	xchg 	al,[m_kbnum]
 	and 	al,al
 	jz 		kallt2					;:no
 	call 	putbuf					;put it in buffer
kallt2:
 	ret

	;
	; right alt
	;
kalrt:
 	or 		byte [m_kbf3],kb_fral	;set flag
 	and 	cl,cl					;break ?
 	jns 	kallt1
 	xor 	byte [m_kbf3],kb_fral	;:clear flag
 	jmp 	kallt1

	;
	; handle toggle keys &pd fixed autorepeat 980115
	;
kcaps:
	mov		ch,kb_fcaps				;caps lock
	jmp		short ktog
kscrl:
	mov		ch,kb_fscrs				;scroll lock
	jmp		short ktog
knums:
	mov		ch,kb_fnums
ktog:
	and		cl,cl					;break ?
	jns		knums2					;:no
	not		ch						;clear key pressed flag
	and		[m_kbf1],ch
knums1:
	ret

knums2:
	test	[m_kbf1],ch				;already pressed ?
	jnz		knums3					;:don't toggle again
	xor		[m_kbf],ch				;toggle numlock flag
knums3:
	or		[m_kbf1],ch				;set pressed flag
 	ret

	;
	; pause
	;
kpaus:
 	and		cl,cl					;break ?
	js		knums1					;:ignore
	test	byte [m_kbf1],kb_fhld	;in hold mode ?
	jnz		knums1					;:yes -> ret
 	or 		byte [m_kbf1],kb_fhld	;set hold flag
 	mov 	al,eoi					;reset interrupt controller
 	out 	pic0,al
 	call	enakbd					;enable keyboard
kpaus1:
 	sti								;wait for next event
	hlt
	test 	byte [m_kbf1],kb_fhld	;still on ?
 	jnz 	kpaus1					;yes: hold
 	pop 	ax						;remove return address
 	jmp 	done2					;exit

	;
	; print screen
	;
kprts:
 	and		cl,cl
	js		knums1					;:ignore break
	cli
 	mov 	al,eoi					;reset interrupt controller
 	out 	pic0,al
 	int 	5						;do screen dump
 	pop 	ax						;remove return address
 	jmp 	done2					;return

	;
	; reboot system
	;
kboot: 
	mov 	word [m_rstflg],1234h	;set cookie
; 	jmp	0f000h:0fff0h			;reset jump

	db	0EAh
	dw	WarmReset
	dw	0F000h

	;
	; system request
	;
ksysr:
 	mov 	al,eoi					;reset interrupt controller
 	out 	pic0,al
 	mov 	ax,8500h
 	and 	cl,cl
 	jns 	ksysr1					;:make
 	inc 	ax						;break code
ksysr1:
	int 	15h						;sys req interrupt
 	pop 	ax						;remove return address
 	jmp 	done2					;exit

	;
	; break
	;
kbrk:
	and		cl,cl					;ignore key release
	js		knums1
	or 		byte [m_brkflg],128		;set break flag
 	mov 	ax,[m_kbstart]			;clear kbd buffer
 	mov 	[m_kbhead],ax
 	mov 	[m_kbtail],ax
 	int 	1bh						;break interrupt
 	xor 	ax,ax
 	jmp 	putbuf					;put break char

	;
	; alt + digit
	;
kdigtab:
 	db 	7,8,9,0,4,5,6,0,1,2,3,0

kdig:
 	and		cl,cl					;ignore break
	js		kdig1
	test 	byte [m_kbf3],kb_fe0	;E0 prefix ?
	jnz		kdig2					;yes: cursor keys, not Alt-number
	mov 	al,cl
; 	mov 	bx,offset kdigtab-47h
; 	cs: 	xlat
 	mov 	bx, kdigtab-47h
 	xlat
 	mov 	ch,al
 	mov 	al,[m_kbnum]			;old value * 10
 	mov 	ah,10
 	mul 	ah
 	add 	al,ch					;add digit
 	mov 	[m_kbnum],al
kdig1:
 	ret
 	
kdig2:
	mov		ah,cl					;handle Alt-cursor keys
	add		ah,50h
	mov		al,0
	jmp		putbuf

	;
	; enable AT kbd
	;
enakbd: 
	cli
 	call 	waitkbd
 	mov 	al,0aeh
 	out 	kb_stat,al
 	sti
 	ret

	;
	; ring bell
	;
beep:
	push	ax
	push	cx
	mov		al,0b6h     		;set beep timer
	out		timer+3,al
	mov		al,33h				;beep frequency
	out		timer+2,al
	mov		al,05h
	out		timer+2,al
	in		al,port61			;enable beep
	mov		ah,al
	or		al,3	
	out		port61,al
	xor		cx,cx				;delay
bell1:
	out		iowait,ax
	loop	bell1
	mov		al,ah
	out		port61,al
	pop		cx
	pop		ax
	ret
