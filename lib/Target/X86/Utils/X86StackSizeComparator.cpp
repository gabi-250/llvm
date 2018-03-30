#include "X86StackSizeComparator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/raw_ostream.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

namespace llvm {

std::string getRealName(StringRef name) {
  if (name.startswith(UNOPT_PREFIX)) {
    return name.drop_front(StringRef(UNOPT_PREFIX).size()).str();
  }
  return name.str();
}

uint64_t getStoredStackSize(StringRef funName, uint64_t stackSize) {
  int fd = open((TEMP_PREFIX + getRealName(funName)).c_str(),
                O_RDWR | O_CREAT, 0666);
  if (fd == -1) {
    errs() << "Failed to open stack size file\n";
    return false;
  }
  llvm::raw_fd_ostream out(fd, true);
  auto ret =
    MemoryBuffer::getOpenFile(fd, TEMP_PREFIX + getRealName(funName), -1);
  if (ret) {
    StringRef data = ret.get()->getBuffer();
    uint64_t storedStackSize = 0;
    if (data.empty() || data.trim().getAsInteger(10, storedStackSize)
        || storedStackSize < stackSize) {
      out << stackSize;
      return stackSize;
    } else {
      return storedStackSize;
    }
  } else {
    errs() << "Failed to create memory buffer\n";
  }
  return stackSize;
}

}
