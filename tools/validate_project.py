#!/usr/bin/env python3
"""Validation hors ligne d'un projet .cineforge avant rendu.

Le validateur contrôle les index média, les durées, les chevauchements et la
continuité des chapitres. Il ne lit ni n'envoie le contenu vers un service distant.
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
        errors.append("Le projet ne contient aucun chapitre.")
        return errors, warnings

    expected_start = 0.0
    total_duration = 0.0
    for chapter_index, chapter in enumerate(chapters, 1):
        chapter_duration = float(chapter.get("duration", 0.0) or 0.0)
        chapter_start = float(chapter.get("startTime", expected_start) or 0.0)
        if chapter_duration < 0:
            errors.append(f"Chapitre {chapter_index}: durée négative.")
        if chapter_start + 0.25 < expected_start:
            errors.append(f"Chapitre {chapter_index}: début avant la fin du chapitre précédent.")
        elif chapter_start > expected_start + 0.25:
            warnings.append(f"Chapitre {chapter_index}: trou de {chapter_start - expected_start:.2f}s.")

        for track_index, track in enumerate(chapter.get("tracks", [])):
            previous_end = 0.0
            for clip_index, clip in enumerate(track.get("clips", []), 1):
                media_index = int(clip.get("mediaIndex", -1))
                if media_index < 0 or media_index >= len(media):
                    errors.append(f"Chapitre {chapter_index}, piste {track_index}, clip {clip_index}: mediaIndex invalide.")
                start = float(clip.get("start", 0.0) or 0.0)
                duration = float(clip.get("duration", 0.0) or 0.0)
                if start < 0 or duration <= 0:
                    errors.append(f"Chapitre {chapter_index}, piste {track_index}, clip {clip_index}: durée ou position invalide.")
                if start + 0.001 < previous_end:
                    warnings.append(f"Chapitre {chapter_index}, piste {track_index}: chevauchement autour du clip {clip_index}.")
                previous_end = max(previous_end, start + duration)
                if media_index >= 0 and media_index < len(media):
                    source_duration = float(media[media_index].get("duration", 0.0) or 0.0)
                    source_in = float(clip.get("sourceIn", 0.0) or 0.0)
                    source_out = float(clip.get("sourceOut", source_in + duration) or 0.0)
                    if source_duration and source_out > source_duration + 0.5:
                        warnings.append(f"Chapitre {chapter_index}, clip {clip_index}: sourceOut dépasse la durée du média.")

        expected_start = chapter_start + chapter_duration
        total_duration = max(total_duration, expected_start)

    declared = float(project.get("narrative", {}).get("durationSeconds", total_duration) or 0.0)
    if abs(declared - total_duration) > 1.0:
        warnings.append(f"Durée narrative déclarée {declared:.2f}s différente de la durée calculée {total_duration:.2f}s.")
    return errors, warnings


def main() -> int:
    parser = argparse.ArgumentParser(description="Valide un projet CineForge avant rendu")
    parser.add_argument("project", type=Path)
    parser.add_argument("--strict", action="store_true", help="Traiter les avertissements comme des erreurs")
    args = parser.parse_args()
    try:
        project = json.loads(args.project.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"Impossible de lire le projet: {exc}", file=sys.stderr)
        return 2
    errors, warnings = validate(project)
    for warning in warnings:
        print(f"AVERTISSEMENT: {warning}")
    for error in errors:
        print(f"ERREUR: {error}", file=sys.stderr)
    if errors or (args.strict and warnings):
        print(f"Projet invalide — {len(errors)} erreur(s), {len(warnings)} avertissement(s)", file=sys.stderr)
        return 1
    print(f"Projet valide — {len(project.get('chapters', []))} chapitre(s), {len(warnings)} avertissement(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
