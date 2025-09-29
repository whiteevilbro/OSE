[BITS 16]
[ORG 0x7C00]

; === 2 === (14 bytes)

jmp st ; 2 bytes

; 0x10
db 0xEF, 0xBE, 0x90, 0x7D, 0xAD, 0xDE, 0x07, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x01, 0x13,
db 0x00, 0x00
st:
push BYTE 0
pop ss
mov sp, 0x7C00
popa
pop es
int 0x10
hlt

times 0x190-($-$$) db 0

; al == 0 || al == 1
str: db "Hello, world!"

; al == 2 || al == 3
; str: db 'H', 7, 'e', 7, 'l', 7, 'l', 7, 'o', 7, ',', 7, ' ', 7, 'w', 7, 'o', 7, 'r', 7, 'l', 7, 'd', 7, '!', 7

times 510-($-$$) db 0
dw 0xAA55