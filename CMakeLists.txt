cmake_minimum_required(VERSION 3.10)
project(SubOptimal C CXX)

set(CMAKE_C_STANDARD 23)

# link whisper and ggml statically
set(WHISPER_SHARED OFF CACHE BOOL "" FORCE)
set(GGML_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# enable CUDA GPU acceleration
set(GGML_CUDA ON CACHE BOOL "" FORCE)
add_subdirectory(whisper.cpp)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

add_executable(sub_optimal_app
    src/main.c
    src/loader.c
    src/loader.h
src/parser.c
src/parser.h
src/defaults.h
src/transcribe.c
src/transcribe.h
src/file_helpers.c
src/file_helpers.h
)

target_link_libraries(sub_optimal_app PRIVATE whisper)

target_include_directories(sub_optimal_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)