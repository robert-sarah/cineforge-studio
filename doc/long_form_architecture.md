# Long-Form Architecture: CineForge Studio

This document defines the architectural principles enabling CineForge Studio to process, edit, and export very long-form videos (45-minute to 3-hour documentaries, full courses, streams), while remaining robust, performant, and 100% offline.

## 1. Challenges of Long-Form Formats
1. **Memory and UI:** Loading hundreds of clips and thumbnails on a 3-hour timeline saturates RAM and slows down the interface.
2. **AI Analysis:** Analyzing 3 hours of video scene by scene (faces, sharpness) would take days on a standard CPU.
3. **FFmpeg Rendering:** A 3-hour export can crash (RAM, disk full) or be interrupted. Starting over from scratch is unacceptable.
4. **Storytelling:** A local agent with a small context window (e.g., 2048 tokens) cannot memorize or structure an entire documentary.

## 2. Architectural Solutions

### 2.1 Timeline and Project by Chapters
- The `.cineforge` project model becomes **hierarchical**: `Project -> Chapters -> Tracks -> Clips`.
- The interface only loads the **current chapter** into detailed memory. The global view displays "Chapter" blocks.
- The agent first plans a **narrative skeleton** (list of chapters with their intent), then generates the edit chapter by chapter.

### 2.2 Segmented Rendering and Concatenation (Concat Demuxer)
- The C++ engine `FfmpegRenderer` no longer tries to export 3 hours at once.
- Each chapter is rendered into a temporary file: `chapter_01.mp4`, `chapter_02.mp4`.
- In case of a crash or interruption, the render **resumes only at the unfinished chapter**.
- At the end, FFmpeg uses the `concat demuxer` (ultra-fast, no re-encoding) to assemble the final chapters.

### 2.3 Progressive Analysis and Sampling
- The Python analyzer no longer decodes every frame.
- **Pass 1 (Fast):** `ffprobe` extracts duration, resolution, and basic metadata.
- **Pass 2 (Sampled):** OpenCV extracts 1 frame per second (or every 2 seconds) to detect faces and cuts, dividing analysis time by 30 or 60.
- Proxies are generated in the background with low priority.

### 2.4 Local Agent and Narrative RAG
- The agent uses a **sliding context** system: when editing Chapter 3, it receives the generated summary of Chapter 2 and the intent of Chapter 3, but not the details of Chapter 1.
- Subtitle generation (Whisper) is also split by audio segments.

## 3. Acceptance Criteria (Testability)
- [ ] The project can load 100 media files without freezing the interface.
- [ ] The agent generates a JSON containing an array of chapters.
- [ ] The C++ engine exports the video in multiple segments and assembles them.
- [ ] If the render process is killed at chapter 2, restarting resumes at chapter 2.
