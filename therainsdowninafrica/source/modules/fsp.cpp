/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "fsp.h"

#include "sm.h"
#include "ipc.h"
#include "log.h"
#include "hos/hipc.h"
#include "hos/kobjects.h"
#include "hos/kfuncs.h"
#include "hos/svc.h"
#include "hos/fs.h"

#define FSP_EXTRA_DEBUG

const char* hbl_path = "/hbl";

int fsp_redir_ifile_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr, DomainPair* pair)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

#ifdef FSP_EXTRA_DEBUG
    log_printf("IFile cmd %x from %s\r\n", basic->cmd, kproc->name);
#endif

    // Just swap in the handle and domain object ID
    regs_in[0] = pair->h;
    packet->get_domain_header()->objectId = pair->d;

    svcRunFunc(regs_in, regs_out, handler_ptr);

#ifdef FSP_EXTRA_DEBUG
    log_printf("returned %x, err %x\r\n", regs_out[0], basic->ret);
#endif

    return 1;
}

int fsp_redir_ifile_closehook(DomainPair* pair)
{
#ifdef FSP_EXTRA_DEBUG
    log_printf("IFile closed\r\n");
#endif

    // Delete our objects with theirs
    pair->close();
    delete pair;
    
    return 0;
}

int fsp_redir_ifilesystem_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr, char* sd_redir_path)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

#ifdef FSP_EXTRA_DEBUG
    log_printf("IFileSystem cmd %x from %s\r\n", basic->cmd, kproc->name);
#endif

    // CreateFile, DeleteFile, CreateDirectory, DeleteDirectory
    if (basic->cmd == 0 || basic->cmd == 1 || basic->cmd == 2 
        || basic->cmd == 3 || basic->cmd == 4 || basic->cmd == 7
        || basic->cmd == 11 || basic->cmd == 12 || basic->cmd == 13
        || basic->cmd == 14)
    {
        char* file_path_raw = (char*)packet->get_static_descs()[0].get_addr();
        char* file_path = (char*)malloc(0x301);
        
        strncpy(file_path, sd_redir_path, 0x301);
        strncat(file_path, file_path_raw, 0x301);
        
#ifdef FSP_EXTRA_DEBUG
        log_printf("IFileSystem %s -> %s\r\n", file_path_raw, file_path);
#endif
        
        packet->get_static_descs()[0].set_addr((u64)africa_kaddr_to_uaddr(file_path));
        
        svcRunFunc(regs_in, regs_out, handler_ptr);
        
        free(file_path);
        
        return 1;
    }
    
    // RenameFile, RenameDirectory
    if (basic->cmd == 5 || basic->cmd == 6)
    {
        char* file_path_raw1 = (char*)packet->get_static_descs()[0].get_addr();
        char* file_path_raw2 = (char*)packet->get_static_descs()[1].get_addr();
        char* file_path1 = (char*)malloc(0x301);
        char* file_path2 = (char*)malloc(0x301);
        
        strncpy(file_path1, sd_redir_path, 0x301);
        strncat(file_path1, file_path_raw1, 0x301);
        strncpy(file_path2, sd_redir_path, 0x301);
        strncat(file_path2, file_path_raw2, 0x301);
        
#ifdef FSP_EXTRA_DEBUG
        log_printf("IFileSystem %s,%s -> %s,%s\r\n", file_path_raw1, file_path_raw2, file_path1, file_path2);
#endif
        
        packet->get_static_descs()[0].set_addr((u64)africa_kaddr_to_uaddr(file_path1));
        packet->get_static_descs()[1].set_addr((u64)africa_kaddr_to_uaddr(file_path2));
        
        svcRunFunc(regs_in, regs_out, handler_ptr);
        
        free(file_path1);
        free(file_path2);
        
        return 1;
    }

    if (basic->cmd == 8 || basic->cmd == 9)
    {
        FspSrv fsp;
        IFileSystem sdcard;
        IFile redirfile;
        u32 ret, file_ret, created_domain, created_handle;

        char* file_path;
        char* file_path_raw = (char*)packet->get_static_descs()[0].get_addr();
        u32 flags = basic->extra[0];
        bool is_domain = packet->is_domain_message();
        u32 domain_id = packet->get_domain_header()->objectId;

        fsp = sm_get_fspsrv();

        // Get SD card handle, if we can't get a handle then let it be
        ret = fsp.openSdCardFileSystem(&sdcard);
        if (ret)
        {
            fsp.close();
            return 0;
        }

        file_path = (char*)malloc(0x301);
        strncpy(file_path, sd_redir_path, 0x301);
        strncat(file_path, file_path_raw, 0x301);

#ifdef FSP_EXTRA_DEBUG
        log_printf("IFileSystem %s -> %s\r\n", file_path_raw, file_path);
#endif

        // Just return a handle, ezpz
        if (!is_domain)
        {
            // Get <redir>/<file> handle
            u32 out_hand;
            if (basic->cmd == 8)
            {
                IFile file;
                file_ret = sdcard.openFile(file_path, flags, &file);
                out_hand = file.h;
            }
            else
            {
                Handle dir;
                file_ret = sdcard.openDirectory(file_path, flags, &dir);
                out_hand = dir.h;
            }

            // Craft return
            HIPCCraftedPacket* p = new HIPCCraftedPacket();
            p->clear();
            p->type(0)->ipc_ret(file_ret);
            if (!file_ret)
                p->push_move_handle(out_hand);
            p->craft_to(packet);
            
            delete p;

            sdcard.close();
            fsp.close();
            free(file_path);

            return 1;
        }

        // Give us a guaranteed domain ID using OpenDirectory
        strcpy(file_path_raw, "/"); //TODO
        basic->cmd = 9;

        svcRunFunc(regs_in, regs_out, handler_ptr);

        packet = get_current_packet();
        basic = packet->get_data<HIPCBasicPacket>();

        created_domain = basic->extra[0];
#ifdef FSP_EXTRA_DEBUG
        log_printf("returned %x, err %x, hand %x\r\n", regs_out[0], basic->ret, created_domain);
#endif

        // Get <redir>/<file> handle
        file_ret = sdcard.openFile(file_path, flags, &redirfile);

#ifdef FSP_EXTRA_DEBUG
        log_printf("OpenFile(%s) got return %x, handle %x\r\n", file_path, file_ret, redirfile.h);
#endif

        // Craft return
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        p->clear();
        p->type(0)->ipc_ret(file_ret)->to_domain(0)->craft_to(packet);
        
        basic = packet->get_data<HIPCBasicPacket>();
        basic->extra[0] = created_domain;
        packet->dataSize++;

        if (!file_ret)
        {
            DomainPair* pair = new DomainPair(redirfile.h, redirfile.toDomainId());
            ipc_bind_domainsessionpair_to_handler(client, basic->extra[0], (void*)fsp_redir_ifile_hook, (void*)pair);
            ipc_bind_domainsessionpair_to_closehandler(client, basic->extra[0], (void*)fsp_redir_ifile_closehook, (void*)pair);
        }
        else
        {
            DomainPair pair(regs_in[0], created_domain);
            pair.closeDomain();
        }

        sdcard.close();
        fsp.close();
        free(file_path);

        return 1;
    }

    return 0;
}

int fsp_ldr_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr, void* extra)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

    if (basic->cmd == 0) // OpenCodeFileSystem
    {
        u64 tid = packet->get_data<u64>()[2];
#ifdef FSP_EXTRA_DEBUG
        log_printf("OpenCodeFileSystem(%016llx, %s) from %s, %x\r\n", tid, packet->get_static_descs()[0].get_addr(), kproc->name, packet->get_domain_header()->objectId);
#endif

        svcRunFunc(regs_in, regs_out, handler_ptr);

        packet = get_current_packet();
        basic = packet->get_data<HIPCBasicPacket>();

        if (!basic->ret && tid == 0x010000000000100d) // album applet
        {
            fsp_redirect_domain_ifilesystem(client, basic->extra[0], hbl_path);
        }

        return 1;
    }

    return 0;
}

int fsp_srv_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr, void* extra)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KClientSession* client = kproc->getKObjectFromHandle<KClientSession>(regs_in[0]);

    HIPCPacket* packet = get_current_packet();
    HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();

#if 0
    if (basic->cmd == 18) // OpenSdCardFileSystem
    {
        
#ifdef FSP_EXTRA_DEBUG
        log_printf("OpenSdCardFileSystem from %s\r\n", kproc->name);
#endif

        svcRunFunc(regs_in, regs_out, handler_ptr);

        packet = get_current_packet();
        basic = packet->get_data<HIPCBasicPacket>();
        
        if (basic->ret) return 0;
        
        if (packet->is_domain_message())
        {
            fsp_redirect_domain_ifilesystem(client, basic->extra[0], "/sd_redir");
        }
        else
        {
            KClientSession* sdcard = kproc->getKObjectFromHandle<KClientSession>(packet->get_handle(0));
            fsp_redirect_ifilesystem(sdcard, "/sd_redir");
        }

        return 1;
    }
#endif

    return 0;
}

void fsp_redirect_domain_ifilesystem(KClientSession* client, u32 domain, const char* path)
{
    ipc_bind_domainsessionpair_to_handler(client, domain, (void*)fsp_redir_ifilesystem_hook, (void*)path);
}

void fsp_redirect_ifilesystem(KClientSession* client, const char* path)
{
    ipc_bind_client_to_handler(client, (void*)fsp_redir_ifilesystem_hook, (void*)path);
}

void fsp_module_init()
{
    ipc_register_handler_for_named_port("fsp-ldr", (void*)fsp_ldr_hook);
    ipc_register_handler_for_named_port("fsp-srv", (void*)fsp_srv_hook);
    
    log_printf("fsp module initialized\r\n");
}
