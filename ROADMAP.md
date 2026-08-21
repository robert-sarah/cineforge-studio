# CineForge Studio Roadmap

CineForge Studio is in active development to become a complete, professional, and fully offline C++/Qt video editing software.

## Phase 1: The Agentic Engine (Completed)
- [x] C++ and Qt 6 Architecture.
- [x] FFmpeg integration for rendering and zooming (Ken Burns).
- [x] Local AI Services: Piper (TTS) and Whisper (Transcription).
- [x] Automatic rendering styles (Viral MrBeast, Cinematic landscape).
- [x] Local models manager for `.onnx` and `.bin` weights.
- [x] Recursive import and automatic sorting (natural, alphabetical, chronological).

## Phase 2: The Interactive Timeline (In Progress)
- [ ] Drag-and-drop media from the library to the timeline.
- [ ] Cutting tool (Cut/Razor) to split clips.
- [ ] Free movement and duration adjustment of blocks.
- [ ] Multi-track audio mixer (music, SFX, voice).
- [ ] Real-time GPU preview with OpenGL/Vulkan in Qt.

## Phase 3: Universal Smart Editing
- [ ] **Scene Detection**: Analysis of imported videos to automatically isolate the best shots.
- [ ] **Screenwriter Agent**: Connection with `llama.cpp` to generate voiceover text from a theme.
- [ ] **Advanced Subtitle Editor**: Word-by-word animation, custom colors, and karaoke.
- [ ] **Automatic Sound Effects (Auto-SFX)**: Adding noises (whoosh, pop) on transitions and text appearances.
- [ ] Visual effects and transitions library (Glitch, Fade, Blur).

## Phase 4: Distribution and Ecosystem
- [ ] Windows Installer (MSI/NSIS) pre-packaged with FFmpeg.
- [ ] Proxy formats support for smooth 4K video editing.
- [ ] C++ plugin interface to add new generators (images, music).
