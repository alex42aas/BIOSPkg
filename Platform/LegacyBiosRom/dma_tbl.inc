tdmatab:

	db	4,dma0+8			;disable dma0
	db	4,dma1+8+8			;disable dma1
	db	0,dma0+0dh			;reset dma0 (data don't care)
	db	0,dma1+0dh+0dh		;reset dma1 (data don't care)
	db	0fh,dma0+0fh		;mask all channels
	db	0fh,dma1+0fh+0fh	;mask all channels
	db	0,dma0+8			;Enable dma0
	db	0,dma1+8+8			;Enable dma1

	db	0,dma1+8+8			;Enable dma1
	db	0c0h,dma1+0bh+0bh	;Cascade mode, channel 4
	db	041h,dma1+0bh+0bh	;Single mode, channel 5
	db	042h,dma1+0bh+0bh	;Single mode, channel 6
	db	043h,dma1+0bh+0bh	;Single mode, channel 7
	db	0,dma1+09h+09h		;clear DRQ4 request
	db	0,dma1+0ah+0ah		;unmask DRQ4 -> enable cascade
	db	0,dma0+8			;Enable dma0
	db	40h,dma0+0bh		;Single mode, channel 0
	db	41h,dma0+0bh		;Single mode, channel 1
	db	42h,dma0+0bh		;Single mode, channel 2
	db	43h,dma0+0bh		;Single mode, channel 3
	
	db	0,npx+1				;clear x87 interrupt
	
;	db	11h,pic0			;ICW1: edge, ICW4
;	db	08h,pic0+1			;ICW2: interrupt vector 08..0F
;	db	04h,pic0+1			;ICW3: IRQ2 is used for cascade
;	db	01h,pic0+1			;ICW4: 8086 mode
;	db	0ffh,pic0+1			;OCW: mask all interrupts
	
;	db	11h,pic1			;ICW1: edge, ICW4
;	db	70h,pic1+1			;ICW2: interrupt vector 70..77
;	db	02h,pic1+1			;ICW3: slave identification number
;	db	01h,pic1+1			;ICW4: 8086 mode
;	db	0ffh,pic1+1			;OCW: mask all interrupts
	
;	db	26h,port92			;enable A20 gate

	db	0ffh,0ffh			;end of table
