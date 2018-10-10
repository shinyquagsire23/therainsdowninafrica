/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "svc_bind.h"

/*
 * This is currently really slow with large SVC_MAX_CALLBACKS values,
 * due to lazy implementation. This can totally be fixed.
 */

/*
 * pre-handler return values:
 * 0 - run Nintendo's SVC handler (and therefore our post-handler)
 * 1 - abort this SVC, returning to user the contents of the "stack" pointer as registers.
 */
struct {
    svcNumber number;
    int (*handler)();
} svc_pre_table[SVC_MAX_CALLBACKS] = {0};

struct {
    svcNumber number;
    void (*handler)();
} svc_post_table[SVC_MAX_CALLBACKS] = {0};

int svc_pre(svcNumber number, u64* regs_in, u64* regs_out, void* default_handler)
{
    int ret = 0;

    for(int i = 0; i < SVC_MAX_CALLBACKS; i++)
    {
        if(!svc_pre_table[i].handler) continue;

        if(svc_pre_table[i].number == idAll)
        {
            ret = svc_pre_table[i].handler(number, regs_in, regs_out, default_handler);
            if(ret) return ret;
        }

        if(svc_pre_table[i].number == number)
        {
            ret = svc_pre_table[i].handler(regs_in, regs_out, default_handler);
            if(ret) return ret;
        }
    }

    return ret;
}

int svc_post(svcNumber number, u64* regs_in, u64* regs_out, void* default_handler)
{
    int ret = 0;

    for(int i = 0; i < SVC_MAX_CALLBACKS; i++)
    {
        if(!svc_post_table[i].handler) continue;

        if(svc_post_table[i].number == idAll)
        {
            svc_post_table[i].handler(number, regs_in, regs_out, default_handler);
            if(ret) return ret;
        }

        if(svc_post_table[i].number == number)
        {
            svc_post_table[i].handler(regs_in, regs_out, default_handler);
            if(ret) return ret;
        }
    }

    return ret;
}

int svc_pre_bind(svcNumber number, void* callback)
{
    if(!callback) return -1;

    for(int i = 0; i < SVC_MAX_CALLBACKS; i++)
    {
        if(svc_pre_table[i].handler) continue;

        svc_pre_table[i].number = number;
        svc_pre_table[i].handler = callback;
        //cp15_dcache_clean(); //TODO

        return 0;
    }

    return 1;
}

int svc_pre_unbind(void* callback)
{
    if(!callback) return -1;

    for(int i = 0; i < SVC_MAX_CALLBACKS; i++)
    {
        if(svc_pre_table[i].handler != callback) continue;

        svc_pre_table[i].number = 0;
        svc_pre_table[i].handler = NULL;
    }

    //cp15_dcache_clean(); //TODO

    return 0;
}

int svc_post_bind(svcNumber number, void* callback)
{
    if(!callback) return -1;

    for(int i = 0; i < SVC_MAX_CALLBACKS; i++)
    {
        if(svc_pre_table[i].handler) continue;

        svc_post_table[i].number = number;
        svc_post_table[i].handler = callback;
        //cp15_dcache_clean(); //TODO

        return 0;
    }

    return 1;
}

int svc_post_unbind(void* callback)
{
    if(!callback) return -1;

    for(int i = 0; i < SVC_MAX_CALLBACKS; i++)
    {
        if(svc_post_table[i].handler != callback) continue;

        svc_post_table[i].number = 0;
        svc_post_table[i].handler = NULL;
    }

    //cp15_dcache_clean(); //TODO

    return 0;
}
