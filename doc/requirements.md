# Specifications and Requirements - CineForge Studio

This document lists the requirements extracted from the user request and defines how they will be implemented and tested in CineForge Studio.

## 1. Timeline and Editing
- **Requirement**: Multitrack timeline with selection, movement, zooming, and cutting.
- **Target State**: Must allow drag-and-drop of media from the library, free movement of clips with snapping (snap to grid/clips), trimming (edge adjustment), and cutting (razor tool) via keyboard shortcuts.

## 2. Preview
- **Requirement**: Integrated Qt video player.
- **Target State**: The player must be synchronized with the timeline playhead. It must display a real-time (or near real-time via proxies) preview of stacked clips, with support for scrubbing (fast forwarding/rewinding).

## 3. Smart Editing and Analysis
- **Requirement**: Smart detection of best passages, faces, blur, and scenes. Agent that truly analyzes the content.
- **Target State**: The Python analyzer (`analyze_media.py`) must be extended to use vision libraries (e.g., OpenCV/dlib) to detect faces, calculate sharpness (blur detection), and segment scenes. The GGUF agent will use this metadata to generate a JSON plan selecting the most relevant segments.

## 4. Advanced Audio
- **Requirement**: Professional audio mixing, waveform, ducking, and noise reduction.
- **Target State**: The timeline must display waveforms. The FFmpeg engine must integrate ducking filters (lowering music volume under voiceover) and normalization/noise reduction filters (e.g., `afftdn`, `loudnorm`).

## 5. Effects and Animation
- **Requirement**: Advanced keyframes, transitions, masks, effects, and speed ramping.
- **Target State**: The C++ project model must support keyframes for scale, position, and opacity. The FFmpeg engine must generate complex filters (`zoompan`, `fade`, `overlay`) based on these keyframes.

## 6. Performance and Distribution
- **Requirement**: GPU acceleration, 4K proxies, and Windows installer.
- **Target State**: Implement low-resolution proxy generation for the timeline. Use hardware encoders (`h264_nvenc`, `h264_vaapi`) during final rendering if available. Create an NSIS/InnoSetup packaging script for Windows.
