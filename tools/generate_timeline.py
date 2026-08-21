#!/usr/bin/env python3
"""Long-form narrative timeline generator for CineForge Studio.

The script consumes the catalog produced by ``analyze_media.py`` and generates a
.cineforge JSON project. It preserves the narrative order of files, filters out
shots that are too short or blurry, creates sub-clips, and then splits the timeline
into chapters to enable multi-hour projects.

Processing is entirely local and never modifies source media.
"""
from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path
from typing import Any


def item_kind(item: dict[str, Any]) -> str:
    return str(item.get("kind", item.get("type", "unknown"))).lower()


def item_duration(item: dict[str, Any]) -> float:
    value = item.get("duration_seconds", item.get("duration", 0.0))
    try:
        return max(0.0, float(value or 0.0))
    except (TypeError, ValueError):
        return 0.0


def quality_score(item: dict[str, Any]) -> float:
    """Calculates a deterministic, explainable score without a remote model."""
    blur = float(item.get("blur_score") or 0.0)
    faces = int(item.get("face_count_max", item.get("max_faces", 0)) or 0)
    scene_cuts = int(item.get("scene_cuts_estimate") or 0)
    quality = float(item.get("quality_hint") or 0.0)
    # Sharpness is capped to prevent a very large number from dominating.
    sharpness = min(100.0, math.log1p(max(0.0, blur)) * 8.0)
    return round(sharpness + min(30.0, faces * 6.0) + min(15.0, scene_cuts * 1.5) + quality * 10.0, 3)


def select_clips(media: list[dict[str, Any]], style: str, preserve_order: bool = True) -> list[dict[str, Any]]:
    target_duration = 3.0 if style in {"high-energy", "gaming", "vlog", "short", "social"} else 6.0
    clips: list[dict[str, Any]] = []
    source_order = 0

    for item in media:
        kind = item_kind(item)
        duration = item_duration(item)
        if kind != "video" or duration < 1.0:
            continue

        blur = float(item.get("blur_score") or 0.0)
        # Without vision analysis, blur_score is absent and the shot remains eligible.
        if blur and blur < 30.0:
            continue

        count = max(1, int(math.ceil(duration / target_duration)))
        for index in range(count):
            start = min(duration, index * target_duration)
            end = min(duration, start + target_duration)
            if end - start < 1.0:
                continue
            clips.append({
                "media_path": item.get("path") or item.get("relative_path"),
                "source_in": round(start, 3),
                "source_out": round(end, 3),
                "duration": round(end - start, 3),
                "score": quality_score(item),
                "source_order": source_order,
            })
        source_order += 1

    if preserve_order:
        # The agent can later reorder, but the default preserves shooting continuity
        # instead of creating an artificially disordered edit.
        clips.sort(key=lambda clip: (clip["source_order"], clip["source_in"]))
    else:
        clips.sort(key=lambda clip: (-clip["score"], clip["source_order"], clip["source_in"]))
    return clips


def media_entries(catalog: dict[str, Any]) -> tuple[list[dict[str, Any]], dict[str, int]]:
    entries: list[dict[str, Any]] = []
    mapping: dict[str, int] = {}
    for item in catalog.get("items", catalog.get("media", [])):
        path = item.get("path") or item.get("media_path")
        if not path:
            continue
        entry = {
            "path": path,
            "relativePath": item.get("relative_path", path),
            "type": item_kind(item),
            "duration": item_duration(item),
            "width": int(item.get("width") or 0),
            "height": int(item.get("height") or 0),
            "fps": item.get("fps"),
        }
        mapping[str(path)] = len(entries)
        entries.append(entry)
    return entries, mapping


def clip_json(clip: dict[str, Any], media_map: dict[str, int], track_index: int, start: float) -> dict[str, Any] | None:
    path = str(clip["media_path"])
    if path not in media_map:
        return None
    return {
        "mediaIndex": media_map[path],
        "trackIndex": track_index,
        "start": round(start, 3),
        "duration": clip["duration"],
        "sourceIn": clip["source_in"],
        "sourceOut": clip["source_out"],
        "opacity": 1.0,
        "scale": 1.0,
        "score": clip["score"],
    }


def build_project(input_dir: Path, output_file: Path, catalog_path: Path, style: str,
                  chapter_minutes: float = 10.0, preserve_order: bool = True,
                  max_duration: float = 0.0) -> dict[str, Any]:
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    entries, media_map = media_entries(catalog)
    raw_items = catalog.get("items", catalog.get("media", []))
    clips = select_clips(raw_items, style, preserve_order=preserve_order)

    image_items = [item for item in raw_items if item_kind(item) == "image"]
    if not clips and image_items:
        for item in image_items:
            path = item.get("path") or item.get("relative_path")
            if path not in media_map:
                continue
            duration = 2.0 if style in {"high-energy", "gaming", "short", "social"} else 4.0
            clips.append({
                "media_path": path,
                "source_in": 0.0,
                "source_out": duration,
                "duration": duration,
                "score": quality_score(item),
                "source_order": len(clips),
            })

    if max_duration > 0:
        accumulated = 0.0
        limited: list[dict[str, Any]] = []
        for clip in clips:
            if accumulated >= max_duration:
                break
            remaining = max_duration - accumulated
            if clip["duration"] > remaining:
                clip = dict(clip)
                clip["duration"] = round(remaining, 3)
                clip["source_out"] = round(clip["source_in"] + remaining, 3)
            if clip["duration"] >= 1.0:
                limited.append(clip)
                accumulated += clip["duration"]
        clips = limited

    chapter_seconds = max(60.0, chapter_minutes * 60.0)
    chapters: list[dict[str, Any]] = []
    current_chapter: dict[str, Any] | None = None
    chapter_time = 0.0
    global_time = 0.0
    chapter_index = 1

    for clip in clips:
        if current_chapter is None or (chapter_time >= chapter_seconds and current_chapter["tracks"][0]["clips"]):
            current_chapter = {
                "title": f"Chapter {chapter_index}",
                "startTime": round(global_time, 3),
                "duration": 0.0,
                "tracks": [{"name": "V1", "audio": False, "clips": []}],
                "subtitles": [],
            }
            chapters.append(current_chapter)
            chapter_index += 1
            chapter_time = 0.0

        generated = clip_json(clip, media_map, 0, chapter_time)
        if generated is None:
            continue
        current_chapter["tracks"][0]["clips"].append(generated)
        chapter_time += clip["duration"]
        global_time += clip["duration"]
        current_chapter["duration"] = round(chapter_time, 3)

    audio_clips: list[dict[str, Any]] = []
    audio_time = 0.0
    for index, item in enumerate(raw_items):
        if item_kind(item) != "audio":
            continue
        path = item.get("path") or item.get("relative_path")
        if path not in media_map:
            continue
        duration = item_duration(item)
        audio_clips.append({
            "mediaIndex": media_map[path],
            "trackIndex": 1,
            "start": round(audio_time, 3),
            "duration": duration,
            "sourceIn": 0.0,
            "sourceOut": duration,
            "opacity": 1.0,
            "scale": 1.0,
        })
        audio_time += duration

    for chapter in chapters:
        chapter["tracks"].append({"name": "A1", "audio": True, "clips": audio_clips})

    project = {
        "formatVersion": 2,
        "schema": "cineforge-project/v2",
        "inputDirectory": str(input_dir.expanduser().resolve()),
        "outputFile": "output.mp4",
        "style": style,
        "offline": True,
        "media": entries,
        "chapters": chapters,
        # Compatibility with versions that only display a flat track.
        "tracks": chapters[0]["tracks"] if chapters else [],
        "narrative": {
            "chapterCount": len(chapters),
            "durationSeconds": round(global_time, 3),
            "preserveSourceOrder": preserve_order,
            "analysisSource": str(catalog_path.resolve()),
        },
    }
    output_file.parent.mkdir(parents=True, exist_ok=True)
    output_file.write_text(json.dumps(project, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return project


def main() -> int:
    parser = argparse.ArgumentParser(description="Generates a long-form chapter-based CineForge timeline")
    parser.add_argument("folder", type=Path)
    parser.add_argument("catalog", type=Path)
    parser.add_argument("--output", type=Path, default=Path("auto_project.cineforge"))
    parser.add_argument("--style", default="standard")
    parser.add_argument("--chapter-minutes", type=float, default=10.0)
    parser.add_argument("--best-first", action="store_true", help="Order by score instead of preserving source order")
    parser.add_argument("--max-duration", type=float, default=0.0, help="Maximum duration in seconds; 0 means no limit")
    args = parser.parse_args()

    if not args.catalog.exists():
        print(f"Catalog not found: {args.catalog}", file=sys.stderr)
        return 1
    project = build_project(args.folder, args.output, args.catalog, args.style,
                            chapter_minutes=args.chapter_minutes,
                            preserve_order=not args.best_first,
                            max_duration=args.max_duration)
    print(f"Project generated: {args.output} — {len(project['chapters'])} chapter(s), "
          f"{project['narrative']['durationSeconds']:.1f} second(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
