# Spécifications et Exigences - CineForge Studio

Ce document liste les exigences extraites de la demande utilisateur et définit comment elles seront implémentées et testées dans CineForge Studio.

## 1. Timeline et Édition
- **Exigence** : Timeline multipiste avec sélection, déplacement, zoom et découpe.
- **État cible** : Doit permettre le glisser-déposer de médias depuis la bibliothèque, le déplacement libre des clips avec magnétisme (snap to grid/clips), le trim (ajustement des bords) et la découpe (razor tool) via raccourcis clavier.

## 2. Prévisualisation
- **Exigence** : Lecteur vidéo Qt intégré.
- **État cible** : Le lecteur doit être synchronisé avec le playhead de la timeline. Il doit afficher un aperçu en temps réel (ou quasi-réel via proxies) des clips empilés, avec support du scrubbing (déplacement rapide).

## 3. Montage Intelligent et Analyse
- **Exigence** : Détection intelligente des meilleurs passages, visages, flou et scènes. Agent qui analyse réellement le contenu.
- **État cible** : L'analyseur Python (`analyze_media.py`) doit être étendu pour utiliser des bibliothèques de vision (ex: OpenCV/dlib) afin de détecter les visages, calculer la netteté (blur detection) et segmenter les scènes. L'agent GGUF utilisera ces métadonnées pour générer un plan JSON sélectionnant les segments les plus pertinents.

## 4. Audio Avancé
- **Exigence** : Mixage audio professionnel, waveform, ducking et réduction du bruit.
- **État cible** : La timeline doit afficher les formes d'onde (waveforms). Le moteur FFmpeg doit intégrer des filtres de ducking (baisse du volume de la musique sous la voix off) et de normalisation/réduction de bruit (ex: `afftdn`, `loudnorm`).

## 5. Effets et Animation
- **Exigence** : Keyframes avancées, transitions, masques, effets et speed ramping.
- **État cible** : Le modèle de projet C++ doit supporter des keyframes pour l'échelle, la position et l'opacité. Le moteur FFmpeg doit générer des filtres complexes (`zoompan`, `fade`, `overlay`) basés sur ces keyframes.

## 6. Performances et Distribution
- **Exigence** : Accélération GPU, proxies 4K et installeur Windows.
- **État cible** : Implémenter la génération de proxies basse résolution pour la timeline. Utiliser les encodeurs matériels (`h264_nvenc`, `h264_vaapi`) lors du rendu final si disponibles. Créer un script de packaging NSIS/InnoSetup pour Windows.
