[BITS 32]

global hack
global rewrite
extern printf
extern clear_console
extern memmove

st: db "Lorem ipsum et dolor...", 0

hack:
  pushf
  pop eax
  and eax, 0x200
  jz .hacked

  sub esp, 12
  and esp, ~0xf
  mov eax, clear_console
  mov [esp], eax
  mov eax, rewrite
  mov [esp + 4], eax
  mov eax, 7
  mov [esp + 8], eax
  call memmove

  jmp $ ; wait for timer tick

  .hacked:
    and esp, ~0xf
    lea eax, st
    mov [esp], eax
    call printf
    jmp $

rewrite:
  mov eax, hack
  call eax
