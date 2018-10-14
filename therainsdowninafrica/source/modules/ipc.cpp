/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "ipc.h"

#include "log.h"
#include "hos/hipc.h"
#include "hos/svc.h"
#include "hos/kobjects.h"
#include "core/svc_bind.h"
#include "arm/cache.h"
#include "arm/tls.h"

#include "regmap.h"

//#define EXTRA_DEBUG

#define INITTED_MAGIC 0xF00FB00BB00BF00F

struct DomainSessionPair
{
    KClientSession* session;
    u32 domain;
    u32 pad;
};

struct HandlerPair
{
    void* handler;
    void* extra;

    HandlerPair() : handler(nullptr), extra(nullptr) {}
    HandlerPair(void* handler) : handler(handler), extra(nullptr) {}
    HandlerPair(void* handler, void* extra) : handler(handler), extra(extra) {}
};

u64 replacement_vtable[0x20];
int (*old_destruct)(KClientSession* client);

RegMap<u64, void*, 0x200> registered_handlers_by_name;

RegMap<DomainSessionPair, HandlerPair, 0x1000> clientdomainpair_to_handler;
RegMap<DomainSessionPair, HandlerPair, 0x1000> clientdomainpair_to_closehandler;

void ipc_register_handler_for_named_port(char* portnamestr, void* handler)
{
    u64 portname = 0;
    for (int i = 0; i < 8; i++)
    {
        portname |= ((u64)portnamestr[i] << (i * 8));
        if (!portnamestr[i]) break;
    }

    ipc_register_handler_for_portnameval(portname, handler);
}

void ipc_register_handler_for_portnameval(u64 portname, void* handler)
{
    registered_handlers_by_name.set(portname, handler);

#ifdef EXTRA_DEBUG
    log_printf("register name %016llx (%.8s) to handler %p\r\n", portname, (char*)&portname, handler);
#endif
}

int destruct_intercept(KClientSession* client)
{
#ifdef EXTRA_DEBUG
    log_printf("client %p fucking died\r\n", client);
#endif

    old_destruct(client);

    if (!client->refcnt)
    {
        void (*handler)(void* extra) = (void(*)(void* extra))client->parentSession->closeHandler;
        if (handler)
            handler(client->parentSession->closeExtra);

        client->parentSession->magicTouch = 0;
        client->parentSession->sendHandler = nullptr;
        client->parentSession->sendExtra = nullptr;
        client->parentSession->closeHandler = nullptr;
        client->parentSession->closeExtra = nullptr;
    }
}

void kclientsession_swap_vtable(KClientSession* client)
{
    // Swap vtable
    memcpy(replacement_vtable, client->vtable, sizeof(replacement_vtable));
    old_destruct = (int (*)(KClientSession* client))replacement_vtable[2];

    replacement_vtable[2] = (u64)destruct_intercept;
    client->vtable = (void**)replacement_vtable;
}

void ipc_bind_client_to_handler(KClientSession* client, void* handler, void* extra)
{
    if (client->parentSession->magicTouch != INITTED_MAGIC)
    {
        client->parentSession->magicTouch = INITTED_MAGIC;
        client->parentSession->closeHandler = nullptr;
        client->parentSession->closeExtra = nullptr;
    }
    client->parentSession->sendHandler = handler;
    client->parentSession->sendExtra = extra;

    kclientsession_swap_vtable(client);

#ifdef EXTRA_DEBUG
    log_printf("bind client %p to handler %p\r\n", client, handler);
#endif
}

void ipc_bind_client_to_closehandler(KClientSession* client, void* handler, void* extra)
{
    if (client->parentSession->magicTouch != INITTED_MAGIC)
    {
        client->parentSession->magicTouch = INITTED_MAGIC;
        client->parentSession->sendHandler = nullptr;
        client->parentSession->sendExtra = nullptr;
    }
    client->parentSession->closeHandler = handler;
    client->parentSession->closeExtra = extra;

    kclientsession_swap_vtable(client);

#ifdef EXTRA_DEBUG
    log_printf("bind client %p to close handler %p\r\n", client, handler);
#endif
}

void ipc_bind_domainsessionpair_to_handler(KClientSession* client, u32 domain, void* handler, void* extra)
{
    DomainSessionPair p = {client, domain, 0};
    clientdomainpair_to_handler.set(p, HandlerPair(handler, extra));

#ifdef EXTRA_DEBUG
    log_printf("bind pair %p-%x to handler %p,%p\r\n", client, domain, handler);
#endif
}

void ipc_bind_domainsessionpair_to_closehandler(KClientSession* client, u32 domain, void* handler, void* extra)
{
    DomainSessionPair p = {client, domain, 0};
    clientdomainpair_to_closehandler.set(p, HandlerPair(handler, extra));

#ifdef EXTRA_DEBUG
    log_printf("bind pair %p-%x to close handler %p\r\n", client, domain, handler);
#endif
}

void ipc_bind_client_by_name(KClientSession* clientsession, u64 kportname)
{
    void* client_handler = registered_handlers_by_name.get(kportname);

    if (client_handler != nullptr)
        ipc_bind_client_to_handler(clientsession, client_handler);
}

int connectToNamedPort_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    u64 kportname = *(u64*)regs_in[1];

    // Get SVC output
    svcRunFunc(regs_in, regs_out, handler_ptr);

    if (regs_out[0]) return 1;

    KClientSession* clientsession = kproc->getKObjectFromHandle<KClientSession>(regs_out[1]);

    ipc_bind_client_by_name(clientsession, kportname);

#ifdef EXTRA_DEBUG
    log_printf("%s connected to KSession %.8s\r\n", kproc->name, (char*)&kportname);
#endif

    return 1;
}

int sendSyncRequest_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);
    HIPCPacket* packet = (HIPCPacket*)getTls();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();
    int cmd = basic->cmd;
    int type = packet->type;
    bool is_domain = packet->is_domain_message();

    bool handler_handled = false;
    HandlerPair handler_pair;
    if (client->parentSession->magicTouch == INITTED_MAGIC)
    {
        handler_pair = HandlerPair(client->parentSession->sendHandler, client->parentSession->sendExtra);
    }

    if (type == HIPCPacketType_Request || type == HIPCPacketType_RequestWithContext || type == HIPCPacketType_LegacyRequest)
    {
        if (is_domain)
        {
            DomainSessionPair p = {client, packet->get_domain_header()->objectId, 0};
            if (packet->get_domain_header()->cmd == HIPCDomainCommand_CloseVirtualHandle)
            {
                handler_pair = clientdomainpair_to_closehandler.get(p);
                clientdomainpair_to_handler.clear(p);
                clientdomainpair_to_closehandler.clear(p);

                if (handler_pair.handler)
                {
                    void (*handler)(void* extra) = (void(*)(void* extra))handler_pair.handler;
                    handler(handler_pair.extra);

                    handler_pair.handler = nullptr;
                }
            }
            else
                handler_pair = clientdomainpair_to_handler.get(p);
        }

        if (handler_pair.handler)
        {
            int (*handler)(u64 *regs_in, u64 *regs_out, void* handler_ptr, void* extra) = (int(*)(u64 *regs_in, u64 *regs_out, void* handler_ptr, void* extra))handler_pair.handler;
            handler_handled = handler(regs_in, regs_out, handler_ptr, handler_pair.extra);
        }
    }
    else if (type == HIPCPacketType_Control || type == HIPCPacketType_ControlWithContext)
    {
        u32 arg = basic->extra[0];

        svcRunFunc(regs_in, regs_out, handler_ptr);
        handler_handled = true;

        basic = packet->get_data<HIPCBasicPacket>();
        if (cmd == 0)
        {
            if (handler_pair.handler)
            {
#ifdef EXTRA_DEBUG
                log_printf("map client-domain %p-%x to handler %p\r\n", client, basic->extra[0], handler_pair.handler);
#endif

                ipc_bind_domainsessionpair_to_handler(client, basic->extra[0], handler_pair.handler, handler_pair.extra);
            }
        }
        else if (cmd == 1)
        {
            KClientSession* client_new = kproc->getKObjectFromHandle<KClientSession>(packet->get_handle(0));

#ifdef EXTRA_DEBUG
                log_printf("moved domain object (%x) to handle (%x), %s, %p %p\r\n", arg, packet->get_handle(0), kproc->name, client, client_new);
#endif

            DomainSessionPair p = {client, arg, 0};

            //TODO: close handlers for normal handles
            handler_pair = clientdomainpair_to_handler.get(p);

            if (handler_pair.handler != nullptr)
                ipc_bind_client_to_handler(client_new, handler_pair.handler, handler_pair.extra);
        }
        else if (cmd == 2)
        {
            KClientSession* client_new = kproc->getKObjectFromHandle<KClientSession>(packet->get_handle(0));

#ifdef EXTRA_DEBUG
                log_printf("cloned current object to handle (%x), %s, %p %p\r\n", packet->get_handle(0), kproc->name, client, client_new);
#endif

            if (handler_pair.handler != nullptr)
                ipc_bind_client_to_handler(client_new, handler_pair.handler, handler_pair.extra);
        }
        else if (cmd == 4)
        {
            KClientSession* client_new = kproc->getKObjectFromHandle<KClientSession>(packet->get_handle(0));

#ifdef EXTRA_DEBUG
            log_printf("cloned(ex %x) current object to handle (%x), %s, %p %p\r\n", arg, packet->get_handle(0), kproc->name, client, client_new);
#endif

            if (handler_pair.handler != nullptr)
                ipc_bind_client_to_handler(client_new, handler_pair.handler, handler_pair.extra);
        }
        else if (cmd != 3)
        {
#ifdef EXTRA_DEBUG
            log_printf("cmd %x\r\n", cmd);
#endif
        }
    }

    return handler_handled;
}

int replyAndRecieve_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    HIPCPacket* header = (HIPCPacket*)getTls();

    /*if (header->numStatic | header->numSend | header->numRecv | header->numExch)
    {
        log_printf("replyandrecieve %s->\r\n", kproc->name);
        header->debug_print();
    }*/

    return 0;
}

void ipc_module_init()
{
    svc_pre_bind(idConnectToNamedPort, (void*)connectToNamedPort_handler);
    svc_pre_bind(idSendSyncRequest, (void*)sendSyncRequest_handler);
    svc_pre_bind(idSendSyncRequest, (void*)replyAndRecieve_handler);

    memset(&registered_handlers_by_name, 0, sizeof(registered_handlers_by_name));
    memset(&clientdomainpair_to_handler, 0, sizeof(clientdomainpair_to_handler));
    memset(&clientdomainpair_to_closehandler, 0, sizeof(clientdomainpair_to_closehandler));

    log_printf("ipc module initialized\r\n");
}
