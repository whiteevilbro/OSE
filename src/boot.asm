[BITS 16]
[ORG 0x7C00]

%define CYLINDERS 79
%define HEADS 1
%define SECTORS_PER_CYLLINDER 18

%define NK N * 1024
%define SECTORS_TO_READ ((NK / 512) + ((NK % 512) != 0))
; %define HEADS_TO_READ (SECTORS_TO_READ / SECTORS_PER_CYLLINDER) + ((SECTORS_TO_READ % SECTORS_PER_CYLLINDER) != 0)
; %define CYLINDERS_TO_READ (HEADS_TO_READ / HEADS) + ((HEADS_TO_READ % HEADS) != 0)

; stack
cli
xor ax, ax
mov ss, ax
mov sp, 0x7C00

mov ax, 0x7E0
mov es, ax

xor bx, bx
mov ds, bx

mov al, 1

xor ch, ch 
mov cl, 2
xor dh, dh  

mov di, SECTORS_TO_READ

.read:
  ; ah - reserved
  ; al - 1
  ; ch - cylinder
  ; cl - sector
  ; dh - head
  ; dl - reserved
  ; es:bx - buffer

  mov ah, 2
  int 0x13
  jc .errr

  mov si, es
  add si, 0x20
  mov es, si

  inc cl
  cmp cl, SECTORS_PER_CYLLINDER
  jle .skip

  mov cl, 1
  inc dh
  cmp dh, HEADS
  jle .skip

  xor dh, dh
  inc ch
  cmp ch, CYLINDERS
  jg .err

.skip:
  dec di
  jnz .read

.suc:
mov bx, succ
jmp .print

.end:
jmp $

.err:
mov bx, error
jmp .print

.errr:
mov bx, errror
jmp .print

.print:
  xor cx, cx
  mov ah, 0x0E
  ; mov bx, error
  .ploop:
    mov al, [bx]
    test al, al
    jz .end
    int 0x10
    inc bx
    jmp .ploop

error db "too much", 0 
errror db "fail to read", 0 

succ db "Success",0

times 510-($-$$) db 0
dw 0xAA55
