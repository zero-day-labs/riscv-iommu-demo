#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <sysregs.h>

static inline uint64_t get_cpuid(){
    uint64_t cpuid = MRS(MPIDR_EL1);
    return cpuid & MPIDR_CPU_MASK;
}

static bool cpu_is_master() {
    return get_cpuid() == 0;
}

#endif
