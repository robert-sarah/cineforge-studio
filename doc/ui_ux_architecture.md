# UI/UX Architecture - Professional Version

To transform the current interface (which looks like a simple form) into a professional video editing tool (style CapCut, Premiere, DaVinci), we will use the powerful features of **Qt 6**.

## Global Theme
The application will use a native dark theme (Dark Mode). Qt allows configuring the color palette via QPalette and QSS (Qt Style Sheets).
- **Main background:** Very dark gray (`#1e1e24`)
- **Panels:** Dark gray (`#2b2b36`)
- **Accents (Buttons, selection):** Blue/Purple (`#5a5af0` or `#007aff`)
- **Text:** White/Light gray (`#e0e0e0`)

## Main Layout
The main window (`MainWindow`) will be divided into three major areas, typical of editing software:

1. **Top Left Area: Media Pool & Templates Panel**
   - A "Media" tab: Displays imported files (videos, images, audio) as a grid or list.
   - A "Templates/Styles" tab: Allows choosing the editing style (e.g., "MrBeast", "Cinematic", "Tutorial").
   - An "Agent" tab: A chat-like text field to interact with the local AI (e.g., "Cut silences and add yellow subtitles").

2. **Top Right Area: Player & Inspector**
   - **Player:** Displays the video currently being edited. Will use a `QLabel` (to display images/frames) or `QMediaPlayer` if available, but since we render with FFmpeg, we will likely use a custom player that reads generated frames.
   - **Inspector:** Displays the properties of the selected element (Clip, Text). Allows modifying scale, position, subtitle font, etc.

3. **Bottom Area: Multitrack Timeline**
   - This is the core of the application. A horizontally scrollable view (`QGraphicsView` or a custom widget with `QPainter`).
   - Video/image tracks.
   - Audio/voice tracks.
   - Subtitle track.
   - Playhead indicating the current position.

## Technical Implementation (Qt)

- **QDockWidget:** To make panels (Media, Inspector) detachable or resizable.
- **QGraphicsScene / QGraphicsView:** The best approach to create a complex Timeline. Each clip will be a `QGraphicsItem`.
- **QStyleSheet:** To apply the dark theme.

## AI and Models Integration
- The "Agent" tab will send its commands to the existing `LocalAgent` class.
- The interface will clearly display if Whisper/Piper models are loaded (green/red status indicators).
