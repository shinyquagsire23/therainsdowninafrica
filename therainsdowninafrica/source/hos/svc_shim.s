/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

.section ".text"

.global svcRunFunc
svcRunFunc:
	sub sp, sp, #0x40
	str lr, [sp, #0x0]
	stp x19, x20, [sp, #0x10]
	
	mov x9, x0
	mov x10, x1
	mov x11, x2
	stp x9, x10, [sp, #0x20]
	stp x11, x12, [sp, #0x30]
	ldp x0, x1, [x9, #0x0]
	ldp x2, x3, [x9, #0x10]
	ldp x4, x5, [x9, #0x20]
	ldp x6, x7, [x9, #0x30]
	blr x11
	
	ldp x11, x12, [sp, #0x30]
	ldp x9, x10, [sp, #0x20]
	
	stp x0, x1, [x10, #0x0]
	stp x2, x3, [x10, #0x10]
	stp x4, x5, [x10, #0x20]
	stp x6, x7, [x10, #0x30]
	
	ldp x19, x20, [sp, #0x10]
	ldr lr, [sp, #0x0]
	add sp, sp, #0x40
	ret
