#!/bin/bash
set -e

BUILD_DIR="cmake-asr-eval-build"
MODEL="${1:-$HOME/.local/share/SubOptimal/ggml-large-v3-turbo.bin}"
VAD="${2:-$HOME/.local/share/SubOptimal/ggml-silero-v6.2.0.bin}"
LANG="${3:-en}"

mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

cmake .. \
    -DBUILD_EVALUATION_TOOLS=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build . --target suboptimal-eval -j$(nproc)

cd ..
if [ ! -f "eval/librispeech_test_clean_manifest.tsv" ]; then
    echo "Downloading LibriSpeech..."
    python3 eval/asr/download_librispeech.py \
        --output-dir eval/asr \
        --manifest eval/librispeech_test_clean_manifest.tsv
fi

cd "$BUILD_DIR"
./suboptimal-eval \
    "$MODEL" \
    "$VAD" \
    ../eval/librispeech_test_clean_manifest.tsv \
    "$LANG"