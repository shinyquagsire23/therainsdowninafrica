/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "types.h"
#include "hos/kfuncs.h"
#include "io/uart.h"

#include "arm/exceptions.h"

#include "main.h"

extern "C"
{
    u64* exception_handle(u64* context)
    {
        u32 ec = get_esr_el1() >> 26;
        u32 ifsc = get_esr_el1() & 0x1F;
        
        char* dabt_string = "unknown exception";
        char* ec_string = "unknown exception code";
        
        switch (ec)
        {
            case 1:
                ec_string = "WFI/WFE";
                break;

            case 3:
                ec_string = "CP15 MCR/MRC";
                break;

            case 4:
                ec_string = "CP15 MCRR/MRRC";
                break;

            case 5:
                ec_string = "CP14 MCR/MRC";
                break;

            case 6:
                ec_string = "CP14 LDC/STC";
                break;

            case 7:
                ec_string = "ASIMD";
                break;

            case 8:
                ec_string = "CP10 MRC/VMRS";
                break;

            case 0xC:
                ec_string = "CP14 MCRR/MRRC";
                break;

            case 0xE:
                ec_string = "PSTATE.IL";
                break;

            case 0x11:
                ec_string = "SVC (AArch32)";
                break;

            case 0x12:
                ec_string = "HVC (AArch32)";
                break;

            case 0x13:
                ec_string = "SMC (AArch32)";
                break;

            case 0x15:
                ec_string = "SVC (AArch64)";
                break;

            case 0x16:
                ec_string = "HVC (AArch64)";
                break;

            case 0x17:
                ec_string = "SMC (AArch64)";
                break;

            case 0x18:
                ec_string = "MSR/MRS (AArch64)";
                break;

            case 0x19:
                ec_string = "SVE";
                break;

            case 0x1f:
                ec_string = "EL3 IMP DEF";
                break;

            case 0x20:
                ec_string = "IABT (lower EL)";
                break;

            case 0x21:
                ec_string = "IABT (current EL)";
                break;

            case 0x22:
                ec_string = "PC Alignment";
                break;

            case 0x24:
                ec_string = "DABT (lower EL)";
                break;

            case 0x25:
                ec_string = "DABT (current EL)";
                break;

            case 0x26:
                ec_string = "SP Alignment";
                break;

            case 0x28:
                ec_string = "FP (AArch32)";
                break;

            case 0x2C:
                ec_string = "FP (AArch64)";
                break;

            case 0x2F:
                ec_string = "SError";
                break;

            case 0x30:
                ec_string = "Breakpoint (lower EL)";
                break;

            case 0x31:
                ec_string = "Breakpoint (current EL)";
                break;

            case 0x32:
                ec_string = "Software Step (lower EL)";
                break;

            case 0x33:
                ec_string = "Software Step (current EL)";
                break;

            case 0x34:
                ec_string = "Watchpoint (lower EL)";
                break;

            case 0x35:
                ec_string = "Watchpoint (current EL)";
                break;

            case 0x38:
                ec_string = "BKPT (AArch32)";
                break;

            case 0x3A:
                ec_string = "Vector catch (AArch32)";
                break;

            case 0x3C:
                ec_string = "BRK (AArch64)";
                break;
        }
        
        switch (ifsc)
        {
            case 0b000000:
                dabt_string = "Address size fault in TTBR0 or TTBR1.";
                break;

            case 0b000101:
                dabt_string = "Translation fault, 1st level.";
                break;

            case 0b000110:
                dabt_string = "Translation fault, 2nd level.";
                break;

            case 0b000111:
                dabt_string = "Translation fault, 3rd level.";
                break;

            case 0b001001:
                dabt_string = "Access flag fault, 1st level.";
                break;

            case 0b001010:
                dabt_string = "Access flag fault, 2nd level.";
                break;

            case 0b001011:
                dabt_string = "Access flag fault, 3rd level.";
                break;

            case 0b001101:
                dabt_string = "Permission fault, 1st level.";
                break;

            case 0b001110:
                dabt_string = "Permission fault, 2nd level.";
                break;

            case 0b001111:
                dabt_string = "Permission fault, 3rd level.";
                break;

            case 0b010000:
                dabt_string = "Synchronous external abort.";
                break;

            case 0b011000:
                dabt_string = "Synchronous parity error on memory access.";
                break;

            case 0b010101:
                dabt_string = "Synchronous external abort on translation table walk, 1st level.";
                break;

            case 0b010110:
                dabt_string = "Synchronous external abort on translation table walk, 2nd level.";
                break;

            case 0b010111:
                dabt_string = "Synchronous external abort on translation table walk, 3rd level.";
                break;

            case 0b011101:
                dabt_string = "Synchronous parity error on memory access on translation table walk, 1st level.";
                break;

            case 0b011110:
                dabt_string = "Synchronous parity error on memory access on translation table walk, 2nd level.";
                break;

            case 0b011111:
                dabt_string = "Synchronous parity error on memory access on translation table walk, 3rd level.";
                break;

            case 0b100001:
                dabt_string = "Alignment fault.";
                break;

            case 0b100010:;
                dabt_string = "Debug event.";
                break;
        }
    
        bool is_dabt = false;
        if (ec != 0x24 && ec != 0x25)
        {
            uart_debug_printf("Exception occurred: %s\r\n", ec_string);
        }
        else
        {
            is_dabt = true;
            uart_debug_printf("Exception occurred: %s (%s)\r\n", ec_string, dabt_string);
        }

        uart_debug_printf("x0  %016llx x1  %016llx x2  %016llx x3  %016llx \r\n", context[0], context[1], context[2], context[3]);
        uart_debug_printf("x4  %016llx x5  %016llx x6  %016llx x7  %016llx \r\n", context[4], context[5], context[6], context[7]);
        uart_debug_printf("x8  %016llx x9  %016llx x10 %016llx x11 %016llx \r\n", context[8], context[9], context[10], context[11]);
        uart_debug_printf("x12 %016llx x13 %016llx x14 %016llx x15 %016llx \r\n", context[12], context[13], context[14], context[15]);
        uart_debug_printf("x16 %016llx x17 %016llx x18 %016llx x19 %016llx \r\n", context[16], context[17], context[18], context[19]);
        uart_debug_printf("x20 %016llx x21 %016llx x22 %016llx x23 %016llx \r\n", context[20], context[21], context[22], context[23]);
        uart_debug_printf("x24 %016llx x25 %016llx x26 %016llx x27 %016llx \r\n", context[24], context[25], context[26], context[27]);
        uart_debug_printf("x28 %016llx\r\n", context[28]);
        uart_debug_printf("sp  %016llx lr  %016llx pc  %016llx\r\n\r\n", context[29], context[30], context[32]-(is_dabt ? 4 : 0));
        uart_debug_printf("sp_el0 %016llx spsr_el1 %016llx tpidr %016llx\r\n", context[31], context[33], context[34]);
        uart_debug_printf("esr    %016llx afsr0    %016llx afsr1 %016llx\r\n", get_esr_el1(), get_afsr0_el1(), get_afsr1_el1());
        
        return context;
    }
}
