        ;
        ; RTC int1A dispatch table
        ;
        align 2
rtc_int1atab:
        dw      tm_get  			;AH=0: get time
        dw      tm_set  			;AH=1: set time
        dw      rtc_get 			;AH=2: get RTC time
        dw      rtc_set 			;AH=3: set RTC time
        dw      rtc_date        	;AH=4: get RTC date
        dw      rtc_sdat        	;AH=5: set RTC date
        dw      rtc_alrm        	;AH=6: set RTC alarm
        dw      rtc_snz 			;AH=7: clear RTC alarm

rtc_tab:        
	db      0,cm_ss         ;00: секунда            
	db      0,cm_ssa        ;01: alarm секунда
    db      0,cm_mm         ;02: минута
	db      0,cm_mma        ;03: alarm минута
	db      0,cm_hh         ;04: час
	db      0,cm_hha        ;05: alarm час
	db      0,cm_day        ;06: день недели
	db      1,cm_dd         ;07: день
	db      1,cm_mo         ;08: месяц
	db      00h,cm_yy       ;09: год
	db      80h,cm_dia      ;0E: power loss флаг
	db      20h,cm_cent     ;32: век
rtc_tab9:                       ;end of table 