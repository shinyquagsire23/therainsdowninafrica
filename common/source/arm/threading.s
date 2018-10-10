/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

.section ".text"

.global get_core
get_core:
    mrs x0, mpidr_el1
    and x0, x0, #0xf
    ret

.global mutex_lock
mutex_lock:
    mov     w1, #0x1
loop:
    LDxr   w2, [x0]
    CMP     w2, w1
	BEQ     goto_loop
    STxr w2, x1, [x0]
    CMP   w2, #1
    BEQ     loop
    DMB sy
    ret
goto_loop:
    B       loop

.global mutex_unlock
mutex_unlock:
    mov w1, #0x0
    DMB sy
    STR     w1, [x0]
    ret

.pool
