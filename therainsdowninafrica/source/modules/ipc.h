/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef MODULES_IPC_H
#define MODULES_IPC_H

#include "types.h"
#include "hos/kobjects.h"

void ipc_register_handler_for_named_port(char* portnamestr, void* handler);
void ipc_register_handler_for_portnameval(u64 portname, void* handler);
void ipc_bind_client_to_handler(KClientSession* client, void* handler, void* extra = nullptr);
void ipc_bind_client_to_closehandler(KClientSession* client, void* handler, void* extra = nullptr);
void ipc_bind_domainsessionpair_to_handler(KClientSession* client, u32 domain, void* handler, void* extra = nullptr);
void ipc_bind_domainsessionpair_to_closehandler(KClientSession* client, u32 domain, void* handler, void* extra = nullptr);
void ipc_bind_client_by_name(KClientSession* clientsession, u64 kportname);

void ipc_module_init();

#endif // MODULES_IPC_H
