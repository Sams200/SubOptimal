find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(ASR_DATA_DIR ${CMAKE_SOURCE_DIR}/eval/asr)
set(LIBRI_MANIFEST ${CMAKE_SOURCE_DIR}/eval/librispeech_test_clean_manifest.tsv)

add_custom_target(download-librispeech
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/download_librispeech.py
            --output-dir ${ASR_DATA_DIR}
            --manifest ${LIBRI_MANIFEST}
    COMMENT "Checking/Downloading LibriSpeech test-clean dataset..."
    VERBATIM
)

set(TRANSCRIBE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/pipeline/transcribe.c
    ${CMAKE_SOURCE_DIR}/src/pipeline/cancel.c
    ${CMAKE_SOURCE_DIR}/src/subtitles/subtitles.c
    ${CMAKE_SOURCE_DIR}/src/pipeline/loader.c
)

add_executable(suboptimal-eval
    evaluate_asr.c
    wer.c
    ${TRANSCRIBE_SOURCES}
)

add_dependencies(suboptimal-eval whisper)

target_include_directories(suboptimal-eval PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_SOURCE_DIR}/ctranslate2/include
    ${CMAKE_SOURCE_DIR}/drwav  # If drwav is needed by loader.c
)

target_link_libraries(suboptimal-eval PRIVATE
    whisper
    ggml
    ${CMAKE_THREAD_LIBS_INIT}
    m
)

if(OpenMP_C_FOUND)
    target_link_libraries(suboptimal-eval PRIVATE OpenMP::OpenMP_C)
endif()

set_target_properties(suboptimal-eval PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run-asr-eval
    COMMAND ${CMAKE_BINARY_DIR}/suboptimal-eval
            "$ENV{WHISPER_MODEL}"
            "$ENV{VAD_MODEL}"
            ${LIBRI_MANIFEST}
            "$ENV{EVAL_LANG}"
    DEPENDS suboptimal-eval download-librispeech
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running ASR evaluation..."
    VERBATIM
)

add_custom_command(TARGET suboptimal-eval POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Built suboptimal-eval at ${CMAKE_BINARY_DIR}/suboptimal-eval"
    COMMAND ${CMAKE_COMMAND} -E echo "Usage: ./suboptimal-eval <whisper-model> <vad-model> <manifest> [language]"
    VERBATIM
)