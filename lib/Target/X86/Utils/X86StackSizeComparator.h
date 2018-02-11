#ifndef LLVM_LIB_TARGET_X86_UTILS_X86STACKSIZECOMPARATOR_H
#define LLVM_LIB_TARGET_X86_UTILS_X86STACKSIZECOMPARATOR_H

#include "llvm/ADT/StringRef.h"
#include <sys/types.h>
#include <string>

#define TMP std::string("/tmp/__stack_resizer_")
#define UNOPT_PREFIX "__unopt_"

namespace llvm {

std::string getRealName(StringRef name);
uint64_t getStoredStackSize(StringRef funName, uint64_t stackSize);

}

#endif
