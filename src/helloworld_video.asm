[BITS 16]
[ORG 0x7C00]

; cs:ip is set to 0x7C00, right?
; save ip for later
call st ; near call (push ip to stack) - 3 bytes

; 0x7C03  ; 20 bytes of data
dw 0xDEAD ; di - available
dw 0xDEAD ; si - available
dw 0xDEAD ; bp - available
dw 0xDEAD ; <> - discarded
dw 0x0000 ; bx -> buffer offset
dw 0x0000 ; dx -> dh=0 dl=0 -> zeroth head, zeroth (A) floppy 
dw 0x0002 ; cx -> ch=0 cl=2 -> second sector, zeroth cylinder
dw 0x0201 ; ax -> ah=2 al=1 -> read one sector
dw 0xB800 ; es -> buffer base
dw 0x0002 ; flags -> all flags cleared. 1 in reserved bit

st: ; 0x7c00 + 0x0017; 0x17 == 20 + 3
push cs   ; byte
; load ss:sp with 0x7C00 + size(call rel16)
pop ss    ; byte
pop sp    ; byte 
; right now ss:sp is set to 0x7C03, funny, innit?
popa      ; byte
pop es    ; byte
popf      ; byte ; can be replaced with cli, but this is more stylish
int 0x13  ; 2 bytes
hlt       ; byte

; Total: 3 + (1 + 1 + 1) + (1 + 1 + 1) + (2 + 1) = 12

times 510-($-$$) db 0
dw 0xAA55