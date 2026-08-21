#include "ova/Engines.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace ova {

std::string LocalAgent::lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::filesystem::path LocalAgent::extractDirectory(const std::string& instruction,
                                                    const std::filesystem::path& fallback) {
    std::regex quoted(R"((?:dossier|répertoire|folder)\s*["']([^"']+)["'])",
                      std::regex::icase);
    std::smatch match;
    if (std::regex_search(instruction, match, quoted) && match.size() > 1) {
        return std::filesystem::path(match[1].str());
    }
    return fallback;
}

RenderPlan LocalAgent::interpret(const std::string& instruction,
                                 const std::filesystem::path& defaultDirectory) const {
    const std::string text = lower(instruction);
    RenderPlan plan;
    plan.inputDirectory = extractDirectory(instruction, defaultDirectory);
    plan.outputFile = "output.mp4";
    plan.options.width = 1080;
    plan.options.height = 1920;
    plan.options.fps = 30;
    plan.options.addZoomToImages = true;
    plan.options.burnSubtitles = true;

    if (text.find("paysage") != std::string::npos || text.find("youtube horizontal") != std::string::npos) {
        plan.options.width = 1920;
        plan.options.height = 1080;
    } else if (text.find("carré") != std::string::npos || text.find("square") != std::string::npos) {
        plan.options.width = 1080;
        plan.options.height = 1080;
    }

    if (text.find("mrbeast") != std::string::npos || text.find("viral") != std::string::npos ||
        text.find("dynamique") != std::string::npos || text.find("rapide") != std::string::npos) {
        plan.style = "high-energy";
        plan.options.addZoomToImages = true;
        plan.options.removeSilences = true;
    } else if (text.find("cinématique") != std::string::npos || text.find("cinematic") != std::string::npos) {
        plan.style = "cinematic";
        plan.options.addZoomToImages = true;
    }

    if (text.find("sans sous-titre") != std::string::npos || text.find("sans subtitle") != std::string::npos) {
        plan.options.burnSubtitles = false;
    }

    std::regex outputRegex(R"((?:sortie|export|output)\s*[=:]?\s*["']?([^\s"']+\.mp4))",
                           std::regex::icase);
    std::smatch outputMatch;
    if (std::regex_search(instruction, outputMatch, outputRegex) && outputMatch.size() > 1) {
        plan.outputFile = outputMatch[1].str();
    }

    std::regex subtitleRegex(R"((?:sous-titres?|subtitles?)\s*(?:dans|file|fichier|=|:)\s*["']?([^\s"']+\.(?:srt|vtt)))",
                             std::regex::icase);
    std::smatch subtitleMatch;
    if (std::regex_search(instruction, subtitleMatch, subtitleRegex) && subtitleMatch.size() > 1) {
        plan.options.subtitlesFile = subtitleMatch[1].str();
    }

    std::regex voiceRegex(R"((?:voix|voice|voix off)\s*(?:dans|file|fichier|=|:)\s*["']?([^\s"']+\.(?:wav|mp3|m4a)))",
                          std::regex::icase);
    std::smatch voiceMatch;
    if (std::regex_search(instruction, voiceMatch, voiceRegex) && voiceMatch.size() > 1) {
        plan.options.voiceOverFile = voiceMatch[1].str();
    }
    return plan;
}

std::string LocalAgent::explainPlan(const RenderPlan& plan) const {
    std::ostringstream out;
    out << "Plan local : style=" << plan.style << ", format=" << plan.options.width << "x"
        << plan.options.height << " @ " << plan.options.fps << " fps, "
        << plan.media.size() << " média(s), zoom=" << (plan.options.addZoomToImages ? "oui" : "non")
        << ", sous-titres=" << (plan.options.burnSubtitles ? "oui" : "non");
    return out.str();
}

} // namespace ova
