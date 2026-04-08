cmake_minimum_required(VERSION 3.10)
project(SubOptimal C CXX)

set(CMAKE_C_STANDARD 23)

# link whisper statically
set(WHISPER_SHARED OFF CACHE BOOL "" FORCE)
set(GGML_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# enable CUDA GPU acceleration
set(GGML_CUDA ON CACHE BOOL "" FORCE)
add_subdirectory(whisper.cpp)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

# link ctranslate2 static as well
set(CT2_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CT2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CT2_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(WITH_MKL OFF CACHE BOOL "" FORCE)
set(WITH_OPENBLAS OFF CACHE BOOL "" FORCE)
set(WITH_CUDA ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_CLI OFF CACHE BOOL "" FORCE)
add_subdirectory(ctranslate2)

# sentencepiece
add_subdirectory(sentencepiece)
set(SPM_ENABLE_SHARED OFF CACHE BOOL "" FORCE)

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
src/translate.cpp
src/translate.h
src/post_processing.c
src/post_processing.h
src/subtitles.h


)

target_link_libraries(sub_optimal_app PRIVATE
    whisper
    ctranslate2
    sentencepiece-static
)

target_include_directories(sub_optimal_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ctranslate2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sentencepiece/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)