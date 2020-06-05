find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "-Wall -fno-rtti")
add_subdirectory(Instr)
