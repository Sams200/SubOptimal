#!/usr/bin/env python3
"""
Requires pip install huggingface_hub (or huggingface-cli installed) and hf auth login
"""
import argparse
import json
import subprocess
from pathlib import Path

def download_with_hf_cli(repo_id, filename, local_dir):
    cmd = [
        "hf", "download",
        repo_id,
        filename,
        "--local-dir", str(local_dir),
        "--repo-type", "dataset",
        "--quiet"
    ]
    print(f"  Running: hf download {repo_id}/{filename}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"  Error: {result.stderr}")
        return None

    expected = local_dir / filename
    if expected.exists():
        return expected

    for path in local_dir.rglob(filename):
        return path

    return None

def jsonl_to_text(jsonl_path, text_path):
    lines = []
    with open(jsonl_path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                data = json.loads(line)
                # Flores+ format: {"id": "...", "text": "..."} or {"sentence": "..."}
                text = data.get("text") or data.get("sentence") or data.get("source")
                if text:
                    lines.append(text)
            except json.JSONDecodeError:
                continue

    with open(text_path, 'w', encoding='utf-8') as f:
        for line in lines:
            f.write(line + '\n')

    return len(lines)

def main():
    parser = argparse.ArgumentParser(description='Download Flores+ dataset from HuggingFace')
    parser.add_argument('--output-dir', required=True, help='Directory to store dataset')
    parser.add_argument('--split', choices=['dev', 'devtest'], default='devtest',
                       help='Dataset split to download')
    parser.add_argument('--source-lang', default='eng_Latn', help='Source language code (e.g., eng_Latn)')
    parser.add_argument('--target-lang', default='spa_Latn', help='Target language code (e.g., spa_Latn)')
    parser.add_argument('--manifest', required=True, help='Output manifest TSV path')
    parser.add_argument('--max-lines', type=int, default=0, help='Limit to N lines (0=all)')
    parser.add_argument('--repo-id', default='openlanguagedata/flores_plus',
                       help='HuggingFace repo ID')

    args = parser.parse_args()

    print("Remember to run: hf auth login")
    print("Note: You must accept the dataset terms at:")
    print("https://huggingface.co/datasets/openlanguagedata/flores_plus")

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    src_file = f"{args.split}/{args.source_lang}.jsonl"
    tgt_file = f"{args.split}/{args.target_lang}.jsonl"

    print(f"Downloading Flores+ from {args.repo_id}")
    print(f"Split: {args.split}")
    print(f"Languages: {args.source_lang} -> {args.target_lang}")

    print(f"\nDownloading {args.source_lang}...")
    src_jsonl = download_with_hf_cli(args.repo_id, src_file, output_dir)
    if not src_jsonl:
        print(f"Failed to download {src_file}")
        return 1

    print(f"Downloading {args.target_lang}...")
    tgt_jsonl = download_with_hf_cli(args.repo_id, tgt_file, output_dir)
    if not tgt_jsonl:
        print(f"Failed to download {tgt_file}")
        return 1

    src_txt = output_dir / f"{args.source_lang}.{args.split}.txt"
    tgt_txt = output_dir / f"{args.target_lang}.{args.split}.txt"

    src_count = jsonl_to_text(src_jsonl, src_txt)
    tgt_count = jsonl_to_text(tgt_jsonl, tgt_txt)

    if src_count != tgt_count:
        print(f"Warning: Mismatched line counts ({src_count} vs {tgt_count})")

    print(f"Cleaning up JSONL files...")
    src_jsonl.unlink()
    tgt_jsonl.unlink()
    import shutil
    for subdir in output_dir.glob(f"{args.repo_id}*"):
        if subdir.is_dir():
            shutil.rmtree(subdir)

    limit = min(args.max_lines, src_count) if args.max_lines > 0 else src_count
    if limit < src_count:
        print(f"Limiting evaluation to first {limit} lines")

    manifest_path = Path(args.manifest)
    manifest_path.parent.mkdir(parents=True, exist_ok=True)

    with open(manifest_path, 'w', encoding='utf-8') as f:
        f.write("# Flores+ NMT Evaluation Manifest\n")
        f.write(f"# Source: {args.source_lang}, Target: {args.target_lang}, Split: {args.split}\n")
        f.write("SourcePath\tRefPath\tSrcLang\tTgtLang\n")
        f.write(f"{src_txt.absolute()}\t{tgt_txt.absolute()}\t{args.source_lang}\t{args.target_lang}\n")

    print(f"\nComplete!")
    print(f"  Manifest: {manifest_path}")
    print(f"  Total sentences: {limit}")
    print(f"\nRun evaluation with:")
    print(f"  ./suboptimal-eval-nmt <model_dir> <spm.model> {manifest_path} [max_lines]")

    return 0

if __name__ == "__main__":
    exit(main())