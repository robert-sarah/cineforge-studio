#pragma once

#include "ova/Project.hpp"

#include <filesystem>
#include <string>

namespace ova {

class PiperVoiceEngine {
public:
    explicit PiperVoiceEngine(std::string executable = "piper");

    bool synthesize(const std::string& text,
                    const std::filesystem::path& model,
                    const std::filesystem::path& outputWav,
                    std::string* error = nullptr) const;

private:
    std::string executable_;
};

class WhisperSubtitleEngine {
public:
    explicit WhisperSubtitleEngine(std::string executable = "whisper-cli");

    bool transcribe(const std::filesystem::path& audio,
                    const std::filesystem::path& model,
                    const std::filesystem::path& outputSrt,
                    std::string* error = nullptr) const;

private:
    std::string executable_;
};

} // namespace ova
