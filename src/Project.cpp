#include "ova/Project.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <stdexcept>

namespace ova {
namespace {
std::string extensionOf(const std::filesystem::path& path) {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension;
}

bool in(const std::string& value, std::initializer_list<const char*> values) {
    for (const auto* candidate : values) {
        if (value == candidate) return true;
    }
    return false;
}

std::string lowerName(const std::filesystem::path& path) {
    auto value = path.filename().string();
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string naturalKey(const std::filesystem::path& path) {
    const auto name = lowerName(path);
    std::string key;
    for (std::size_t i = 0; i < name.size();) {
        if (std::isdigit(static_cast<unsigned char>(name[i]))) {
            std::size_t end = i;
            while (end < name.size() && std::isdigit(static_cast<unsigned char>(name[end]))) ++end;
            const auto number = name.substr(i, end - i);
            key += std::string(12 > number.size() ? 12 - number.size() : 0, '0') + number;
            i = end;
        } else {
            key += name[i++];
        }
    }
    return key;
}
} // namespace

void Project::setInputDirectory(const std::filesystem::path& directory) {
    inputDirectory_ = directory;
}

void Project::setOutputFile(const std::filesystem::path& output) {
    outputFile_ = output;
}

void Project::setRenderOptions(const RenderOptions& options) {
    options_ = options;
}

MediaType Project::detectType(const std::filesystem::path& path) {
    const auto extension = extensionOf(path);
    if (in(extension, {".jpg", ".jpeg", ".png", ".webp", ".bmp", ".gif", ".tiff", ".tif"})) return MediaType::Image;
    if (in(extension, {".mp4", ".mov", ".mkv", ".avi", ".webm", ".m4v", ".mpeg", ".mpg"})) return MediaType::Video;
    if (in(extension, {".mp3", ".wav", ".m4a", ".aac", ".flac", ".ogg", ".opus"})) return MediaType::Audio;
    return MediaType::Unknown;
}

void Project::scanMedia(bool recursive) {
    media_.clear();
    if (inputDirectory_.empty() || !std::filesystem::exists(inputDirectory_)) return;

    std::error_code error;
    if (recursive) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDirectory_, error)) {
            if (error) break;
            if (!entry.is_regular_file(error)) continue;
            const auto type = detectType(entry.path());
            if (type == MediaType::Unknown || type == MediaType::Audio) continue;
            media_.push_back(MediaItem{entry.path(), type, 0.0});
        }
    } else {
        for (const auto& entry : std::filesystem::directory_iterator(inputDirectory_, error)) {
            if (error) break;
            if (!entry.is_regular_file(error)) continue;
            const auto type = detectType(entry.path());
            if (type == MediaType::Unknown || type == MediaType::Audio) continue;
            media_.push_back(MediaItem{entry.path(), type, 0.0});
        }
    }

    switch (sortStrategy_) {
        case SortStrategy::Filename:
            std::sort(media_.begin(), media_.end(), [](const MediaItem& a, const MediaItem& b) {
                return lowerName(a.path) < lowerName(b.path);
            });
            break;
        case SortStrategy::ModifiedTime:
            std::sort(media_.begin(), media_.end(), [](const MediaItem& a, const MediaItem& b) {
                std::error_code ea, eb;
                const auto ta = std::filesystem::last_write_time(a.path, ea);
                const auto tb = std::filesystem::last_write_time(b.path, eb);
                if (ea || eb || ta == tb) return lowerName(a.path) < lowerName(b.path);
                return ta < tb;
            });
            break;
        case SortStrategy::NaturalFilename:
            std::sort(media_.begin(), media_.end(), [](const MediaItem& a, const MediaItem& b) {
                const auto ka = naturalKey(a.path);
                const auto kb = naturalKey(b.path);
                return ka == kb ? lowerName(a.path) < lowerName(b.path) : ka < kb;
            });
            break;
    }
}

RenderPlan Project::makePlan() const {
    RenderPlan plan;
    plan.inputDirectory = inputDirectory_;
    plan.outputFile = outputFile_;
    plan.media = media_;
    plan.options = options_;
    return plan;
}

std::string Project::toString(SortStrategy strategy) {
    switch (strategy) {
        case SortStrategy::Filename: return "filename";
        case SortStrategy::NaturalFilename: return "natural-filename";
        case SortStrategy::ModifiedTime: return "modified-time";
    }
    return "natural-filename";
}

std::string toString(MediaType type) {
    switch (type) {
        case MediaType::Image: return "image";
        case MediaType::Video: return "video";
        case MediaType::Audio: return "audio";
        default: return "unknown";
    }
}

} // namespace ova
