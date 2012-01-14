        ;
        ; 32 bit PCI BIOS entry point
        ;
        ;use32
BITS 32

pci_32: pushad          ;build stack frame
        mov     ebp,esp

        cmp     al,0ah  ;0A: read config dword
        jnz     pci_3a
        call    pci3readd
        jmp     near pci_3z

pci_3a: cmp     al,09   ;09: read config word
        jnz     pci_3b
        call    pci3readw
        jmp near  pci_3z

pci_3b: cmp     al,08   ;08: read config byte
        jnz     pci_3c
        call    pci3readb
        jmp near pci_3z

pci_3c: cmp     al,0dh  ;0D: write config dword
        jnz     pci_3d
        call    pci3writd
        jmp     short pci_3z

pci_3d: cmp     al,0ch  ;0C: write config word
        jnz     pci_3e
        call    pci3writw
        jmp     short pci_3z

pci_3e: cmp     al,0bh  ;0B: write config byte
        jnz     pci_3f
        call    pci3writb
        jmp     short pci_3z

pci_3f: cmp     al,01   ;01: PCI BIOS present
        jnz     pci_3g
        call    pci3pres
        jmp     short pci_3z

pci_3g: cmp     al,02   ;02: find PCI device
        jnz     pci_3h
        call    pci3find
        jmp     short pci_3z

pci_3h: cmp     al,03   ;03: find PCI class code
        jnz     pci_3j
        call    pci3find
        jmp     short pci_3z

pci_3j: cmp     al,06   ;06: generate special cycle
        jnz     pci_3k
        call    pci3spec
        jmp     short pci_3z

pci_3k: call    pci3badc        ;others: bad commands

pci_3z: jb      pci_32b ;:error
        mov     al,0    ;return ok status
pci_32b:        mov     [ebp+_eax+1],al ;status code -> AH
        popad           ;restore registers
        retf
        ;
        ; bad command
        ;
pci3badc:       mov     al,81h  ;return error code
        stc
        ret
        ;
        ; 01: PCI BIOS present
        ;
pci3pres:       mov     word [ebp+_ebx],pci_ver ;PCI 2.1 -> BX
        mov     dword [ebp+_edx],20494350h      ;"PCI " -> EDX
        call    getlbus ;get number of last PCI bus -> AL
        mov     [ebp+_ecx],al   ;-> return in CL
        mov     byte [ebp+_eax],11h     ;hardware mechanism 1 -> AL
        clc
        ret
        ;
        ; 08: read config byte
        ;
pci3readb: mov  al,0    ;address mask (forbidden bits)
        call    pci3idx ;set index
        in      al,dx   ;read data
        mov     [ebp+_ecx],al   ;return in CL
        ret
        ;
        ; 09: read config word
        ;
pci3readw: mov  al,1    ;address mask
        call    pci3idx ;set index
        in      ax,dx   ;read data
        mov     [ebp+_ecx],ax   ;return in CX
        ret
        ;
        ; 0A: read config dword
        ;
pci3readd: mov  al,03   ;address mask
        call    pci3idx ;set index
        in      eax,dx  ;read data
        mov     [ebp+_ecx],eax  ;return in ECX
        ret
        ;
        ; 0B: write config byte
        ;
pci3writb: mov  al,00   ;address mask
        call    pci3idx ;set index
        out     dx,al   ;write data
        ret
        ;
        ; 0C: write config word
        ;
pci3writw: mov  al,01   ;address mask
        call    pci3idx ;set index
        out     dx,ax   ;write data
        ret
        ;
        ; 06: generate special cycle
        ;
pci3spec:       call    getlbus ;bus number ok ?
        cmp     bh,al
        ja      pci3badc        ;:bad
        mov     bl,0ffh ;device number = FF
        mov     ecx,edx ;special cycle data -> ECX
        xor     edi,edi ;register number 0
                        ;fall through
        ;
        ; 0D: write config dword
        ;
pci3writd: mov  al,03   ;address mask
        call    pci3idx ;set index
        out     dx,eax  ;write data
        ret
        ;
        ; set PCI configuration index
        ;
pci3idx:        ror     ebx,8
        mov     bh,80h
        rol     ebx,16
        mov     dx,[ebp+_edi]   ;get index
        test    dl,al   ;forbidden bits set ?
        jnz     pci3idx1        ;:yes
        and     dh,dh   ;high byte ?
        jnz     pci3idx1        ;:yes
        mov     bl,dl   ;set index
        mov     dx,pci_ad       ;set port address
        mov     eax,ebx ;get index -> eax
        and     al,0fch ;mask low bits
        out     dx,eax  ;set index
        mov     dl,bl   ;get low address
        or      dl,pci_dat ;assume pci_dat $fc (implicit clc)
        mov     eax,ecx ;write data -> EAX
        ret

pci3idx1:       pop     eax     ;return address
        mov     al,87h  ;bad register number
        stc
        ret             ;return to dispatcher
        ;
        ; 02: find PCI device
        ;
pci3find:       inc     dx      ;vendor ID FFFF ?
        jnz     pci3find2       ;:ok
        mov     al,83h  ;bad vendor ID
        stc
        ret

pci3find2: dec  dx      ;restore vendor ID
        mov     ebx,80000000h   ;bus address
        shl     ecx,16  ;device ID -> bits 31..16
        mov     cx,dx   ;vendor ID -> bits 15..00
        mov     edi,0ffffffffh  ;mask
        jmp     short pci3find4
        ;
        ; 03: find PCI class code
        ;
pci3class: shl  ecx,8   ;class code is bits 31..08
        mov     edi,0ffffff00h  ;mask
        mov     ebx,80000000h+p_class   ;bus address
          ;
          ; search all buses / devices
          ;
pci3find4: mov  dx,pci_ad
        mov     eax,ebx ;device address
        mov     al,p_id ;vendor / device ID
        out     dx,eax
        mov     dl,pci_dat
        in      eax,dx  ;read device / vendor ID
        cmp     ax,0ffffh       ;not present ?
        jz      pci3find7       ;:skip entire device
        cmp     bl,0    ;looking for class code ?
        jz      pci3find5       ;:no
        mov     dl,pci_ad
        mov     eax,ebx ;restore register offset
        out     dx,eax
        mov     dl,pci_dat
        in      eax,dx  ;read class code
pci3find5: and  eax,edi ;mask relevant bits
        cmp     eax,ecx ;same ?
        jnz     pci3find6       ;:no
        dec     si      ;device count
        js      pci3found       ;:this is the one

        ; try next function

pci3find6: test bh,7    ;function 0 ?
        jnz     pci3find8       ;:no
        mov     dl,pci_ad
        mov     eax,ebx ;index
        mov     al,p_hedt & 0fch        ;header type
        out     dx,eax
        mov     dl,pci_dat + 2
        in      al,dx   ;read header type
        test    al,80h  ;multifunction device ?
        jnz     pci3find8       ;:yes
pci3find7: or   bh,7    ;skip the rest of this device

        ; try next device / function

pci3find8: inc  bh      ;next device / function
        jnz     pci3find4       ;:ok

        ; try next bus

        ror     ebx,16
        inc     bl      ;next bus
        call    getlbus
        cmp     bl,al
        ja      pci3find9       ;:not found
        ror     ebx,16  ;restore
        jmp     short pci3find4

        ; didn't find it

pci3find9: mov  al,86h  ;device not found
        stc
        ret

        ; found device

pci3found: shr  ebx,8   ;return bus, device number in BX
        mov     [ebp+_ebx],bx
        clc             ;ok return
        ret 
BITS 16