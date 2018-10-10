/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

.section ".text"

.global smcDebugPrint
smcDebugPrint:
	mov x1, x0
	ldr x0, =0xf00ff00f
	smc #1
	ret

.pool
