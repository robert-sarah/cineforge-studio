#include "ova/Engines.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace ova {
namespace {

std::string shellQuote(const std::string& value) {
    std::string result = "'";
    for (const char c : value) {
        if (c == '\'') result += "'\\''";
        else result += c;
    }
    result += "'";
    return result;
}

std::string readText(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

} // namespace

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
    plan.options.loudnessNormalization = true;
    plan.options.duckMusicUnderVoice = true;
    plan.options.useProxyPreview = false;

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
    } else if (text.find("cinématique") != std::string::npos || text.find("cinematic") != std::string::npos ||
               text.find("film") != std::string::npos) {
        plan.style = "cinematic";
        plan.options.width = 1920;
        plan.options.height = 1080;
        plan.options.addZoomToImages = true;
    } else if (text.find("documentaire") != std::string::npos || text.find("documentary") != std::string::npos) {
        plan.style = "documentary";
        plan.options.width = 1920;
        plan.options.height = 1080;
        plan.options.addZoomToImages = false;
    } else if (text.find("vlog") != std::string::npos || text.find("voyage") != std::string::npos) {
        plan.style = "vlog";
        plan.options.addZoomToImages = true;
    } else if (text.find("gaming") != std::string::npos || text.find("jeu vidéo") != std::string::npos ||
               text.find("stream") != std::string::npos) {
        plan.style = "gaming";
        plan.options.addZoomToImages = false;
    } else if (text.find("podcast") != std::string::npos || text.find("interview") != std::string::npos) {
        plan.style = "podcast";
        plan.options.width = 1920;
        plan.options.height = 1080;
        plan.options.addZoomToImages = false;
    } else if (text.find("formation") != std::string::npos || text.find("cours") != std::string::npos ||
               text.find("tutoriel") != std::string::npos || text.find("tutorial") != std::string::npos) {
        plan.style = "tutorial";
        plan.options.width = 1920;
        plan.options.height = 1080;
        plan.options.addZoomToImages = false;
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

RenderPlan LocalAgent::interpretWithGguf(const std::string& instruction,
                                         const std::filesystem::path& modelPath,
                                         const std::filesystem::path& defaultDirectory) const {
    RenderPlan fallback = interpret(instruction, defaultDirectory);
    if (modelPath.empty() || !std::filesystem::is_regular_file(modelPath)) return fallback;

    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto work = std::filesystem::temp_directory_path() / ("cineforge_agent_" + std::to_string(stamp));
    std::error_code ignored;
    std::filesystem::create_directories(work, ignored);
    const auto promptFile = work / "prompt.txt";
    const auto resultFile = work / "result.txt";
    const std::string prompt =
        "You are the local agent of CineForge Studio. Analyze the editing request and reply only with a compact JSON. "
        "Schema: {\\\"style\\\":\\\"high-energy|cinematic|documentary|vlog|gaming|podcast|tutorial|standard\\\","
        "\\\"width\\\":1080 or 1920,\\\"height\\\":1080 or 1920,\\\"zoom\\\":true or false,"
        "\\\"subtitles\\\":true or false,\\\"remove_silences\\\":true or false,"
        "\\\"normalize_audio\\\":true or false,\\\"duck_music\\\":true or false,"
        "\\\"use_proxies\\\":true or false}. Request: " + instruction;
    {
        std::ofstream file(promptFile);
        file << prompt;
    }

    const std::string command = "llama-cli -m " + shellQuote(modelPath.string()) +
        " -f " + shellQuote(promptFile.string()) + " -n 256 --temp 0.1 > " + shellQuote(resultFile.string()) + " 2>/dev/null";
    if (std::system(command.c_str()) != 0 || !std::filesystem::exists(resultFile)) {
        std::filesystem::remove_all(work, ignored);
        return fallback;
    }

    const std::string output = readText(resultFile);
    std::smatch match;
    RenderPlan plan = fallback;
    if (std::regex_search(output, match, std::regex(R"(\"style\"\s*:\s*\"([^\"]+)\")")) && match.size() > 1) plan.style = match[1].str();
    if (std::regex_search(output, match, std::regex(R"(\"width\"\s*:\s*(\d+))")) && match.size() > 1) plan.options.width = std::stoi(match[1].str());
    if (std::regex_search(output, match, std::regex(R"(\"height\"\s*:\s*(\d+))")) && match.size() > 1) plan.options.height = std::stoi(match[1].str());
    if (std::regex_search(output, match, std::regex(R"(\"zoom\"\s*:\s*(true|false))")) && match.size() > 1) plan.options.addZoomToImages = match[1].str() == "true";
    if (std::regex_search(output, match, std::regex(R"(\"subtitles\"\s*:\s*(true|false))")) && match.size() > 1) plan.options.burnSubtitles = match[1].str() == "true";
    if (std::regex_search(output, match, std::regex(R"(\"remove_silences\"\s*:\s*(true|false))")) && match.size() > 1) plan.options.removeSilences = match[1].str() == "true";
    if (std::regex_search(output, match, std::regex(R"(\"normalize_audio\"\s*:\s*(true|false))")) && match.size() > 1) plan.options.loudnessNormalization = match[1].str() == "true";
    if (std::regex_search(output, match, std::regex(R"(\"duck_music\"\s*:\s*(true|false))")) && match.size() > 1) plan.options.duckMusicUnderVoice = match[1].str() == "true";
    if (std::regex_search(output, match, std::regex(R"(\"use_proxies\"\s*:\s*(true|false))")) && match.size() > 1) plan.options.useProxyPreview = match[1].str() == "true";
    std::filesystem::remove_all(work, ignored);
    return plan;
}

std::string LocalAgent::explainPlan(const RenderPlan& plan) const {
    std::ostringstream out;
    out << "Local plan: style=" << plan.style << ", format=" << plan.options.width << "x"
        << plan.options.height << " @ " << plan.options.fps << " fps, "
        << plan.media.size() << " media, zoom=" << (plan.options.addZoomToImages ? "yes" : "no")
        << ", subtitles=" << (plan.options.burnSubtitles ? "yes" : "no")
        << ", audio_pro=" << (plan.options.loudnessNormalization ? "yes" : "no");
    return out.str();
}

} // namespace ova
