/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef MODULES_FSP_H
#define MODULES_FSP_H

#include "hos/kobjects.h"

void fsp_module_init();
void fsp_redirect_ifilesystem(KClientSession* client, const char* path);
void fsp_redirect_domain_ifilesystem(KClientSession* client, u32 domain, const char* path);

#endif // MODULES_FSP_H
