[BITS 16]
[ORG 0x7C00]

CYLINDERS_LIMIT equ 79
HEADS_LIMIT equ 1
SECTORS_LIMIT equ 18

%ifndef N
  ; N - kB to read, normally passed at compilation
  N equ 0
%endif
BYTES_TO_READ equ N * 1024
SECTORS_TO_READ equ ((BYTES_TO_READ / 512) + ((BYTES_TO_READ % 512) != 0))

TRACKS_TO_READ equ (SECTORS_TO_READ / SECTORS_LIMIT) + ((SECTORS_TO_READ % SECTORS_LIMIT) != 0)
CYLINDERS_TO_READ equ (TRACKS_TO_READ / HEADS_LIMIT) + ((TRACKS_TO_READ / HEADS_LIMIT) != 0)
%if (CYLINDERS_TO_READ > CYLINDERS_LIMIT) || (N > 481)
  %error "Too big of a payload."
%endif

; stack
xor cx, cx
mov ds, cx
mov ss, cx
mov sp, 0x7C00

mov ax, 0x7E0
mov es, ax

mov al, 1
; cx == 0
mov cl, 2
xor dh, dh  

mov di, SECTORS_TO_READ + 1


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
  jc .read_error

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

.end:
jmp $

.read_error:
mov bp, read_error_str
call print
jmp .end_read


; ds:bp - null terminated string fully contained in es effective address space
; ax - return value
; di, ax registers are not saved. Others are.
strlen:
  mov di, bp
  .strlen_loop:
    mov al, ds:[di]
    test al, al
    jz .strlen_loop_end
    inc di
    jmp .strlen_loop
  .strlen_loop_end:
  sub di, bp
  mov ax, di
  ret


; ds:bp - pointer to null-terminated string fully contained in ds effective address space
; ax, bx, cx, dx, si registers are not saved. Others are.
print:
  push es

  call strlen
  ; save result for later
  mov si, ax
  
  ; get current video mode (we need only page number in bh)
  mov ah, 0x0f
  int 0x10

  ; get cursor position of a cursor in dh,dl
  mov ah, 0x03
  int 0x10

  ; set es
  mov si, ds
  mov es, si
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

succes_str db "Success",0

times 510-($-$$) db 0
dw 0xAA55
