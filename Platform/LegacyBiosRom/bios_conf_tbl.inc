	;
	; BIOS configuration table (see int 15h AH=C0)
	;
bios_configuration_table:
%if 0
	dw		0Eh			; number of bytes following
	db		0FCh		; model			\
	db		01h			; submodel		-	= "PC3270/AT"
	db		00h			; BIOS revision	/
	db		74h			; feature byte 1 - 2nd 8259, rtc, int09 calls int 15/4F, ebda
	db		02h			; feature byte 2 - support data streaming
	db		00h			; feature byte 3 - none additional features supported
	db		00h			; feature byte 4
	db		00h			; feature byte 5
	db		'A'
	db		'l'
	db		't'
	db		'e'
	db		'l'
	db		'l'
%else
	dw		0x0e
	db		0xfc
	db		0x01
	db		0x00
	db		0x74
	db		0x02
	db		0x00
	db		0x00
	db		0x00
	db		'A'
	db		'l'
	db		't'
	db		'e'
	db		'l'
	db		'l'
%endif