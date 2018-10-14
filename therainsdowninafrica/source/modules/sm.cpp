/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "sm.h"

#include "log.h"
#include "hos/svc.h"
#include "hos/kobjects.h"
#include "hos/hipc.h"
#include "hos/kfuncs.h"
#include "hos/fs.h"
#include "core/svc_bind.h"
#include "arm/threading.h"

#include "modules/ipc.h"
#include "regmap.h"

#include <map>
#include <cstring>

//#define SM_EXTRA_DEBUG

#define NUM_SD_SERVICES 4
char* sdcard_services[NUM_SD_SERVICES] = {"pcv", "gpio", "pinmux", "psc:c"};
bool sdcard_services_initted[NUM_SD_SERVICES] = {false, false, false, false};
bool sdcard_usable = false;

KClientSession* loader_fspsrv_sess = nullptr;

u64 sm_service_to_u64(char* service)
{
    char tmp[9] = {0,0,0,0,0,0,0,0,0};
    strncpy(tmp, service, 8);
    
    return *(u64*)tmp;
}

bool sm_is_sdcard_usable()
{
    return sdcard_usable;
}

FspSrv sm_get_fspsrv()
{
    FspSrv ret;

    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    kproc_add_handle(&kproc->handleTable, (u32*)&ret, loader_fspsrv_sess, KClientSessionTypeId);

    return ret;
}

int sm_ipc_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr, void* extra)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

    if (basic->cmd == 1) // GetService
    {
        u64 service_requested = packet->get_data<u64>()[2];
        svcRunFunc(regs_in, regs_out, handler_ptr);

        basic = packet->get_data<HIPCBasicPacket>();
        if (!basic->ret)
        {
#ifdef SM_EXTRA_DEBUG
            //log_printf("%s requesting service %.8s, got %x\r\n", kproc->name, &service_requested, packet->get_handle(0));
#endif
            // TODO this is kinda terrible, hooks would be good for this
            if (!strcmp(kproc->name, "Loader") && !strcmp((char*)&service_requested, "fsp-ldr"))
            {
                HIPCCraftedPacket* p = new HIPCCraftedPacket();

                // Get fsp-srv handle
                p->clear();
                p->ipc_cmd(1)->push_arg<u64>(sm_service_to_u64("fsp-srv"))->send_to(regs_in[0]);

                u32 loader_fspsrv_hand = p->get_handle(0);
                p->free_data();

                // initialize it w/ pid
                p->clear();
                p->ipc_cmd(1)->push_arg<u64>(0)->send_pid()->send_to(loader_fspsrv_hand);

                basic = p->get_data<HIPCBasicPacket>();
                p->free_data();

                delete p;
                
                loader_fspsrv_sess = kproc->getKObjectFromHandle<KClientSession>(loader_fspsrv_hand);
            }
            u32 handle = packet->get_handle(0);
            KClientSession* smclient = kproc->getKObjectFromHandle<KClientSession>(handle);

            ipc_bind_client_by_name(smclient, service_requested);
        }

        return 1;
    }
    else if (basic->cmd == 2) // RegisterService
    {
        u64 service_requested = packet->get_data<u64>()[2];
        
        bool sdcard_gucci = true;
        for (int i = 0; i < NUM_SD_SERVICES; i++)
        {
            sdcard_gucci &= sdcard_services_initted[i];
            if (sm_service_to_u64(sdcard_services[i]) == service_requested)
                sdcard_services_initted[i] = true;
        }
        
        sdcard_usable = sdcard_gucci;
    }

    return 0;
}

int test_ipc_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr, void* extra)
{
    log_printf("test handler\r\n");

    //bind nameless handles w/ ipc_bind_client_to_handler
    return 0;
}

void sm_module_init()
{
    ipc_register_handler_for_named_port("sm:", (void*)sm_ipc_handler);
    ipc_register_handler_for_named_port("ipc_test", (void*)test_ipc_handler);

    log_printf("sm module initialized\r\n");
}
