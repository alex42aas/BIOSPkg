	;
	; baud rates, H/L swapped
	;
rs_baud:
	db	04,17h			;110 baud
	db	03,00h			;150 baud
	db	01,80h			;300 baud
	db	00,0c0h			;600 baud
	db	00,60h			;1200 baud
	db	00,30h			;2400 baud
	db	00,18h			;4800 baud
	db	00,0ch			;9600 baud

rs_ports:       
	dw      3ffh,2ffh,3efh,2efh,287h,28fh
	dw      297h,29fh,2F7h,377h,0   ;0 = end of table 

COM_INIT	equ	11100011b	;9600 8N1 serial port initialization
