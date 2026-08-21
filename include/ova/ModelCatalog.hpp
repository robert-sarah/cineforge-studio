#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ova {

enum class ModelKind {
    Whisper,
    PiperVoice,
    Agent
};

struct ModelInfo {
    std::string id;
    std::string name;
    std::string provider;
    ModelKind kind = ModelKind::Whisper;
    std::string filename;
    std::string url;
    std::string description;
    std::string sizeHint;
};

class ModelCatalog final {
public:
    static std::vector<ModelInfo> builtIn();
    static std::filesystem::path defaultDirectory(const std::filesystem::path& projectRoot = {});
    static bool isInstalled(const ModelInfo& model, const std::filesystem::path& directory);
    static std::filesystem::path modelPath(const ModelInfo& model, const std::filesystem::path& directory);
    static std::string kindLabel(ModelKind kind);
};

} // namespace ova
