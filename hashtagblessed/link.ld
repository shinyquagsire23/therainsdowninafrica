OUTPUT_ARCH(aarch64)
ENTRY(_start)

SECTIONS
{
    . = 0x90080000;
    .init : { *(.init) }
    . = ALIGN(8);
    .text : { *(.text) }
    . = ALIGN(8);
    .rodata : { *(.rodata) }
    . = ALIGN(8);
    .data : { *(.data) }
    . = ALIGN(8);
    .bss : { *(.bss) }
    . = ALIGN(8);
    
    . = 0x900006CC;
    .hook : { *(.hook) }
}
