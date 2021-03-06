/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

.section ".text"
    
.global smc1
smc1:
    smc #1
    ret
    
.global svc_intr_hook
svc_intr_hook:
    mov x9, #0xffffffff00000000
    blr x9

.global except_hook
except_hook:
    mov x1, #0xffffffff00000000
    add x1, x1, #0x4
    blr x1

    .pool
