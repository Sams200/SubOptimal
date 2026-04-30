# SubOptimal

A fast, lightweight application for generating subtitles using whisper.cpp. Local translation using CTranslate2.

## Features

- **Fast transcription** - GPU-accelerated via CUDA
- **Minimal footprint** - Statically linked whisper.cpp and ggml for portability
- **SRT output** - Standard subtitle format ready for use
- **Translation** - Local text translation using Helsinki-NLB and Facebook NLLB
- **Validation** - Subtitle validation through Ollama
- **GUI** - QT6 widgets

## Requirements

- Linux with CUDA-capable GPU
- NVIDIA drivers and CUDA toolkit
- CMake 3.10+
- FFmpeg
- Curl
- QT6
- OpenMP

## Building

```bash
# Clone the repository (includes whisper.cpp as submodule)
git clone --recursive https://github.com/Sams200/SubOptimal.git
cd SubOptimal

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

**Important:** Use `--recursive` when cloning to pull the whisper.cpp submodule. If you already cloned without it:
```bash
git submodule update --init --recursive
```

## Usage

### Basic Usage

```bash
./sub_optimal_app --headless -s input.mp4 -o output.srt -m base.en --translate eng_Latn -Q gemma4:31b-cloud
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
| `-l`, `--language` | (Optional) Input source language |
| `-Q`, `--ollama-model` | Ollama model for enabling validation |
| `-U`, `--ollama-host` | (Optional) URL for ollama server |

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

## Running validation through Ollama
1. Run ```ollama serve``` or start the ollama service
2. If using a cloud model, log into your account by running ```ollama login```, otherwise download a model using ```ollama pull <model>```
3. Select an appropriate model from the [models list](https://ollama.com/search). Good models should focus on language processing and have reduced size.

## Architecture Notes

- Audio is extracted from video using FFmpeg
- Transcription and translation models are downloaded automatically on first run
- CUDA acceleration is enabled by default for GPU inference
- Static linking ensures portability across systems