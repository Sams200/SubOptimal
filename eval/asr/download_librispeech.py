#!/usr/bin/env python3
import os
import sys
import tarfile
import urllib.request
import subprocess
import argparse
from pathlib import Path

LIBRISPEECH_URL = "http://www.openslr.org/resources/12/test-clean.tar.gz"

def download_with_progress(url, dest):
    print(f"Downloading LibriSpeech test-clean...")
    print(f"URL: {url}")

    def reporthook(count, block_size, total_size):
        percent = min(int(count * block_size * 100 / total_size), 100)
        sys.stdout.write(f"\r  Progress: {percent}% ({count * block_size // 1024 // 1024}MB / {total_size // 1024 // 1024}MiB)")
        sys.stdout.flush()

    try:
        urllib.request.urlretrieve(url, dest, reporthook)
        print()  # Newline after progress
    except Exception as e:
        print(f"\nError downloading: {e}")
        sys.exit(1)

def convert_flac_to_wav(flac_path, wav_path):
    cmd = [
        "ffmpeg", "-y", "-hide_banner", "-loglevel", "error",
        "-i", str(flac_path),
        "-ar", "16000", "-ac", "1",  # 16kHz mono
        "-map_metadata", "-1",        # Strip metadata
        str(wav_path)
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"\nWarning: Failed to convert {flac_path.name}: {result.stderr.strip()}")
        return False
    return True

def extract_transcript(flac_path):
    dir_path = flac_path.parent
    chapter_id = dir_path.name
    reader_id = dir_path.parent.name

    trans_file = dir_path / f"{reader_id}-{chapter_id}.trans.txt"

    if not trans_file.exists():
        return None

    utt_id = flac_path.stem  # filename without .flac
    with open(trans_file, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line.startswith(utt_id):
                parts = line.split(' ', 1)
                if len(parts) == 2:
                    return parts[1]
    return None

def main():
    parser = argparse.ArgumentParser(description='Download and prepare LibriSpeech test-clean for ASR evaluation')
    parser.add_argument('--output-dir', required=True, help='Directory to store dataset')
    parser.add_argument('--manifest', required=True, help='Output manifest TSV path')
    parser.add_argument('--max-files', type=int, default=500, help='Limit processing to N files (0=all)')
    args = parser.parse_args()

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    tar_path = output_dir / "test-clean.tar.gz"
    test_clean_dir = output_dir / "LibriSpeech" / "test-clean"

    if test_clean_dir.exists() and any(test_clean_dir.glob("**/*.flac")):
        print(f"LibriSpeech test-clean already exists at {test_clean_dir}")
    else:
        if not tar_path.exists():
            download_with_progress(LIBRISPEECH_URL, tar_path)
            print(f"Saved to {tar_path}")
        else:
            print(f"Using existing archive: {tar_path}")

        with tarfile.open(tar_path, 'r:gz') as tar:
            tar.extractall(path=output_dir)
        print(f"Extracted to {test_clean_dir}")

    flac_files = sorted(test_clean_dir.rglob("*.flac"))
    if args.max_files > 0:
        flac_files = flac_files[:args.max_files]

    print(f"Found {len(flac_files)} FLAC files")

    entries = []
    skipped = 0
    converted = 0

    for i, flac_path in enumerate(flac_files):
        if (i + 1) % 100 == 0 or i == 0:
            print(f"  Processing {i+1}/{len(flac_files)}...", end='\r')

        wav_path = flac_path.with_suffix('.wav')

        if not wav_path.exists():
            if convert_flac_to_wav(flac_path, wav_path):
                converted += 1
            else:
                skipped += 1
                continue

        transcript = extract_transcript(flac_path)
        if transcript is None:
            print(f"\nNo transcript found for {flac_path}")
            skipped += 1
            continue

        entries.append((wav_path.resolve(), transcript))

    manifest_path = Path(args.manifest)
    manifest_path.parent.mkdir(parents=True, exist_ok=True)

    with open(manifest_path, 'w', encoding='utf-8') as f:
        f.write("# LibriSpeech test-clean manifest\n")
        for wav_path, transcript in entries:
            clean_transcript = transcript.replace('\t', ' ').replace('\n', ' ').replace('\r', '')
            f.write(f"{wav_path}\t{clean_transcript}\n")

    print(f"\nComplete!")
    print(f"  Manifest: {manifest_path}")
    print(f"  Total entries: {len(entries)}")

if __name__ == "__main__":
    main()