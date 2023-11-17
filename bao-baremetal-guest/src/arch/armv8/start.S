/** 
 * Bao, a Lightweight Static Partitioning Hypervisor 
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *      Sandro Pinto <sandro.pinto@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details. 
 *
 */

#include "sysregs.h"

#define STACK_SIZE  0x4000

.section .start, "ax"
.global _start
_start:
    mrs x0, MPIDR_EL1
    and x0, x0, MPIDR_CPU_MASK

    adr x1, _exception_vector
    msr	VBAR_EL1, x1

    ldr x1, =MAIR_EL1_DFLT
    msr	MAIR_EL1, x1

    ldr x1, =0x0000000000802510
    msr TCR_EL1, x1

    adr x1, root_page_table
    msr TTBR0_EL1, x1

    // Enable floating point
    mov x1, #(3 << 20)
    msr CPACR_EL1, x1

    //TODO: invalidate caches, bp, .. ?

    tlbi	vmalle1
	dsb	nsh
	isb

    ldr x1, =(SCTLR_RES1 | SCTLR_M | SCTLR_C | SCTLR_I)
    msr SCTLR_EL1, x1

    tlbi	vmalle1
	dsb	nsh
	isb

    cbnz x0, 1f

    ldr x16, =__bss_start 
    ldr x17, =__bss_end   
    bl  clear

    .pushsection .data
    .align 3
wait_flag:
    .dword 0x0
    .popsection

    adr x1, wait_flag
    mov x2, #1
    str x2, [x1]

1:
    adr x1, wait_flag
    ldr x2, [x1]
    cbz x2, 1b

    mov x3, #SPSel_SP							
	msr SPSEL, x3	

    adr x1, _stack_base
    ldr x2, =STACK_SIZE
    add x1, x1, x2
#ifndef SINGLE_CORE
    madd x1, x0, x2, x1
#endif
    mov sp, x1
   
    //TODO: other c runtime init (ctors, etc...)

    b _init
    b _exit

.global psci_wake_up
psci_wake_up:
    b .

 .func clear
clear:
2:
	cmp	x16, x17			
	b.ge 1f				
	str	xzr, [x16], #8	
	b	2b				
1:
	ret
.endfunc
