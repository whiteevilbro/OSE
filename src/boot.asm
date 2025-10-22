; === IMPORTS & EXPORTS ===

extern kernel_entry
extern __kernel_size_sectors
global halt

; === BOOT DEVICE READ CONFIG ===

CYLINDERS_LIMIT equ 79
HEADS_LIMIT equ 1
SECTORS_LIMIT equ 36

; === CODE ===
; #region boot

[BITS 16]

; stack setup
main:
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
  cmp cl, SECTORS_LIMIT
  jbe .read

  mov cl, 1
  ; inc dh
  xor dh, 1
  ; cmp dh, HEADS_LIMIT
  jnz .read

  ; xor dh, 1
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

; ; get current video mode
; ; ah = number of character columns
; ; bh = active page
; mov ah, 0x0F
; int 0x10

; movzx di, ah

; ; get cursor position
; ; dh = row
; ; dl = column
; mov ah, 0x03
; int 0x10

; ; hide cursor
; mov ah, 0x01
; mov ch, 0x20
; int 0x10

; movzx si, dh

; ; push on stack according to System V i386 ABI
; sub sp, 4
; push edx
; push esi
; push edi

protected_mode_switch:
lgdt [gdt_pseudodescriptor]
cli
cld
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE:protected_mode_trampoline


[BITS 32]
protected_mode_trampoline:
mov eax, DATA
mov ds, eax
mov ss, eax
mov es, eax
mov fs, eax
mov gs, eax

; and sp, ~0xf
sub sp, 4

jmp kernel_entry
; #endregion

; never ending loop
halt:
  jmp $


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

; 8 bytes alignment
times 0x1E0-($-$$) db 0
; 24 bytes
gdt:
  .null_descriptor:
    dq 0xDEADBEEFDEADFACE ; NULL descriptor

  CODE equ 8
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

  DATA equ 16
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

times 510-($-$$) db 0
dw 0xAA55
