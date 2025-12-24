[BITS 32]

global exp
global exit
global hack
global rewrite
extern clear_console
extern globali
extern memmove
extern printf

global nrec

exp:
  sub sp, 4092
  call exp

nrec:
  mov eax, [esp + 4]
  cmp eax, 0
  jz .to_ret
    sub esp, 4088
    sub eax, 1
    push eax
    call nrec
    add esp, 4092
  .to_ret:
  ret


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

  .lop:
    jmp .lop ; wait for timer tick

  .hacked:
    and esp, ~0xf
    lea eax, st
    mov [esp], eax
    call printf
    jmp $

rewrite:
  mov eax, hack
  call eax
