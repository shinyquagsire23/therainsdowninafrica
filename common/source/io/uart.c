/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "uart.h"
#include "car.h"
#include "pinmux.h"
#include "timer.h"

const int UART_OFFSETS[5] = {0, 0x40, 0x200, 0x300, 0x400};

u64 uart_lock = 0;
u64 uart_print_lock = 0;
char log_buf[0x200];

void uart_configure_a(void)
{
    PINMUX_AUX_UART1_TX_0 = 0;
    PINMUX_AUX_UART1_RX_0 = PINMUX_INPUT;
    PINMUX_AUX_UART1_RTS_0 = 0;
    PINMUX_AUX_UART1_CTS_0 = PINMUX_INPUT;
}

void uart_configure_b(void)
{
    PINMUX_AUX_UART2_TX_0 = 0;
    PINMUX_AUX_UART2_RX_0 = PINMUX_INPUT;
    PINMUX_AUX_UART2_RTS_0 = 0;
    PINMUX_AUX_UART2_CTS_0 = PINMUX_INPUT;
}

void uart_configure_c(void)
{
    PINMUX_AUX_UART3_TX_0 = 0;
    PINMUX_AUX_UART3_RX_0 = PINMUX_INPUT;
    PINMUX_AUX_UART3_RTS_0 = 0;
    PINMUX_AUX_UART3_CTS_0 = PINMUX_INPUT;
}

void uart_configure_d(void)
{
    PINMUX_AUX_UART4_TX_0 = 0;
    PINMUX_AUX_UART4_RX_0 = PINMUX_INPUT;
    PINMUX_AUX_UART4_RTS_0 = 0;
    PINMUX_AUX_UART4_CTS_0 = PINMUX_INPUT;
}

void uart_set_baudrate(int id, u32 baudrate)
{
    UART_LCR_0(id) |= UART_DLAB_ENABLE;
    UART_THR_DLAB_0_0(id) = UART_BAUDRATE_CALC(baudrate) & 0xFF;
    UART_IER_DLAB_0_0(id) = (UART_BAUDRATE_CALC(baudrate) >> 8) & 0xFF;
    UART_LCR_0(id) &= ~UART_DLAB_ENABLE;
    UART_IIR_FCR_0(id) = UART_FCR_EN_FIFO | UART_RX_CLR | UART_TX_CLR;

    // Perform one read
    (void)UART_LSR_0(id);

    // Wait a bit
    timer_wait(8000);

    UART_LCR_0(id) |= UART_WORD_LENGTH_8;
    UART_MCR_0(id) = 0;
    UART_MSR_0(id) = 0;
    UART_IRDA_CSR_0(id) = 0;
    UART_RX_FIFO_CFG_0(id) = UART_RX_FIFO_TRIG(1);
    UART_MIE_0(id) = 0;
}

void uart_csr_set(int id, u32 bits)
{
    do
    {
        while (UART_MIE_0(id));
        UART_IRDA_CSR_0(id) |= bits;
    }
    while (UART_MIE_0(id));
}

void uart_csr_unset(int id, u32 bits)
{
    do
    {
        while (UART_MIE_0(id));
        UART_IRDA_CSR_0(id) &= ~bits;
    }
    while (UART_MIE_0(id));
}

void uart_enable_rts(int id)
{
    UART_MCR_0(id) |= UART_RTS_EN;
}

void uart_enable_cts(int id)
{
    UART_MCR_0(id) |= UART_CTS_EN;
}

void uart_init(int id, u32 baudrate)
{
    switch(id)
    {
        case UART_A:
            uart_configure_a();
            car_enable_uart_a();
            break;
        case UART_B:
            uart_configure_b();
            car_enable_uart_b();
            break;
        case UART_C:
            uart_configure_c();
            car_enable_uart_c();
            break;
        case UART_D:
            uart_configure_d();
            car_enable_uart_d();
            break;
    }

    uart_set_baudrate(id, baudrate);
    uart_interrupt_enable(id, 0);
    uart_csr_set(id, UART_BAUD_PULSE_4_14);
}

void uart_shutdown(int id)
{
    uart_csr_unset(id, UART_CSR_ALL);

    switch(id)
    {
        case UART_A:
            car_disable_uart_a();
            break;
        case UART_B:
            car_disable_uart_b();
            break;
        case UART_C:
            car_disable_uart_c();
            break;
        case UART_D:
            car_disable_uart_d();
            break;
    }
}

void uart_interrupt_enable(int id, u32 interrupts)
{
    // Enable IER
    UART_LCR_0(id) &= ~UART_DLAB_ENABLE;

    UART_IER_DLAB_0_0(id) |= interrupts;
}

void uart_interrupt_disable(int id, u32 interrupts)
{
    // Enable IER
    UART_LCR_0(id) &= ~UART_DLAB_ENABLE;

    UART_IER_DLAB_0_0(id) &= ~interrupts;
}

void uart_enable_double_stop_bits(int id)
{
    UART_LCR_0(id) |= UART_STOP_BITS_DOUBLE;
}

void uart_disable_double_stop_bits(int id)
{
    UART_LCR_0(id) &= ~UART_STOP_BITS_DOUBLE;
}

void uart_write(int id, u8 *data, u32 size)
{
    for (u8 *tx = data; tx < data + size; tx++)
    {
        while (UART_LSR_0(id) & UART_TX_FIFO_FULL);

        UART_THR_DLAB_0_0(id) = *tx;
    }
}

void uart_wait_for_write(int id)
{
    while (!(UART_LSR_0(id) & UART_TMTY));
}

int uart_read_blocking(int id, u8 *data, u32 size, int timeout)
{
    for (int i = 0; i < size; i++)
    {
        if (timeout)
        {
            if (UART_LSR_0(id) & UART_RX_FIFO_EMPTY)
                timer_wait(timeout);

            if (UART_LSR_0(id) & UART_RX_FIFO_EMPTY)
                return i;
        }
        else
        {
            while (UART_LSR_0(id) & UART_RX_FIFO_EMPTY);
        }

        data[i] = UART_THR_DLAB_0_0(id);
    }

    return size;
}

int uart_read_nonblocking(int id, u8 *data, u32 size)
{
    for (int i = 0; i < size; i++)
    {
        if (UART_LSR_0(id) & UART_RX_FIFO_EMPTY)
            return i;

        data[i] = UART_THR_DLAB_0_0(id);
    }

    return size;
}
