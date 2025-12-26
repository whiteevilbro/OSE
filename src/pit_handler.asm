[BITS 32]

global timer_handler

extern millis
extern millis_fractions
extern interrupt_millis
extern interrupt_millis_fractions

extern collect_ctx

extern current_quant_limit_millis
extern current_quant_limit_millis_fractions

current_quant_millis: dd 0
current_quant_millis_fractions: dd 0

timer_handler:
  push eax
  pushfd

  mov eax, DWORD [interrupt_millis_fractions]
  add DWORD [millis_fractions], eax
  mov eax, DWORD [interrupt_millis]
  adc DWORD [millis], eax

  mov eax, DWORD [interrupt_millis_fractions]
  sub DWORD [current_quant_millis_fractions], eax
  mov eax, DWORD [interrupt_millis]
  sbb DWORD [current_quant_millis], eax
  jbe sswitch

  .standard:
  popfd
  pop eax
  iret

global sswitch
sswitch:
  .switch:
  mov eax, [current_quant_limit_millis]
  mov [current_quant_millis], eax
  mov eax, [current_quant_limit_millis_fractions]
  mov [current_quant_millis_fractions], eax

  popfd
  pop eax

  push DWORD 0
  push DWORD 0x20
  jmp collect_ctx