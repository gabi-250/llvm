#include "X86.h"
#include "X86InstrInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorOr.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define X86_STACK_RESIZER_PASS_NAME "X86 Stack Resizer Pass"
#define STACK_SIZE_FILE "/tmp/__trace_stack_size.txt"

using namespace llvm;

namespace {

  struct X86StackResizer: public MachineFunctionPass {
    static char ID;

    X86StackResizer() : MachineFunctionPass(ID) {
     initializeX86StackResizerPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnMachineFunction(MachineFunction &mfun) override;

    StringRef getPassName() const override {
      return X86_STACK_RESIZER_PASS_NAME;
    }
  };
  char X86StackResizer::ID = 0;

  bool X86StackResizer::runOnMachineFunction(MachineFunction &mfun) {
    outs() << "Running StackResizerPass on " << mfun.getName() << '\n';
    if (mfun.getName() != "trace" && mfun.getName() != "__unopt_trace") {
      return false;
    }
    uint64_t cur_stack_size = mfun.getFrameInfo().getStackSize();
    int fd = open(STACK_SIZE_FILE, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
      errs() << "Failed to open stack size file\n";
      return false;
    }
    llvm::raw_fd_ostream out(fd, true);
    auto ret = MemoryBuffer::getOpenFile(fd, STACK_SIZE_FILE, -1);
    if (ret) {
      StringRef data = ret.get()->getBuffer();
      uint64_t stack_size = 0;
      if (data.empty() || data.trim().getAsInteger(10, stack_size)
          || stack_size <= cur_stack_size) {
        out << cur_stack_size;
        return true;
      } else if (mfun.getName() == "trace") {
        mfun.getFrameInfo().setStackSize(stack_size);
        for (auto &mbb: mfun) {
          for (auto &minstr: mbb) {
            if (minstr.getOpcode() == X86::SUB64ri8) {
              auto operand = minstr.getOperand(0);
              if (operand.isReg()) {
                MachineOperand &operand = minstr.getOperand(2);
                if (operand.isImm() || operand.isCImm()) {
                  operand.ChangeToImmediate(
                      operand.getImm() + stack_size - cur_stack_size);
                  return true;
                }
              }
            }
          }
        }
      }
    }
    return false;
  }
}

INITIALIZE_PASS(X86StackResizer, "x86-stack-resizer",
                X86_STACK_RESIZER_PASS_NAME,
                false,  // CFG
                false  // analysis
                )

namespace llvm {
FunctionPass *createX86StackResizer() { return new X86StackResizer(); }
}
