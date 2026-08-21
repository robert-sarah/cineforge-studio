#pragma once

#include "ova/Project.hpp"

#include <functional>
#include <string>

namespace ova {

using ProgressCallback = std::function<void(double progress, const std::string& message)>;

class FfmpegRenderer {
public:
    explicit FfmpegRenderer(std::string ffmpegExecutable = "ffmpeg");

    bool render(const RenderPlan& plan, const ProgressCallback& progress = {}, std::string* error = nullptr) const;
    bool writeSubtitleFile(const std::vector<SubtitleCue>& cues,
                           const std::filesystem::path& destination,
                           std::string* error = nullptr) const;

private:
    std::string ffmpegExecutable_;
};

class AudioAnalyzer {
public:
    explicit AudioAnalyzer(std::string ffmpegExecutable = "ffmpeg");
    bool extractWaveform(const std::filesystem::path& mediaPath, AudioWaveform& outWaveform, std::string* error = nullptr) const;
private:
    std::string ffmpegExecutable_;
};

class LocalAgent {
public:
    RenderPlan interpret(const std::string& instruction,
                         const std::filesystem::path& defaultDirectory = {}) const;
                         
    RenderPlan interpretWithGguf(const std::string& instruction,
                                 const std::filesystem::path& modelPath,
                                 const std::filesystem::path& defaultDirectory = {}) const;

    std::string explainPlan(const RenderPlan& plan) const;

private:
    static std::filesystem::path extractDirectory(const std::string& instruction,
                                                  const std::filesystem::path& fallback);
    static std::string lower(std::string value);
};

} // namespace ova
