# Architecture CineForge Studio (Version Pro)

Ce document décrit l'architecture cible pour transformer CineForge Studio en un logiciel de montage complet, intelligent et professionnel.

## 1. Moteur C++ / Qt (Cœur)
- **Timeline & UI** : Interface Qt 6 avec accélération OpenGL/Vulkan pour le lecteur vidéo et la timeline (QGraphicsView ou QML pour les performances).
- **Rendu** : `libavcodec`/`libavfilter` (FFmpeg) en C++ natif pour la prévisualisation temps réel, le proxy vidéo, et le rendu final avec accélération matérielle (NVENC, VAAPI).
- **Modèle de Projet** : Format JSON `.cineforge` gérant les pistes, clips, keyframes, transitions et effets. Sauvegarde automatique et historique Undo/Redo.

## 2. Services d'Analyse (Python)
L'analyse intelligente des médias nécessite des bibliothèques plus adaptées (OpenCV, scikit-learn, librosa) pour :
- **Détection de scènes** : Séparer les plans dans les vidéos longues.
- **Analyse d'image** : Détecter les visages, le sujet principal, la netteté et le flou.
- **Analyse audio** : Détecter les silences, la musique, et normaliser le volume.
- *Intégration* : Ces scripts Python seront appelés par le moteur C++ via des processus légers communiquant en JSON ou via pybind11.

## 3. Agent Local et Modèles (C++ / GGUF)
- **Llama.cpp** : Le cerveau de l'agent, utilisant un modèle GGUF local (ex. Llama 3 8B Instruct) pour transformer le langage naturel en JSON structuré (plan de montage, sélection de médias).
- **Whisper.cpp** : Transcription et sous-titres précis (SRT/VTT/ASS).
- **Piper TTS** : Génération de voix hors ligne, avec support de multiples locuteurs et réglages de débit.

## 4. Pipeline de Montage Intelligent
1. **Import** : L'utilisateur dépose un dossier.
2. **Analyse (Python)** : Les médias sont analysés (scènes, qualité, audio).
3. **Orchestration (Llama.cpp)** : L'agent reçoit le prompt utilisateur et les métadonnées des médias, puis produit un script JSON.
4. **Assemblage (C++)** : Le moteur C++ convertit le JSON en timeline multipiste.
5. **Édition Manuelle (Qt)** : L'utilisateur peut modifier librement la timeline, les cuts, les effets et les keyframes.
6. **Export (FFmpeg)** : Rendu haute performance avec les presets sélectionnés.
