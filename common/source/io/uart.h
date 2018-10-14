/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef _UART_H_
#define _UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "utils.h"
#include "arm/threading.h"

#include <string.h>
#include <stdio.h>

#define UART_A (0)
#define UART_B (1)
#define UART_C (2)
#define UART_D (3)
#define UART_E (4)

#define UART_PADDR                       ((void*)0x70006000)
#define UART_VADDR                       ((void*)0xffffffff70006000)
#define UART_BASE(index)                 ((void*)UART_VADDR + UART_OFFSETS[index])

#define UART_THR_DLAB_0_0(index)         (*(vu32*)(UART_BASE(index) + 0x0))
#define UART_IER_DLAB_0_0(index)         (*(vu32*)(UART_BASE(index) + 0x4))
#define UART_IIR_FCR_0(index)            (*(vu32*)(UART_BASE(index) + 0x8))
#define UART_LCR_0(index)                (*(vu32*)(UART_BASE(index) + 0xC))
#define UART_MCR_0(index)                (*(vu32*)(UART_BASE(index) + 0x10))
#define UART_LSR_0(index)                (*(vu32*)(UART_BASE(index) + 0x14))
#define UART_MSR_0(index)                (*(vu32*)(UART_BASE(index) + 0x18))
#define UART_SPR_0(index)                (*(vu32*)(UART_BASE(index) + 0x1C))
#define UART_IRDA_CSR_0(index)           (*(vu32*)(UART_BASE(index) + 0x20))
#define UART_RX_FIFO_CFG_0(index)        (*(vu32*)(UART_BASE(index) + 0x24))
#define UART_MIE_0(index)                (*(vu32*)(UART_BASE(index) + 0x28))
#define UART_VENDOR_STATUS_0_0(index)    (*(vu32*)(UART_BASE(index) + 0x2C))
#define UART_ASR_0(index)                (*(vu32*)(UART_BASE(index) + 0x3C))

// UART_IER_DLAB_0_0
#define UART_IE_EORD          BIT(5)
#define UART_IE_RX_TIMEOUT    BIT(4)
#define UART_IE_MSI           BIT(3)
#define UART_IE_RXS           BIT(2)
#define UART_IE_THR           BIT(1)
#define UART_IE_RHR           BIT(0)

// UART_IIR_FCR_0
#define UART_FCR_EN_FIFO      BIT(0)
#define UART_RX_CLR           BIT(1)
#define UART_TX_CLR           BIT(2)

// UART_LCR_0
#define UART_DLAB_ENABLE      BIT(7)
#define UART_PARITY_EVEN      BIT(5)
#define UART_PARITY_ENABLE    BIT(7)
#define UART_STOP_BITS_DOUBLE BIT(2)
#define UART_WORD_LENGTH_8    (3)

// UART_MCR_0
#define UART_RTS_EN           BIT(6)
#define UART_CTS_EN           BIT(5)
#define UART_LOOPBACK_ENABLE  BIT(4)
#define UART_FORCE_RTS_HI_LO  BIT(1)
#define UART_FORCE_CTS_HI_LO  BIT(0)

// UART_LSR_0
#define UART_RX_FIFO_EMPTY    BIT(9)
#define UART_TX_FIFO_FULL     BIT(8)
#define UART_TMTY             BIT(6)
#define UART_RDR              BIT(0)

// UART_IRDA_CSR_0
#define UART_BAUD_PULSE_4_14  BIT(6)
#define UART_INVERT_RTS       BIT(3)
#define UART_INVERT_CTS       BIT(2)
#define UART_INVERT_TXD       BIT(1)
#define UART_INVERT_RXD       BIT(0)
#define UART_CSR_ALL          (~0)

// UART_RX_FIFO_CFG_0
#define UART_RX_FIFO_TRIG(level) (level & 0x3F)

// Misc
#define UART_BAUDRATE_CALC(rate) (((8 * rate + 408000000) / (16 * rate)))

extern const int UART_OFFSETS[5];

void uart_set_baudrate(int id, u32 baudrate);
void uart_csr_set(int id, u32 bits);
void uart_csr_unset(int id, u32 bits);
void uart_enable_rts(int id);
void uart_enable_cts(int id);

void uart_init(int id, u32 baudrate);
void uart_shutdown(int id);

void uart_interrupt_enable(int id, u32 interrupts);
void uart_interrupt_disable(int id, u32 interrupts);

void uart_enable_double_stop_bits(int id);
void uart_disable_double_stop_bits(int id);

void uart_write(int id, u8 *data, u32 size);
void uart_wait_for_write(int id);

int uart_read_blocking(int id, u8 *data, u32 size, int timeout);
int uart_read_nonblocking(int id, u8 *data, u32 size);

extern u64 uart_print_lock;
extern u64 uart_lock;
extern char log_buf[0x200];

#define uartDebugPrint(str) \
    { mutex_lock(&uart_print_lock); uart_init(UART_A, 115200); uart_write(UART_A, (u8*)str, strlen(str)); uart_wait_for_write(UART_A); mutex_unlock(&uart_print_lock); }

#define uart_debug_printf(...) \
    { mutex_lock(&uart_lock); snprintf(log_buf, 0x200, __VA_ARGS__); \
    uartDebugPrint(log_buf); mutex_unlock(&uart_lock); }

#ifdef __cplusplus
}
#endif

#endif // _UART_H_
