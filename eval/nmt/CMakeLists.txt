find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(FLORES_DATA_DIR ${CMAKE_SOURCE_DIR}/eval/nmt/data)
set(FLORES_MANIFEST ${CMAKE_SOURCE_DIR}/eval/nmt/flores_manifest.tsv)
set(FLORES_MAX_LINES 100 CACHE STRING "Max sentences to evaluate (0=all)")
set(FLORES_REPO_ID "openlanguagedata/flores_plus" CACHE STRING "HuggingFace repo ID")

# Language pair for Flores
set(SOURCE_LANG eng_Latn CACHE STRING "Source language (Flores-200 code)")
set(TARGET_LANG spa_Latn CACHE STRING "Target language (Flores-200 code)")

add_custom_target(download-flores
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/download_flores.py
            --output-dir ${FLORES_DATA_DIR}
            --source-lang ${SOURCE_LANG}
            --target-lang ${TARGET_LANG}
            --split devtest
            --manifest ${FLORES_MANIFEST}
            --max-lines ${FLORES_MAX_LINES}
            --repo-id ${FLORES_REPO_ID}
    COMMENT "Downloading Flores+ dataset (${SOURCE_LANG} -> ${TARGET_LANG}) from HuggingFace..."
    VERBATIM
)

add_executable(suboptimal-eval-nmt
    evaluate_nmt.cpp
    ${CMAKE_SOURCE_DIR}/src/pipeline/translate.cpp
    ${CMAKE_SOURCE_DIR}/src/pipeline/cancel.c
    ${CMAKE_SOURCE_DIR}/src/subtitles/subtitles.c
)

add_dependencies(suboptimal-eval-nmt ctranslate2 sentencepiece-static)

target_include_directories(suboptimal-eval-nmt PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/whisper.cpp/include
    ${CMAKE_SOURCE_DIR}/ctranslate2/include
    ${CMAKE_SOURCE_DIR}/sentencepiece/src
)

target_link_libraries(suboptimal-eval-nmt PRIVATE
    ctranslate2
    sentencepiece-static
    ${CMAKE_THREAD_LIBS_INIT}
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(suboptimal-eval-nmt PRIVATE OpenMP::OpenMP_CXX)
endif()

set_target_properties(suboptimal-eval-nmt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run-nmt-eval
    COMMAND ${CMAKE_BINARY_DIR}/suboptimal-eval-nmt
            "$ENV{NMT_MODEL_DIR}"
            "$ENV{NMT_SPM_MODEL}"
            ${FLORES_MANIFEST}
            ${FLORES_MAX_LINES}
    DEPENDS suboptimal-eval-nmt download-flores
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running NMT evaluation..."
    VERBATIM
)

add_custom_command(TARGET suboptimal-eval-nmt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Built suboptimal-eval-nmt"
    COMMAND ${CMAKE_COMMAND} -E echo "Usage: ./suboptimal-eval-nmt <model_dir> <spm.model> <manifest> [max_lines]"
    VERBATIM
)