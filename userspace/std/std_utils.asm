[BITS 32]

global print_char

extern main

startup:
  sub esp, 16
  mov [esp], esi
  mov [esp+4], edi
  call main
  xor eax, eax
  int 0x30
  ; unreachable

print_char:
  mov eax, [esp + 4]
  int 0x31
  ret