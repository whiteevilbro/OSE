[BITS 16]
[ORG 0x7C00]

entry:

; === 1 === (14 bytes)

jmp st ; 2 bytes

; 0x13
; si bp <ax> bx dx cx ax 
db 0x00, 0xb8, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x02

; es
db 0x00, 0xb8

; padding
db 0x00, 0x00

st: ; 0x7c00 + 0x0014
push BYTE 0
pop ss
mov sp, 0x7C00
popa
pop es
int 0x13
hlt


; === 2 === (15 bytes)

; jmp st ; 2 bytes

; 0x10
; db 0xEF, 0xBE, 0x90, 0x7D, 0xAD, 0xDE, 0x07, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x01, 0x13,

; st:
; xor ax, ax
; mov ss, ax
; mov sp, 0x7C00
; popa
; pop es
; int 0x10
; hlt

; ===   ===

; === SERIAL ===
; xor cx, cx
; mov ds, cx
; mov dx, 0x3f8
; lea si, str
; ploop:
; outsb
; cmp si, str+13-1
; jb ploop
; hlt



times 0x160-($-$$) db 0

times 0x180-($-$$) db 0

g db 0x60, 0x7D, 0, 0, 0, 00

times 0x190-($-$$) db 0

str: db "Hello, world!"
; str: db 'H', 7, 'e', 7, 'l', 7, 'l', 7, 'o', 7, ',', 7, ' ', 7, 'w', 7, 'o', 7, 'r', 7, 'l', 7, 'd', 7, '!', 7

times 510-($-$$) db 0
dw 0xAA55