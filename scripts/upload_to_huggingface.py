#!/usr/bin/env python3
"""
Upload Helsinki-NLP CTranslate2 models to HuggingFace Hub.
Each model gets its own repository.
"""

import os
import sys
from pathlib import Path
from huggingface_hub import HfApi, create_repo, upload_folder
from huggingface_hub.utils import RepositoryNotFoundError

# Models directory
MODELS_DIR = Path("/mnt/home2/Models/Thesis/helsinki-models")

HF_USERNAME = "Sams200"

# Organization (optional - leave as None to upload to personal account)
HF_ORGANIZATION = None  # e.g., "Helsinki-NLP" or your org name

def get_repo_id(model_name: str) -> str:
    """Get full repo id (username/model-name or org/model-name)"""
    if HF_ORGANIZATION:
        return f"{HF_ORGANIZATION}/{model_name}"
    return f"{HF_USERNAME}/{model_name}"

def upload_model(model_dir: Path) -> bool:
    """Upload a single model to HuggingFace Hub"""
    model_name = model_dir.name

    # Check if model.bin exists (converted CT2 model)
    model_bin = model_dir / "model.bin"
    if not model_bin.exists():
        print(f"  SKIP: No model.bin found (not converted)")
        return False

    repo_id = get_repo_id(model_name)

    print(f"\n{'='*60}")
    print(f"Uploading: {model_name}")
    print(f"Repository: {repo_id}")
    print(f"{'='*60}")

    try:
        api = HfApi()

        # Create repository (public)
        print(f"  Creating repository...")
        try:
            create_repo(
                repo_id=repo_id,
                repo_type="model",
                exist_ok=True,
                private=False
            )
            print(f"  OK: Repository ready")
        except Exception as e:
            print(f"  ERROR: Failed to create repo: {e}")
            return False

        # Upload files
        print(f"  Uploading files...")
        upload_folder(
            folder_path=str(model_dir),
            repo_id=repo_id,
            repo_type="model",
            commit_message=f"Upload {model_name} CTranslate2 model"
        )

        print(f"  SUCCESS: Model uploaded")
        print(f"  URL: https://huggingface.co/{repo_id}")
        return True

    except Exception as e:
        print(f"  ERROR: {e}")
        return False


def main():
    print("Helsinki-NLP CTranslate2 Models - HuggingFace Upload")
    print("="*60)

    # Check authentication
    try:
        api = HfApi()
        user_info = api.whoami()
        print(f"Logged in as: {user_info['name']}")
    except Exception as e:
        print(f"ERROR: Not logged in to HuggingFace")
        sys.exit(1)

    # Check models directory
    if not MODELS_DIR.exists():
        print(f"ERROR: Models directory not found: {MODELS_DIR}")
        sys.exit(1)

    # Get list of model directories
    models = []
    for model_dir in MODELS_DIR.iterdir():
        if model_dir.is_dir() and model_dir.name.startswith("opus-mt-"):
            models.append(model_dir)

    print(f"Found {len(models)} models to upload")
    print(f"Target: {'https://huggingface.co/' + HF_ORGANIZATION if HF_ORGANIZATION else 'https://huggingface.co/' + HF_USERNAME}")

    # Upload each model
    uploaded = []
    failed = []
    skipped = []

    for model_dir in models:
        if upload_model(model_dir):
            uploaded.append(model_dir.name)
        else:
            if not (model_dir / "model.bin").exists():
                skipped.append(model_dir.name)
            else:
                failed.append(model_dir.name)

    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    print(f"Uploaded: {len(uploaded)}")
    print(f"Skipped:  {len(skipped)} (not converted)")
    print(f"Failed:   {len(failed)}")

    if uploaded:
        print(f"\nUploaded models:")
        for model in sorted(uploaded):
            repo = get_repo_id(model)
            print(f"  - https://huggingface.co/{repo}")

    if failed:
        print(f"\nFailed uploads:")
        for model in sorted(failed):
            print(f"  - {model}")

    return len(failed) == 0


if __name__ == "__main__":
    main()
