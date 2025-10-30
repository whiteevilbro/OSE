; === IMPORTS & EXPORTS ===

extern kernel_entry
extern __kernel_size_sectors
extern universal_interrupt_handler
global halt
global collect_ctx
global exp

; === BOOT DEVICE READ CONFIG ===

CYLINDERS_LIMIT equ 79
HEADS_LIMIT equ 1
SECTORS_PER_TRACK_LIMIT equ 36

; === CODE ===
; #region boot

[BITS 16]

; stack setup
entry:
  xor cx, cx
  mov ds, cx
  mov ss, cx
  mov sp, 0x7C00

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

    mov bp, succes_str
    call print

  .end_read:

video_settings:
  ; turn blinking bit off
  mov ax, 0x1003
  ; bl == 0
  int 0x10

  protected_mode_switch:
  lgdt [gdt_pseudodescriptor]
  cli
  cld
  mov eax, cr0
  or eax, 1
  mov cr0, eax
  jmp CODE_SEGMENT:protected_mode_trampoline_to_c


[BITS 32]
protected_mode_trampoline_to_c:
  mov eax, DATA_SEGMENT
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
  mov eax, DATA_SEGMENT
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax

  ; stack alignment
  sub esp, 4
  and esp, ~0xf

  mov DWORD [esp], ebx
  call universal_interrupt_handler
  jmp halt

exp:
  mov eax, 6
  mov ecx, 5
  mov edx, 4
  mov ebx, 3
  mov ebp, 2
  mov esi, 1
  mov edi, 0

  int 0xff

  ; div edi

  ; sti

  call halt

memcpy:
memmove:
  

; #endregion

; #region 16-bit utilities

[BITS 16]

read_error:
mov bp, read_error_str
call print
jmp $


; es:di - null terminated string fully contained in es effective address space
; ax - return value
; di, ax registers are not saved. Others are.
strlen:
  xor al, al
  mov cx, 0xffff
  repne scasb
  sub di, bp
  mov ax, di
  dec ax
  ret


; ds:bp - pointer to null-terminated string fully contained in ds effective address space
; ax, bx, cx, dx, si registers are not saved. Others are.
print:
  push es
  
  ; set es
  mov si, ds
  mov es, si
  ; set di
  mov di, bp

  call strlen
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

read_error_str db "Failed to read sector. Halting", 0 

succes_str db "Success",0xD,0xA,0

; #endregion

; #region gdt

; 8 bytes alignment
times 0x1E0-($-$$) db 0
; 24 bytes
gdt:
  .null_descriptor:
    dq 0xDEADBEEFDEADFACE ; NULL descriptor

  CODE_SEGMENT equ 8
  .code_descriptor:
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

  DATA_SEGMENT equ 16
  .data_descriptor:
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

    ; base high                     :8
    db 0x0  

gdt_pseudodescriptor:
  dw 0x18 - 1
  dd gdt

; #endregion

times 510-($-$$) db 0
dw 0xAA55
