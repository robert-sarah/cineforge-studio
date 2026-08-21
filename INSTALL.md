# Installing CineForge Studio

CineForge Studio is designed to be easily compiled on Linux and Windows, with minimal heavy dependencies, while providing a professional interface and local AI capabilities.

## Required Dependencies

### For Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-multimedia-dev libqt6multimedia6-backends ffmpeg python3
```

### For Windows
1. Install **Visual Studio 2022** (with the C++ workload).
2. Install **CMake**.
3. Install **Qt 6** via the Qt Online Installer (check Qt Multimedia).
4. Install **Python 3**.
5. Download **FFmpeg** binaries and add them to your PATH.

## Compilation

```bash
git clone https://github.com/robert-sarah/cineforge-studio.git
cd cineforge-studio
cmake -S . -B build -DCINEFORGE_BUILD_GUI=ON
cmake --build build -j$(nproc)
```

The executables will be located in the `build/` folder:
- `cineforge-studio`: The professional graphical interface.
- `cineforge-cli`: The command-line tool for automation.

## Installing Local Models (AI)

CineForge Studio can run 100% offline, without sending your data to remote servers.
To enable AI, run the included installation script:

```bash
./tools/download_models.sh --help
./tools/download_models.sh whisper-tiny
./tools/download_models.sh piper-fr-siwis
./tools/download_models.sh agent-llama3-8b
```

The models will be downloaded to the `models/` folder and automatically detected by the software. Large weights remain excluded from the public Git repository and must comply with their respective licenses.

## Smart Media Analysis

To generate the local catalog used by the agent, run:

```bash
python3 tools/analyze_media.py /path/to/my_media --vision
python3 tools/generate_timeline.py /path/to/my_media catalog.json --style high-energy
python3 tools/make_proxies.py /path/to/my_media
```

The `--vision` mode uses OpenCV when installed to calculate a sharpness index, count visible faces in images or video samples, and estimate scene changes. Without OpenCV, the analysis remains functional using `ffprobe` metadata.

## Audio and Hardware Acceleration

CineForge detects audio files dropped in the folder, normalizes the volume, and can automatically duck the music under a voiceover. The project encoder field accepts `libx264`, `h264_nvenc`, `h264_vaapi`, and `h264_videotoolbox` when the hardware and FFmpeg version support them. An unavailable encoder must be replaced by `libx264`.

## Running Tests

```bash
python3 -m unittest discover tests/
```
