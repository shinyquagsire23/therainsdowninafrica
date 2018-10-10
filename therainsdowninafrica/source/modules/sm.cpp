/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "sm.h"

#include "io/uart.h"
#include "hos/svc.h"
#include "hos/kobjects.h"
#include "hos/hipc.h"
#include "core/svc_bind.h"
#include "arm/threading.h"

#include "modules/ipc.h"
#include "regmap.h"

#include <map>
#include <cstring>

//#define SM_EXTRA_DEBUG

u32 loader_fspsrv_hand = 0;

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
            //uart_debug_printf("%s requesting service %.8s, got %x\r\n", kproc->name, &service_requested, packet->get_handle(0));
#endif
            // TODO this is kinda terrible, hooks would be good for this
            if (!strcmp(kproc->name, "Loader") && !strcmp((char*)&service_requested, "fsp-ldr"))
            {
                HIPCCraftedPacket* p = new HIPCCraftedPacket();

                // Get fsp-srv handle
                u64 service;
                strcpy((char*)&service, "fsp-srv");
                p->clear();
                p->ipc_cmd(1)->push_arg<u64>(service)->send_to(regs_in[0]);

                loader_fspsrv_hand = p->get_handle(0);
                p->free_data();

                // initialize it w/ pid
                p->clear();
                p->ipc_cmd(1)->push_arg<u64>(0)->send_pid()->send_to(loader_fspsrv_hand);

                basic = p->get_data<HIPCBasicPacket>();
                p->free_data();

                delete p;
            }
            u32 handle = packet->get_handle(0);
            KClientSession* smclient = kproc->getKObjectFromHandle<KClientSession>(handle);

            ipc_bind_client_by_name(smclient, service_requested);
        }

        return 1;
    }

    return 0;
}

int test_ipc_handler(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    uart_debug_printf("test handler\r\n");

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
    uart_debug_printf("CodeFile cmd %x from %s\r\n", basic->cmd, kproc->name);
#endif

    // Just swap in the handle and domain object ID
    regs_in[0] = file_hand_to_use;
    packet->get_domain_header()->objectId = file_domain_to_use;

    svcRunFunc(regs_in, regs_out, handler_ptr);

#ifdef SM_EXTRA_DEBUG
    uart_debug_printf("returned %x, err %x\r\n", regs_out[0], basic->ret);
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
    uart_debug_printf("CodeFileSystem cmd %x from %s\r\n", basic->cmd, kproc->name);
#endif

    if (basic->cmd == 8)
    {
        char* file_path = (char*)malloc(0x301);
        char* file_path_raw = (char*)packet->get_static_descs()[0].get_addr();
        u32 domain_id = packet->get_domain_header()->objectId;
#ifdef SM_EXTRA_DEBUG
        uart_debug_printf("CodeFileSystem OpenFile(%s)\r\n", file_path_raw);
#endif

        // Pick a file which always exists, so extra files can be added
        // and ones which don't exist can error out
        strcpy(file_path, file_path_raw);
        strcpy(file_path_raw, "/main");

        svcRunFunc(regs_in, regs_out, handler_ptr);

        packet = get_current_packet();
        basic = packet->get_data<HIPCBasicPacket>();
#ifdef SM_EXTRA_DEBUG
        uart_debug_printf("returned %x, err %x, hand %x\r\n", regs_out[0], basic->ret, basic->extra[0]);
#endif

        HIPCCraftedPacket* p = new HIPCCraftedPacket();

        // Get SD card handle
        p->clear();
        p->ipc_cmd(18)->push_arg<u64>(0)->send_to(loader_fspsrv_hand);

        u32 sdcard_hand = p->get_handle(0);
#ifdef SM_EXTRA_DEBUG
        uart_debug_printf("sdcard got return %x, err %x handle %x\r\n", p->ret, p->ipcRet, sdcard_hand);
#endif
        p->free_data();

        // Get hbl/<file> handle
        u32 file_hand, file_ret;
        p->clear();

        char* path_temp = (char*)packet + 0x80;
        strcpy(path_temp, hbl_path);
        strcat(path_temp, file_path);

        p->ipc_cmd(8)->push_arg<u32>(1)->push_static_buffer(path_temp, 0x301)->send_to(sdcard_hand);
        file_ret = p->ret ? p->ret : p->ipcRet;
        file_hand = p->get_handle(0);
#ifdef SM_EXTRA_DEBUG
        uart_debug_printf("OpenFile(%s) got return %x, err %x handle %x\r\n", path_temp, p->ret, p->ipcRet, file_hand);
#endif
        p->free_data();

        // Convert to domain
        u32 file_domain;
        if (!file_ret)
        {
            p->clear();

            p->type(HIPCPacketType_Control)->ipc_cmd(0)->send_to(file_hand);
            u32* retdata = p->get_data<u32>();

            file_domain = retdata[0];
#ifdef SM_EXTRA_DEBUG
            uart_debug_printf("OpenFile->domain got return %x, err %x handle %x\r\n", p->ret, p->ipcRet, file_domain);
#endif
            p->free_data();
        }

        ksvcCloseHandle(sdcard_hand);
        free(file_path);

        // Map output returns to SD returns
        basic = packet->get_data<HIPCBasicPacket>();
        basic->ret = file_ret;

        if (!file_ret)
        {
            file_mappings.set(basic->extra[0], ((u64)file_hand << 32) | file_domain);
            ipc_bind_domainsessionpair_to_handler(client, basic->extra[0], (void*)fsp_ldr_ifile_hook);
            ipc_bind_domainsessionpair_to_closehandler(client, basic->extra[0], (void*)fsp_ldr_ifile_closehook);
        }
        else
        {
            // Clean up straggling handles if we're returning errors
            p->clear();
            p->domain_cmd(HIPCDomainCommand_CloseVirtualHandle)->send_to_domain(regs_in[0], domain_id);
            p->free_data();
        }

        delete p;

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
        uart_debug_printf("OpenCodeFileSystem(%016llx, %s) from %s, %x\r\n", tid, packet->get_static_descs()[0].get_addr(), kproc->name, packet->get_domain_header()->objectId);
#endif

        memset(&packet->get_data<u64>()[3], 0, 0x40);

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

    uart_debug_printf("sm module initialized\r\n");
}
