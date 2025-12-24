file build/os.elf
add-symbol-file userspace/user/user1.elf
target remote localhost:1234
set disassembly-flavor intel
set arch i8086
br kernel_entry
br kernel_panic
br pagefault_handler
br switch_process
br startup
c