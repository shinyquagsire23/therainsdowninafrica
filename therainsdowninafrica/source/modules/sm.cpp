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

u32 loader_fspsrv_hand = 0;
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

KClientSession* sm_get_fspsrv()
{
    return loader_fspsrv_sess;
}

int sm_ipc_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr)
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

                loader_fspsrv_hand = p->get_handle(0);
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

int test_ipc_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    log_printf("test handler\r\n");

    //bind nameless handles w/ ipc_bind_client_to_handler
    return 0;
}

const char* hbl_path = "/hbl";
RegMap<u64, u64, 0x40> file_mappings;

int fsp_ldr_ifile_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

    u32 file_hand_to_use = file_mappings.get(packet->get_domain_header()->objectId) >> 32;
    u32 file_domain_to_use = file_mappings.get(packet->get_domain_header()->objectId) & 0xFFFFFFFF;

#ifdef SM_EXTRA_DEBUG
    log_printf("CodeFile cmd %x from %s\r\n", basic->cmd, kproc->name);
#endif

    // Just swap in the handle and domain object ID
    regs_in[0] = file_hand_to_use;
    packet->get_domain_header()->objectId = file_domain_to_use;

    svcRunFunc(regs_in, regs_out, handler_ptr);

#ifdef SM_EXTRA_DEBUG
    log_printf("returned %x, err %x\r\n", regs_out[0], basic->ret);
#endif

    return 1;
}

int fsp_ldr_ifile_closehook(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

    u32 file_hand_to_use = file_mappings.get(packet->get_domain_header()->objectId) >> 32;
    u32 file_domain_to_use = file_mappings.get(packet->get_domain_header()->objectId) & 0xFFFFFFFF;

    // Delete our objects with theirs
    ksvcCloseHandle(file_hand_to_use);
    file_mappings.clear(packet->get_domain_header()->objectId);

    return 0;
}

int fsp_ldr_ifilesystem_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

#ifdef SM_EXTRA_DEBUG
    log_printf("CodeFileSystem cmd %x from %s\r\n", basic->cmd, kproc->name);
#endif

    if (basic->cmd == 8)
    {
        FspSrv fsp = FspSrv(loader_fspsrv_hand);
        IFileSystem sdcard;
        IFile logfile;
        u32 ret, file_ret;

        char* file_path;
        char* file_path_raw = (char*)packet->get_static_descs()[0].get_addr();
        u32 domain_id = packet->get_domain_header()->objectId;
#ifdef SM_EXTRA_DEBUG
        log_printf("CodeFileSystem OpenFile(%s)\r\n", file_path_raw);
#endif

        // Get SD card handle, if we can't get a handle then let it be
        ret = fsp.openSdCardFileSystem(&sdcard);
        if (ret) return 0;

        // Pick a file which always exists, so extra files can be added
        // and ones which don't exist can error out
        file_path = (char*)malloc(0x301);
        strcpy(file_path, hbl_path);
        strcat(file_path, file_path_raw);
        strcpy(file_path_raw, "/main");

        svcRunFunc(regs_in, regs_out, handler_ptr);

        packet = get_current_packet();
        basic = packet->get_data<HIPCBasicPacket>();
#ifdef SM_EXTRA_DEBUG
        log_printf("returned %x, err %x, hand %x\r\n", regs_out[0], basic->ret, basic->extra[0]);
#endif

        // Get hbl/<file> handle
        file_ret = sdcard.openFile(file_path, IFILE_READABLE, &logfile);

#ifdef SM_EXTRA_DEBUG
        log_printf("OpenFile(%s) got return %x, handle %x\r\n", file_path, file_ret, logfile.h);
#endif

        // Map output returns to SD returns
        basic = packet->get_data<HIPCBasicPacket>();
        basic->ret = file_ret;

        if (!file_ret)
        {
            u32 file_domain = logfile.toDomainId();
            file_mappings.set(basic->extra[0], ((u64)logfile.h << 32) | file_domain);
            ipc_bind_domainsessionpair_to_handler(client, basic->extra[0], (void*)fsp_ldr_ifile_hook);
            ipc_bind_domainsessionpair_to_closehandler(client, basic->extra[0], (void*)fsp_ldr_ifile_closehook);
        }
        else
        {
            DomainPair pair(regs_in[0], domain_id);
            pair.closeDomain();
        }

        sdcard.close();
        free(file_path);

        return 1;
    }

    return 0;
}

int fsp_ldr_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

    if (basic->cmd == 0) // OpenCodeFileSystem
    {
        u64 tid = packet->get_data<u64>()[2];
#ifdef SM_EXTRA_DEBUG
        log_printf("OpenCodeFileSystem(%016llx, %s) from %s, %x\r\n", tid, packet->get_static_descs()[0].get_addr(), kproc->name, packet->get_domain_header()->objectId);
#endif

        svcRunFunc(regs_in, regs_out, handler_ptr);

        packet = get_current_packet();
        basic = packet->get_data<HIPCBasicPacket>();

        if (!basic->ret && tid == 0x010000000000100d) // album applet
        {
            ipc_bind_domainsessionpair_to_handler(client, basic->extra[0], (void*)fsp_ldr_ifilesystem_hook);
        }

        return 1;
    }

    return 0;
}

void sm_module_init()
{
    ipc_register_handler_for_named_port("sm:", (void*)sm_ipc_handler);
    ipc_register_handler_for_named_port("ipc_test", (void*)test_ipc_handler);

    ipc_register_handler_for_named_port("fsp-ldr", (void*)fsp_ldr_hook);

    log_printf("sm module initialized\r\n");
}
