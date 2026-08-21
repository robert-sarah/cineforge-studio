#include "ova/Engines.hpp"
#include "ova/LocalServices.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace {
void usage(const char* program) {
    std::cout << "Offline Video Agent - prototype 0.1\n\n"
              << "Usage:\n"
              << "  " << program << " --folder <dossier> [--output <fichier.mp4>]\n"
              << "  " << program << " --folder <dossier> --command \"monte une vidéo virale...\"\n\n"
              << "Options:\n"
              << "  --folder        Dossier contenant images et vidéos\n"
              << "  --output        Fichier MP4 de sortie\n"
              << "  --command       Instruction locale en français\n"
              << "  --subtitles     Fichier SRT à incruster\n"
              << "  --voice         Fichier audio de voix off\n"
              << "  --script        Texte à convertir en voix off avec Piper\n"
              << "  --piper-model   Modèle Piper .onnx\n"
              << "  --audio         Audio à transcrire avec Whisper\n"
              << "  --whisper-model Modèle Whisper .bin\n"
              << "  --no-zoom       Désactive le zoom animé sur les images\n"
              << "  --sort          natural | filename | date\n";
}
}

int main(int argc, char** argv) {
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    std::filesystem::path folder;
    std::filesystem::path output = "output.mp4";
    std::filesystem::path subtitles;
    std::filesystem::path voice;
    std::filesystem::path piperModel;
    std::filesystem::path audio;
    std::filesystem::path whisperModel;
    std::string script;
    std::string command;
    bool noZoom = false;
    ova::SortStrategy sortStrategy = ova::SortStrategy::NaturalFilename;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        auto next = [&]() -> std::string {
            return (i + 1 < argc) ? std::string(argv[++i]) : std::string();
        };
        if (argument == "--folder") folder = next();
        else if (argument == "--output") output = next();
        else if (argument == "--command") command = next();
        else if (argument == "--subtitles") subtitles = next();
        else if (argument == "--voice") voice = next();
        else if (argument == "--script") script = next();
        else if (argument == "--piper-model") piperModel = next();
        else if (argument == "--audio") audio = next();
        else if (argument == "--whisper-model") whisperModel = next();
        else if (argument == "--no-zoom") noZoom = true;
        else if (argument == "--sort") {
            const auto value = next();
            if (value == "filename") sortStrategy = ova::SortStrategy::Filename;
            else if (value == "date") sortStrategy = ova::SortStrategy::ModifiedTime;
            else sortStrategy = ova::SortStrategy::NaturalFilename;
        }
        else if (argument == "--help" || argument == "-h") { usage(argv[0]); return 0; }
    }

    if (folder.empty()) {
        std::cerr << "Erreur : --folder est obligatoire.\n";
        return 2;
    }

    if (!script.empty() && !piperModel.empty()) {
        if (voice.empty()) voice = folder / "voiceover.wav";
        ova::PiperVoiceEngine piper;
        std::string serviceError;
        if (!piper.synthesize(script, piperModel, voice, &serviceError)) {
            std::cerr << "Échec Piper : " << serviceError << "\n";
            return 4;
        }
    }

    if (!audio.empty() && !whisperModel.empty()) {
        if (subtitles.empty()) subtitles = folder / "subtitles.srt";
        ova::WhisperSubtitleEngine whisper;
        std::string serviceError;
        if (!whisper.transcribe(audio, whisperModel, subtitles, &serviceError)) {
            std::cerr << "Échec Whisper : " << serviceError << "\n";
            return 5;
        }
    }

    ova::Project project;
    project.setInputDirectory(folder);
    project.setOutputFile(output);
    project.setSortStrategy(sortStrategy);
    project.scanMedia(true);
    auto plan = project.makePlan();

    if (!command.empty()) {
        ova::LocalAgent agent;
        const auto interpreted = agent.interpret(command, folder);
        plan.options = interpreted.options;
        plan.style = interpreted.style;
        if (interpreted.outputFile != "output.mp4") plan.outputFile = interpreted.outputFile;
        std::cout << agent.explainPlan(plan) << "\n";
    }

    plan.options.subtitlesFile = subtitles;
    plan.options.voiceOverFile = voice;
    if (noZoom) plan.options.addZoomToImages = false;

    ova::FfmpegRenderer renderer;
    std::string error;
    const bool success = renderer.render(plan,
        [](double progress, const std::string& message) {
            std::cout << "[" << static_cast<int>(progress * 100.0) << "%] " << message << "\n";
        }, &error);

    if (!success) {
        std::cerr << "Échec : " << error << "\n";
        return 3;
    }
    return 0;
}
