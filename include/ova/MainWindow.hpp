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
    void importFolder(const std::filesystem::path& folder);
    void applyTemplate(const QString& name, const QString& instruction);
    void startRender();
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
    QComboBox* formatCombo_ = nullptr;
    QComboBox* fpsCombo_ = nullptr;
    QComboBox* sortCombo_ = nullptr;
    QCheckBox* zoomCheck_ = nullptr;
    QCheckBox* subtitlesCheck_ = nullptr;
    QSpinBox* crfSpin_ = nullptr;
    TimelineWidget* timeline_ = nullptr;
};

} // namespace ova
