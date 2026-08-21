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
    std::size_t visualCount = 0;
    for (const auto& item : plan.media) if (item.type != MediaType::Audio) ++visualCount;
    if (visualCount == 0) {
        if (error) *error = "Le dossier ne contient aucun média vidéo ou image exploitable.";
        return false;
    }
    for (const auto& item : plan.media) {
        if (item.type == MediaType::Audio) continue;
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
        const bool hardware = o.videoEncoder == "h264_nvenc" || o.videoEncoder == "h264_vaapi" || o.videoEncoder == "h264_videotoolbox";
        command += "-r " + std::to_string(o.fps) + " -an -c:v " + (o.videoEncoder.empty() ? "libx264" : o.videoEncoder) +
                   " -pix_fmt yuv420p ";
        if (hardware) command += "-preset p4 -cq " + std::to_string(std::min(35, o.crf + 3));
        else command += "-preset " + o.preset + " -crf " + std::to_string(o.crf);
        command += " " + quote(segment);
        if (progress) progress(static_cast<double>(index) / visualCount * 0.55,
                               "Préparation du plan " + std::to_string(index) + "/" + std::to_string(visualCount));
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
        } else if (plan.style == "documentary") {
            style = "FontName=Georgia,FontSize=22,PrimaryColour=&H00FFFFFF,OutlineColour=&H00101010,BorderStyle=1,Outline=2,Shadow=1,Alignment=2,MarginV=70";
        } else if (plan.style == "vlog") {
            style = "FontName=Arial,FontSize=27,PrimaryColour=&H00FFFFFF,OutlineColour=&H00000000,BorderStyle=1,Outline=3,Shadow=2,Alignment=2,MarginV=110";
        } else if (plan.style == "gaming") {
            style = "FontName=Arial,FontSize=30,PrimaryColour=&H0000FFFF,OutlineColour=&H00000000,BorderStyle=1,Outline=4,Shadow=2,Alignment=2,MarginV=180";
        } else if (plan.style == "podcast") {
            style = "FontName=Arial,FontSize=24,PrimaryColour=&H00FFFFFF,OutlineColour=&H00303030,BorderStyle=1,Outline=2,Alignment=2,MarginV=55";
        } else if (plan.style == "tutorial") {
            style = "FontName=Arial,FontSize=23,PrimaryColour=&H00FFFFFF,OutlineColour=&H00000000,BorderStyle=1,Outline=2,Alignment=2,MarginV=85";
        }
        finalFilter = " -vf \"subtitles='" + filterPath(o.subtitlesFile) + "':force_style='" + style + "'\"";
    }

    std::filesystem::path voice = o.voiceOverFile;
    std::filesystem::path music = o.musicFile;
    if ((voice.empty() || !std::filesystem::exists(voice)) && (music.empty() || !std::filesystem::exists(music))) {
        for (const auto& item : plan.media) {
            if (item.type == MediaType::Audio && std::filesystem::exists(item.path)) {
                music = item.path;
                break;
            }
        }
    }
    const bool hasVoice = !voice.empty() && std::filesystem::exists(voice);
    const bool hasMusic = !music.empty() && std::filesystem::exists(music);
    std::string finish = quote(ffmpegExecutable_) + " -y -hide_banner -loglevel error -i " + quote(silentVideo);
    if (hasVoice && hasMusic && o.duckMusicUnderVoice) {
        finish += " -i " + quote(voice) + " -i " + quote(music) + " -map 0:v:0 -filter_complex \"[1:a]";
        if (o.loudnessNormalization) finish += "loudnorm=I=-16:TP=-1.5:LRA=11";
        finish += "[voice];[2:a]volume=0.18[music];[music][voice]sidechaincompress=threshold=0.03:ratio=8:attack=20:release=300[ducked];[voice][ducked]amix=inputs=2:duration=longest:normalize=0[aout]\" -map \"[aout]\" -c:a aac -shortest";
    } else if (hasVoice || hasMusic) {
        const auto& audio = hasVoice ? voice : music;
        finish += " -i " + quote(audio) + " -map 0:v:0 -map 1:a:0";
        if (o.loudnessNormalization) finish += " -af loudnorm=I=-16:TP=-1.5:LRA=11";
        finish += " -c:a aac -shortest";
    } else {
        finish += " -an";
    }
    const bool hardware = o.videoEncoder == "h264_nvenc" || o.videoEncoder == "h264_vaapi" || o.videoEncoder == "h264_videotoolbox";
    finish += finalFilter + " -c:v " + (o.videoEncoder.empty() ? "libx264" : o.videoEncoder) + " -pix_fmt yuv420p ";
    if (hardware) finish += "-preset p4 -cq " + std::to_string(std::min(35, o.crf + 3));
    else finish += "-preset " + o.preset + " -crf " + std::to_string(o.crf);
    finish += " -movflags +faststart " + quote(plan.outputFile);
    if (progress) progress(0.70, "Assemblage final et sous-titres");
    const bool success = run(finish, error);
    if (success && progress) progress(1.0, "Export terminé : " + plan.outputFile.string());
    std::error_code ignored;
    std::filesystem::remove_all(work, ignored);
    return success;
}

} // namespace ova
