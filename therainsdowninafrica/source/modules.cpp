/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "modules.h"

#include "modules/sm.h"
#include "modules/fsp.h"
#include "modules/ipc.h"

void modules_init(void)
{
    ipc_module_init();
    sm_module_init();
    fsp_module_init();
}
