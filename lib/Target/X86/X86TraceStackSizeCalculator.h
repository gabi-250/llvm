#ifndef TRACE_STACK_SIZE_CALCULATOR_H
#define TRACE_STACK_SIZE_CALCULATOR_H

#include "X86.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Pass.h"
#define X86_TRACE_STACK_SIZE_PASS_NAME "X86 Trace Stack Calculator Pass"

using namespace llvm;

namespace tracing {

  struct X86TraceStackSizeCalculator: public MachineFunctionPass {
    static char ID;

    X86TraceStackSizeCalculator();
    virtual bool runOnMachineFunction(MachineFunction &mfun) override;
    StringRef getPassName() const override;
  };
}

#endif // TRACE_STACK_SIZE_CALCULATOR_H
