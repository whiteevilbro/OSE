[BITS 32]

extern printf
global main

string: db "Get goodbye world'd, doofus %d", 0x0A, 0

main:
  sub esp, 16
  xor ebx, ebx
  .loop:
    mov eax, string
    mov [esp], eax
    mov [esp + 4], ebx
    call printf
    inc ebx
    jmp .loop
