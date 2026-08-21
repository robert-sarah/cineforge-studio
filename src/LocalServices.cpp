#include "ova/LocalServices.hpp"

#include <cstdlib>
#include <fstream>

namespace ova {
namespace {
std::string quote(const std::filesystem::path& value) {
    std::string text = value.string();
    std::string escaped = "'";
    for (char c : text) {
        if (c == '\'') escaped += "'\\''";
        else escaped += c;
    }
    escaped += "'";
    return escaped;
}

bool fail(const std::string& message, std::string* error) {
    if (error) *error = message;
    return false;
}
}

PiperVoiceEngine::PiperVoiceEngine(std::string executable)
    : executable_(std::move(executable)) {}

bool PiperVoiceEngine::synthesize(const std::string& text,
                                  const std::filesystem::path& model,
                                  const std::filesystem::path& outputWav,
                                  std::string* error) const {
    if (text.empty()) return fail("Le texte de voix off est vide.", error);
    if (!std::filesystem::exists(model)) return fail("Modèle Piper introuvable : " + model.string(), error);

    const auto textFile = outputWav.parent_path() / (outputWav.stem().string() + ".txt");
    {
        std::ofstream out(textFile);
        if (!out) return fail("Impossible de créer le texte temporaire Piper.", error);
        out << text;
    }

    const std::string command = "cat " + quote(textFile) + " | " + quote(executable_) +
        " --model " + quote(model) + " --output_file " + quote(outputWav);
    const int result = std::system(command.c_str());
    std::error_code ignored;
    std::filesystem::remove(textFile, ignored);
    if (result != 0) return fail("Piper n'a pas pu générer la voix off.", error);
    return std::filesystem::exists(outputWav);
}

WhisperSubtitleEngine::WhisperSubtitleEngine(std::string executable)
    : executable_(std::move(executable)) {}

bool WhisperSubtitleEngine::transcribe(const std::filesystem::path& audio,
                                       const std::filesystem::path& model,
                                       const std::filesystem::path& outputSrt,
                                       std::string* error) const {
    if (!std::filesystem::exists(audio)) return fail("Audio introuvable : " + audio.string(), error);
    if (!std::filesystem::exists(model)) return fail("Modèle Whisper introuvable : " + model.string(), error);

    const auto base = outputSrt.parent_path() / outputSrt.stem();
    const std::string command = quote(executable_) + " -m " + quote(model) + " -f " + quote(audio) +
                                " -osrt -of " + quote(base);
    const int result = std::system(command.c_str());
    if (result != 0) return fail("Whisper n'a pas pu générer les sous-titres.", error);

    const auto generated = base.string() + ".srt";
    if (!std::filesystem::exists(generated)) return fail("Whisper n'a pas produit le fichier SRT attendu.", error);
    std::error_code ignored;
    std::filesystem::rename(generated, outputSrt, ignored);
    if (ignored) return fail("Impossible de déplacer le fichier SRT généré.", error);
    return true;
}

} // namespace ova
