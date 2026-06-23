cmake_minimum_required(VERSION 3.20)
project(fast-linker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(TBB REQUIRED)
find_package(mimalloc REQUIRED)

add_executable(fast-linker
    src/main.cpp
    src/linker.cpp
    src/macho.cpp
    src/util.cpp
)

target_link_libraries(fast-linker
    PRIVATE
    TBB::tbb
    mimalloc
)

target_compile_options(fast-linker PRIVATE
    -O3
    -march=native
    -flto
    -Wall
    -Wextra
)
