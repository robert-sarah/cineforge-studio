#include "ova/Engines.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

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

std::string quote(const std::string& value) {
    return quote(std::filesystem::path(value));
}

std::string filterPath(const std::filesystem::path& value) {
    std::string text = value.string();
    std::string escaped;
    for (char c : text) {
        if (c == '\\') escaped += "\\\\";
        else if (c == ':') escaped += "\\:";
        else if (c == '\'') escaped += "\\\'";
        else escaped += c;
    }
    return escaped;
}

bool run(const std::string& command, std::string* error) {
    const int code = std::system(command.c_str());
    if (code != 0 && error) *error = "Commande FFmpeg échouée (code " + std::to_string(code) + ")";
    return code == 0;
}

} // namespace

FfmpegRenderer::FfmpegRenderer(std::string ffmpegExecutable)
    : ffmpegExecutable_(std::move(ffmpegExecutable)) {}

bool FfmpegRenderer::writeSubtitleFile(const std::vector<SubtitleCue>& cues,
                                        const std::filesystem::path& destination,
                                        std::string* error) const {
    std::ofstream output(destination);
    if (!output) {
        if (error) *error = "Impossible d'écrire le fichier de sous-titres : " + destination.string();
        return false;
    }

    auto timestamp = [](double seconds) {
        if (seconds < 0.0) seconds = 0.0;
        const auto millis = static_cast<long long>(seconds * 1000.0);
        const auto hours = millis / 3600000;
        const auto minutes = (millis / 60000) % 60;
        const auto secs = (millis / 1000) % 60;
        const auto ms = millis % 1000;
        std::ostringstream result;
        result << std::setfill('0') << std::setw(2) << hours << ':'
               << std::setw(2) << minutes << ':' << std::setw(2) << secs << ','
               << std::setw(3) << ms;
        return result.str();
    };

    for (std::size_t index = 0; index < cues.size(); ++index) {
        output << index + 1 << "\n";
        output << timestamp(cues[index].startSeconds) << " --> " << timestamp(cues[index].endSeconds) << "\n";
        output << cues[index].text << "\n\n";
    }
    return true;
}

bool FfmpegRenderer::render(const RenderPlan& plan,
                            const ProgressCallback& progress,
                            std::string* error) const {
    if (plan.media.empty()) {
        if (error) *error = "Aucun fichier image ou vidéo n'a été trouvé dans le dossier source.";
        return false;
    }

    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto work = std::filesystem::temp_directory_path() / ("ova_" + std::to_string(stamp));
    std::filesystem::create_directories(work);
    const auto concatFile = work / "concat.txt";
    std::ofstream concat(concatFile);
    if (!concat) {
        if (error) *error = "Impossible de créer le fichier de concaténation temporaire.";
        return false;
    }

    const auto& o = plan.options;
    const std::string size = std::to_string(o.width) + ":" + std::to_string(o.height);
    std::size_t index = 0;
    for (const auto& item : plan.media) {
        const auto segment = work / ("segment_" + std::to_string(index++) + ".mp4");
        std::string command = quote(ffmpegExecutable_) + " -y -hide_banner -loglevel error ";
        if (item.type == MediaType::Image) {
            command += "-loop 1 -i " + quote(item.path) + " -t 3 ";
            if (o.addZoomToImages) {
                command += "-vf \"scale=" + size + ":force_original_aspect_ratio=increase,crop=" + size +
                           ",zoompan=z='min(zoom+0.0012,1.12)':d=" + std::to_string(o.fps * 3) +
                           ":s=" + std::to_string(o.width) + "x" + std::to_string(o.height) + ":fps=" + std::to_string(o.fps) + ",setsar=1\" ";
            } else {
                command += "-vf \"scale=" + size + ":force_original_aspect_ratio=increase,crop=" + size + ",setsar=1\" ";
            }
        } else {
            command += "-i " + quote(item.path) + " -vf \"scale=" + size +
                       ":force_original_aspect_ratio=increase,crop=" + size + ",setsar=1\" ";
        }
        command += "-r " + std::to_string(o.fps) + " -an -c:v libx264 -pix_fmt yuv420p -preset " +
                   o.preset + " -crf " + std::to_string(o.crf) + " " + quote(segment);
        if (progress) progress(static_cast<double>(index - 1) / plan.media.size() * 0.55,
                               "Préparation du plan " + std::to_string(index) + "/" + std::to_string(plan.media.size()));
        if (!run(command, error)) return false;
        concat << "file " << quote(segment) << "\n";
    }
    concat.close();

    const auto silentVideo = work / "assembled.mp4";
    std::string assemble = quote(ffmpegExecutable_) + " -y -hide_banner -loglevel error -f concat -safe 0 -i " +
                           quote(concatFile) + " -c copy " + quote(silentVideo);
    if (!run(assemble, error)) return false;

    std::string finalFilter;
    if (o.burnSubtitles && !o.subtitlesFile.empty() && std::filesystem::exists(o.subtitlesFile)) {
        std::string style = "FontName=Impact,FontSize=24,PrimaryColour=&H00FFFFFF,OutlineColour=&H00000000,BorderStyle=1,Outline=3,Alignment=2,MarginV=120";
        if (plan.style == "high-energy") {
            style = "FontName=Impact,FontSize=32,PrimaryColour=&H0000FFFF,OutlineColour=&H00000000,BorderStyle=1,Outline=4,Shadow=2,Alignment=2,MarginV=250";
        } else if (plan.style == "cinematic") {
            style = "FontName=Arial,FontSize=18,PrimaryColour=&H00FFFFFF,OutlineColour=&H00000000,BorderStyle=1,Outline=1,Alignment=2,MarginV=40";
        }
        finalFilter = " -vf \"subtitles='" + filterPath(o.subtitlesFile) + "':force_style='" + style + "'\"";
    }

    std::string finish = quote(ffmpegExecutable_) + " -y -hide_banner -loglevel error -i " + quote(silentVideo);
    if (!o.voiceOverFile.empty() && std::filesystem::exists(o.voiceOverFile)) {
        finish += " -i " + quote(o.voiceOverFile) + " -map 0:v:0 -map 1:a:0 -c:a aac -shortest";
    }
    if (!o.musicFile.empty() && std::filesystem::exists(o.musicFile)) {
        finish += " -i " + quote(o.musicFile);
    }
    finish += finalFilter + " -c:v libx264 -pix_fmt yuv420p -movflags +faststart " + quote(plan.outputFile);
    if (progress) progress(0.70, "Assemblage final et sous-titres");
    const bool success = run(finish, error);
    if (success && progress) progress(1.0, "Export terminé : " + plan.outputFile.string());
    std::error_code ignored;
    std::filesystem::remove_all(work, ignored);
    return success;
}

} // namespace ova
