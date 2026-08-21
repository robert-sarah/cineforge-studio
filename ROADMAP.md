# Roadmap CineForge Studio

CineForge Studio est en plein développement pour devenir un logiciel de montage C++/Qt professionnel complet et entièrement hors ligne.

## Phase 1 : Le Moteur Agentique (Terminée)
- [x] Architecture C++ et Qt 6.
- [x] Intégration de FFmpeg pour le rendu et le zoom (Ken Burns).
- [x] Services IA locaux : Piper (TTS) et Whisper (Transcription).
- [x] Styles de rendu automatiques (MrBeast viral, Cinématique paysage).
- [x] Gestionnaire de modèles locaux pour les poids `.onnx` et `.bin`.
- [x] Import récursif et tri automatique (naturel, alphabétique, chronologique).

## Phase 2 : La Timeline Interactive (En cours)
- [ ] Glisser-déposer de médias depuis la bibliothèque vers la timeline.
- [ ] Outil de découpe (Cut/Razor) pour scinder les clips.
- [ ] Déplacement et ajustement libre de la durée des blocs.
- [ ] Mixeur audio multi-pistes (musique, SFX, voix).
- [ ] Aperçu GPU en temps réel avec OpenGL/Vulkan dans Qt.

## Phase 3 : Le Montage Intelligent Universel
- [ ] **Détection de scènes** : Analyse des vidéos importées pour isoler automatiquement les meilleurs plans.
- [ ] **Agent Scénariste** : Connexion avec `llama.cpp` pour générer le texte de la voix off à partir d'un thème.
- [ ] **Éditeur de Sous-titres Avancé** : Animation mot par mot, couleurs personnalisées et karaoké.
- [ ] **Effets Sonores Automatiques (Auto-SFX)** : Ajout de bruits (whoosh, pop) sur les transitions et apparitions de texte.
- [ ] Bibliothèque d'effets visuels et de transitions (Glitch, Fade, Blur).

## Phase 4 : Distribution et Écosystème
- [ ] Installateur Windows (MSI/NSIS) pré-packagé avec FFmpeg.
- [ ] Support des formats proxy pour le montage fluide de vidéos 4K.
- [ ] Interface de plugins C++ pour ajouter de nouveaux générateurs (images, musique).
