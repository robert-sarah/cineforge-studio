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
class QDoubleSpinBox;
class QSlider;
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
    QWidget* makeAudioMixerDock();
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
    QLineEdit* musicEdit_ = nullptr;
    QLineEdit* whisperModelEdit_ = nullptr;
    QLineEdit* piperModelEdit_ = nullptr;
    QLineEdit* scriptEdit_ = nullptr;
    QPlainTextEdit* promptEdit_ = nullptr;
    QListWidget* mediaList_ = nullptr;
    QPlainTextEdit* agentLog_ = nullptr;
    
    // Clip Inspector
    QDoubleSpinBox* clipOpacitySpin_ = nullptr;
    QDoubleSpinBox* clipScaleSpin_ = nullptr;
    QDoubleSpinBox* clipPosXSpin_ = nullptr;
    QDoubleSpinBox* clipPosYSpin_ = nullptr;
    QDoubleSpinBox* clipRotationSpin_ = nullptr;
    QComboBox* clipBlendModeCombo_ = nullptr;
    QComboBox* clipTransitionInCombo_ = nullptr;
    QComboBox* clipTransitionOutCombo_ = nullptr;
    QComboBox* clipMaskCombo_ = nullptr;
    QPushButton* addKeyframeBtn_ = nullptr;
    QLineEdit* clipTextOverlayEdit_ = nullptr;
    QComboBox* clipTextStyleCombo_ = nullptr;
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
    QComboBox* encoderCombo_ = nullptr;
    QComboBox* sortCombo_ = nullptr;
    QCheckBox* zoomCheck_ = nullptr;
    QCheckBox* subtitlesCheck_ = nullptr;
    QCheckBox* normalizeAudioCheck_ = nullptr;
    QCheckBox* duckMusicCheck_ = nullptr;
    QCheckBox* proxyCheck_ = nullptr;
    
    // Audio Mixer
    QSlider* masterVolumeSlider_ = nullptr;
    QSlider* voiceVolumeSlider_ = nullptr;
    QSlider* musicVolumeSlider_ = nullptr;
    QProgressBar* masterMeter_ = nullptr;
    QLineEdit* agentModelEdit_ = nullptr;
    QSpinBox* crfSpin_ = nullptr;
    QSpinBox* proxyWidthSpin_ = nullptr;
    TimelineWidget* timeline_ = nullptr;
};

} // namespace ova
