#include <config.h>

VM_IMAGE(baremetal_image, XSTR(/path/to/baremetal.bin));

struct config config = {
    
    CONFIG_HEADER

    .vmlist_size = 1,
    .vmlist = {
        {
            .image = {
                .base_addr = 0x90000000,
                .load_addr = VM_IMAGE_OFFSET(baremetal_image),
                .size = VM_IMAGE_SIZE(baremetal_image)
            },

            .entry = 0x90000000,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x90000000,
                        .size = 0x20000000,
                        .place_phys = true,
                        .phys = 0x90000000,
                    }
                },

                .dev_num = 7,
                .devs =  (struct vm_dev_region[]) {
                    {   // UART
                        .pa = 0x10000000,   
                        .va = 0x10000000,  
                        .size = 0x00010000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {1}
                    },
                    {   // Timer
                        .pa = 0x18000000,   
                        .va = 0x18000000,  
                        .size = 0x00001000,  
                        .interrupt_num = 4,
                        .interrupts = (irqid_t[]) {4,5,6,7}
                    },
                    {   // SPI
                        .pa = 0x20000000,   
                        .va = 0x20000000,  
                        .size = 0x00001000,  
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {2}
                    },
                    {   // Ethernet
                        .pa = 0x30000000,   
                        .va = 0x30000000,  
                        .size = 0x00008000,  
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {3}
                    },
                    {   // GPIO
                        .pa = 0x40000000,   
                        .va = 0x40000000,  
                        .size = 0x00010000,  
                        .interrupt_num = 0,
                        .interrupts = (irqid_t[]) {}
                    },
                    {   // iDMA
                        .pa = 0x50000000,
                        .va = 0x50000000,
                        .size = 0x00001000,
                        .interrupt_num = 0,
                        .interrupts = (irqid_t[]) {},
                        .id = 1
                    },
                    {   // IOMMU (demo only)
                        .pa = 0x50010000,   
                        .va = 0x50010000,  
                        .size = 0x00001000,  
                        .interrupt_num = 0,
                        .interrupts = (irqid_t[]) {}
                    },
                },

                .arch = {
                   .plic_base = 0xc000000
                }
            }
        }
    }
};