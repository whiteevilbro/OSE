target remote localhost:1234
set disassembly-flavor intel
set arch i8086
b *0x7c00
display/3i $pc
c

# set arch i80386
