#!/usr/bin/env python3
"""Generates editing proxies in a separate cache, without modifying originals."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

VIDEO_EXTENSIONS = {".mp4", ".mov", ".mkv", ".avi", ".webm", ".m4v", ".mpeg", ".mpg"}


def main() -> int:
    parser = argparse.ArgumentParser(description="Create CineForge video proxies")
    parser.add_argument("folder", type=Path)
    parser.add_argument("--output", type=Path, default=None)
    parser.add_argument("--width", type=int, default=960)
    parser.add_argument("--ffmpeg", default="ffmpeg")
    args = parser.parse_args()
    root = args.folder.expanduser().resolve()
    if not root.is_dir():
        print(f"Folder not found: {root}", file=sys.stderr)
        return 2
    output = (args.output or (root / ".cineforge-cache" / "proxies")).resolve()
    output.mkdir(parents=True, exist_ok=True)
    processed = 0
    for source in sorted(root.rglob("*")):
        if not source.is_file() or source.suffix.lower() not in VIDEO_EXTENSIONS:
            continue
        relative = source.relative_to(root).with_suffix(".proxy.mp4")
        destination = output / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        if destination.exists() and destination.stat().st_mtime >= source.stat().st_mtime:
            continue
        command = [args.ffmpeg, "-y", "-hide_banner", "-loglevel", "error", "-i", str(source),
                   "-vf", f"scale={args.width}:-2:force_original_aspect_ratio=decrease",
                   "-c:v", "libx264", "-preset", "veryfast", "-crf", "28", "-an", str(destination)]
        try:
            subprocess.run(command, check=True)
            processed += 1
            print(f"Proxy: {source.relative_to(root)} -> {destination.relative_to(output)}")
        except (OSError, subprocess.CalledProcessError) as exc:
            print(f"Proxy failed for {source}: {exc}", file=sys.stderr)
    print(f"{processed} proxy(s) created in {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
