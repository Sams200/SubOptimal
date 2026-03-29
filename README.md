# SubOptimal

A fast, lightweight application for generating subtitles using whisper.cpp. Local translation using CTranslate2.

## Features

- **Fast transcription** - GPU-accelerated via CUDA
- **Minimal footprint** - Statically linked whisper.cpp and ggml for portability
- **SRT output** - Standard subtitle format ready for use
- **Translation** - Local text translation using Helsinki-NLB and Facebook NLLB

## Requirements

- Linux with CUDA-capable GPU
- NVIDIA drivers and CUDA toolkit
- CMake 3.10+
- FFmpeg
- Curl

## Building

```bash
# Clone the repository (includes whisper.cpp as submodule)
git clone --recursive https://github.com/Sams200/SubOptimal.git
cd SubOptimal

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**Important:** Use `--recursive` when cloning to pull the whisper.cpp submodule. If you already cloned without it:
```bash
git submodule update --init --recursive
```

## Usage

### Basic Usage

```bash
./sub_optimal_app --headless -s input.mp4 -o output.srt -m ggml-base.en.bin
```

### Command-Line Options

| Option | Description |
|--------|-------------|
| `-H`, `--headless` | Run in headless (no-GUI) mode |
| `-s`, `--source`   | Path to input video file (required) |
| `-o`, `--output`   | Path to output SRT file |
| `-m`, `--model`    | Whisper.cpp model to use |
| `-c`, `--config`   | Path to YAML config file |
| `-t`, `--translate` | Language to translate into |
| `-l`, `--language` | Input source language |

### Available Models

| Model | Size | Speed | Accuracy |
|-------|------|-------|----------|
| `tiny` | 75 MB | Fastest | Basic |
| `base` | 142 MB | Fast | Good |
| `small` | 466 MB | Medium | Better |
| `medium` | 1.5 GB | Medium | High |
| `turbo` | 1.5 GB | Large | Highest |

English-only models (`.en`) are more performant for English audio.

## Configuration

SubOptimal stores configuration and models in `~/.local/share/SubOptimal/`.

### Config File (`~/.local/share/SubOptimal/config.yaml`)

```yaml
# SubOptimal configuration
model:  ggml-base.en.bin
source: ""
output: transcript.srt
```

Configuration lookup order:
1. `~/.local/share/SubOptimal/config.yaml` (or path from `--config`)
2. Command-line flags (override config values)

## Architecture Notes

- Audio is extracted from video using FFmpeg
- Models are downloaded automatically on first run
- CUDA acceleration is enabled by default for GPU inference
- Static linking ensures portability across systems
- Translation uses Helsinki-NLB models when available