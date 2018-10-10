/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef _PINMUX_H_
#define _PINMUX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define PINMUX_PADDR                     ((void*)0x70003000)
#define PINMUX_VADDR                     ((void*)0xFFFFFFFF70003000)

#define PINMUX_AUX_UART1_BASE            ((void*)PINMUX_VADDR + 0xE4)
#define PINMUX_AUX_UART1_TX_0            (*(vu32*)(PINMUX_AUX_UART1_BASE + 0x0))
#define PINMUX_AUX_UART1_RX_0            (*(vu32*)(PINMUX_AUX_UART1_BASE + 0x4))
#define PINMUX_AUX_UART1_RTS_0           (*(vu32*)(PINMUX_AUX_UART1_BASE + 0x8))
#define PINMUX_AUX_UART1_CTS_0           (*(vu32*)(PINMUX_AUX_UART1_BASE + 0xC))

#define PINMUX_AUX_UART2_BASE            ((void*)PINMUX_VADDR + 0xF4)
#define PINMUX_AUX_UART2_TX_0            (*(vu32*)(PINMUX_AUX_UART2_BASE + 0x0))
#define PINMUX_AUX_UART2_RX_0            (*(vu32*)(PINMUX_AUX_UART2_BASE + 0x4))
#define PINMUX_AUX_UART2_RTS_0           (*(vu32*)(PINMUX_AUX_UART2_BASE + 0x8))
#define PINMUX_AUX_UART2_CTS_0           (*(vu32*)(PINMUX_AUX_UART2_BASE + 0xC))

#define PINMUX_AUX_UART3_BASE            ((void*)PINMUX_VADDR + 0x104)
#define PINMUX_AUX_UART3_TX_0            (*(vu32*)(PINMUX_AUX_UART3_BASE + 0x0))
#define PINMUX_AUX_UART3_RX_0            (*(vu32*)(PINMUX_AUX_UART3_BASE + 0x4))
#define PINMUX_AUX_UART3_RTS_0           (*(vu32*)(PINMUX_AUX_UART3_BASE + 0x8))
#define PINMUX_AUX_UART3_CTS_0           (*(vu32*)(PINMUX_AUX_UART3_BASE + 0xC))

#define PINMUX_AUX_UART4_BASE            ((void*)PINMUX_VADDR + 0x114)
#define PINMUX_AUX_UART4_TX_0            (*(vu32*)(PINMUX_AUX_UART4_BASE + 0x0))
#define PINMUX_AUX_UART4_RX_0            (*(vu32*)(PINMUX_AUX_UART4_BASE + 0x4))
#define PINMUX_AUX_UART4_RTS_0           (*(vu32*)(PINMUX_AUX_UART4_BASE + 0x8))
#define PINMUX_AUX_UART4_CTS_0           (*(vu32*)(PINMUX_AUX_UART4_BASE + 0xC))

#define PINMUX_SCHMT      (BIT(12))
#define PINMUX_LPDR       (BIT(8))
#define PINMUX_LOCK       (BIT(7))
#define PINMUX_INPUT      (BIT(6))
#define PINMUX_PARKED     (BIT(5))
#define PINMUX_TRISTATE   (BIT(4))

#define PINMUX_PM         (0b11)
#define PINMUX_PULL_DOWN  (BIT(2))
#define PINMUX_PULL_UP    (BIT(3))

#define PINMUX_DRIVE      (0b11 << 13)
#define PINMUX_DRIVE_2X   (1 << 13)

#define PINMUX_RSVD2   (2)

#ifdef __cplusplus
}
#endif

#endif // _PINMUX_H_
