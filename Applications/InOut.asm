;------------------------------------------------------------------------------
;
; Copyright (c) 2006, Intel Corporation. All rights reserved.<BR>
; This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http://opensource.org/licenses/bsd-license.php.
;
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
; Module Name:
;
;   InOut.Asm
;
; Abstract:
;
;   inportb, outportb function
;
; Notes:
;
;------------------------------------------------------------------------------

    .code

;------------------------------------------------------------------------------
; int
; EFIAPI
; inportb (
;   int port	    |	rcx
;   );
;------------------------------------------------------------------------------
;
inportb   PROC
	push	rdx
	mov	edx, ecx
	xor	eax, eax
   	in      al, dx
	pop	rdx
    	ret
inportb   ENDP


;------------------------------------------------------------------------------
; int
; EFIAPI
; inport (
;   int port	    |	rcx
;   );
;------------------------------------------------------------------------------
;
inport   PROC
	mov	edx, ecx
	xor	eax, eax
   	in      ax, dx
    	ret
inport   ENDP


;------------------------------------------------------------------------------
; int
; EFIAPI
; inportd (
;   int port	    |	rcx
;   );
;------------------------------------------------------------------------------
;
inportd   PROC
	mov	edx, ecx
	xor	eax, eax
   	in      eax, dx
    	ret
inportd   ENDP

;------------------------------------------------------------------------------
; void
; EFIAPI
; outportb (
;   int port,		|   rcx
;   int data		|   rdx
;   );
;------------------------------------------------------------------------------
outportb   PROC
	mov	al, dl	    ; data
	mov	edx, ecx    ; port
   	out     dx, al
    	ret
outportb   ENDP


;------------------------------------------------------------------------------
; void
; EFIAPI
; outport (
;   int port,		|   rcx
;   int data		|   rdx
;   );
;------------------------------------------------------------------------------
outport   PROC
	mov	eax, edx    ; data
	mov	edx, ecx    ; port
   	out     dx, ax
    	ret
outport   ENDP

;------------------------------------------------------------------------------
; void
; EFIAPI
; outportd (
;   int port,		|   rcx
;   int data		|   rdx
;   );
;------------------------------------------------------------------------------
outportd   PROC
	mov	eax, edx    ; data
	mov	edx, ecx    ; port
   	out     dx, eax
    	ret
outportd   ENDP


;------------------------------------------------------------------------------
; UIN32
; EFIAPI
; retStack (void);
;
retStack    PROC
	mov	rax, rsp
	ret
retStack    ENDP    




MSR_APIC_BASE	equ		0001Bh		; индекс MSR
FLAG_BSP	equ		(1 SHL 8)	; тип CPU


;==============================================================
;   int	isBootCpu(void);
;
;---------------------------------------------------------------
isBootCpu   PROC
	push    rdx
	push    rcx
	mov	ecx, MSR_APIC_BASE				; 
	rdmsr							;
	and	eax, FLAG_BSP					; 
	je	l1
	mov	eax, 1
l1:	pop	rcx
	pop	rdx					
	ret							; TRUE - BSP, FALSE - AP
isBootCpu   ENDP


;==============================================================
;   int	getApicId(void);
;
;---------------------------------------------------------------
getApicId   PROC
	mov	ecx, MSR_APIC_BASE				; 
	rdmsr							;
	ret							; 
getApicId   ENDP


;==============================================================
;   UINT64	readMsr(UINT32 msr);
;
;---------------------------------------------------------------
readMsr   PROC
	rdmsr							; ecx = msr
	shl     rdx, 20h
	or      rax, rdx
	ret							; eax, edx = [msr] 
readMsr   ENDP

;==============================================================
;   UINT32	readFlags(void);
;
;---------------------------------------------------------------
readFlags   PROC
	pushfq							;
	pop	rax						;
	ret							; 
readFlags   ENDP

    END
