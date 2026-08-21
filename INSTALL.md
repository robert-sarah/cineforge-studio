# Installation de CineForge Studio

CineForge Studio est conçu pour être compilé facilement sous Linux et Windows, avec un minimum de dépendances lourdes, tout en offrant une interface professionnelle et des capacités d'IA locales.

## Dépendances requises

### Pour Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-multimedia-dev libqt6multimedia6-backends ffmpeg python3
```

### Pour Windows
1. Installez **Visual Studio 2022** (avec la charge de travail C++).
2. Installez **CMake**.
3. Installez **Qt 6** via le Qt Online Installer (cochez Qt Multimedia).
4. Installez **Python 3**.
5. Téléchargez les binaires de **FFmpeg** et ajoutez-les au PATH.

## Compilation

```bash
git clone https://github.com/robert-sarah/cineforge-studio.git
cd cineforge-studio
cmake -S . -B build -DCINEFORGE_BUILD_GUI=ON
cmake --build build -j$(nproc)
```

Les exécutables se trouveront dans le dossier `build/` :
- `cineforge-studio` : L'interface graphique professionnelle.
- `cineforge-cli` : L'outil en ligne de commande pour l'automatisation.

## Installation des modèles locaux (IA)

CineForge Studio peut fonctionner à 100% hors ligne, sans envoyer vos données sur des serveurs distants.
Pour activer l'IA, exécutez le script d'installation inclus :

```bash
./tools/download_models.sh --help
./tools/download_models.sh whisper-tiny
./tools/download_models.sh piper-fr-siwis
./tools/download_models.sh agent-llama3-8b
```

Les modèles seront téléchargés dans le dossier `models/` et automatiquement détectés par le logiciel.

## Lancer les tests

```bash
python3 -m unittest discover tests/
```
