# Official Sources for Local Models

## whisper.cpp

Source: https://github.com/ggml-org/whisper.cpp

The official documentation indicates that models converted to the ggml format can be downloaded with `sh ./models/download-ggml-model.sh base.en`. The `whisper-cli` binary compiles with CMake and then transcribes a WAV file. The documentation also specifies that the expected input for the CLI example is a 16-bit WAV; FFmpeg can convert a source to `16000 Hz`, mono, signed 16-bit PCM with `ffmpeg -i input.mp3 -ar 16000 -ac 1 -c:a pcm_s16le output.wav`. The ggml models bundle the parameters, mel filters, vocabulary, and weights into a local binary file.

## Piper and llama.cpp

The official pages to check before adding download catalogs are:

- Piper: https://github.com/rhasspy/piper and https://huggingface.co/rhasspy/piper-voices
- llama.cpp: https://github.com/ggml-org/llama.cpp
- GGUF/llama.cpp Documentation: https://huggingface.co/docs/hub/en/gguf-llamacpp

Model weights will not be automatically bundled into the application archive: their size, license, and hardware compatibility vary. The application must provide a local manager allowing users to import a file, verify its existence, and indicate its path, while the user explicitly chooses the models they want to install.

## Verification on August 21, 2026

The official whisper.cpp script retrieves models from `https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-<name>.bin`. The built-in catalog can therefore offer options such as `tiny`, `base`, `small`, `medium`, `large-v3`, and `large-v3-turbo`, leaving the user to choose based on their memory and processor.

The historical repository `rhasspy/piper` has been archived since October 6, 2025, and points to `https://github.com/OHF-Voice/piper1-gpl`. The historical voices are visible at `https://huggingface.co/rhasspy/piper-voices`. The application must display this provenance and not redistribute the weights without checking individual licenses.
