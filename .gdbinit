file build/os.elf
target remote localhost:1234
set disassembly-flavor intel
set arch i8086
br kernel_entry
br universal_interrupt_handler
br write_hex
c