# Compiling CineForge Studio on Windows

CineForge Studio is fully compatible with Windows 10 and 11. It uses CMake and Visual Studio 2022 to build the C++ core and the Qt6 interface.

## Prerequisites

1. **Visual Studio 2022**
   - Install the "Desktop development with C++" workload.
   - Ensure the Windows 10/11 SDK and MSVC v143 build tools are selected.

2. **CMake (3.20 or newer)**
   - Installed automatically with Visual Studio, or available at cmake.org.

3. **Qt 6.6 or newer**
   - Use the Qt Online Installer.
   - Select `Qt 6.x.x` -> `MSVC 2022 64-bit`.
   - Ensure the **Qt Multimedia** module is checked.

4. **Python 3.10+**
   - Required for the analysis tools.

5. **FFmpeg**
   - Download a pre-compiled Windows build (e.g., from gyan.dev).
   - Extract it and add the `bin` folder to your System `PATH`.

## Building the Project

1. Open a **x64 Native Tools Command Prompt for VS 2022**.
2. Navigate to the CineForge Studio source directory.
3. Configure the project using the Windows preset:
   ```cmd
   cmake --preset windows-release -DCMAKE_PREFIX_PATH="C:\Qt\6.6.2\msvc2022_64"
   ```
   *(Adjust the Qt path according to your installation)*
4. Build the project:
   ```cmd
   cmake --build --preset windows-release --config Release
   ```

## Running the Application

The executable will be located in `build\windows-release\Release\cineforge-studio.exe`.
Since it depends on Qt DLLs, you must either run it from an environment where Qt is in the `PATH`, or use the `windeployqt` tool to copy the necessary DLLs into the output folder:

```cmd
windeployqt build\windows-release\Release\cineforge-studio.exe
```

## Advanced Features
- **Script & Prompt Workflow**: You can now load a `.md` or `.txt` script in the Agent panel, type a free-form prompt, and the GGUF agent will analyze both to build the timeline and generate the voiceover.
- **Audio & Waveform**: The timeline displays audio waveforms, and the Audio Mixer dock allows master/voice/music volume control.
- **Keyframes & Compositing**: The Clip Inspector allows precise control over position, scale, rotation, blend modes, and cubic ease-out keyframes.

## Python Tools

The Python tools in the `tools/` directory run natively on Windows. Ensure you install the required dependencies:
```cmd
pip install opencv-python numpy
```
