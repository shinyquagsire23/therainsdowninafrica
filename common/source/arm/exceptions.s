/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

.global get_esr_el1
get_esr_el1:
	mrs x0, esr_el1
	ret

.global get_afsr0_el1
get_afsr0_el1:
	mrs x0, afsr0_el1
	ret

.global get_afsr1_el1
get_afsr1_el1:
	mrs x0, afsr1_el1
	ret
