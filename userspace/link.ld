SECTIONS {
    . = 0x800000;

    .std : {
        std/std_utils.o (.text);
    }

    .app : {
        *(.text);
        *(.rodata);
        *(.rodata.*);
        *(.data);
        *(.bss);
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.note.gnu.property)
        *(.comment)
    }
}
