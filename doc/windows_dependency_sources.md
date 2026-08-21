# Windows dependency sources

## Qt 6 Windows MinGW
Qt 6.4.2 Windows desktop architectures were queried through aqtinstall. The available architecture included `win64_mingw`, and the Multimedia module `qtmultimedia` was available.

## FFmpeg Windows shared build
Source page: https://www.gyan.dev/ffmpeg/builds/

The page identifies the release full shared package as `ffmpeg-release-full-shared.7z`, which redirects to the versioned package `ffmpeg-9.0.1-full_build-shared.7z`. The downloaded package contained Windows executables `ffmpeg.exe`, `ffprobe.exe`, `ffplay.exe` and shared libraries including `avcodec-63.dll`, `avdevice-63.dll`, `avfilter-12.dll`, `avformat-63.dll`, `avutil-61.dll`, `swresample-7.dll`, and `swscale-10.dll`.

FFmpeg official download page: https://ffmpeg.org/download.html

The official page states that FFmpeg provides source code and links to compiled Windows builds from third-party providers. It lists FFmpeg 9.0.1 as the latest stable release as of the retrieved page.
