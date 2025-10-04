SECTIONS {
    . = 0x7C00;
    .start = .;

    .boot : {
        BUILD_DIR/boot.o (.text);
    }

    .kernel : {
        *(.text);
        *(.rodata);
        *(.rodata.*);
        *(.data);
        *(.bss);
    }
    .end = .;
    .sizeb = .end - .start;
    __kernel_size_sectors = (.sizeb / 512) + ((.sizeb % 512) != 0);

    /DISCARD/ : {
        *(.eh_frame)
        *(.note.gnu.property)
        *(.comment)
    }
}
