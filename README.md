# llvm-instr
Instrumentation pass that uses regular expressions to find targets

## Building
- Make sure LLVM is installed either from source or using a package manager
- In the root project directory, run `cmake .`
- Run `make`
- The compiled pass is *libLLVMInstr.so*

## Notes
- genlog.c uses the posix threads library. It must be compiled and linked to the instrumented program with the `-pthread` flag
