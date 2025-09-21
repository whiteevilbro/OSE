target remote localhost:1234
set disassembly-flavor intel
set arch i386
b *0x7c00
c

