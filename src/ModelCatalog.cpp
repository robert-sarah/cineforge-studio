#include "ova/ModelCatalog.hpp"

#include <cstdlib>

namespace ova {

std::vector<ModelInfo> ModelCatalog::builtIn() {
    return {
        {"whisper-tiny", "Whisper Tiny", "ggml-org / whisper.cpp", ModelKind::Whisper,
         "ggml-tiny.bin", "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin",
         "Très rapide, adapté aux petites machines et aux sous-titres courts.", "~75 MB"},
        {"whisper-base", "Whisper Base", "ggml-org / whisper.cpp", ModelKind::Whisper,
         "ggml-base.bin", "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin",
         "Bon compromis entre vitesse et précision multilingue.", "~142 MB"},
        {"whisper-small", "Whisper Small", "ggml-org / whisper.cpp", ModelKind::Whisper,
         "ggml-small.bin", "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin",
         "Meilleure précision, demande davantage de mémoire.", "~466 MB"},
        {"piper-fr-siwis", "Piper Français — SIWIS", "OHF-Voice / Piper", ModelKind::PiperVoice,
         "fr_FR-siwis-medium.onnx", "https://huggingface.co/rhasspy/piper-voices/tree/main/fr/fr_FR/siwis/medium",
         "Voix française locale à utiliser avec Piper ou piper1-gpl.", "~60 MB + config"},
        {"piper-en-lessac", "Piper English — Lessac", "OHF-Voice / Piper", ModelKind::PiperVoice,
         "en_US-lessac-medium.onnx", "https://huggingface.co/rhasspy/piper-voices/tree/main/en/en_US/lessac/medium",
         "Voix anglaise énergique pour les formats courts.", "~60 MB + config"},
        {"agent-qwen2.5-3b", "Agent local — GGUF 3B", "llama.cpp / GGUF", ModelKind::Agent,
         "agent-qwen2.5-3b-instruct.gguf", "https://huggingface.co/Qwen/Qwen2.5-3B-Instruct-GGUF",
         "Option de cerveau local pour les plans agentiques et scripts.", "~2–3 GB"}
    };
}

std::filesystem::path ModelCatalog::defaultDirectory(const std::filesystem::path& projectRoot) {
    if (!projectRoot.empty()) return projectRoot / "models";
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / ".offline-video-agent" / "models";
    }
    return std::filesystem::current_path() / "models";
}

std::filesystem::path ModelCatalog::modelPath(const ModelInfo& model, const std::filesystem::path& directory) {
    return directory / model.filename;
}

bool ModelCatalog::isInstalled(const ModelInfo& model, const std::filesystem::path& directory) {
    const auto path = modelPath(model, directory);
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path) && std::filesystem::file_size(path) > 0;
}

std::string ModelCatalog::kindLabel(ModelKind kind) {
    switch (kind) {
        case ModelKind::Whisper: return "SOUS-TITRES";
        case ModelKind::PiperVoice: return "VOIX";
        case ModelKind::Agent: return "AGENT";
    }
    return "MODÈLE";
}

} // namespace ova
