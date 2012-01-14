APM_CNT_INT13_CMD   	equ 0xab
SOFT_SMI_PORT       	equ 0xb2
COMM_BUFFER_OFFSET      equ 0x0000
COMM_BUFER_SEGMENT		equ 0xE000
CD_DVD_BOOT_EVENT		equ 0xFFFF

struc CommBuffer
	  .BbsEntryOffset:	resw    1 
	  .BbsEntrySegment:	resw    1 
	  .BbsDeviceType:	resw    1

	  .regs_flags:		resw    1
	  .regs_ax:		resw    1
	  .regs_bx:		resw    1
	  .regs_cx:		resw    1
	  .regs_dx:		resw    1
	  .regs_si:		resw    1
	  .regs_di:		resw    1
	  .regs_bp:		resw    1
	  .regs_sp:		resw    1
	  .regs_ds:		resw    1
	  .regs_es:		resw    1
	  .regs_ss:		resw    1
endstruc

int13Smi:
	mov	    al, APM_CNT_INT13_CMD
	out	    SOFT_SMI_PORT, al
	ret

;   sp	    -> addr_ret
;   sp + 2  -> addr_iret
;   sp + 6  -> flags
saveRegs:
	push	bp
	mov	bp, sp
;
;   bp	    -> bp
;   bp + 2  -> addr_ret
;   bp + 4  -> addr_iret
;   bp + 8  -> flags
;
	push	ax	; [bp - 2] = ax
	push	ds	; [bp - 4] = ds

	mov	ax, COMM_BUFER_SEGMENT
	mov	ds, ax
	mov	ax, word [ss:bp - 2]    ;old ax
	mov	word [COMM_BUFFER_OFFSET + CommBuffer.regs_ax], ax
	mov	ax, bx
	mov	word [COMM_BUFFER_OFFSET + CommBuffer.regs_bx], ax
	mov	ax, cx
	mov	word [COMM_BUFFER_OFFSET + CommBuffer.regs_cx], ax
	mov	ax, dx
	mov	word [COMM_BUFFER_OFFSET + CommBuffer.regs_dx], ax
	mov	ax, si
	mov	word [COMM_BUFFER_OFFSET + CommBuffer.regs_si], ax
	mov	ax, di
	mov	word [COMM_BUFFER_OFFSET + CommBuffer.regs_di], ax
	mov	ax, word [ss:bp]	;old bp
	mov	word [COMM_BUFFER_OFFSET + CommBuffer.regs_bp], ax
	mov	ax, bp
	add	ax, 4			; при входе в INT13 sp = bp + 4
	mov	word [COMM_BUFFER_OFFSET+CommBuffer.regs_sp], ax
	mov	ax, word [ss:bp - 4]	;old ds
	mov	word [COMM_BUFFER_OFFSET+CommBuffer.regs_ds], ax
	mov	ax, es
	mov	word [COMM_BUFFER_OFFSET+CommBuffer.regs_es], ax
	mov	ax, ss
	mov	word [COMM_BUFFER_OFFSET+CommBuffer.regs_ss], ax
	mov	ax, word [ss:bp + 8]	; flags при входе в INT13
	mov	word [COMM_BUFFER_OFFSET+CommBuffer.regs_flags], ax

	pop	ds
	pop	ax
	pop	bp
	ret
    
restoreRegs:
	push ds

	mov ax, COMM_BUFER_SEGMENT
	mov ds, ax
	mov ax, word [COMM_BUFFER_OFFSET+CommBuffer.regs_bx]
	mov bx, ax
	mov ax, word [COMM_BUFFER_OFFSET+CommBuffer.regs_cx]
	mov cx, ax
	mov ax, word [COMM_BUFFER_OFFSET+CommBuffer.regs_dx]
	mov dx, ax
	mov ax, word [COMM_BUFFER_OFFSET+CommBuffer.regs_si]
	mov si, ax
	mov ax, word [COMM_BUFFER_OFFSET+CommBuffer.regs_di]
	mov di, ax
	

	push bp
	mov bp, sp	
	mov ax, word [COMM_BUFFER_OFFSET+CommBuffer.regs_flags]
	mov [ss:bp + 0x0A], ax ;flags on stack
	pop bp

	mov ax, word [COMM_BUFFER_OFFSET+CommBuffer.regs_ax]

	;bp, sp, ds, es do not restore

	pop ds
	ret


Int13:
%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
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
	push    mes_crlf
	call    SendTxt
%endif
%endif
	cli
;   sp	    -> addr_iret
;   sp + 4  -> flags
;
;
	call saveRegs

	call int13Smi

	call restoreRegs

	iret    