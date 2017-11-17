cmake_minimum_required(VERSION 3.8)
project(debugger)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp debugger.h debugger.cpp utils.h utils.cpp linenoise.c linenoise.h breakpoint.h breakpoint.cpp reg.h reg.cpp)
add_executable(debugger ${SOURCE_FILES})
add_custom_target(
        libelfin
        COMMAND make
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libelfin
)

target_link_libraries(debugger
        /home/darmao/CLionProjects/debugger/libelfin/dwarf/libdwarf++.so
        /home/darmao/CLionProjects/debugger/libelfin/elf/libelf++.so)

add_dependencies(debugger libelfin)