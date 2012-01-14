	;
	; floppy vector table
	;
fd_vectab: 
	dw	(fd_rst)	;AH=00: recalibrate drive
	dw	(fd_status)	;AH=01: get status
	dw	(fd_read)	;AH=02: read
	dw	(fd_write)	;AH=03: write
	dw	(fd_verify)	;AH=04: verify
	dw	(fd_format)	;AH=05: format track
	dw	(fd_badcmd)	;AH=06: bad
	dw	(fd_badcmd)	;AH=07: bad
	dw	(fd_drvprm)	;AH=08: read drive parameters
	dw	(fd_badcmd)	;AH=09: bad
	dw	(fd_badcmd)	;AH=0A: bad
	dw	(fd_badcmd)	;AH=0B: bad
	dw	(fd_badcmd)	;AH=0C: bad
	dw	(fd_badcmd)	;AH=0D: bad
	dw	(fd_badcmd)	;AH=0E: bad
	dw	(fd_badcmd)	;AH=0F: bad
	dw	(fd_badcmd)	;AH=10: bad
	dw	(fd_badcmd)	;AH=11: bad
	dw	(fd_badcmd)	;AH=12: bad
	dw	(fd_badcmd)	;AH=13: bad
	dw	(fd_badcmd)	;AH=14: bad
	dw	(fd_gettyp)	;AH=15: get drive type
	dw	(fd_dskchg)	;AH=16: get disk change status
	dw	(fd_fmttyp)	;AH=17: set drive type for format
	dw	(fd_medtyp)	;AH=18: set media type for format

	;
	; error codes
	;
fd_errtab:
	db	20h			;(bit overflow)
	db	4			;sector not found
	db	20h			;(not used)
	db	10h			;CRC error
	db	8			;DMA overrun
	db	20h			;(not used)
	db	4			;sector not found
	db	3			;write protect
	db	2			;address mark not found

	;
	; Floppy parameters
	;
fd_ptab:
	db	0dfh		;step rate, head unload
	db	02			;head load, DMA mode
	db	25h			;motor wait
	db	02			;512 bytes per sector
	db	18			;end of track
	db	24h			;normal gap
	db	0ffh		;DTL
	db	54h			;gap length for format
	db	0f6h		;fill byte for format
	db	15			;head settle time (x 1 ms)
	db	8			;motor start time (x 125 ms)
