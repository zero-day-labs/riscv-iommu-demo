#include <platform.h>
#include <arch/plic.h>

struct platform platform = {

    .cpu_num = 1,

    .region_num = 1,
    .regions =  (struct mem_region[]) {
        {
            .base = 0x80200000,
            .size = 0x40000000 - 0x200000
        }
    },

    .console = {
        .base = 0x10000000,
    },

    .arch = {
        .plic_base = 0xc000000,
        .iommu = {
            .base = 0x50010000,
            .mode = 0x2,
            .fq_irq_id = 10
        }
    }

};
