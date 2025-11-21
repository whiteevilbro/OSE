file build/os.elf
target remote localhost:1234
set disassembly-flavor intel
set arch i8086
br *0x7C00
br kernel_entry
br malloc_TITLE_CARD
br halt
c
c