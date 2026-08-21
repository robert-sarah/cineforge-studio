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

void writeRenderState(const std::filesystem::path& path,
                      const std::string& status,
                      std::size_t chapter,
                      std::size_t total,
                      const std::string& detail = {}) {
    std::ofstream state(path, std::ios::trunc);
    if (!state) return;
    state << "status=" << status << "\\n"
          << "chapter=" << chapter << "\\n"
          << "total_chapters=" << total << "\\n";
    if (!detail.empty()) state << "detail=" << detail << "\\n";
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
    if (plan.chapters.empty()) {
        if (error) *error = "Aucun chapitre défini dans le projet.";
        return false;
    }

    const auto work = plan.outputFile.parent_path() / ".cineforge_cache";
    std::filesystem::create_directories(work);
    const auto stateFile = work / "render_state.txt";
    writeRenderState(stateFile, "running", 0, plan.chapters.size(), plan.outputFile.string());
    const auto masterConcatFile = work / "master_concat.txt";
    std::ofstream masterConcat(masterConcatFile);
    if (!masterConcat) {
        if (error) *error = "Impossible de créer le fichier de concaténation maître.";
        return false;
    }

    const auto& o = plan.options;
    const std::string size = std::to_string(o.width) + ":" + std::to_string(o.height);

    for (std::size_t cIdx = 0; cIdx < plan.chapters.size(); ++cIdx) {
        const auto& chapter = plan.chapters[cIdx];
        writeRenderState(stateFile, "chapter", cIdx + 1, plan.chapters.size(), "rendering");
        const auto chapterFile = work / ("chapter_" + std::to_string(cIdx) + ".mp4");

        std::size_t visualCount = 0;
        for (const auto& track : chapter.tracks) {
            if (!track.audio) visualCount += track.clips.size();
        }

        // Un chapitre vide ne doit jamais être ajouté au manifeste concat.
        if (visualCount == 0) {
            if (progress) progress(static_cast<double>(cIdx + 1) / plan.chapters.size() * 0.9,
                                   "Chapitre vide ignoré : " + std::to_string(cIdx + 1));
            continue;
        }

        // Le fichier est ajouté seulement après validation qu'il possède des plans.
        masterConcat << "file " << quote(chapterFile) << "\n";

        // Reprise après interruption : un chapitre valide est réutilisé.
        if (std::filesystem::exists(chapterFile) && std::filesystem::file_size(chapterFile) > 1024) {
            if (progress) progress(static_cast<double>(cIdx + 1) / plan.chapters.size() * 0.9,
                                   "Reprise du chapitre existant " + std::to_string(cIdx + 1));
            continue;
        }

        const auto chapterConcatFile = work / ("concat_c" + std::to_string(cIdx) + ".txt");
        std::ofstream chapterConcat(chapterConcatFile);
        if (!chapterConcat) {
            if (error) *error = "Impossible de créer le manifeste du chapitre " + std::to_string(cIdx + 1);
            return false;
        }

        std::size_t clipIndex = 0;
        for (const auto& track : chapter.tracks) {
            if (track.audio) continue;
            for (const auto& clip : track.clips) {
                if (clip.mediaIndex >= plan.media.size()) continue;
                const auto& item = plan.media[clip.mediaIndex];

                const auto segment = work / ("c" + std::to_string(cIdx) + "_segment_" + std::to_string(clipIndex++) + ".mp4");
                std::string command = quote(ffmpegExecutable_) + " -y -hide_banner -loglevel error ";

                if (item.type == MediaType::Image) {
                    command += "-loop 1 -i " + quote(item.path) + " -t " + std::to_string(clip.durationSeconds) + " ";
                    if (o.addZoomToImages) {
                        command += "-vf \"scale=" + size + ":force_original_aspect_ratio=increase,crop=" + size +
                                   ",zoompan=z='min(zoom+0.0012,1.12)':d=" + std::to_string(o.fps * clip.durationSeconds) +
                                   ":s=" + std::to_string(o.width) + "x" + std::to_string(o.height) + ":fps=" + std::to_string(o.fps) + ",setsar=1\" ";
                    } else {
                        command += "-vf \"scale=" + size + ":force_original_aspect_ratio=increase,crop=" + size + ",setsar=1\" ";
                    }
                } else {
                    command += "-ss " + std::to_string(clip.sourceInSeconds) + " -t " + std::to_string(clip.durationSeconds) + " -i " + quote(item.path) +
                               " -vf \"scale=" + size + ":force_original_aspect_ratio=increase,crop=" + size + ",setsar=1\" ";
                }

                const bool hardware = o.videoEncoder == "h264_nvenc" || o.videoEncoder == "h264_vaapi" || o.videoEncoder == "h264_videotoolbox";
                command += "-r " + std::to_string(o.fps) + " -an -c:v " + (o.videoEncoder.empty() ? "libx264" : o.videoEncoder) +
                           " -pix_fmt yuv420p ";
                if (hardware) command += "-preset p4 -cq " + std::to_string(std::min(35, o.crf + 3));
                else command += "-preset " + o.preset + " -crf " + std::to_string(o.crf);
                command += " " + quote(segment);

                if (progress) progress(static_cast<double>(cIdx) / plan.chapters.size() * 0.9,
                                       "Chapitre " + std::to_string(cIdx + 1) + " - Plan " + std::to_string(clipIndex) + "/" + std::to_string(visualCount));
                if (!run(command, error)) return false;
                chapterConcat << "file " << quote(segment) << "\n";
            }
        }
        chapterConcat.close();

        std::string assemble = quote(ffmpegExecutable_) + " -y -hide_banner -loglevel error -f concat -safe 0 -i " +
                               quote(chapterConcatFile) + " -c copy " + quote(chapterFile);
        if (!run(assemble, error)) {
            writeRenderState(stateFile, "failed", cIdx + 1, plan.chapters.size(), error ? *error : "chapter assemble failed");
            return false;
        }
        if (!std::filesystem::exists(chapterFile) || std::filesystem::file_size(chapterFile) <= 1024) {
            if (error) *error = "Le chapitre " + std::to_string(cIdx + 1) + " n'a pas produit un fichier valide.";
            writeRenderState(stateFile, "failed", cIdx + 1, plan.chapters.size(), error ? *error : "invalid chapter");
            return false;
        }
    }
    masterConcat.close();
    if (std::filesystem::file_size(masterConcatFile) == 0) {
        if (error) *error = "Aucun plan vidéo valide à assembler dans les chapitres.";
        return false;
    }

    const auto silentVideo = work / "master_assembled.mp4";
    std::string masterAssemble = quote(ffmpegExecutable_) + " -y -hide_banner -loglevel error -f concat -safe 0 -i " +
                                 quote(masterConcatFile) + " -c copy " + quote(silentVideo);
    if (!run(masterAssemble, error)) return false;

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
    if (success) writeRenderState(stateFile, "complete", plan.chapters.size(), plan.chapters.size(), plan.outputFile.string());
    else writeRenderState(stateFile, "failed", plan.chapters.size(), plan.chapters.size(), error ? *error : "final assemble failed");
    if (success && progress) progress(1.0, "Export terminé : " + plan.outputFile.string());
    std::error_code ignored;
    std::filesystem::remove_all(work, ignored);
    return success;
}

} // namespace ova
