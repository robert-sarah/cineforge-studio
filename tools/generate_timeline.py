#!/usr/bin/env python3
"""
Générateur de timeline intelligente pour CineForge Studio.
Prend en entrée le catalogue JSON de l'analyseur multimédia et génère
un fichier .cineforge avec les meilleurs clips découpés et ordonnés
selon le style narratif demandé.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any


def select_best_clips(media: list[dict[str, Any]], style: str) -> list[dict[str, Any]]:
    clips = []
    for item in media:
        if item.get("type") != "Video":
            continue
        duration = item.get("duration", 0.0)
        if duration < 1.0:
            continue
        
        # Découpe intelligente basée sur l'analyse de scène (simulée ici par un ratio)
        # Dans une vraie analyse, on lirait item["scenes"] et item["faces"]
        faces = item.get("max_faces", 0)
        blur = item.get("blur_score", 100.0)
        
        if blur < 30.0:
            continue  # Plan trop flou
            
        target_duration = 3.0 if style in ("high-energy", "gaming", "vlog") else 5.0
        
        # Créer des sous-clips
        num_clips = max(1, int(math.floor(duration / target_duration)))
        for i in range(num_clips):
            start = i * target_duration
            end = min(duration, start + target_duration)
            if end - start < 1.0:
                break
            clips.append({
                "media_path": item["path"],
                "source_in": start,
                "source_out": end,
                "duration": end - start,
                "score": faces * 10 + blur / 100.0
            })
    
    # Trier par score (les meilleurs plans d'abord)
    clips.sort(key=lambda c: c["score"], reverse=True)
    return clips


def build_project(input_dir: Path, output_file: Path, catalog_path: Path, style: str) -> dict[str, Any]:
    with open(catalog_path, "r", encoding="utf-8") as f:
        catalog = json.load(f)
        
    media_list = catalog.get("media", [])
    
    # 1. Sélectionner et ordonner les médias
    media_entries = []
    media_map = {}
    for i, item in enumerate(media_list):
        media_entries.append({
            "path": item["path"],
            "type": item["type"],
            "duration": item["duration"]
        })
        media_map[item["path"]] = i
        
    # 2. Générer les clips intelligents
    best_clips = select_best_clips(media_list, style)
    
    # 3. Construire la piste vidéo
    v1_clips = []
    current_time = 0.0
    
    # Si aucun clip vidéo n'est trouvé, on utilise les images
    if not best_clips:
        for item in media_list:
            if item["type"] == "Image":
                duration = 2.0 if style == "high-energy" else 4.0
                v1_clips.append({
                    "mediaIndex": media_map[item["path"]],
                    "trackIndex": 0,
                    "start": current_time,
                    "duration": duration,
                    "sourceIn": 0.0,
                    "sourceOut": duration,
                    "opacity": 1.0,
                    "scale": 1.0
                })
                current_time += duration
    else:
        # On limite à 10 clips pour l'exemple
        for clip in best_clips[:10]:
            v1_clips.append({
                "mediaIndex": media_map[clip["media_path"]],
                "trackIndex": 0,
                "start": current_time,
                "duration": clip["duration"],
                "sourceIn": clip["source_in"],
                "sourceOut": clip["source_out"],
                "opacity": 1.0,
                "scale": 1.0
            })
            current_time += clip["duration"]
            
    # 4. Construire la piste audio
    a1_clips = []
    audio_time = 0.0
    for item in media_list:
        if item["type"] == "Audio":
            a1_clips.append({
                "mediaIndex": media_map[item["path"]],
                "trackIndex": 1,
                "start": audio_time,
                "duration": item["duration"],
                "sourceIn": 0.0,
                "sourceOut": item["duration"],
                "opacity": 1.0,
                "scale": 1.0
            })
            audio_time += item["duration"]
            
    project = {
        "formatVersion": 1,
        "inputDirectory": str(input_dir.resolve()),
        "outputFile": "output.mp4",
        "style": style,
        "media": media_entries,
        "tracks": [
            {
                "name": "V1",
                "audio": False,
                "clips": v1_clips
            },
            {
                "name": "A1",
                "audio": True,
                "clips": a1_clips
            }
        ]
    }
    
    with open(output_file, "w", encoding="utf-8") as f:
        json.dump(project, f, indent=2)
        
    return project


def main() -> int:
    parser = argparse.ArgumentParser(description="Générateur de timeline CineForge")
    parser.add_argument("folder", type=Path)
    parser.add_argument("catalog", type=Path)
    parser.add_argument("--output", type=Path, default=Path("auto_project.cineforge"))
    parser.add_argument("--style", default="standard")
    args = parser.parse_args()
    
    if not args.catalog.exists():
        print(f"Catalogue introuvable : {args.catalog}", file=sys.stderr)
        return 1
        
    build_project(args.folder, args.output, args.catalog, args.style)
    print(f"Projet généré : {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
