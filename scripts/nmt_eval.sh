#!/bin/bash
set -e

BUILD_DIR="cmake-build-nmt-eval"

MODEL_DIR="${1:-$HOME/.local/share/SubOptimal/nllb-200-distilled-600M-ct2}"
SPM_MODEL="${2:-$HOME/.local/share/SubOptimal/nllb-200-distilled-600M-ct2/sentencepiece.bpe.model}"
SRC_LANG="${3:-eng_Latn}"
TGT_LANG="${4:-spa_Latn}"
MAX_LINES="${5:-0}"

mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

cmake .. \
    -DBUILD_NMT_EVALUATION=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DSOURCE_LANG="$SRC_LANG" \
    -DTARGET_LANG="$TGT_LANG" \
    -DFLORES_MAX_LINES="$MAX_LINES"

cmake --build . --target suboptimal-eval-nmt -j$(nproc)

cmake --build . --target download-flores

./suboptimal-eval-nmt \
    "$MODEL_DIR" \
    "$SPM_MODEL" \
    ../eval/nmt/flores_manifest.tsv \
    "$MAX_LINES"