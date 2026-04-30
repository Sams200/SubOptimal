cmake_minimum_required(VERSION 3.16)
project(SubOptimal C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Check for Qt6 and enable GUI if available
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(WITH_GUI ON)
else()
    set(WITH_GUI OFF)
    message(WARNING "Qt6 not found. Building without GUI.")
endif()

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
    src/cancel.c
    src/cancel.h
    cjson/cJSON.c
    cjson/cJSON.h
)

if(WITH_GUI)
    target_sources(suboptimal PRIVATE
        src/ui/gui.cpp
        src/ui/gui.h
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/ui/PipelineWorker.cpp
        src/ui/PipelineWorker.h
    )
endif()

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

if(WITH_GUI)
    target_link_libraries(suboptimal PRIVATE Qt6::Widgets)
    target_compile_definitions(suboptimal PRIVATE WITH_GUI)
endif()

target_include_directories(suboptimal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ctranslate2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sentencepiece/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/cjson
)

if(WITH_GUI)
    target_include_directories(suboptimal PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    )
endif()
