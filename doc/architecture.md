# CineForge Studio Architecture (Pro Version)

This document describes the target architecture to transform CineForge Studio into a complete, intelligent, and professional video editing software.

## 1. C++ / Qt Engine (Core)
- **Timeline & UI**: Qt 6 interface with OpenGL/Vulkan acceleration for the video player and timeline (QGraphicsView or QML for performance).
- **Rendering**: `libavcodec`/`libavfilter` (FFmpeg) in native C++ for real-time preview, video proxying, and final rendering with hardware acceleration (NVENC, VAAPI).
- **Project Model**: JSON `.cineforge` format managing tracks, clips, keyframes, transitions, and effects. Auto-save and Undo/Redo history.

## 2. Analysis Services (Python)
Intelligent media analysis requires more suitable libraries (OpenCV, scikit-learn, librosa) for:
- **Scene Detection**: Splitting shots in long videos.
- **Image Analysis**: Detecting faces, main subject, sharpness, and blur.
- **Audio Analysis**: Detecting silences, music, and normalizing volume.
- *Integration*: These Python scripts will be called by the C++ engine via lightweight processes communicating in JSON or via pybind11.

## 3. Local Agent and Models (C++ / GGUF)
- **Llama.cpp**: The agent's brain, using a local GGUF model (e.g., Llama 3 8B Instruct) to transform natural language into structured JSON (editing plan, media selection).
- **Whisper.cpp**: Accurate transcription and subtitles (SRT/VTT/ASS).
- **Piper TTS**: Offline voice generation, with support for multiple speakers and speed adjustments.

## 4. Smart Editing Pipeline
1. **Import**: The user drops a folder.
2. **Analysis (Python)**: Media is analyzed (scenes, quality, audio).
3. **Orchestration (Llama.cpp)**: The agent receives the user prompt and media metadata, then produces a JSON script.
4. **Assembly (C++)**: The C++ engine converts the JSON into a multitrack timeline.
5. **Manual Editing (Qt)**: The user can freely modify the timeline, cuts, effects, and keyframes.
6. **Export (FFmpeg)**: High-performance rendering with the selected presets.
