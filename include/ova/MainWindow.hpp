#pragma once

#include <QMainWindow>

#include "ova/Project.hpp"

#include <filesystem>
#include <vector>

class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QMediaPlayer;
class QAudioOutput;
class QVideoWidget;
class QStackedLayout;

namespace ova {

class TimelineWidget;

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QWidget* makeMediaDock();
    QWidget* makeInspectorDock();
    QWidget* makeAgentDock();
    QWidget* makePreview();
    QWidget* makeTimelinePanel();
    void chooseFolder();
    void saveProject();
    void openProject();
    void importFolder(const std::filesystem::path& folder);
    void applyTemplate(const QString& name, const QString& instruction);
    void startRender();
    void loadPreviewFile(const std::filesystem::path& file);
    void appendLog(const QString& text);
    void refreshMediaList();
    void rescanCurrentFolder();

    std::filesystem::path currentFolder_;
    std::vector<MediaItem> currentMedia_;

    QLineEdit* outputEdit_ = nullptr;
    QLineEdit* commandEdit_ = nullptr;
    QLineEdit* subtitlesEdit_ = nullptr;
    QLineEdit* voiceEdit_ = nullptr;
    QLineEdit* whisperModelEdit_ = nullptr;
    QLineEdit* piperModelEdit_ = nullptr;
    QLineEdit* scriptEdit_ = nullptr;
    QListWidget* mediaList_ = nullptr;
    QPlainTextEdit* agentLog_ = nullptr;
    QPlainTextEdit* renderLog_ = nullptr;
    QProgressBar* progressBar_ = nullptr;
    QPushButton* renderButton_ = nullptr;
    QLabel* previewLabel_ = nullptr;
    QLabel* mediaCountLabel_ = nullptr;
    QMediaPlayer* player_ = nullptr;
    QAudioOutput* audioOutput_ = nullptr;
    QVideoWidget* videoWidget_ = nullptr;
    QStackedLayout* previewStack_ = nullptr;
    QComboBox* formatCombo_ = nullptr;
    QComboBox* fpsCombo_ = nullptr;
    QComboBox* sortCombo_ = nullptr;
    QCheckBox* zoomCheck_ = nullptr;
    QCheckBox* subtitlesCheck_ = nullptr;
    QLineEdit* agentModelEdit_ = nullptr;
    QSpinBox* crfSpin_ = nullptr;
    TimelineWidget* timeline_ = nullptr;
};

} // namespace ova
