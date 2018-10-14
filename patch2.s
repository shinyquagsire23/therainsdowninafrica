.arch armv8-a

mov w0, #(((900 * 0x140)-1) & 0xFFFF)
movk w0, #((((900 * 0x140)-1) & 0xFFFF0000) >> 16),LSL#16
mov w22, #0x140
sub x22, x22, #0x140

.pool
