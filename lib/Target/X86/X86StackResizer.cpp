#include "X86.h"
#include "X86TraceStackSizeCalculator.h"
#include "X86InstrInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorOr.h"
#define X86_STACK_RESIZER_PASS_NAME "X86 Stack Resizer Pass"

using namespace llvm;

namespace tracing {

  struct X86StackResizer: public MachineFunctionPass {
    static char ID;

    X86StackResizer() : MachineFunctionPass(ID) {
     initializeX86StackResizerPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnMachineFunction(MachineFunction &mfun) override;

    virtual void getAnalysisUsage(AnalysisUsage &usage) const override;

    StringRef getPassName() const override {
      return X86_STACK_RESIZER_PASS_NAME;
    }
  };
  char X86StackResizer::ID = 0;

  void X86StackResizer::getAnalysisUsage(AnalysisUsage &usage) const {
    usage.addRequired<tracing::X86TraceStackSizeCalculator>();
    MachineFunctionPass::getAnalysisUsage(usage);
  }

  bool X86StackResizer::runOnMachineFunction(MachineFunction &mfun) {
    outs() << "Running StackResizerPass on " << mfun.getName() << '\n';
    if (mfun.getName() == "trace") {
      auto ret = llvm::MemoryBuffer::getFileAsStream(
          "/tmp/__trace_stack_size.txt");
      if (ret) {
        StringRef data = ret.get()->getBuffer();
        uint64_t stack_size = 0;
        if (data.empty() || data.trim().getAsInteger(10, stack_size)) {
          errs() << "Could not set the stack size of trace\n";
        } else {
          mfun.getFrameInfo().setStackSize(stack_size);
        }
      }
    }
    return false;
  }
}

using namespace tracing;
INITIALIZE_PASS(X86StackResizer, "x86-stack-resizer",
                X86_STACK_RESIZER_PASS_NAME,
                false,  // CFG
                false  // analysis
                )

namespace llvm {
FunctionPass *createX86StackResizer() { return new X86StackResizer(); }
}
