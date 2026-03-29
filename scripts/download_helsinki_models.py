#!/usr/bin/env python3
"""
Download and convert Helsinki-NLP opus-mt models to CTranslate2 format.
"""

import os
import subprocess
import sys
from pathlib import Path

# Output directory for converted models
OUTPUT_DIR = Path("/mnt/home2/Models/Thesis/helsinki-models")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# Language pairs to download (source_target format)
# English -> Other languages
EN_TO_LANG = [
    "af",      # Afrikaans
    "ar",      # Arabic
    "az",      # Azerbaijani
    "bg",      # Bulgarian
    "ca",      # Catalan
    "cs",      # Czech
    "cy",      # Welsh
    "da",      # Danish
    "de",      # German
    "el",      # Greek
    "es",      # Spanish
    "et",      # Estonian
    "eu",      # Basque
    "fa",      # Persian
    "fi",      # Finnish
    "fr",      # French
    "gl",      # Galician
    "gu",      # Gujarati
    "he",      # Hebrew
    "hi",      # Hindi
    "hr",      # Croatian
    "hu",      # Hungarian
    "hy",      # Armenian
    "id",      # Indonesian
    "is",      # Icelandic
    "it",      # Italian
    "ja",      # Japanese
    "ka",      # Georgian
    "kk",      # Kazakh
    "km",      # Khmer
    "kn",      # Kannada
    "ko",      # Korean
    "ky",      # Kyrgyz
    "lo",      # Lao
    "lt",      # Lithuanian
    "lv",      # Latvian
    "mk",      # Macedonian
    "ml",      # Malayalam
    "mn",      # Mongolian
    "mr",      # Marathi
    "ms",      # Malay
    "mt",      # Maltese
    "my",      # Myanmar
    "nb",      # Norwegian Bokmål
    "ne",      # Nepali
    "nl",      # Dutch
    "no",      # Norwegian
    "pa",      # Punjabi
    "pl",      # Polish
    "pt",      # Portuguese
    "ro",      # Romanian
    "ru",      # Russian
    "si",      # Sinhala
    "sk",      # Slovak
    "sl",      # Slovenian
    "sq",      # Albanian
    "sr",      # Serbian
    "sv",      # Swedish
    "sw",      # Swahili
    "ta",      # Tamil
    "te",      # Telugu
    "tg",      # Tajik
    "th",      # Thai
    "tl",      # Tagalog
    "tr",      # Turkish
    "tt",      # Tatar
    "uk",      # Ukrainian
    "ur",      # Urdu
    "uz",      # Uzbek
    "vi",      # Vietnamese
    "zh",      # Chinese (Simplified)
]

# Other languages -> English
LANG_TO_EN = [
    "af",      # Afrikaans
    "ar",      # Arabic
    "az",      # Azerbaijani
    "bg",      # Bulgarian
    "ca",      # Catalan
    "cs",      # Czech
    "cy",      # Welsh
    "da",      # Danish
    "de",      # German
    "el",      # Greek
    "es",      # Spanish
    "et",      # Estonian
    "eu",      # Basque
    "fa",      # Persian
    "fi",      # Finnish
    "fr",      # French
    "gl",      # Galician
    "gu",      # Gujarati
    "he",      # Hebrew
    "hi",      # Hindi
    "hr",      # Croatian
    "hu",      # Hungarian
    "hy",      # Armenian
    "id",      # Indonesian
    "is",      # Icelandic
    "it",      # Italian
    "ja",      # Japanese
    "ka",      # Georgian
    "kk",      # Kazakh
    "km",      # Khmer
    "kn",      # Kannada
    "ko",      # Korean
    "ky",      # Kyrgyz
    "lo",      # Lao
    "lt",      # Lithuanian
    "lv",      # Latvian
    "mk",      # Macedonian
    "ml",      # Malayalam
    "mn",      # Mongolian
    "mr",      # Marathi
    "ms",      # Malay
    "mt",      # Maltese
    "my",      # Myanmar
    "nb",      # Norwegian Bokmål
    "ne",      # Nepali
    "nl",      # Dutch
    "no",      # Norwegian
    "pa",      # Punjabi
    "pl",      # Polish
    "pt",      # Portuguese
    "ro",      # Romanian
    "ru",      # Russian
    "si",      # Sinhala
    "sk",      # Slovak
    "sl",      # Slovenian
    "sq",      # Albanian
    "sr",      # Serbian
    "sv",      # Swedish
    "sw",      # Swahili
    "ta",      # Tamil
    "te",      # Telugu
    "tg",      # Tajik
    "th",      # Thai
    "tl",      # Tagalog
    "tr",      # Turkish
    "tt",      # Tatar
    "uk",      # Ukrainian
    "ur",      # Urdu
    "uz",      # Uzbek
    "vi",      # Vietnamese
    "zh",      # Chinese (Simplified)
]


def download_and_convert(source: str, target: str) -> bool:
    """Download a Helsinki model and convert it to CTranslate2 format."""
    model_name = f"Helsinki-NLP/opus-mt-{source}-{target}"
    output_subdir = OUTPUT_DIR / f"opus-mt-{source}-{target}"

    print(f"\n{'='*60}")
    print(f"Processing: {model_name}")
    print(f"Output: {output_subdir}")
    print(f"{'='*60}")

    # Skip if already converted
    model_bin = output_subdir / "model.bin"
    if model_bin.exists():
        print(f"  Already converted ({model_bin})")
        return True

    # Create output directory
    output_subdir.mkdir(parents=True, exist_ok=True)

    # Download model using huggingface_hub Python API
    print(f"  Downloading...")
    try:
        from huggingface_hub import snapshot_download
        snapshot_download(
            repo_id=model_name,
            local_dir=str(output_subdir),
            local_dir_use_symlinks=False
        )
    except Exception as e:
        print(f"  ERROR: Download failed: {e}")
        return False

    # Convert to CTranslate2 format
    print(f"  Converting to CTranslate2...")
    try:
        result = subprocess.run([
            "ct2-transformers-converter",
            f"--model={output_subdir}",
            f"--output_dir={output_subdir}",
            "--force"
        ], capture_output=True, text=True)

        if result.returncode != 0:
            print(f"  ERROR: Conversion failed")
            if result.stdout:
                print(f"  STDOUT: {result.stdout}")
            if result.stderr:
                print(f"  STDERR: {result.stderr}")
            return False

        print(f"  SUCCESS: Model converted")
        return True

    except FileNotFoundError:
        return False


def main():
    successful = []
    failed = []
    skipped = []

    # English -> Other languages
    print("\n" + "="*60)
    print("English -> Other Languages")
    print("="*60)
    for lang in EN_TO_LANG:
        if download_and_convert("en", lang):
            if (OUTPUT_DIR / f"opus-mt-en-{lang}" / "model.bin").exists():
                successful.append(f"en-{lang}")
            else:
                failed.append(f"en-{lang}")
        else:
            failed.append(f"en-{lang}")

    # Other languages -> English
    print("\n" + "="*60)
    print("Other Languages -> English")
    print("="*60)
    for lang in LANG_TO_EN:
        if download_and_convert(lang, "en"):
            if (OUTPUT_DIR / f"opus-mt-{lang}-en" / "model.bin").exists():
                successful.append(f"{lang}-en")
            else:
                failed.append(f"{lang}-en")
        else:
            failed.append(f"{lang}-en")

    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    print(f"Successful: {len(successful)}")
    print(f"Failed: {len(failed)}")

    if failed:
        print(f"\nFailed models:")
        for model in failed:
            print(f"  - {model}")

    # Create manifest file
    manifest_path = OUTPUT_DIR / "manifest.txt"
    with open(manifest_path, "w") as f:
        f.write("# Helsinki-NLP CTranslate2 Models\n")
        f.write(f"# Generated: {subprocess.run(['date'], capture_output=True, text=True).stdout}")
        f.write(f"\n# Total models: {len(successful)}\n\n")
        for model in sorted(successful):
            f.write(f"opus-mt-{model}\n")

    print(f"\nManifest written to: {manifest_path}")

    return len(failed) == 0


if __name__ == "__main__":
    # Check dependencies
    missing = []

    try:
        import huggingface_hub
    except ImportError:
        missing.append("huggingface_hub")

    try:
        import ctranslate2
    except ImportError:
        missing.append("ctranslate2")

    try:
        import transformers
    except ImportError:
        missing.append("transformers")

    try:
        import torch
    except ImportError:
        missing.append("torch")

    if missing:
        print(f"Missing dependencies: {', '.join(missing)}")
        print(f"pip install {' '.join(missing)}")
        sys.exit(1)

    success = main()
    sys.exit(0 if success else 1)
