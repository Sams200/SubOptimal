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
        src/ui/MainWindow.cpp
        src/ui/PipelineWorker.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    )
endif()
