#ifndef HIPC_H
#define HIPC_H

#include "types.h"
#include "arm/tls.h"
#include "io/uart.h"

#include <hos/svc.h>

#include <malloc.h>

#define MAGIC_SFCI 0x49434653
#define MAGIC_SFCO 0x4F434653

#define HIPC_MAX_BUFS 8
#define HIPC_MAX_OBJS 8

enum HIPCPacketType : u16
{
    HIPCPacketType_Invalid = 0,
    HIPCPacketType_LegacyRequest = 1,
    HIPCPacketType_Close = 2,
    HIPCPacketType_LegacyControl = 3,
    HIPCPacketType_Request = 4,
    HIPCPacketType_Control = 5,
    HIPCPacketType_RequestWithContext = 6,
    HIPCPacketType_ControlWithContext = 7
};

enum HIPCDomainCommand : u8
{
    HIPCDomainCommand_Send = 1,
    HIPCDomainCommand_CloseVirtualHandle = 2,
};

struct HIPCHandleDesc
{
    u32 sendPid : 1;
    u32 handlesToCopy : 4;
    u32 handlesToMove : 4;
    u32 unused : 23;

    u8 data[];

    int copy_handles_pos()
    {
        return sendPid ? sizeof(u64) : 0;
    }

    int move_handles_pos()
    {
        return copy_handles_pos() + handlesToCopy * sizeof(u32);
    }

    int num_handles()
    {
        return handlesToCopy + handlesToMove;
    }

    int length()
    {
        return sizeof(u32) + move_handles_pos() + handlesToMove * sizeof(u32);
    }

    u64 get_pid()
    {
        if (sendPid)
        {
            return *(u64*)data;
        }

        return 0;
    }

    u32 get_copy_handle(int index)
    {
        if (index >= handlesToCopy) return 0;
        u32* copyHandles = (u32*)&data[copy_handles_pos()];

        return copyHandles[index];
    }

    u32 get_move_handle(int index)
    {
        if (index >= handlesToMove) return 0;
        u32* moveHandles = (u32*)&data[move_handles_pos()];

        return moveHandles[index];
    }

    void set_copy_handle(int index, u32 handle)
    {
        if (index >= handlesToCopy) return;
        u32* copyHandles = (u32*)&data[copy_handles_pos()];

        copyHandles[index] = handle;
    }

    void set_move_handle(int index, u32 handle)
    {
        if (index >= handlesToMove) return;
        u32* moveHandles = (u32*)&data[move_handles_pos()];

        moveHandles[index] = handle;
    }

    u32 get_handle(int index)
    {
        return index >= handlesToCopy ? get_move_handle(index - handlesToCopy) : get_copy_handle(index);
    }
};

struct HIPCAddrSplit
{
    u32 addr31to0;
    u32 addr35to32 : 4;
    u32 addr38to36 : 3;
};

struct HIPCSizeSplit
{
    u32 size31to0;
    u32 size35to32 : 4;
};

struct HIPCStaticIndexSplit
{
    u16 index5to0 : 6;
    u16 index8to6 : 3;
    u16 index11to9 : 3;
};

struct HIPCStaticDesc
{
    u16 index5to0 : 6;
    u16 addr38to36 : 3;
    u16 index11to9 : 3;
    u16 addr35to32 : 4;
    u16 size : 16;
    u32 addr31to0;

    u64 get_size()
    {
        return (u64)size;
    }

    u64 get_addr()
    {
        return addr31to0 | ((u64)addr35to32 << 32) | ((u64)addr38to36 << 36);
    }

    u32 get_index()
    {
        return index5to0 | (index11to9 << 9);
    }

    void set_size(u16 new_size)
    {
        size = new_size;
    }

    void set_addr(u64 addr)
    {
        HIPCAddrSplit* split = (HIPCAddrSplit*)&addr;
        addr31to0 = split->addr31to0;
        addr35to32 = split->addr35to32;
        addr38to36 = split->addr38to36;
    }

    void set_index(u16 index)
    {
        HIPCStaticIndexSplit* split = (HIPCStaticIndexSplit*)&index;
        index5to0 = split->index5to0;
        index11to9 = split->index11to9;
    }
};

struct HIPCSendRecvExchDesc
{
    u32 size31to0;
    u32 addr31to0;
    u32 flags : 2;
    u32 addr38to36 : 3;
    u32 size35to32 : 4;
    u32 addr35to32 : 4;

    u64 get_size()
    {
        return size31to0 | ((u64)size35to32 << 32);
    }

    u64 get_addr()
    {
        return addr31to0 | ((u64)addr35to32 << 32) | ((u64)addr38to36 << 36);
    }

    u32 get_flags()
    {
        return flags;
    }

    void set_size(u64 new_size)
    {
        HIPCSizeSplit* split = (HIPCSizeSplit*)&new_size;
        size31to0 = split->size31to0;
        size35to32 = split->size35to32;
    }

    void set_addr(u64 addr)
    {
        HIPCAddrSplit* split = (HIPCAddrSplit*)&addr;
        addr31to0 = split->addr31to0;
        addr35to32 = split->addr35to32;
        addr38to36 = split->addr38to36;
    }

    void set_flags(u8 new_flags)
    {
        flags = new_flags;
    }
};

struct HIPCDomainHeader
{
    u8 cmd;
    u8 numObjs;
    u16 dataPayloadSize;
    u32 objectId;
    u32 pad;
    u32 token;

    u8 data[];

    u32 get_object(int index)
    {
        if (index >= numObjs) return 0;

        return ((u32*)(&data[dataPayloadSize]))[index];
    }
};

struct HIPCPacket
{
    u32 type : 16;
    u32 numStatic : 4;
    u32 numSend : 4;
    u32 numRecv : 4;
    u32 numExch : 4;

    u32 dataSize : 10;
    u32 recvStaticFlags : 4;
    u32 unk1 : 7;
    u32 unk2 : 10;
    u32 enableHandle : 1;

    u8 data[];

    int get_static_desc_pos()
    {
        return enableHandle ? get_handle_desc()->length() : 0;
    }

    int get_send_desc_pos()
    {
        return get_static_desc_pos() + numStatic * sizeof(HIPCStaticDesc);
    }

    int get_recv_desc_pos()
    {
        return get_send_desc_pos() + numSend * sizeof(HIPCSendRecvExchDesc);
    }

    int get_exch_desc_pos()
    {
        return get_recv_desc_pos() + numRecv * sizeof(HIPCSendRecvExchDesc);
    }

    int get_raw_data_pos()
    {
        int pos = get_exch_desc_pos() + numExch * sizeof(HIPCSendRecvExchDesc);
        return (((pos + sizeof(u32) * 2) + 0xF) & ~0xF) - 0x8; // align to 0x10
    }



    HIPCHandleDesc* get_handle_desc()
    {
        return (HIPCHandleDesc*)&data[0];
    }

    HIPCStaticDesc* get_static_descs()
    {
        return (HIPCStaticDesc*)&data[get_static_desc_pos()];
    }

    HIPCSendRecvExchDesc* get_send_descs()
    {
        return (HIPCSendRecvExchDesc*)&data[get_send_desc_pos()];
    }

    HIPCSendRecvExchDesc* get_recv_descs()
    {
        return (HIPCSendRecvExchDesc*)&data[get_recv_desc_pos()];
    }

    HIPCSendRecvExchDesc* get_exch_descs()
    {
        return (HIPCSendRecvExchDesc*)&data[get_exch_desc_pos()];
    }

    u8* get_raw_data()
    {
        return (u8*)&data[get_raw_data_pos()];
    }


    bool is_domain_message()
    {
        u32 magic = *(u32*)get_raw_data();
        return (magic != MAGIC_SFCI && magic != MAGIC_SFCO);
    }

    HIPCDomainHeader* get_domain_header()
    {
        return (HIPCDomainHeader*)get_raw_data();
    }

    template <class T>
    T* get_data()
    {
        return (T*)(is_domain_message() ? &get_domain_header()->data[0] : get_raw_data());
    }

    int num_domain_objects()
    {
        return is_domain_message() ? get_domain_header()->numObjs : 0;
    }

    int get_domain_object(int idx)
    {
        return is_domain_message() ? get_domain_header()->get_object(idx) : 0;
    }

    int num_handles()
    {
        return enableHandle ? get_handle_desc()->num_handles() : 0;
    }

    u32 get_handle(int index)
    {
        return enableHandle ? get_handle_desc()->get_handle(index) : 0;
    }

    void debug_print()
    {
        uart_debug_printf("type %x, %u %u %u %u\r\n", type, numStatic, numSend, numRecv, numExch);
        uart_debug_printf("data size %x, %x %x\r\n", dataSize, recvStaticFlags, enableHandle);

        for (int i = 0; i < numStatic; i++)
        {
            HIPCStaticDesc* desc = &get_static_descs()[i];
            uart_debug_printf("static %u: addr %016llx size %x\r\n", i, desc->get_addr(), desc->get_size());

            for (int i = 0; i < 10; i++)
            {
                uart_debug_printf("%x\r\n", ((u32*)desc->get_addr())[-i]);
            }
        }

        for (int i = 0; i < numSend; i++)
        {
            HIPCSendRecvExchDesc* desc = &get_send_descs()[i];
            uart_debug_printf("send %u: addr %016llx size %x\r\n", i, desc->get_addr(), desc->get_size());

            for (int i = 0; i < 10; i++)
            {
                uart_debug_printf("%x\r\n", ((u32*)desc->get_addr())[-i]);
            }
        }

        for (int i = 0; i < numRecv; i++)
        {
            HIPCSendRecvExchDesc* desc = &get_recv_descs()[i];
            uart_debug_printf("recv %u: addr %016llx size %x\r\n", i, desc->get_addr(), desc->get_size());
        }

        for (int i = 0; i < numExch; i++)
        {
            HIPCSendRecvExchDesc* desc = &get_exch_descs()[i];
            uart_debug_printf("exch %u: addr %016llx size %x\r\n", i, desc->get_addr(), desc->get_size());

            for (int i = 0; i < 10; i++)
            {
                uart_debug_printf("%x\r\n", ((u32*)desc->get_addr())[-i]);
            }
        }

        uart_debug_printf("data is_domain %x, [0] %x\r\n", is_domain_message(), get_data<u32>()[0]);
    }
};

struct HIPCBasicPacket
{
    u32 magic;
    u32 unk;
    union
    {
        u32 cmd;
        u32 ret;
    };
    u32 token;
    u32 extra[];
};

static inline HIPCPacket* get_current_packet()
{
    return (HIPCPacket*)getTls();
}



enum HIPCBufferType : u8
{
    HIPCBufferType_Send = 0,
    HIPCBufferType_Recv = 1,
    HIPCBufferType_Exch = 2,
};

enum HIPCHandleType : u8
{
    HIPCHandleType_Copy = 0,
    HIPCHandleType_Move = 1,
};

struct HIPCCraftedPacket
{
    u16 ipcType;
    union
    {
        u32 ipcCmd;
        u32 ipcRet;
    };
    u32 ipcToken;
    u32 domainCmd;
    u32 domainId;

    u8 numSendStatic;
    u8 numRecvStatic;
    u8 numSend;
    u8 numRecv;
    u8 numExch;

    u8 handlesToCopy;
    u8 handlesToMove;
    u32 handles[HIPC_MAX_OBJS];
    u8 handleTypes[HIPC_MAX_OBJS];

    u8 numObjIds;
    u32 objects[HIPC_MAX_OBJS];

    u64 buffers[HIPC_MAX_BUFS];
    u64 bufferSizes[HIPC_MAX_BUFS];
    u8 bufferTypes[HIPC_MAX_BUFS];

    u64 staticBuffers[HIPC_MAX_BUFS];
    u16 staticSizes[HIPC_MAX_BUFS];
    u16 staticIdxs[HIPC_MAX_BUFS];

    void* data;
    u32 dataSize;

    u32 ret;
    u64 pidRet;

    bool sendPid;

    HIPCCraftedPacket()
    {
        clear();
    }

    ~HIPCCraftedPacket()
    {
        if (data)
            free(data);
    }

    HIPCCraftedPacket* type(u16 type)
    {
        ipcType = type;
        return this;
    }

    HIPCCraftedPacket* token(u32 token)
    {
        ipcToken = token;
        return this;
    }

    HIPCCraftedPacket* ipc_cmd(u32 cmd)
    {
        ipcCmd = cmd;
        return this;
    }

    HIPCCraftedPacket* domain_cmd(u32 cmd)
    {
        domainCmd = cmd;
        return this;
    }

    HIPCCraftedPacket* send_pid(void)
    {
        sendPid = true;
        return this;
    }

    template <class T>
    HIPCCraftedPacket* push_arg(T arg)
    {
        data = realloc(data, dataSize + sizeof(arg));

        if (data)
        {
            memcpy(data + dataSize, &arg, sizeof(arg));
            dataSize += sizeof(arg)/sizeof(u32);
        }

        return this;
    }

    HIPCCraftedPacket* push_static_buffer(void* buf, u16 size)
    {
        if (numSendStatic >= HIPC_MAX_BUFS) return this;

        staticBuffers[numSendStatic] = (u64)buf;
        staticSizes[numSendStatic] = size;
        numSendStatic++;
        return this;
    }

    HIPCCraftedPacket* push_send_buffer(void* buf, u64 size)
    {
        if (numSend + numRecv + numExch >= HIPC_MAX_BUFS) return this;

        buffers[numSend + numRecv + numExch] = (u64)buf;
        bufferSizes[numSend + numRecv + numExch] = size;
        bufferTypes[numSend + numRecv + numExch] = HIPCBufferType_Send;

        numSend++;
        return this;
    }

    HIPCCraftedPacket* push_recv_buffer(void* buf, u64 size)
    {
        if (numSend + numRecv + numExch >= HIPC_MAX_BUFS) return this;

        buffers[numSend + numRecv + numExch] = (u64)buf;
        bufferSizes[numSend + numRecv + numExch] = size;
        bufferTypes[numSend + numRecv + numExch] = HIPCBufferType_Recv;

        numRecv++;
        return this;
    }

    HIPCCraftedPacket* push_exch_buffer(void* buf, u64 size)
    {
        if (numSend + numRecv + numExch >= HIPC_MAX_BUFS) return this;

        buffers[numSend + numRecv + numExch] = (u64)buf;
        bufferSizes[numSend + numRecv + numExch] = size;
        bufferTypes[numSend + numRecv + numExch] = HIPCBufferType_Send;

        numExch++;
        return this;
    }

    HIPCCraftedPacket* push_copy_handle(u32 handle)
    {
        if (handlesToCopy + handlesToMove >= HIPC_MAX_OBJS) return this;

        handles[handlesToCopy + handlesToMove] = handle;
        handleTypes[handlesToCopy + handlesToMove] = HIPCHandleType_Copy;

        handlesToCopy++;
        return this;
    }

    HIPCCraftedPacket* push_move_handle(u32 handle)
    {
        if (handlesToCopy + handlesToMove >= HIPC_MAX_OBJS) return this;

        handles[handlesToCopy + handlesToMove] = handle;
        handleTypes[handlesToCopy + handlesToMove] = HIPCHandleType_Move;

        handlesToMove++;
        return this;
    }

    HIPCCraftedPacket* send_to_domain(u32 domain, u32 handle)
    {
        domainId = domain;
        return send_to(handle);
    }

    HIPCCraftedPacket* send_to(u32 handle)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCPacket* packetTemp = (HIPCPacket*)malloc(0x400);

        memcpy(packetTemp, get_current_packet(), 0x400);
        memset(packet, 0, 0x10);

        packet->type = ipcType;
        packet->dataSize = dataSize + 4 /*header*/ + 4 /*padding*/;
        packet->numStatic = numSendStatic;
        packet->numSend = numSend;
        packet->numRecv = numRecv;
        packet->numExch = numExch;

        if (sendPid || handlesToCopy || handlesToMove)
            packet->enableHandle = 1;

        if (packet->enableHandle)
        {
            packet->get_handle_desc()->sendPid = sendPid;
            packet->get_handle_desc()->handlesToCopy = handlesToCopy;
            packet->get_handle_desc()->handlesToMove = handlesToMove;
        }

        int sentCopy = 0, sentMove = 0;
        for (int i = 0; i < handlesToCopy + handlesToMove; i++)
        {
            switch (handleTypes[i])
            {
                case HIPCHandleType_Copy:
                    packet->get_handle_desc()->set_copy_handle(sentCopy++, handles[i]);
                    break;
                case HIPCHandleType_Move:
                    packet->get_handle_desc()->set_move_handle(sentMove++, handles[i]);
                    break;
            }
        }

        for (int i = 0; i < numSendStatic; i++)
        {
            HIPCStaticDesc* desc = &packet->get_static_descs()[i];

            desc->set_addr(staticBuffers[i]);
            desc->set_size(staticSizes[i]);
            desc->set_index(i);
        }

        int sentSend = 0, sentRecv = 0, sentExch = 0;
        for (int i= 0; i < numSend + numRecv + numExch; i++)
        {
            HIPCSendRecvExchDesc* desc;
            switch (bufferTypes[i])
            {
                case HIPCBufferType_Send:
                    desc = &packet->get_send_descs()[sentSend++];
                    break;
                case HIPCBufferType_Recv:
                    desc = &packet->get_recv_descs()[sentRecv++];
                    break;
                case HIPCBufferType_Exch:
                    desc = &packet->get_exch_descs()[sentExch++];
                    break;
            }

            desc->set_addr(buffers[i]);
            desc->set_size(bufferSizes[i]);
        }

        HIPCBasicPacket* basic = (HIPCBasicPacket*)packet->get_raw_data();
        if (domainId)
        {
            HIPCDomainHeader* domain = packet->get_domain_header();
            packet->dataSize += 4;

            domain->cmd = domainCmd;
            domain->numObjs = numObjIds;
            domain->objectId = domainId;
            if (domainCmd != HIPCDomainCommand_CloseVirtualHandle)
                domain->dataPayloadSize = dataSize;
            else
                domain->dataPayloadSize = 0;
            domain->token = ipcToken;

            basic = packet->get_data<HIPCBasicPacket>();
        }
        else
        {
             basic->token = ipcToken;
        }

        basic->magic = MAGIC_SFCI;
        basic->cmd = ipcCmd;
        memcpy((void*)basic->extra, data, dataSize*sizeof(u32));

#if 0
        uart_debug_printf("sent:\r\n");
        for (int i = 0; i < 0x10; i++)
        {
            uart_debug_printf("%x: %08x\r\n", i, ((u32*)packet)[i]);
        }
#endif

        ret = ksvcSendSyncRequest(handle);

#if 0
        uart_debug_printf("ret: %x\r\n", ret);
        for (int i = 0; i < 0x10; i++)
        {
            uart_debug_printf("%x: %08x\r\n", i, ((u32*)packet)[i]);
        }
#endif

        if (!ret)
            parse(get_current_packet());

        memcpy(get_current_packet(), packetTemp, 0x400);
        free(packetTemp);

        return this;
    }

    HIPCCraftedPacket* parse(HIPCPacket* packet)
    {
        ipcType = packet->type;
        dataSize = packet->dataSize - 4 /*header*/ - 4 /*padding*/;

        if (packet->enableHandle)
        {
            sendPid = packet->get_handle_desc()->sendPid;
            pidRet = packet->get_handle_desc()->get_pid();

            for (int i = 0; i < packet->get_handle_desc()->handlesToCopy; i++)
            {
                push_copy_handle(packet->get_handle_desc()->get_copy_handle(i));
            }

            for (int i = 0; i < packet->get_handle_desc()->handlesToMove; i++)
            {
                push_move_handle(packet->get_handle_desc()->get_move_handle(i));
            }
        }

        for (int i = 0; i < packet->numStatic; i++)
        {
            HIPCStaticDesc* desc = &packet->get_static_descs()[i];

            push_static_buffer((void*)desc->get_addr(), desc->get_size());
        }

        for (int i = 0; i < packet->numSend; i++)
        {
            HIPCSendRecvExchDesc* desc = &packet->get_send_descs()[i];

            push_send_buffer((void*)desc->get_addr(), desc->get_size());
        }

        for (int i = 0; i < packet->numRecv; i++)
        {
            HIPCSendRecvExchDesc* desc = &packet->get_recv_descs()[i];

            push_recv_buffer((void*)desc->get_addr(), desc->get_size());
        }

        for (int i = 0; i < packet->numExch; i++)
        {
            HIPCSendRecvExchDesc* desc = &packet->get_exch_descs()[i];

            push_exch_buffer((void*)desc->get_addr(), desc->get_size());
        }

        HIPCBasicPacket* basic = packet->get_data<HIPCBasicPacket>();
        if (packet->is_domain_message())
        {
            HIPCDomainHeader* domain = packet->get_domain_header();
            dataSize -= 4;

            domainCmd = domain->cmd;
            numObjIds = domain->numObjs;
            domainId = domain->objectId;
            dataSize = domain->dataPayloadSize;
            ipcToken = domain->token;
        }
        else
        {
            ipcToken = basic->token;
        }

        ipcCmd = basic->cmd;
        data = realloc(data, dataSize*sizeof(u32));
        memcpy(data, (void*)basic->extra, dataSize*sizeof(u32));

        return this;
    }

    u32 get_handle(int idx)
    {
        if (idx >= handlesToCopy + handlesToMove) return 0;

        return handles[idx];
    }

    template <class T>
    T* get_data()
    {
        return (T*)data;
    }

    void free_data()
    {
        if (data) free(data);

        data = NULL;
    }

    void clear()
    {
        ipcType = HIPCPacketType_Request;
        ipcCmd = 0;
        ipcToken = 0;
        domainCmd = HIPCDomainCommand_Send;
        domainId = 0;

        numSendStatic = 0;
        numRecvStatic = 0;
        numSend = 0;
        numRecv = 0;
        numExch = 0;

        handlesToCopy = 0;
        handlesToMove = 0;

        numObjIds = 0;

        data = NULL;
        dataSize = 0;

        ret = 0;
        pidRet = 0;
        sendPid = false;
    }
};

#endif // HIPC_H
