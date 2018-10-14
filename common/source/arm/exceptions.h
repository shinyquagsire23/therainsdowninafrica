/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef ARM_EXCEPTIONS_H
#define ARM_EXCEPTIONS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

u32 get_esr_el1();
u32 get_afsr0_el1();
u32 get_afsr1_el1(); 

#ifdef __cplusplus
}
#endif

#endif // ARM_EXCEPTIONS_H
