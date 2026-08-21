# CineForge Studio

**CineForge Studio** est un logiciel de montage vidéo C++/Qt **entièrement hors ligne**, piloté par un agent local et un moteur de rendu FFmpeg. Il est conçu pour automatiser l'organisation et l'édition de tous types de contenus multimédias (vidéos virales, formats cinématiques, documentaires), sans jamais envoyer vos médias sur un serveur distant.

## Nouveautés de la version Studio

- **Timeline Multipiste Interactive** : clips déplaçables, magnétisme, découpe, zoom et pistes audio/vidéo.
- **Lecteur Vidéo Intégré** : prévisualisation du rendu directement dans l'interface via Qt Multimedia.
- **Agent IA Local GGUF** : connexion à Llama 3 (8B) via `llama.cpp` pour comprendre les requêtes complexes et générer des plans de montage JSON.
- **Analyseur Python Hors Ligne** : extraction des métadonnées avec `ffprobe` et `OpenCV` pour la détection des visages, du flou et le montage intelligent.
- **Proxies Vidéo 4K** : script Python pour générer des proxies légers et accélérer le montage des médias lourds.
- **Sauvegarde de Projet** : format JSON `.cineforge` pour conserver et restaurer vos timelines et médias.
- **Import Automatique** : détection d'images, vidéos et musiques, avec tri naturel et chronologique.
- **Rendu FFmpeg Avancé** : génération de formats multiples, keyframes de zoom, sous-titres, et ducking/mixage audio.
- **Encodeurs Matériels** : support natif de `NVENC`, `VAAPI` et `VideoToolbox` pour des exports très rapides.
- **Historique d'Édition** : support complet du Undo/Redo dans la timeline interactive.
- **Modèles de Montage** : MrBeast, Cinématique, Shorts, Vlog, Documentaire, Gaming, Podcast et Tutoriel.
- **Interface Qt Professionnelle** : thématique sombre, inspecteur, panneaux dockables et gestionnaire de modèles locaux.

## Workflow long-form

CineForge Studio organise les projets en **chapitres**, afin de préparer des documentaires de 45 minutes, 3 heures ou davantage sans charger tous les médias en mémoire en même temps. `tools/analyze_media.py` analyse progressivement les fichiers et échantillonne les vidéos longues. `tools/generate_timeline.py` produit ensuite un projet `.cineforge` avec des chapitres, des pistes et des clips. Avant le rendu, `tools/validate_project.py projet.cineforge` vérifie les index médias, les durées, les trous et les incohérences de timeline.

Le moteur FFmpeg rend chaque chapitre séparément dans `.cineforge_cache/`, vérifie les fichiers générés et conserve un manifeste `render_state.txt` pendant le traitement. En cas d’interruption, les chapitres valides peuvent être réutilisés au prochain lancement. Les proxies restent recommandés pour les sources 4K et les projets de plusieurs heures.

```bash
python3 tools/analyze_media.py ./mes_medias --vision --sample-seconds 5 --output catalog.json
python3 tools/generate_timeline.py ./mes_medias catalog.json --style documentary --chapter-minutes 12 --output documentaire.cineforge
python3 tools/validate_project.py documentaire.cineforge
```

## Compilation (Linux / WSL)

Assurez-vous d'avoir installé `cmake`, un compilateur C++ (gcc/clang), `ffmpeg` et `qt6-base-dev`.

```bash
mkdir build && cd build
cmake .. -DOVA_BUILD_GUI=ON
cmake --build . -j$(nproc)
```

## Utilisation de l'interface

Lancez l'application avec :
```bash
./bin/cineforge-studio
```

1. Cliquez sur **Importer un dossier** pour charger vos images et vidéos.
2. Dans l'onglet **Styles**, choisissez "MrBeast / High Energy" pour appliquer automatiquement le plan viral.
3. Dans l'onglet **Modèles**, vérifiez que les modèles locaux sont installés (ex. `ggml-tiny.bin`). Un double-clic les associe au projet.
4. Dans l'**Inspecteur**, ajustez le script de la voix off.
5. Cliquez sur **Créer la vidéo**. Le moteur C++ va :
   - Générer la voix off via Piper.
   - Transcrire l'audio via Whisper pour générer un fichier `.srt`.
   - Utiliser FFmpeg pour appliquer les zooms, incruster les sous-titres avec le style viral, et assembler le tout.

## Utilisation du CLI

Le moteur reste utilisable en ligne de commande pour l'automatisation :
```bash
./bin/cineforge-cli --folder ./mon_dossier --command "crée un short viral dynamique mrbeast"
```

## Installation des modèles locaux

Un script sécurisé est fourni pour télécharger les modèles Whisper officiels :
```bash
./tools/download_models.sh models whisper-tiny
```
Les autres modèles (Piper, GGUF) doivent être placés manuellement dans le dossier `models/` pour respecter leurs licences respectives.
