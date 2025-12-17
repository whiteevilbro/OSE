[BITS 32]

global timer_handler

extern millis
extern millis_fractions
extern interrupt_millis
extern interrupt_millis_fractions

timer_handler:
  push eax
  pushfd
  mov eax, DWORD [interrupt_millis_fractions]
  add DWORD [millis_fractions], eax
  mov eax, DWORD [interrupt_millis]
  adc DWORD [millis], eax
  popfd
  pop eax
  iret