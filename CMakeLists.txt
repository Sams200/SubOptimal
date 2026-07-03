cmake_minimum_required(VERSION 3.16)
project(SubOptimal C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Check for Qt6 and enable GUI if available
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(CMAKE_AUTORCC ON)
    set(WITH_GUI ON)
else()
    set(WITH_GUI OFF)
    message(WARNING "Qt6 not found. Building without GUI.")
endif()

# Force static linking where possible
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Force CTranslate2 to use GNU OpenMP instead of Intel
set(OPENMP_RUNTIME "COMP" CACHE STRING "OpenMP runtime" FORCE)
set(WITH_MKL OFF CACHE BOOL "" FORCE)

# Ensure GCC OpenMP flags consistently
find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# link whisper statically
set(WHISPER_SHARED OFF CACHE BOOL "" FORCE)
set(GGML_SHARED OFF CACHE BOOL "" FORCE)
set(GGML_CUDA ON CACHE BOOL "" FORCE)
# Force GGML to use OpenMP
set(GGML_OPENMP ON CACHE BOOL "" FORCE)
add_subdirectory(whisper.cpp)

# link ctranslate2 static as well
set(CT2_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CT2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CT2_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CT2_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(WITH_OPENBLAS OFF CACHE BOOL "" FORCE)
set(WITH_CUDA ON CACHE BOOL "" FORCE)
set(WITH_MKL OFF CACHE BOOL "" FORCE)
set(OPENMP_RUNTIME "COMP" CACHE STRING "" FORCE)
add_subdirectory(ctranslate2)

# sentencepiece
set(SPM_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
set(SPM_USE_BUILTIN_PROTOBUF ON CACHE BOOL "" FORCE)
add_subdirectory(sentencepiece)

# curl for Ollama API
find_package(CURL REQUIRED)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib;$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
endif()

add_executable(suboptimal
    src/main.c
    src/defaults.h
    
    src/cli/cli.c
    src/cli/cli.h
    src/cli/parser.c
    src/cli/parser.h

    src/fs/file_helpers.c
    src/fs/file_helpers.h

    src/pipeline/loader.c
    src/pipeline/loader.h
    src/pipeline/transcribe.c
    src/pipeline/transcribe.h
    src/pipeline/translate.cpp
    src/pipeline/translate.h
    src/pipeline/context_check.c
    src/pipeline/context_check.h
    src/pipeline/cancel.c
    src/pipeline/cancel.h

    src/subtitles/subtitles.c
    src/subtitles/subtitles.h
    src/subtitles/validator.c
    src/subtitles/validator.h

    cjson/cJSON.c
    cjson/cJSON.h
src/pipeline/pipeline.c
src/pipeline/pipeline.h
)

if(WITH_GUI)
    target_sources(suboptimal PRIVATE
        src/gui/resources.qrc
        src/gui/gui.cpp
        src/gui/MainWindow.cpp
        src/gui/PipelineWorker.cpp
    )
endif()

# Linking
target_link_libraries(suboptimal PRIVATE
    whisper
    ctranslate2
    sentencepiece-static
    CURL::libcurl
    OpenMP::OpenMP_CXX
    OpenMP::OpenMP_C
)

# Link GNU OpenMP first
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(suboptimal PRIVATE
        gomp
        pthread
    )
endif()

if(WITH_GUI)
    target_link_libraries(suboptimal PRIVATE Qt6::Widgets)
    target_compile_definitions(suboptimal PRIVATE WITH_GUI)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET suboptimal POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:suboptimal>)
endif()

target_include_directories(suboptimal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ctranslate2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sentencepiece/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/cjson
)

if(WITH_GUI)
    target_include_directories(suboptimal PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    )
endif()

option(BUILD_EVALUATION_TOOLS "Build ASR evaluation tools (requires Python3)" OFF)

if(BUILD_EVALUATION_TOOLS)
    # Ensure whisper is built before eval
    if(NOT TARGET whisper)
        message(FATAL_ERROR "whisper target not found")
    endif()
    add_subdirectory(eval/asr)
endif()

option(BUILD_NMT_EVALUATION "Build NMT evaluation tools" OFF)

if(BUILD_NMT_EVALUATION)
    add_subdirectory(eval/nmt)
endif()