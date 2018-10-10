/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

.section ".init"

.global _start
_start:
stub_start:
    # replaced
    msr ttbr0_el1, x24

    adr x0, _start
    mov sp, x0

    sub sp, sp, #0x10
    str lr, [sp]

    mov x0, x20
    bl main
    
    ldr lr, [sp]
    add sp, sp, #0x10

    b _hook_ret 
    
    .pool
    
.section ".hook"
.global _hook
_hook:
    nop
.global _hook_ret
_hook_ret:
