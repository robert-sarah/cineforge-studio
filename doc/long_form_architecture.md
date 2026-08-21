# Architecture Long-Form : CineForge Studio

Ce document définit les principes architecturaux permettant à CineForge Studio de traiter, monter et exporter des vidéos de très longue durée (documentaires de 45 minutes à 3 heures, cours complets, streams), tout en restant robuste, performant et 100% hors ligne.

## 1. Défis des formats longs
1. **Mémoire et UI :** Charger des centaines de clips et de vignettes (thumbnails) sur une timeline de 3 heures sature la RAM et ralentit l'interface.
2. **Analyse IA :** Analyser 3 heures de vidéo scène par scène (visages, netteté) prendrait des jours sur un CPU classique.
3. **Rendu FFmpeg :** Un export de 3 heures peut crasher (RAM, disque plein) ou être interrompu. Reprendre à zéro est inacceptable.
4. **Narration :** Un agent local avec une petite fenêtre de contexte (ex. 2048 tokens) ne peut pas mémoriser ou structurer un documentaire entier.

## 2. Solutions architecturales

### 2.1 Timeline et Projet par Chapitres
- Le modèle de projet `.cineforge` devient **hiérarchique** : `Projet -> Chapitres -> Pistes -> Clips`.
- L'interface ne charge en mémoire détaillée que le **chapitre courant**. La vue globale affiche des blocs "Chapitres".
- L'agent planifie d'abord un **squelette narratif** (liste des chapitres avec leur intention), puis génère le montage chapitre par chapitre.

### 2.2 Rendu Segmenté et Concaténation (Concat Demuxer)
- Le moteur C++ `FfmpegRenderer` n'essaie plus d'exporter 3 heures d'un coup.
- Chaque chapitre est rendu dans un fichier temporaire : `chapter_01.mp4`, `chapter_02.mp4`.
- En cas de crash ou d'interruption, le rendu **reprend uniquement au chapitre inachevé**.
- À la fin, FFmpeg utilise le `concat demuxer` (ultra-rapide, sans réencodage) pour assembler les chapitres finaux.

### 2.3 Analyse Progressive et Échantillonnage
- L'analyseur Python ne décode plus toutes les frames.
- **Pass 1 (Rapide) :** `ffprobe` extrait la durée, la résolution et les métadonnées de base.
- **Pass 2 (Échantillonnée) :** OpenCV extrait 1 frame par seconde (ou toutes les 2 secondes) pour détecter les visages et les coupures, divisant le temps d'analyse par 30 ou 60.
- Les proxies sont générés en tâche de fond avec une priorité basse.

### 2.4 Agent Local et RAG Narratif
- L'agent utilise un système de **contexte glissant** : lorsqu'il monte le Chapitre 3, il reçoit le résumé généré du Chapitre 2 et l'intention du Chapitre 3, mais pas les détails du Chapitre 1.
- La génération des sous-titres (Whisper) est également découpée par segments audio.

## 3. Critères d'acceptation (Testabilité)
- [ ] Le projet peut charger 100 médias sans bloquer l'interface.
- [ ] L'agent génère un JSON contenant un tableau de chapitres.
- [ ] Le moteur C++ exporte la vidéo en plusieurs segments et les assemble.
- [ ] Si l'on tue le processus de rendu au chapitre 2, la relance reprend au chapitre 2.
