#!/usr/bin/env python3
"""Diagnostic local des capacités CineForge.

Le script ne contacte aucun service distant. Il inspecte uniquement ffmpeg,
ffprobe et les encodeurs visibles dans l'environnement courant.
"""
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
from pathlib import Path


def command_output(command: list[str]) -> str:
    try:
        return subprocess.run(command, check=False, capture_output=True, text=True, timeout=15).stdout
    except (OSError, subprocess.TimeoutExpired):
        return ""


def main() -> int:
    parser = argparse.ArgumentParser(description="Diagnostiquer les capacités locales de CineForge")
    parser.add_argument("--output", type=Path, help="Écrire le rapport JSON à cet emplacement")
    args = parser.parse_args()

    ffmpeg = shutil.which("ffmpeg")
    ffprobe = shutil.which("ffprobe")
    encoders_text = command_output([ffmpeg, "-hide_banner", "-encoders"] if ffmpeg else [])
    encoders = set()
    for line in encoders_text.splitlines():
        fields = line.split()
        if len(fields) >= 2 and not fields[0].startswith("-") and fields[1] != "=":
            encoders.add(fields[1])

    hardware = {
        "nvenc": "h264_nvenc" in encoders or "hevc_nvenc" in encoders,
        "vaapi": "h264_vaapi" in encoders or "hevc_vaapi" in encoders,
        "videotoolbox": "h264_videotoolbox" in encoders,
        "qsv": "h264_qsv" in encoders,
        "software_x264": "libx264" in encoders,
        "software_x265": "libx265" in encoders,
    }
    report = {
        "schema": "cineforge-capabilities/v1",
        "ffmpeg": ffmpeg,
        "ffprobe": ffprobe,
        "encoders": sorted(encoders),
        "hardware": hardware,
        "recommendation": (
            "h264_nvenc" if hardware["nvenc"] else
            "h264_vaapi" if hardware["vaapi"] else
            "h264_qsv" if hardware["qsv"] else
            "h264_videotoolbox" if hardware["videotoolbox"] else
            "libx264"
        ),
    }
    rendered = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0 if ffmpeg and ffprobe else 1


if __name__ == "__main__":
    raise SystemExit(main())
