PCH_ACPI_BASE_ADDRESS		EQU 01800h		; PM_BASE
GPIO_BASE_ADDRESS		EQU 0800h		; GPIO_BASE

MICROCODE_FV_BASE_ADDRESS	EQU 0FFF30000h      	; microcode volume base
MICROCODE_START_ADDRESS		EQU 0FFF30060h     	; microcode tables start
MICROCODE_FV_SIZE		EQU 10000h          	; PcdGet32 (PcdFlashMicrocodeFvSize)
