; === IMPORTS & EXPORTS ===

extern kernel_entry
extern __kernel_size_sectors
extern universal_interrupt_handler
global halt
global collect_ctx
global collect_cr
global restore_context
global enable_paging
global disable_paging
global set_cr3
global exp
global tss
global gdt

; === BOOT DEVICE READ CONFIG ===

CYLINDERS_LIMIT equ 79
HEADS_LIMIT equ 1
SECTORS_PER_TRACK_LIMIT equ 36

; === OTHER CONFIG ===

KERNEL_STACK equ 0x7C00

; === CODE ===
; #region boot

[BITS 16]

; stack setup
entry:
  xor cx, cx
  mov ds, cx
  mov ss, cx
  mov sp, KERNEL_STACK

  xor bx, bx
  mov ax, 0x7E0
  mov es, ax

  mov al, 1
  ; ch == 0
  mov cl, 2
  xor dh, dh  

  mov di, __kernel_size_sectors

  ; read (di - 1) consecutive sectors

  .read:
    ; ah = 0x2  - read sector form drive
    ; al = 1    - sectors to read
    ; ch = cylinder index [0, CYLINDERS_LIMIT]
    ; cl = sector index [1, SECTORS_LIMIT]
    ; dh - head index  [0, HEADS_LIMIT]
    ; dl - drive number (passed by BIOS)
    ; es:bx - buffer

    dec di
    jz .end_read_success

    mov ah, 2
    int 0x13
    jc read_error

    mov si, es
    add si, 0x20
    mov es, si

    inc cl
    cmp cl, SECTORS_PER_TRACK_LIMIT
    jbe .read

    mov cl, 1
    xor dh, 1
    jnz .read

    inc ch
    jmp .read

  .end_read_success:

  .end_read:

video_settings:
  ; turn blinking bit off
  mov ax, 0x1003
  ; bl == 0
  int 0x10

  ; hide cursor
  mov ah, 0x01
  mov cx, 0x0706
  int 0x10

memory:
  int 0x12
  sub sp, 12,
  push WORD 0
  push ax

protected_mode_switch:
  lgdt [gdt_pseudodescriptor]
  cli
  cld
  mov eax, cr0
  or eax, 1
  mov cr0, eax
  jmp KERNEL_CODE_SEGMENT:protected_mode_trampoline_to_c


[BITS 32]
protected_mode_trampoline_to_c:
  mov ax, TSS_SEGMENT
  ltr ax

  mov eax, cr4
  or eax, 0x10
  mov cr4, eax

  mov eax, KERNEL_DATA_SEGMENT
  mov ds, eax
  mov ss, eax
  mov es, eax
  mov fs, eax
  mov gs, eax

  ; return address imitation (sets stack to 0x7c00 - 4)
  sub sp, 4

  jmp kernel_entry

; #endregion

; #region utilities
; never ending loop
halt:
  jmp $

; interrupt context collection
collect_ctx:
  push ds
  push es
  push fs
  push gs
  pusha
  mov ebx, esp
  
  cld
  mov eax, KERNEL_DATA_SEGMENT
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax

  ; stack alignment
  sub esp, 4
  and esp, ~0xf

  mov DWORD [esp], ebx
  call universal_interrupt_handler

  mov esp, ebx
unwind_context:
  popa
  pop gs
  pop fs
  pop es
  pop ds
  add esp, 8
  iret

restore_context:
  mov esp, [esp + 4]
  jmp unwind_context

set_cr3:
  mov eax, [esp + 4]
  mov cr3, eax
  ret

enable_paging:
  mov eax, cr0
  or eax, 0x80000000
  mov cr0, eax
  ret

disable_paging:
  mov eax, cr0
  and eax, ~0x80000000
  mov cr0, eax
  ret

collect_cr:
  mov eax, DWORD [esp + 4]
  mov ecx, cr0
  mov DWORD [eax], ecx
  mov ecx, cr2
  mov DWORD [eax + 4], ecx
  mov ecx, cr3
  mov DWORD [eax + 8], ecx
  mov ecx, cr4
  mov DWORD [eax + 12], ecx
  ret

; #endregion

; #region 16-bit utilities

[BITS 16]

read_error:
mov bp, read_error_str
call print
jmp $

; ds:bp - pointer to null-terminated string fully contained in ds effective address space
; ax, bx, cx, dx, si registers are not saved. Others are.
print:
  push es
  
  ; set es
  mov si, ds
  mov es, si
  ; set di
  mov di, bp

  xor al, al
  mov cx, 0xffff
  repne scasb
  sub di, bp
  mov ax, di
  dec ax

  ; save result for later
  mov si, ax
  
  ; get current video mode (we need only page number in bh)
  mov ah, 0x0f
  int 0x10

  ; get cursor position of a cursor in dh,dl
  mov ah, 0x03
  int 0x10

  ; length of a string
  mov cx, si
  ; black background, white text
  mov bl, 0x07
  ; print string, move cursor to end of it
  mov ax, 0x1301
  int 0x10

  pop es
  ret

read_error_str db "Failed to read sector. Halt", 0 

; #endregion

times 0x15C-($-$$) db 0
tss:
  .previous_task_link: dw 0
  ._reserved0: dw 0

  .esp0: dd KERNEL_STACK
  .ss0: dw KERNEL_DATA_SEGMENT
  ._reserved1: dw 0

  .esp1: dd 0
  .ss1: dw 0
  ._reserved2: dw 0
  
  .esp2: dd 0
  .ss2: dw 0
  ._reserved3: dw 0

  .cr3: dd 0
  .eip: dd 0
  .eflags: dd 0
  .eax: dd 0
  .ecx: dd 0
  .edx: dd 0
  .ebx: dd 0
  .esp: dd 0
  .ebp: dd 0
  .esi: dd 0
  .edi: dd 0

  .es: dw 0
  ._reserved4: dw 0
  .cs: dw 0
  ._reserved5: dw 0
  .ss: dw 0
  ._reserved6: dw 0
  .ds: dw 0
  ._reserved7: dw 0
  .fs: dw 0
  ._reserved8: dw 0
  .gs: dw 0
  ._reserved9: dw 0

  .ldt_segment_selector: dw 0
  ._reserved10: dw 0

  .t_reserved: db 1
  ._reserved11: db 0
  .io_map_base_address: dw 108 + 32 ; tss size + interrupt redirection bit map size

  .ssp: dd 0

; #region gdt

; 8 bytes alignment
times 0x1C8-($-$$) db 0
; 48 bytes
gdt:
  .null_descriptor:
    dq 0xDEADBEEFDEADFACE ; NULL descriptor

  KERNEL_CODE_SEGMENT equ 8
  .kernel_code_descriptor:
    ; limit low                         :16
    dw 0xFFFF
    ; base low                          :24
    db 0x0, 0x0, 0x0
    ; present flag                  P   :1
    ; descriptor privilege level    DPL :2
    ; descriptor type flag          S   :1
    ; executalbe flag               E   :1
    ; conforming flag               DC  :1
    ; read-enable flag              RW  :1
    ; accessed flag (CPU feedback)  A   :1
    db 0b_1_00_1_1_0_1_0

    ; granularity flag              G   :1
    ; D flag, 16-bit heresy         DB  :1
    ; L flag (for IA-32e)           L   :1
    ; AVAILABLE                         :1
    ; limit high                        :4
    db 0b_1_1_0_0_1111

    ; base high                         :8
    db 0x0   

  KERNEL_DATA_SEGMENT equ 16
  .kernel_data_descriptor:
    ; limit low                         :16
    dw 0xFFFF
    ; base low                          :24
    db 0x0, 0x0, 0x0
    ; present flag                  P   :1
    ; descriptor privilege level    DPL :2
    ; descriptor type flag          S   :1
    ; executalbe flag               E   :1
    ; direction flag, heresy        DC  :1
    ; write-enable flag             RW  :1
    ; accessed flag (CPU feedback)  A   :1
    db 0b_1_00_1_0_0_1_0

    ; granularity flag              G   :1
    ; D flag, 16-bit heresy         DB  :1
    ; L flag (for IA-32e)           L   :1
    ; AVAILABLE                         :1
    ; limit high                        :4
    db 0b_1_1_0_0_1111

    ; base high                         :8
    db 0x0  

  APP_CODE_SEGMENT equ 24 | 3
  .app_code_descriptor:
    ; limit low                         :16
    dw 0xFFFF
    ; base low                          :24
    db 0x0, 0x0, 0x0
    ; present flag                  P   :1
    ; descriptor privilege level    DPL :2
    ; descriptor type flag          S   :1
    ; executalbe flag               E   :1
    ; conforming flag               DC  :1
    ; read-enable flag              RW  :1
    ; accessed flag (CPU feedback)  A   :1
    db 0b_1_11_1_1_0_1_0

    ; granularity flag              G   :1
    ; D flag, 16-bit heresy         DB  :1
    ; L flag (for IA-32e)           L   :1
    ; AVAILABLE                         :1
    ; limit high                        :4
    db 0b_1_1_0_0_1111

    ; base high                         :8
    db 0x0  

  APP_DATA_SEGMENT equ 32 | 3
  .app_data_descriptor:
    ; limit low                         :16
    dw 0xFFFF
    ; base low                          :24
    db 0x0, 0x0, 0x0
    ; present flag                  P   :1
    ; descriptor privilege level    DPL :2
    ; descriptor type flag          S   :1
    ; executalbe flag               E   :1
    ; direction flag, heresy        DC  :1
    ; write-enable flag             RW  :1
    ; accessed flag (CPU feedback)  A   :1
    db 0b_1_11_1_0_0_1_0

    ; granularity flag              G   :1
    ; D flag, 16-bit heresy         DB  :1
    ; L flag (for IA-32e)           L   :1
    ; AVAILABLE                         :1
    ; limit high                        :4
    db 0b_1_1_0_0_1111

    ; base high                         :8
    db 0x0  

  TSS_SEGMENT equ 40
  .tss_descriptor:
    ; limit low                         :16
    dw 0x006C - 1
    ; base low                          :24
    dw tss
    db 0x0
    ; present flag                  P   :1
    ; descriptor privilege level    DPL :2
    ; descriptor type flag          S   :1
    ; type                              :4
    db 0b_1_00_0_1001

    ; granularity flag              G   :1
    ; D flag, 16-bit heresy         DB  :1
    ; L flag (for IA-32e)           L   :1
    ; AVAILABLE                         :1
    ; limit high                        :4
    db 0b_0_0_0_0_0000 ;! test DB flag set

    ; base high                         :8
    db 0x0
gdt_end:

gdt_pseudodescriptor:
  dw gdt_end - gdt - 1
  dd gdt

; #endregion

times 510-($-$$) db 0
dw 0xAA55
