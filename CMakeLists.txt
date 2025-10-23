cmake_minimum_required(VERSION 3.20)
project(Rivet LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(RIVET_ENABLE_ASAN "Enable AddressSanitizer in Debug builds" ON)

# Warnings
if (MSVC)
  add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

# ---- Sources (declare the target first!) ----
add_executable(rvt
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
)

# Include paths for this target
target_include_directories(rvt PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Put the binary in the build/ root so you can run ./build/rvt
set_target_properties(rvt PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# ASan in Debug (gcc/clang)
if (RIVET_ENABLE_ASAN AND CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(rvt PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(rvt PRIVATE    -fsanitize=address -fno-omit-frame-pointer)
endif()