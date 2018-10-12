/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

.section ".crt0","ax"

.global _start
_start:
	b stub_start
	.ascii "SALT"
	.word (__idk__ - _start)
	.word (__text_end - _start)
	.word (__data_end - _start)

.global africaPAddr
africaPAddr:
	.word 0

.global africaSize
africaSize:
	.word 0

stub_start:
    sub sp, sp, #0x48
    str lr, [sp]
    
    # stash svc table for usage
    adr x0, a64_svc_tbl
    str x10, [x0]
    
    # svcno, args, func
    mov x0, x8
    add x1, sp, #0x48
    add x2, sp, #0x8
    mov x3, x11
    
    bl main
    
    add x9, sp, #0x8
    
    ldp x0, x1, [x9, #0x0]
	ldp x2, x3, [x9, #0x10]
	ldp x4, x5, [x9, #0x20]
	ldp x6, x7, [x9, #0x30]
    
    ldr lr, [sp]
    add sp, sp, #0x48
    ret
    
    .pool
    
.section ".data"
.global a64_svc_tbl
a64_svc_tbl: .dword 0x0
