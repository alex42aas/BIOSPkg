	;
	; bad command
	;
pci_badc:
	pop		ax							;pop return address
pci_badc0:
; mov	al,81h							;bad command
 mov	ax,0081h						;bad command
	;
	; return error
	;
pci_i1a4:

	or		byte [ss:bp+_efl],1			;set carry flag
	jmp		pci_i1a3

	;
	; 01: PCI BIOS present
	;
pci_pres:
	mov		word [ss:bp+_ebx],pci_ver		;PCI 2.1 -> BX
	mov 	dword [ss:bp+_edx],20494350h	;"PCI " -> DX
	mov		al,byte [cs:d_lastbus]		;number of last PCI bus -> CL
	mov		byte [ss:bp+_ecx],al
	mov		byte [ss:bp+_eax],11h			;hardware mechanism 1 -> AL
	clc
	ret

	;
	; 08: read config byte
	;
pci_readb:

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	push	mes_pci_read_config_byte
	call	SendTxt
	push	bx
	call	SendWord
	push	' '
	call	SendChar
	push	di
	call	SendWord
	push	' '
	call	SendChar
%endif
%endif

	mov		ax,0ff00h					;address mask (forbidden bits)
	call	pci_idx						;set index
	in		al,dx						;read data
	mov		[ss:bp+_ecx],al				;return in CL

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	pushf
	push	mes_return
	call	SendTxt
	push	ax
	call	SendByte
	push	mes_crlf
	call	SendTxt
	popf
%endif
%endif

	ret

	;
	; 09: read config word
	;
pci_readw:

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	push	mes_pci_read_config_word
	call	SendTxt
	push	bx
	call	SendWord
	push	' '
	call	SendChar
	push	di
	call	SendWord
	push	' '
	call	SendChar
%endif
%endif

	mov		ax,0ff01h					;address mask
	call	pci_idx						;set index
	in		ax,dx						;read data
	mov		[ss:bp+_ecx],ax				;return in CX

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	pushf
	push	mes_return
	call	SendTxt
	push	ax
	call	SendWord
	push	mes_crlf
	call	SendTxt
	popf
%endif
%endif

	ret

	;
	; 0A: read config dword
	;
pci_readd:

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	push	mes_pci_read_config_dword
	call	SendTxt
	push	bx
	call	SendWord
	push	' '
	call	SendChar
	push	di
	call	SendWord
	push	' '
	call	SendChar
%endif
%endif

	mov		ax,0ff03h					;address mask
	call	pci_idx						;set index

	in 		eax,dx						;read data
	mov 	[ss:bp+_ecx],eax				;return in ECX

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	pushf
	push	mes_return
	call	SendTxt
	push	eax
	call	SendDword
	push	mes_crlf
	call	SendTxt
	popf
%endif
%endif

	ret

	;
	; 0B: write config byte
	;
pci_writb:
	mov		ax,0ff00h					;address mask
	call	pci_idx						;set index
	out		dx,al						;write data
	ret

	;
	; 0C: write config word
	;
pci_writw:
	mov		ax,0ff01h					;address mask
	call	pci_idx						;set index
	out		dx,ax						;write data
	ret

	;
	; 06: generate special cycle
	;
pci_spec:
	cmp 	bh,byte [cs:d_lastbus]		;bus number ok ?
	ja		pci_badc					;:bad
	mov		bl,0ffh						;device number = FF
	mov 	ecx,edx						;special cycle data -> ECX
	xor		di,di						;register number 0 fall through

	;
	; 0D: write config dword
	;
pci_writd:
	mov		ax,0ff03h					;address mask
	call	pci_idx						;set index
	out 	dx,eax						;write data
	ret

	;
	; set PCI configuration index
	;
pci_idx:
	test	di,ax						;any "forbidden" bits set ?
	jnz		pci_idx1
	mov		dx,pci_ad
	mov		ah,80h						;configuration enable
	mov		al,bh						;bus number
	shl 	eax,16						;-> high word
	mov		ax,di						;register number
	and		al,0fch						;clear lower bits
	mov		ah,bl						;device number / function number

	out 	dx,eax						;set index
	mov     dl,pci_datl
	and		di,3						;low bits of register
	add		dx,di						;update register pointer (implicit clc)
	mov 	eax,ecx						;write data -> EAX
	ret
	
pci_idx1:
	pop		ax							;return address
	mov		al,87h						;bad register number
	stc
	ret									;return to dispatcher

	;
	; 02: find PCI device
	;
pci_find:

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	push	mes_pci_find_device
	call	SendTxt
	push	dx
	call	SendWord
	push	' '
	call	SendChar
	push	cx
	call	SendWord
	push	' '
	call	SendChar
%endif
%endif

	inc		dx							;vendor ID FFFF ?
	jnz		pci_find2					;:ok
	mov		al,83h						;bad vendor ID
	stc
	ret

pci_find2:

	dec		dx							;restore vendor ID
	mov 	ebx,80000000h				;bus address
	shl 	ecx,16						;device ID -> bits 31..16
	mov		cx,dx						;vendor ID -> bits 15..00
	mov 	esi,0ffffffffh				;mask
	jmp		short pci_find3

	;
	; 03: find PCI class code
	;
pci_class:
	shl 	ecx,8						;class code is bits 31..08
	mov 	esi,0ffffff00h				;mask
	mov 	ebx,80000000h+p_class		;bus address
	
pci_find3:
	mov		di,[ss:bp+_esi]				;restore device index
          ;
          ; search all buses / devices
          ;
pci_find4:
	mov		dx,pci_ad
	mov 	eax,ebx						;device address
	mov		al,p_id						;vendor / device ID
	out 	dx,eax
	mov		dl,pci_dat
	in 		eax,dx						;read device / vendor ID
	cmp		ax,0ffffh					;not present ?
	jz		pci_find7					;:skip entire device
	cmp		bl,0						;looking for class code ?
	jz		pci_find5					;:no
	mov		dl,pci_ad
	mov 	eax,ebx						;restore register offset
	out 	dx,eax
	mov		dl,pci_dat
	in 		eax,dx						;read class code
pci_find5:
	and 	eax,esi						;mask relevant bits
	cmp 	eax,ecx						;same ?
	jnz		pci_find6					;:no

	dec		di							;device count
	js		pci_found					;:this is the one
	
	; try next function
	
pci_find6:
	test	bh,7						;function 0 ?
	jnz		pci_find8					;:no
	mov		dl,pci_ad
	mov 	eax,ebx						;index
	mov		al,p_hedt					;header type
	and		al,0fch 
	out 	dx,eax
	mov		dl,pci_dat + 2
	in 		al,dx						;read header type
	test	al,80h						;multifunction device ?
	jnz		pci_find8					;:yes
pci_find7:
	or		bh,7						;skip the rest of this device
	
	; try next device / function
	
pci_find8:
	inc		bh							;next device / function
	jnz		pci_find4					;:ok
	
	; try next bus
	
	ror 	ebx,16
	inc		bx							;next bus
	cmp		bl,byte [cs:d_lastbus]
	ja		pci_find9					;:not found
	ror 	ebx,16						;restore
	jmp		pci_find4

	; didn't find it
	
pci_find9:

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	push	mes_pci_not_found
	call	SendTxt
%endif
%endif

	mov		al,86h						;device not found
	stc
	ret
	
	; found device
	
pci_found:

	shr 	ebx,8						;return bus, device number in BX
	mov		[ss:bp+_ebx],bx

%if VERBOSE_PCI_INT
%if SERIAL_DEBUG
	push	mes_pci_found
	call	SendTxt
	push	bx
	call	SendWord
	push	mes_crlf
	call	SendTxt
%endif
%endif

	clc									;ok return
	ret

	;
	; PCI data
	;
	
	; this is a procedure to avoid problems in protected mode access...
	
getlbus:
	db		0b0h						;MOV AL
d_lastbus:
	db		0							;last PCI bus
	ret

mes_pci_read_config_byte:
	db	'PCI read configuration byte ',0
mes_pci_read_config_word:
	db	'PCI read configuration word ',0
mes_pci_read_config_dword:
	db	'PCI read configuration dword ',0
mes_return:
	db	'return ',0
mes_pci_find_device:
	db	'Find PCI device ',0
mes_pci_not_found:
	db	'Failed!',13,10,0
mes_pci_found:
	db	'success at ',0
