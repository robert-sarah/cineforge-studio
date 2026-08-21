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

struct RenderOptions {
    int width = 1080;
    int height = 1920;
    int fps = 30;
    int crf = 20;
    std::string preset = "medium";
    bool addZoomToImages = true;
    bool burnSubtitles = true;
    bool removeSilences = false;
    std::filesystem::path subtitlesFile;
    std::filesystem::path voiceOverFile;
    std::filesystem::path musicFile;
};

struct RenderPlan {
    std::filesystem::path inputDirectory;
    std::filesystem::path outputFile = "output.mp4";
    std::vector<MediaItem> media;
    std::vector<SubtitleCue> subtitles;
    RenderOptions options;
    std::string style = "standard";
    std::string narrationText;
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
