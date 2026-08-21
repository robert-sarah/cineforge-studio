# Sources officielles des modèles locaux

## whisper.cpp

Source : https://github.com/ggml-org/whisper.cpp

La documentation officielle indique que les modèles convertis au format ggml peuvent être téléchargés avec `sh ./models/download-ggml-model.sh base.en`. Le binaire `whisper-cli` se compile avec CMake puis transcrit un fichier WAV. La documentation précise aussi que l’entrée attendue par l’exemple CLI est un WAV 16 bits ; FFmpeg peut convertir une source en `16000 Hz`, mono, PCM signé 16 bits avec `ffmpeg -i input.mp3 -ar 16000 -ac 1 -c:a pcm_s16le output.wav`. Les modèles ggml regroupent les paramètres, filtres mel, vocabulaire et poids dans un fichier binaire local.

## Piper et llama.cpp

Les pages officielles à vérifier avant d’ajouter des catalogues de téléchargement sont :

- Piper : https://github.com/rhasspy/piper et https://huggingface.co/rhasspy/piper-voices
- llama.cpp : https://github.com/ggml-org/llama.cpp
- Documentation GGUF/llama.cpp : https://huggingface.co/docs/hub/en/gguf-llamacpp

Les poids de modèles ne seront pas embarqués automatiquement dans l’archive de l’application : leur taille, leur licence et la compatibilité matérielle varient. L’application doit proposer un gestionnaire local permettant d’importer un fichier, de vérifier son existence et d’indiquer son chemin, tandis que l’utilisateur choisit explicitement les modèles qu’il souhaite installer.

## Vérification du 21 août 2026

Le script officiel whisper.cpp récupère les modèles depuis `https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-<nom>.bin`. Le catalogue intégré peut donc proposer notamment `tiny`, `base`, `small`, `medium`, `large-v3` et `large-v3-turbo`, tout en laissant l’utilisateur choisir selon sa mémoire et son processeur.

Le dépôt historique `rhasspy/piper` est archivé depuis le 6 octobre 2025 et renvoie vers `https://github.com/OHF-Voice/piper1-gpl`. Les voix historiques sont visibles sur `https://huggingface.co/rhasspy/piper-voices`. L’application doit afficher cette provenance et ne pas redistribuer les poids sans vérifier les licences individuelles.
