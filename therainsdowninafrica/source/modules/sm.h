/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef MODULES_SM_H
#define MODULES_SM_H

#include "hos/kobjects.h"

void sm_module_init();
bool sm_is_sdcard_usable();
KClientSession* sm_get_fspsrv();

#endif // MODULES_SM_H
