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
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define X86_TRACE_STACK_SIZE_PASS_NAME "X86 Trace Stack Calculator Pass"

using namespace llvm;
using namespace tracing;

char X86TraceStackSizeCalculator::ID = 0;

X86TraceStackSizeCalculator::X86TraceStackSizeCalculator() : MachineFunctionPass(ID) {
 initializeX86TraceStackSizeCalculatorPass(*PassRegistry::getPassRegistry());
}

StringRef X86TraceStackSizeCalculator::getPassName() const {
  return X86_TRACE_STACK_SIZE_PASS_NAME;
}

bool X86TraceStackSizeCalculator::runOnMachineFunction(MachineFunction &mfun) {
  if (mfun.getName() == "trace" || mfun.getName() == "__unopt_trace") {
    uint64_t stack_size = mfun.getFrameInfo().getStackSize();
    int fd = open("/tmp/__trace_stack_size.txt", O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
      errs() << "Failed to open stack size file\n";
      return false;
    }
    llvm::raw_fd_ostream out(fd, true);
    auto ret = MemoryBuffer::getOpenFile(fd, "/tmp/__trace_stack_size.txt", -1);
    if (!ret) {
      errs() << "Failed to map stack size file\n";
      return false;
    }
    StringRef data = ret.get()->getBuffer();
    uint64_t trace_stack_size = 0;
    if (data.empty() || data.trim().getAsInteger(10, trace_stack_size)) {
      out << stack_size;
    } else {
      if (stack_size > trace_stack_size) {
        out << stack_size;
      }
    }
  }
  return false;
}

INITIALIZE_PASS(X86TraceStackSizeCalculator, "x86-trace-stack-calculator",
                X86_TRACE_STACK_SIZE_PASS_NAME,
                true,  // CFG
                true  // analysis
                )

namespace llvm {
FunctionPass *createX86TraceStackSizeCalculator() {
  return new X86TraceStackSizeCalculator();
}
}
