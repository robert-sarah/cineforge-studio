# CineForge Studio

**CineForge Studio** is a C++/Qt **fully offline** video editing software, driven by a local AI agent and an FFmpeg rendering engine. It is designed to automate the organization and editing of all types of multimedia content (viral videos, cinematic formats, documentaries), without ever sending your media to a remote server.

## New in the Studio Version

- **Interactive Multitrack Timeline**: draggable clips, snapping, trimming, zooming, and audio/video tracks.
- **Integrated Video Player**: real-time render preview directly in the interface via Qt Multimedia.
- **Local GGUF AI Agent**: connection to Llama 3 (8B) via `llama.cpp` to understand complex requests and generate JSON editing plans.
- **Offline Python Analyzer**: metadata extraction using `ffprobe` and `OpenCV` for face detection, blur detection, and smart editing.
- **4K Video Proxies**: Python script to generate lightweight proxies and accelerate the editing of heavy media files.
- **Project Saving**: JSON `.cineforge` format to save and restore your timelines and media.
- **Automatic Import**: detection of images, videos, and music, with natural and chronological sorting.
- **Advanced FFmpeg Rendering**: generation of multiple formats, zoom keyframes, subtitles, and audio ducking/mixing.
- **Hardware Encoders**: native support for `NVENC`, `VAAPI`, and `VideoToolbox` for blazing-fast exports.
- **Edit History**: full Undo/Redo support in the interactive timeline.
- **Editing Templates**: MrBeast, Cinematic, Shorts, Vlog, Documentary, Gaming, Podcast, and Tutorial.
- **Professional Qt Interface**: dark theme, inspector, dockable panels, and local models manager.

## Long-Form Workflow

CineForge Studio organizes projects into **chapters**, allowing you to prepare documentaries of 45 minutes, 3 hours, or more without loading all media into memory at once. `tools/analyze_media.py` progressively analyzes files and samples long videos. `tools/generate_timeline.py` then produces a `.cineforge` project with chapters, tracks, and clips. Before rendering, `tools/validate_project.py project.cineforge` checks media indexes, durations, gaps, and timeline inconsistencies.

The FFmpeg engine renders each chapter separately in `.cineforge_cache/`, verifies the generated files, and maintains a `render_state.txt` manifest during processing. `tools/check_capabilities.py` inspects the actually available encoders and automatically recommends NVENC, VAAPI, QSV, VideoToolbox, or libx264 depending on the machine. In case of interruption, valid chapters can be reused on the next launch. Proxies remain recommended for 4K sources and multi-hour projects.

```bash
python3 tools/analyze_media.py ./my_media --vision --sample-seconds 5 --output catalog.json
python3 tools/generate_timeline.py ./my_media catalog.json --style documentary --chapter-minutes 12 --output documentary.cineforge
python3 tools/validate_project.py documentary.cineforge
```

## Compilation (Linux / WSL)

Make sure you have installed `cmake`, a C++ compiler (gcc/clang), `ffmpeg`, and `qt6-base-dev`.

```bash
mkdir build && cd build
cmake .. -DOVA_BUILD_GUI=ON
cmake --build . -j$(nproc)
```

## Using the GUI

Launch the application with:
```bash
./bin/cineforge-studio
```

1. Click on **Import Folder** to load your images and videos.
2. In the **Styles** tab, choose "MrBeast / High Energy" to automatically apply the viral plan.
3. In the **Models** tab, verify that local models are installed (e.g., `ggml-tiny.bin`). A double-click associates them with the project.
4. In the **Inspector**, adjust the voiceover script.
5. Click on **Create Video**. The C++ engine will:
   - Generate the voiceover via Piper.
   - Transcribe the audio via Whisper to generate a `.srt` file.
   - Use FFmpeg to apply zooms, embed subtitles with the viral style, and assemble everything.

## Using the CLI

The engine remains usable from the command line for automation:
```bash
./bin/cineforge-cli --folder ./my_folder --command "create a dynamic viral mrbeast short"
```

## Installing Local Models

A secure script is provided to download official Whisper models:
```bash
./tools/download_models.sh models whisper-tiny
```
Other models (Piper, GGUF) must be manually placed in the `models/` folder to comply with their respective licenses.
