cmake_minimum_required(VERSION 3.10)
project(ThesisProject C CXX)

set(CMAKE_C_STANDARD 23)

add_subdirectory(whisper.cpp)

# embed the model
add_custom_command(
    OUTPUT model.o
    COMMAND ld -r -b binary -o ${CMAKE_CURRENT_BINARY_DIR}/model.o ggml-base.en.bin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/models
    DEPENDS ${CMAKE_SOURCE_DIR}/models/ggml-base.en.bin
    COMMENT "Embedding model into binary object"
)

add_library(embedded_model STATIC model.o)
set_target_properties(embedded_model PROPERTIES LINKER_LANGUAGE C)

add_executable(thesis_app
    src/main.c
    src/loader.c
    src/loader.h
)

target_link_libraries(thesis_app PRIVATE whisper embedded_model)

target_include_directories(thesis_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)