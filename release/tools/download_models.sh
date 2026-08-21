#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

usage() {
  cat <<'EOF'
Usage: ./tools/download_models.sh [models_directory] [whisper-tiny|whisper-base]

Télécharge uniquement les modèles Whisper.cpp officiellement convertis au format ggml.
Les voix Piper et les modèles GGUF agent doivent être choisis manuellement selon leur licence,
leur langue et la mémoire disponible, puis copiés dans le dossier models/.
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

MODEL_DIR="${1:-$ROOT/models}"
NAME="${2:-whisper-tiny}"
mkdir -p "$MODEL_DIR"
case "$NAME" in
  whisper-tiny)
    FILE="ggml-tiny.bin"
    URL="https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin"
    ;;
  whisper-base)
    FILE="ggml-base.bin"
    URL="https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin"
    ;;
  *)
    echo "Modèle non autorisé par cet installateur : $NAME" >&2
    usage >&2
    exit 2
    ;;
esac

if [[ -s "$MODEL_DIR/$FILE" ]]; then
  echo "Déjà installé : $MODEL_DIR/$FILE"
  exit 0
fi

echo "Téléchargement officiel de $FILE dans $MODEL_DIR"
curl --fail --location --retry 4 --retry-delay 3 --output "$MODEL_DIR/$FILE" "$URL"
echo "Modèle installé : $MODEL_DIR/$FILE"
