file build/os.elf
target remote localhost:1234
set disassembly-flavor intel
set arch i8086
br kernel_entry
br kernel_panic
br user_process
br pagefault_handler
c