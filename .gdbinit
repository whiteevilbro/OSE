file build/os.elf
target remote localhost:1234
set disassembly-flavor intel
set arch i8086
br kernel_entry
#br init_exceptions
br universal_interrupt_handler
br exp
br collect_ctx
c
print sizeof(Context)