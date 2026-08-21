#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ova {

enum class MediaType { Image, Video, Audio, Unknown };

enum class Orientation { Landscape, Portrait, Square };

enum class SortStrategy { Filename, NaturalFilename, ModifiedTime };

struct MediaItem {
    std::filesystem::path path;
    MediaType type = MediaType::Unknown;
    double durationSeconds = 0.0;
};

struct SubtitleCue {
    double startSeconds = 0.0;
    double endSeconds = 0.0;
    std::string text;
};

enum class TransitionType { None, Fade, Crossfade, DipToBlack, DipToWhite, WipeLeft, WipeRight };

struct Keyframe {
    double timeSeconds = 0.0;
    double value = 0.0;
};

struct AudioWaveform {
    std::vector<float> samples;
    double samplesPerSecond = 100.0;
};

struct TimelineClip {
    std::size_t mediaIndex = 0;
    int trackIndex = 0;
    double startSeconds = 0.0;
    double durationSeconds = 3.0;
    double sourceInSeconds = 0.0;
    double sourceOutSeconds = 0.0;
    double opacity = 1.0;
    double scale = 1.0;
    double positionX = 0.5;
    double positionY = 0.5;
    double volume = 1.0;
    std::vector<Keyframe> scaleKeyframes;
    std::vector<Keyframe> opacityKeyframes;
    std::vector<Keyframe> positionXKeyframes;
    std::vector<Keyframe> positionYKeyframes;
    std::vector<Keyframe> volumeKeyframes;
    TransitionType transitionIn = TransitionType::None;
    TransitionType transitionOut = TransitionType::None;
    double transitionInDuration = 0.5;
    double transitionOutDuration = 0.5;
    std::string maskType; // "none", "circle", "rectangle"
    double maskRadius = 0.0;
    double rotation = 0.0;
    std::string blendMode = "normal"; // "normal", "multiply", "screen", "overlay"
    
    // Text and Motion Design Overlays
    std::string textOverlay;
    std::string textStyle; // "pop", "typewriter", "neon"
    std::string textColor = "white";
    int textSize = 48;
};

struct TimelineTrack {
    std::string name;
    bool audio = false;
    double volume = 1.0;
    std::vector<TimelineClip> clips;
};

struct RenderOptions {
    int width = 1080;
    int height = 1920;
    int fps = 30;
    int crf = 20;
    std::string preset = "medium";
    std::string videoEncoder = "libx264";
    bool useProxyPreview = false;
    int proxyWidth = 960;
    bool loudnessNormalization = true;
    bool duckMusicUnderVoice = true;
    bool addZoomToImages = true;
    bool burnSubtitles = true;
    bool removeSilences = false;
    bool validateProject = true;
    std::filesystem::path subtitlesFile;
    std::filesystem::path voiceOverFile;
    std::filesystem::path musicFile;
};

struct Chapter {
    std::string title;
    double startTime = 0.0;
    double duration = 0.0;
    std::vector<TimelineTrack> tracks;
    std::vector<SubtitleCue> subtitles;
    std::filesystem::path segmentOutputFile;
};

struct RenderPlan {
    std::filesystem::path inputDirectory;
    std::filesystem::path outputFile = "output.mp4";
    std::vector<MediaItem> media;
    std::vector<Chapter> chapters;
    
    // Global Audio Mix
    double masterVolume = 1.0;
    double voiceVolume = 1.0;
    double musicVolume = 1.0; // Remplace la timeline globale plate pour les formats longs
    RenderOptions options;
    std::string style = "standard";
    std::string narrationText;
    double targetDurationSeconds = 0.0;
};

class Project {
public:
    void setInputDirectory(const std::filesystem::path& directory);
    void setOutputFile(const std::filesystem::path& output);
    void setRenderOptions(const RenderOptions& options);
    void setSortStrategy(SortStrategy strategy) noexcept { sortStrategy_ = strategy; }
    SortStrategy sortStrategy() const noexcept { return sortStrategy_; }
    void scanMedia(bool recursive = true);

    const std::filesystem::path& inputDirectory() const noexcept { return inputDirectory_; }
    const std::filesystem::path& outputFile() const noexcept { return outputFile_; }
    const std::vector<MediaItem>& media() const noexcept { return media_; }
    const RenderOptions& renderOptions() const noexcept { return options_; }

    RenderPlan makePlan() const;
    static MediaType detectType(const std::filesystem::path& path);
    static std::string toString(SortStrategy strategy);

private:
    std::filesystem::path inputDirectory_;
    std::filesystem::path outputFile_ = "output.mp4";
    std::vector<MediaItem> media_;
    RenderOptions options_;
    SortStrategy sortStrategy_ = SortStrategy::NaturalFilename;
};

std::string toString(MediaType type);

} // namespace ova
