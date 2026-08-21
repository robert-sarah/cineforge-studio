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

try:
    import cv2  # type: ignore
except ImportError:
    cv2 = None

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


def vision_metrics(path: Path, kind: str, sample_count: int = 12) -> dict[str, Any]:
    """Calcule des indices locaux, sans reconnaissance distante ni modification du fichier."""
    if cv2 is None or kind not in {"image", "video"}:
        return {"vision_available": False, "face_count_max": None, "blur_score": None, "scene_cuts_estimate": None}
    frames = []
    capture = None
    try:
        if kind == "image":
            frame = cv2.imread(str(path))
            if frame is not None:
                frames = [frame]
        else:
            capture = cv2.VideoCapture(str(path))
            total = int(capture.get(cv2.CAP_PROP_FRAME_COUNT) or 0)
            if total > 0:
                for position in range(min(sample_count, total)):
                    capture.set(cv2.CAP_PROP_POS_FRAMES, int(position * max(0, total - 1) / max(1, sample_count - 1)))
                    ok, frame = capture.read()
                    if ok and frame is not None:
                        frames.append(frame)
        if not frames:
            return {"vision_available": True, "face_count_max": 0, "blur_score": None, "scene_cuts_estimate": 0}
        cascade_path = str(Path(cv2.data.haarcascades) / "haarcascade_frontalface_default.xml")
        detector = cv2.CascadeClassifier(cascade_path)
        face_count_max = 0
        blur_scores = []
        scene_cuts = 0
        previous = None
        for frame in frames:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            faces = detector.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=4) if not detector.empty() else []
            face_count_max = max(face_count_max, len(faces))
            blur_scores.append(float(cv2.Laplacian(gray, cv2.CV_64F).var()))
            small = cv2.resize(gray, (64, 36))
            if previous is not None and float(cv2.absdiff(previous, small).mean()) > 35.0:
                scene_cuts += 1
            previous = small
        return {
            "vision_available": True,
            "face_count_max": face_count_max,
            "blur_score": round(sum(blur_scores) / len(blur_scores), 2),
            "scene_cuts_estimate": scene_cuts,
        }
    except Exception as exc:
        return {"vision_available": True, "vision_error": str(exc), "face_count_max": None, "blur_score": None, "scene_cuts_estimate": None}
    finally:
        if capture is not None:
            capture.release()


def analyze(path: Path, root: Path, ffprobe_executable: str, vision: bool = False) -> dict[str, Any]:
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
    metrics = vision_metrics(path, kind) if vision else {"vision_available": False}
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
        **metrics,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Analyse un dossier multimédia pour CineForge Studio")
    parser.add_argument("folder", type=Path)
    parser.add_argument("-o", "--output", type=Path, default=None)
    parser.add_argument("--ffprobe", default="ffprobe")
    parser.add_argument("--vision", action="store_true", help="Activer les métriques locales de netteté, visages et scènes (OpenCV requis)")
    args = parser.parse_args()
    root = args.folder.expanduser().resolve()
    if not root.is_dir():
        print(f"Dossier introuvable : {root}", file=sys.stderr)
        return 2

    items = []
    for path in sorted(root.rglob("*")):
        if path.is_file() and media_kind(path) != "unknown":
            items.append(analyze(path, root, args.ffprobe, vision=args.vision))

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


