// Copyright (C) 2024 Verifoxx Limited
// Calls ASM to switch to compartment from capability manager, or capability manager service from compartment

#include <signal.h>
#include "comp_caller.h"

uintptr_t CompartmentCaller(CompEntryAsmFnPtr switcher_fp, void *comp_data, void* target_fp, void* sealed_arg_data, void* sealer_cap)
{
    /* NOTE: For some reason, gcc is optimising out the assembler leaving us with the wrong
    registers set up on call, hence all of the below which seems overkill... */

    //raise(SIGTRAP);

    register volatile uintptr_t c0 asm("c0") = comp_data;
    register volatile uintptr_t c1 asm("c1") = target_fp;
    register volatile uintptr_t c2 asm("c2") = sealed_arg_data;
    register volatile uintptr_t c3 asm("c3") = sealer_cap;

#if 0
    // Worst case, the mov operations are a nop
    asm __volatile__("mov c0, %[c0]\n"
        "mov c1, %[c1]\n"
        "mov c2, %[c2]\n"
        "mov c3, %[c3]\n"
        "blr %[fn]"
        : "+C"(c0)
        : [c0] "C"(c0), [c1]"C"(c1), [c2]"C"(c2), [c3]"C"(c3), [fn]"C"(switcher_fp)
        // Callee-saved registers are not preserved by the compartment switcher, so mark all of them
        // as clobbered to get the compiler to save and restore them.
        // Also mark FP and LR as clobbered, because we are effectively making a function call and
        // therefore the compiler should create a frame record.
        // Note that FP is not actually clobbered, because CompartmentSwitch() does preserve FP.
        : "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30"
        /* Also include all of the input registers in both c and x formats */
        //"c7", "x0", "x1", "x2", "x3", "x4", "x5", "x6"
    );

#else
    // Call Asm

    asm("blr %[fn]"
        : "+C"(c0)
        : [fn] "C"(switcher_fp), "C"(c1), "C"(c2), "C"(c3)
            // Callee-saved registers are not preserved by the compartment switcher, so mark all of them
            // as clobbered to get the compiler to save and restore them.
            // Also mark FP and LR as clobbered, because we are effectively making a function call and
            // therefore the compiler should create a frame record.
            // Note that FP is not actually clobbered, because CompartmentEntry() does preserve FP.
            : "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "c29", "c30");
#endif
    return c0;
}

