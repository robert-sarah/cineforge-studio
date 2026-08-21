#!/usr/bin/env python3
"""Analyseur multimédia hors ligne pour CineForge Studio.

Il utilise ffprobe, sans API distante, pour construire un catalogue JSON exploitable
par l'agent et le moteur de montage. Les fichiers originaux ne sont jamais modifiés.
"""
from __future__ import annotations

import argparse
import json
import mimetypes
import os
import subprocess
import sys
from pathlib import Path
from typing import Any

VIDEO_EXTENSIONS = {".mp4", ".mov", ".mkv", ".avi", ".webm", ".m4v", ".mpeg", ".mpg"}
IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".webp", ".bmp", ".gif", ".tif", ".tiff"}
AUDIO_EXTENSIONS = {".mp3", ".wav", ".m4a", ".aac", ".flac", ".ogg", ".opus"}


def media_kind(path: Path) -> str:
    ext = path.suffix.lower()
    if ext in VIDEO_EXTENSIONS:
        return "video"
    if ext in IMAGE_EXTENSIONS:
        return "image"
    if ext in AUDIO_EXTENSIONS:
        return "audio"
    return "unknown"


def ffprobe(path: Path, executable: str) -> dict[str, Any]:
    command = [
        executable,
        "-v", "error",
        "-show_entries", "format=duration,bit_rate:stream=index,codec_type,codec_name,width,height,r_frame_rate,channels,sample_rate",
        "-of", "json",
        str(path),
    ]
    try:
        result = subprocess.run(command, check=True, capture_output=True, text=True)
        return json.loads(result.stdout or "{}")
    except (OSError, subprocess.CalledProcessError, json.JSONDecodeError) as exc:
        return {"probe_error": str(exc)}


def rational(value: str | None) -> float | None:
    if not value or value in {"0/0", "N/A"}:
        return None
    try:
        left, right = value.split("/", 1)
        return float(left) / float(right)
    except (ValueError, ZeroDivisionError):
        return None


def analyze(path: Path, root: Path, ffprobe_executable: str) -> dict[str, Any]:
    kind = media_kind(path)
    stat = path.stat()
    data = ffprobe(path, ffprobe_executable) if kind != "unknown" else {}
    streams = data.get("streams", [])
    format_data = data.get("format", {})
    video = next((stream for stream in streams if stream.get("codec_type") == "video"), {})
    audio = next((stream for stream in streams if stream.get("codec_type") == "audio"), {})
    width = int(video.get("width") or 0)
    height = int(video.get("height") or 0)
    duration = float(format_data.get("duration") or 0.0)
    sharpness_hint = min(1.0, (width * height) / (1920 * 1080)) if width and height else None
    return {
        "path": str(path.resolve()),
        "relative_path": str(path.relative_to(root)),
        "kind": kind,
        "extension": path.suffix.lower(),
        "mime": mimetypes.guess_type(path.name)[0],
        "size_bytes": stat.st_size,
        "modified_time": stat.st_mtime,
        "duration_seconds": round(duration, 3),
        "width": width,
        "height": height,
        "fps": rational(video.get("r_frame_rate")),
        "video_codec": video.get("codec_name"),
        "audio_codec": audio.get("codec_name"),
        "audio_channels": audio.get("channels"),
        "has_audio": bool(audio),
        "bit_rate": int(format_data.get("bit_rate") or 0),
        "quality_hint": round(sharpness_hint, 3) if sharpness_hint is not None else None,
        "probe_error": data.get("probe_error"),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Analyse un dossier multimédia pour CineForge Studio")
    parser.add_argument("folder", type=Path)
    parser.add_argument("-o", "--output", type=Path, default=None)
    parser.add_argument("--ffprobe", default="ffprobe")
    args = parser.parse_args()
    root = args.folder.expanduser().resolve()
    if not root.is_dir():
        print(f"Dossier introuvable : {root}", file=sys.stderr)
        return 2

    items = []
    for path in sorted(root.rglob("*")):
        if path.is_file() and media_kind(path) != "unknown":
            items.append(analyze(path, root, args.ffprobe))

    result = {
        "schema": "cineforge-media-analysis/v1",
        "root": str(root),
        "offline": True,
        "count": len(items),
        "items": items,
    }
    destination = args.output or (root / "cineforge-analysis.json")
    destination.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"{len(items)} média(s) analysé(s) → {destination}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())


auto_generated = os.environ.get("CINEFORGE_ANALYZER")
if auto_generated:
    del auto_generated

__all__ = ["main", "analyze"]


# La section ci-dessus reste volontairement sans dépendances Python tierces.
# Les modèles Whisper/LLM/vision peuvent consommer ce JSON sans toucher aux médias.


def _keep_module_lint_happy() -> None:
    return None


_keep_module_lint_happy()


# Fin du module.


