        ;
        ; AH=2: get RTC time
        ;
rtc_get:
        call    rtc_uip 					;check for RTC update
        jb      int1a_err       			;:error
        mov     ah,cm_ss        			;read seconds -> DH
        call    rtc_read
        mov     dh,al
        mov     ah,cm_mm        			;read minutes -> CL
        call    rtc_read
        mov     cl,al
        mov     ah,cm_hh        			;read hours -> CH
        call    rtc_read
        mov     ch,al
        mov     ah,cm_b 					;read daylight savings bit -> DL
        call    rtc_read
        and     al,1
        mov     dl,al
        jmp     short int1a_ok

        ;
        ; AH=3: set RTC time
        ;
rtc_set:
        push    cx
        call    rtc_uip 					;check for RTC update
        pop     cx
        mov     ah,cm_ss        			;DH -> seconds
        mov     al,dh
        call    rtc_write
        mov     ah,cm_mm        			;CL -> minutes
        mov     al,cl
        call    rtc_write
        mov     ah,cm_hh        			;CH -> hours
        mov     al,ch
        call    rtc_write
        mov     ah,cm_b 					;read status register B
        call    rtc_read
        and     al,01100010b    			;mask off set, update interrupt,
                        					;square wave, daylight savings
        or      al,2    					;set 24 hour mode
        or      al,dl   					;add daylight savings bit from DL
        call    rtc_write       			;update status register B
int1a_ok:
       clc
int1a_ret:
		sti
        pop     bx
        pop     ds
        ;retf    2
		iret

int1a_err:

		stc
        ;jmp     short int1a_ret
		jmp     int1a_ret

        ;
        ; AH=4: get RTC date
        ;
rtc_date:
		call    rtc_uip 							;check for RTC update
        jb      int1a_err
        mov     ah,cm_dd        					;day -> DL
        call    rtc_read
        mov     dl,al
        mov     ah,cm_mo        					;month -> DH
        call    rtc_read
        mov     dh,al
        mov     ah,cm_yy        					;year -> CL
        call    rtc_read
        mov     cl,al
        mov     ah,cm_cent      					;century -> CH
        call    rtc_read
        mov     ch,al
        cmp     cx,1980h        					;century roll-over ?
        jae     rtc_date9
        mov     ax,cm_cent*256+20h      			;update century register
        call    rtc_write
        mov     ch,20h  							;force 2000
rtc_date9:
		jmp  int1a_ok

        ;
        ; AH=5: set RTC date
        ;
rtc_sdat:
        push    cx
        call    rtc_uip 							;check for RTC update
        pop     cx
        mov     ax,cm_day*256   					;0 -> day of week (not used)
        call    rtc_write
        mov     ah,cm_dd        					;DL -> day
        mov     al,dl
        call    rtc_write
        mov     ah,cm_mo        					;DH -> month
        mov     al,dh
        call    rtc_write
        mov     ah,cm_yy        					;CL -> year
        mov     al,cl
        call    rtc_write
        mov     ah,cm_cent      					;CH -> century
        mov     al,ch
        call    rtc_write
        jmp     int1a_ok

        ;
        ; AH=6: set RTC alarm
        ;
rtc_alrm:
        mov     ah,cm_b 							;read status B
        call    rtc_read
        and     al,20h  							;alarm enabled ?
        jnz     int1a_err       					;:error
        push    cx     	 							;save CX !
        call    rtc_uip 							;check for RTC update
        pop     cx      							;restore...
        mov     ah,cm_ssa       					;DH -> alarm second
        mov     al,dh
        call    rtc_write
        mov     ah,cm_mma       					;CL -> alarm minute
        mov     al,cl
        call    rtc_write
        mov     ah,cm_hha       					;CH -> alarm hour
        mov     al,ch
        call    rtc_write

        in      al,pic1+1       					;read mask register
        and     al,0feh 							;enable RTC interrupt (8)
        out     pic1+1,al

        mov     ah,cm_b 							;read status B
        call    rtc_read
        or      al,20h  							;enable alarm
        call    rtc_write
        jmp     int1a_ok

        ;
        ; AH=7: clear RTC alarm
        ;
rtc_snz:
        mov     ah,cm_b 							;read status B
        call    rtc_read
        and     al,5fh  							;disable alarm interrupt
        call    rtc_write
        jmp     int1a_ok

        ;
        ; AH=0: get time
        ;
tm_get:

		mov     dx, word [m_timer]    					;DX = timer low
        mov     cx, word [m_timer+2]  					;CX = timer high
        mov     al,0
        xchg    al,[m_timofl]   					;AL = timer overflow, reset flag
        jmp     int1a_ok

        ;
        ; AH=1: set time
        ;
tm_set:
		mov     byte [m_timofl],0       			;clear timer overflow flag
        mov     [m_timer],dx   						;DX = timer low
        mov     [m_timer+2],cx  					;CX = timer high
        jmp     int1a_ok

         ;
        ; Clear RTC interrupt, test shutdown byte
        ;
        ; Be sure to leave cm_test non-zero, as this is used by the
        ; master reset code in cs_clr to determine whether to reset
        ; the bus.
        ;
rtc_test:
		mov     al,cm_nmi+cm_c  ;clear pending interrupt
        out     cm_idx,al
        out     iowait,al
        in      al,cm_dat

        mov     bx,8000h+cm_nmi+cm_test 			;bit to test, test register
rtc_test1:
		mov  	al,bl   							;write pattern
        out     cm_idx,al
        out     iowait,al
        mov     al,bh
        out     cm_dat,al
        out     iowait,al

        mov     al,bl   							;read back
        out     cm_idx,al
        out     iowait,al
        in      al,cm_dat
        out     iowait,al
        cmp     al,bh
        jnz     rtc_test8       					;:error - clc -> inverted -> error
        shr     bh,1    							;shift pattern right
        jnb     rtc_test1       					;:another bit
rtc_test8:
		cmc          								;last bit inverted -> no carry if ok
        ret

        ;
        ; Wait for RTC UIP bit cleared
        ;
        ; This bit is set about 250ns before the next update. If clear,
        ; we have at least 250ns to read or write the RTC without
        ; updates coming in between.
        ;
rtc_uip:
        mov     cx,1000
        mov     ah,cm_a
rtc_uip1:       cli
        call    rtc_read
        and     al,80h  							;UIP ?
        jz      rtc_uip9        					;:ok, carry clear
        sti             							;give interrupts a chance
        loop    rtc_uip1        					;:try again
        mov     ax,cm_a*256+26h 					;set 32768 Hz oscillator, 1 ms int
        call    rtc_write      						;to restart...
        stc
rtc_uip9:
        ret
 
        ;
        ; read RTC register [AH] -> AL
        ;
rtc_read:
        mov     al,ah
        out     cm_idx,al
        out     iowait,al
        in      al,cm_dat
        out     iowait,al
        ret

        ;
        ; write RTC register AL -> [AH]
        ;
rtc_write:
		xchg 	al,ah
        out     cm_idx,al
        out     iowait,al
        xchg    al,ah
        out     cm_dat,al
        ret

        ;
        ; clock tick (IRQ0)
        ;
ALIGN 32
Irq0:
		sti    	         								;enable interrupts

%if 0
%if SERIAL_DEBUG
%if DEBUG_PRINT_INT
	push	mes_irq
	call	SendTxt
	push	0
	call	SendByte
	push	mes_crlf
	call	SendTxt
%endif
%endif
%endif

        push    ax
        push    dx
        push    ds

        xor     ax,ax   								;access BIOS segment
        mov     ds,ax
	
        mov     ax,[m_timer]    						;update timer
        mov     dx,[m_timer+2]
        add     ax,1
        adc     dx,0
        cmp     ax,00b2h        						;24 hours ?
        jnz     irq0_1
        cmp     dx,0018h
        jnz     irq0_1
        xor     ax,ax   								;timer overflow - back to 0
        xor     dx,dx
        mov     byte [m_timofl],1
irq0_1:
    		mov     [m_timer],ax
        mov     [m_timer+2],dx

        dec     byte [m_fdcnt]  						;floppy motor timer
        jnz     irq0_2  								;:not yet
        mov     al,0ch
        mov     dx,fdc_ctrl
        out     dx,al   								;turn off motor
        and     byte [m_fdmot],0f0h     				;turn off motor bits

irq0_2:
		int     1ch     								;call user hook

        pop     ds
        pop     dx

        mov     al,eoi  								;signal end of interrupt
        cli
        out     pic0,al
        pop     ax
        iret

        ;
        ; RTC interrupt (IRQ8)
        ;
Irq8:
;++++++++++++++++++++++++++++++++++++++++++++
%if DEBUG_IRQ
  push  0x08
  push  mes_irq
  call  SendTxt
  call  SendByte
  push  mes_crlf
  call  SendTxt
%endif
;++++++++++++++++++++++++++++++++++++++++++++

		push    ax
        mov     al,cm_c 								;check alarm interrupt bit
        out     cm_idx,al
        ;out     iowait,al
        in      al,cm_dat
        test    al,20h
        jz      irq8_1
        push    ax
;;        int     4ah     								;call user hook
        pop     ax
irq8_1:
		mov     al,eoi  								;signal end of interrupt
        out     pic1,al
        out     pic0,al
        pop     ax
        iret

Int4a:
		iret

        ;
        ; Timer initialization -> 18.2 Hz tick
        ; Unmask timer and keyboard interrupts
        ;

tim_init:
	cli
  push  ax

  mov   al,36h
  out   timer+3,al

  mov   ax, 1193			; TICK_RATE
	xchg	al, ah
  out   timer,al
	xchg	al, ah
  waitio
  out   timer,al

	; set interrupt handler
	push	ds
	xor		ax, ax
	mov		ds, ax
	mov		eax, 0x08*4
	mov		word [eax], Irq0
	add		eax, 2
	mov		word [eax], 0xF000
	pop		ds

  in    al,pic0+1       						;enable timer interrupt
  and   al,11111110b
  out   pic0+1,al
	sti
  pop   ax
  ret

        ;
        ; RTC init
        ;
rtc_ini:
        mov     ah,cm_d 								;read status register D
        call    rtc_read
        and     al,80h  								;battery low ?
        jz      rtc_ini0        						;:yes

        ; battery ok - validate the time / date

        mov     ah,2    								;get RTC time
        int     1ah
        jb      rtc_ini0        						;:error
        mov     al,dh   								;validate seconds
        mov     ah,60h
        call    rtc_val
        jb      rtc_ini0        						;:bad
        mov     al,cl   								;validate minutes
        mov     ah,60h
        call    rtc_val
        jb      rtc_ini0        						;:bad
        mov     al,ch   								;validate hours
        mov     ah,24h
        call    rtc_val
        jb      rtc_ini0        						;:bad

        mov     ah,4    								;get RTC date
        int     1ah
        mov     al,dl   								;day
        mov     ah,31h
        call    rtc_val
        jb      rtc_ini0       							;:bad
        mov     al,dh   								;month
        mov     ah,12h
        call    rtc_val
        jb      rtc_ini0        						;:bad
        mov     ax,cx
        cmp     ax,1980h        						;minimum 1980
        jb      rtc_ini0
        cmp     ax,2099h        						;maximum 2099
        ja      rtc_ini0
        mov     ah,99h  								;maximum year
        call    rtc_val
        jb      rtc_ini0        						;:bad

        mov     ax,cm_dia*256   						;clear diag register
        call    rtc_write
        jmp     short rtc_ini2

        ; battery was low or invalid time - initialize the RTC

rtc_ini0:
        inc     byte [tmp_rtc]  						;set RTC failure flag
        mov     si,rtc_tab
rtc_ini1:
        cs      lodsw   								;get value from table
        call    rtc_write       						;write to RTC
        cmp     si, rtc_tab9    						;end of table ?
        jb      rtc_ini1        						;:no

        ; Set timer tick value from RTC time
        ;
        ; Please note that there are different algorithms with varying
        ; accuracy for doing this, there can be slight time discrepancies
        ; depending on what algorithm is used by the OS.

rtc_ini2:
        mov     ah,2    								;get RTC time
        int     1ah
        jb      rtc_ini9        						;:error
        mov     [tmp_ss],dh     						;save second for run check
        mov     [tmp_mm],cl     						;save minute for run check
        push    dx
        xor     ebx,ebx
        mov     al,ch   								;hour
        mov     edx,01000755h   						;18.206 * 3600 * 256
        call    rtc_mul
        mov     al,cl   								;minute
        mov     edx,00044464h   						;18.206 * 60 * 256
        call    rtc_mul
        pop     ax      								;second
        mov     al,ah
        mov     dx,4661 								;18.206 * 256
        call    rtc_mul
        shr     ebx,8   								;timer / 256
        mov     dword [m_timer],ebx     				;set timer
        mov     [tmp_timer],bx  						;backup for run check
rtc_ini9:
        ret

        ;
        ; convert number in AL from BCD -> binary, * EDX -> add to EBX
        ;
rtc_mul:
        mov     ah,al   								;high digit
        and     al,15   								;mask low digit
        shr     ah,4   	 								;high -> low nibble
        aad             								;convert to binary
        db      066h
        cbw             								;clear top of eax
        mul     edx
        add     ebx,eax 								;add ebx,eax
        ret

        ;
        ; validate a BCD number in AL, AH = limit
        ;
rtc_val:
        cmp     al,ah   								;exceed limit ?
        ja      rtc_val9        						;(no carry -> cmc -> error)
        and     al,15   								;high digit is ok, now check low digit
        cmp     al,10   								;(less than 10 -> carry -> cmc -> ok)
rtc_val9:
        cmc             								;return error status
        ret
