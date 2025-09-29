[BITS 16]
[ORG 0x7C00]

; === SERIAL ===
xor cx, cx
mov ds, cx
mov dx, 0x3f8
lea si, str
ploop:
outsb
cmp si, str+13-1
jb ploop
hlt

str: db "Hello, world!"

times 510-($-$$) db 0
dw 0xAA55