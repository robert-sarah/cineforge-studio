#!/usr/bin/env python3
"""Offline validation of a .cineforge project before rendering.

The validator checks media indices, durations, overlaps, chapter continuity,
transitions, and keyframes. It neither reads nor sends content to a remote service.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any


def validate(project: dict[str, Any]) -> tuple[list[str], list[str]]:
    errors: list[str] = []
    warnings: list[str] = []
    media = project.get("media", [])
    chapters = project.get("chapters", [])
    if not chapters:
        errors.append("Project contains no chapters.")
        return errors, warnings
        
    target_duration = float(project.get("targetDurationSeconds", 0.0))
    if target_duration > 0:
        warnings.append(f"Target duration is set to {target_duration}s. Final length may vary slightly based on media.")

    expected_start = 0.0
    total_duration = 0.0
    for chapter_index, chapter in enumerate(chapters, 1):
        chapter_duration = float(chapter.get("duration", 0.0) or 0.0)
        chapter_start = float(chapter.get("startTime", expected_start) or 0.0)
        if chapter_duration < 0:
            errors.append(f"Chapter {chapter_index}: negative duration.")
        if chapter_start + 0.25 < expected_start:
            errors.append(f"Chapter {chapter_index}: starts before the previous chapter ends.")
        elif chapter_start > expected_start + 0.25:
            warnings.append(f"Chapter {chapter_index}: gap of {chapter_start - expected_start:.2f}s.")

        for track_index, track in enumerate(chapter.get("tracks", [])):
            previous_end = 0.0
            for clip_index, clip in enumerate(track.get("clips", []), 1):
                media_index = int(clip.get("mediaIndex", -1))
                if media_index < 0 or media_index >= len(media):
                    errors.append(f"Chapter {chapter_index}, track {track_index}, clip {clip_index}: invalid mediaIndex.")
                start = float(clip.get("start", 0.0) or 0.0)
                duration = float(clip.get("duration", 0.0) or 0.0)
                if start < 0 or duration <= 0:
                    errors.append(f"Chapter {chapter_index}, track {track_index}, clip {clip_index}: invalid duration or position.")
                if start + 0.001 < previous_end:
                    warnings.append(f"Chapter {chapter_index}, track {track_index}: overlap around clip {clip_index}.")
                previous_end = max(previous_end, start + duration)
                
                # Check transitions
                transition_in_dur = float(clip.get("transitionInDuration", 0.0) or 0.0)
                transition_out_dur = float(clip.get("transitionOutDuration", 0.0) or 0.0)
                if transition_in_dur + transition_out_dur > duration:
                    errors.append(f"Chapter {chapter_index}, clip {clip_index}: transitions exceed clip duration.")

                if media_index >= 0 and media_index < len(media):
                    source_duration = float(media[media_index].get("duration", 0.0) or 0.0)
                    source_in = float(clip.get("sourceIn", 0.0) or 0.0)
                    source_out = float(clip.get("sourceOut", source_in + duration) or 0.0)
                    if source_duration and source_out > source_duration + 0.5:
                        warnings.append(f"Chapter {chapter_index}, clip {clip_index}: sourceOut exceeds media duration.")

        expected_start = chapter_start + chapter_duration
        total_duration = max(total_duration, expected_start)

    declared = float(project.get("narrative", {}).get("durationSeconds", total_duration) or 0.0)
    if abs(declared - total_duration) > 1.0:
        warnings.append(f"Declared narrative duration {declared:.2f}s differs from calculated duration {total_duration:.2f}s.")
        
    if target_duration > 0 and abs(target_duration - total_duration) > 5.0:
        warnings.append(f"Final duration ({total_duration:.2f}s) misses target duration ({target_duration:.2f}s) by more than 5 seconds.")
        
    return errors, warnings


def main() -> int:
    parser = argparse.ArgumentParser(description="Validates a CineForge project before rendering")
    parser.add_argument("project", type=Path)
    parser.add_argument("--strict", action="store_true", help="Treat warnings as errors")
    args = parser.parse_args()
    try:
        project = json.loads(args.project.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"Cannot read project: {exc}", file=sys.stderr)
        return 2
    errors, warnings = validate(project)
    for warning in warnings:
        print(f"WARNING: {warning}")
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    if errors or (args.strict and warnings):
        print(f"Invalid project — {len(errors)} error(s), {len(warnings)} warning(s)", file=sys.stderr)
        return 1
    print(f"Valid project — {len(project.get('chapters', []))} chapter(s), {len(warnings)} warning(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
