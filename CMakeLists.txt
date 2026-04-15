cmake_minimum_required(VERSION 3.10)
project(SubOptimal C CXX)

set(CMAKE_C_STANDARD 23)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# link whisper statically
set(WHISPER_SHARED OFF CACHE BOOL "" FORCE)
set(GGML_SHARED OFF CACHE BOOL "" FORCE)
set(GGML_CUDA ON CACHE BOOL "" FORCE)
add_subdirectory(whisper.cpp)

# link ctranslate2 static as well
set(CT2_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CT2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CT2_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(WITH_MKL OFF CACHE BOOL "" FORCE)
set(WITH_OPENBLAS OFF CACHE BOOL "" FORCE)
set(WITH_CUDA ON CACHE BOOL "" FORCE)
set(BUILD_CLI OFF CACHE BOOL "" FORCE)
add_subdirectory(ctranslate2)

# sentencepiece
set(SPM_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(sentencepiece)

# curl for Ollama API
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

add_executable(suboptimal
    src/main.c
    src/cli.c
    src/cli.h
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
    src/subtitles.h
    src/context_check.c
    src/validator.c
    src/validator.h
    src/cli_ui.c
    src/cli_ui.h
    src/subtitles.c
    cjson/cJSON.c
    cjson/cJSON.h
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET suboptimal POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:suboptimal>)
endif()

target_link_libraries(suboptimal PRIVATE
    whisper
    ctranslate2
    sentencepiece-static
    ${CURL_LIBRARIES}
)

target_include_directories(suboptimal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ctranslate2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sentencepiece/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/cjson
)
