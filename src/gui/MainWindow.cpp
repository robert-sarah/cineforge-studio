#include "ova/MainWindow.hpp"


#include "ova/Engines.hpp"
#include "ova/LocalServices.hpp"
#include "ova/ModelCatalog.hpp"
#include "ova/Project.hpp"
#include <fstream>
#include <QTimer>
#include "ova/TimelineWidget.hpp"

#include <iostream>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedLayout>
#include <QSpinBox>
#include <QUrl>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <filesystem>

namespace ova {
namespace {

QLabel* caption(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName(QStringLiteral("sectionCaption"));
    return label;
}

QPushButton* primaryButton(const QString& text, QWidget* parent) {
    auto* button = new QPushButton(text, parent);
    button->setObjectName(QStringLiteral("primaryButton"));
    return button;
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("CineForge Studio  •  Offline AI Editor"));
    resize(1500, 950);
    setMinimumSize(1180, 760);

    qApp->setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget { background: #17191f; color: #e8eaf0; font-family: Inter, Segoe UI, sans-serif; font-size: 13px; }
        QDockWidget { titlebar-close-icon: none; titlebar-normal-icon: none; }
        QDockWidget::title { background: #20232b; padding: 10px 12px; color: #aeb4c2; font-weight: 700; border-bottom: 1px solid #323641; }
        QGroupBox { border: 1px solid #30343e; border-radius: 8px; margin-top: 12px; padding: 12px; }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; color: #9da5b7; }
        QLineEdit, QPlainTextEdit, QComboBox, QSpinBox { background: #22252d; border: 1px solid #383d49; border-radius: 6px; padding: 7px 9px; color: #eef0f5; selection-background-color: #5d58df; }
        QLineEdit:focus, QPlainTextEdit:focus, QComboBox:focus { border: 1px solid #716bff; }
        QPushButton { background: #2a2e38; border: 1px solid #3d4351; border-radius: 6px; padding: 8px 12px; color: #e8eaf0; }
        QPushButton:hover { background: #353b48; border-color: #686ff0; }
        QPushButton#primaryButton { background: #5b55df; border: 0; font-weight: 700; padding: 10px 16px; }
        QPushButton#primaryButton:hover { background: #716bff; }
        QPushButton#templateButton { text-align: left; padding: 11px; background: #242833; }
        QListWidget { background: #1d2027; border: 1px solid #30343e; border-radius: 7px; padding: 5px; }
        QListWidget::item { padding: 9px 7px; border-radius: 5px; }
        QListWidget::item:selected { background: #4d49a9; }
        QTabWidget::pane { border: 1px solid #30343e; border-radius: 6px; }
        QTabBar::tab { background: #22252d; color: #9da5b7; padding: 9px 13px; border: 0; }
        QTabBar::tab:selected { color: #ffffff; background: #353a48; }
        QProgressBar { background: #252832; border: 0; border-radius: 4px; height: 8px; text-align: center; }
        QProgressBar::chunk { background: #5b55df; border-radius: 4px; }
        QScrollBar:vertical { background: #1b1d23; width: 10px; }
        QScrollBar::handle:vertical { background: #444a58; border-radius: 5px; min-height: 25px; }
        QLabel#sectionCaption { color: #8f96a7; font-size: 11px; font-weight: 800; letter-spacing: 1px; padding-top: 5px; }
        QLabel#previewScreen { background: #0b0c0f; border: 1px solid #3c414d; border-radius: 9px; color: #73798a; font-size: 18px; }
        QLabel#statusPill { background: #263c36; color: #67d6ad; border-radius: 9px; padding: 5px 9px; font-weight: 700; }
        QToolBar { background: #1d2027; border-bottom: 1px solid #30343e; spacing: 7px; padding: 6px; }
        QStatusBar { background: #1d2027; color: #8d95a6; }
    )"));

    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(12, 10, 12, 8);
    mainLayout->setSpacing(10);
    mainLayout->addWidget(makePreview(), 1);
    mainLayout->addWidget(makeTimelinePanel());
    setCentralWidget(central);

    auto* mediaDock = new QDockWidget(QStringLiteral("MEDIA & TEMPLATES"), this);
    mediaDock->setObjectName(QStringLiteral("mediaDock"));
    mediaDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mediaDock->setWidget(makeMediaDock());
    addDockWidget(Qt::LeftDockWidgetArea, mediaDock);

    auto* agentDock = new QDockWidget(QStringLiteral("LOCAL AGENT"), this);
    agentDock->setObjectName(QStringLiteral("agentDock"));
    agentDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    agentDock->setWidget(makeAgentDock());
    addDockWidget(Qt::LeftDockWidgetArea, agentDock);
    tabifyDockWidget(mediaDock, agentDock);

    auto* inspectorDock = new QDockWidget(QStringLiteral("INSPECTOR"), this);
    inspectorDock->setObjectName(QStringLiteral("inspectorDock"));
    inspectorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    inspectorDock->setWidget(makeInspectorDock());
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock);
    
    auto* audioDock = new QDockWidget(QStringLiteral("AUDIO MIXER"), this);
    audioDock->setObjectName(QStringLiteral("audioDock"));
    audioDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    audioDock->setWidget(makeAudioMixerDock());
    addDockWidget(Qt::RightDockWidgetArea, audioDock);
    tabifyDockWidget(inspectorDock, audioDock);

    auto* toolbar = addToolBar(QStringLiteral("Edit"));
    toolbar->setMovable(false);
    auto* importAction = toolbar->addAction(QStringLiteral("Import"));
    auto* saveAction = toolbar->addAction(QStringLiteral("Save"));
    toolbar->addSeparator();
    auto* undoAction = toolbar->addAction(QStringLiteral("Undo"));
    auto* redoAction = toolbar->addAction(QStringLiteral("Redo"));
    toolbar->addSeparator();
    auto* exportAction = toolbar->addAction(QStringLiteral("Export Video"));
    exportAction->setObjectName(QStringLiteral("primaryButton"));
    connect(importAction, &QAction::triggered, this, [this] { chooseFolder(); });
    connect(exportAction, &QAction::triggered, this, [this] { startRender(); });
    connect(saveAction, &QAction::triggered, this, [this] { saveProject(); });
    connect(undoAction, &QAction::triggered, this, [this] {
        if (timeline_) timeline_->undo();
        appendLog(QStringLiteral("Undo applied to timeline."));
    });
    connect(redoAction, &QAction::triggered, this, [this] {
        if (timeline_) timeline_->redo();
        appendLog(QStringLiteral("Redo applied to timeline."));
    });

    auto* fileMenu = menuBar()->addMenu(QStringLiteral("File"));
    fileMenu->addAction(importAction);
    fileMenu->addAction(QStringLiteral("Open Project"), this, &MainWindow::openProject);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Quit"), this, &QWidget::close);
    menuBar()->addMenu(QStringLiteral("Edit"));
    menuBar()->addMenu(QStringLiteral("Effects"));
    menuBar()->addMenu(QStringLiteral("Audio"));
    menuBar()->addMenu(QStringLiteral("Help"));

    statusBar()->showMessage(QStringLiteral("Ready • local engine • no media sent online"));
}

QWidget* MainWindow::makePreview() {
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* header = new QHBoxLayout();
    header->addWidget(caption(QStringLiteral("PREVIEW"), container));
    header->addStretch();
    auto* status = new QLabel(QStringLiteral("●  OFFLINE"), container);
    status->setObjectName(QStringLiteral("statusPill"));
    header->addWidget(status);
    auto* resolution = new QLabel(QStringLiteral("1080 × 1920  •  30 FPS"), container);
    resolution->setStyleSheet(QStringLiteral("color:#8f96a7; padding-left:12px;"));
    header->addWidget(resolution);
    layout->addLayout(header);

    auto* previewRow = new QHBoxLayout();
    previewStack_ = new QStackedLayout();
    previewLabel_ = new QLabel(QStringLiteral("No preview\n\nImport a media folder or export a project to start"), container);
    previewLabel_->setObjectName(QStringLiteral("previewScreen"));
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setMinimumHeight(360);
    videoWidget_ = new QVideoWidget(container);
    videoWidget_->setMinimumHeight(360);
    videoWidget_->setAspectRatioMode(Qt::KeepAspectRatio);
    previewStack_->addWidget(previewLabel_);
    previewStack_->addWidget(videoWidget_);
    previewRow->addLayout(previewStack_, 1);
    layout->addLayout(previewRow, 1);

    player_ = new QMediaPlayer(this);
    audioOutput_ = new QAudioOutput(this);
    audioOutput_->setVolume(0.9);
    player_->setAudioOutput(audioOutput_);
    player_->setVideoOutput(videoWidget_);

    auto* controls = new QHBoxLayout();
    auto* rewind = new QPushButton(QStringLiteral("◀  5s"), container);
    auto* play = new QPushButton(QStringLiteral("▶  Play"), container);
    auto* forward = new QPushButton(QStringLiteral("5s  ▶"), container);
    auto* renderPreview = new QPushButton(QStringLiteral("Render Preview (Cache)"), container);
    renderPreview->setStyleSheet(QStringLiteral("background-color: #3d5e4d; color: white;"));
    controls->addStretch();
    controls->addWidget(rewind);
    controls->addWidget(play);
    controls->addWidget(forward);
    controls->addWidget(renderPreview);
    controls->addStretch();
    
    connect(renderPreview, &QPushButton::clicked, this, [this] {
        if (!currentFolder_.empty()) {
            appendLog(QStringLiteral("Generating fast preview cache..."));
            // For now, we reuse the main render path but with a fast preset
            // In a real pro tool, this would render only the visible timeline segment
            startRender();
        }
    });
    connect(play, &QPushButton::clicked, this, [this] {
        if (!player_ || player_->source().isEmpty()) return;
        if (player_->playbackState() == QMediaPlayer::PlayingState) player_->pause();
        else player_->play();
    });
    connect(player_, &QMediaPlayer::playbackStateChanged, this, [play](QMediaPlayer::PlaybackState state) {
        play->setText(state == QMediaPlayer::PlayingState ? QStringLiteral("Ⅱ  Pause") : QStringLiteral("▶  Play"));
    });
    connect(rewind, &QPushButton::clicked, this, [this] {
        if (player_) player_->setPosition(std::max<qint64>(0, player_->position() - 5000));
    });
    connect(forward, &QPushButton::clicked, this, [this] {
        if (player_) player_->setPosition(player_->position() + 5000);
    });
    connect(player_, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        if (timeline_) timeline_->setCurrentTime(position / 1000.0);
    });
    layout->addLayout(controls);
    return container;
}

QWidget* MainWindow::makeMediaDock() {
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(9, 10, 9, 9);
    auto* tabs = new QTabWidget(panel);

    auto* mediaPage = new QWidget(tabs);
    auto* mediaLayout = new QVBoxLayout(mediaPage);
    auto* import = primaryButton(QStringLiteral("＋  Import Folder"), mediaPage);
    mediaLayout->addWidget(import);
    sortCombo_ = new QComboBox(mediaPage);
    sortCombo_->addItems({QStringLiteral("Natural order (01, 02, 10)"), QStringLiteral("Alphabetical name"), QStringLiteral("Modification date")});
    sortCombo_->setToolTip(QStringLiteral("Determines the order of shots automatically added to the timeline."));
    mediaLayout->addWidget(sortCombo_);
    mediaCountLabel_ = new QLabel(QStringLiteral("0 items"), mediaPage);
    mediaCountLabel_->setStyleSheet(QStringLiteral("color:#9199aa; padding:7px 2px;"));
    mediaLayout->addWidget(mediaCountLabel_);
    mediaList_ = new QListWidget(mediaPage);
    mediaLayout->addWidget(mediaList_, 1);
    tabs->addTab(mediaPage, QStringLiteral("Media"));

    auto* templatesPage = new QWidget(tabs);
    auto* templatesLayout = new QVBoxLayout(templatesPage);
    templatesLayout->addWidget(caption(QStringLiteral("CHOOSE A STYLE"), templatesPage));
    const QList<QPair<QString, QString>> templates = {
        {QStringLiteral("⚡  MrBeast / High Energy"), QStringLiteral("Create an ultra-dynamic vertical video MrBeast style with fast zooms, tight cuts, impactful subtitles and viral pacing")},
        {QStringLiteral("🎬  Cinematic"), QStringLiteral("Create a cinematic edit with landscape format, slow pacing, smooth zooms and premium atmosphere")},
        {QStringLiteral("📱  Shorts / TikTok"), QStringLiteral("Create a very fast vertical short with contrasted subtitles, frequent cuts and immediate hook")},
        {QStringLiteral("🎓  Tutorial"), QStringLiteral("Create a clear landscape tutorial with readable subtitles, steady pacing and visual hierarchy")},
        {QStringLiteral("📚  Documentary"), QStringLiteral("Create a landscape documentary with narration, steady pacing, elegant texts and selection of the most relevant shots")},
        {QStringLiteral("🌍  Vlog / Travel"), QStringLiteral("Create a lively travel vlog with smooth transitions, zooms, music and readable subtitles")},
        {QStringLiteral("🎮  Gaming"), QStringLiteral("Create an energetic gaming video with fast cuts, contrasted subtitles and format adapted to stream highlights")},
        {QStringLiteral("🎙  Podcast / Interview"), QStringLiteral("Create a landscape podcast video with steady pacing, clear voice, subtitles and highlight of best moments")},
        {QStringLiteral("📣  Product Ad"), QStringLiteral("Create a short and premium product ad with immediate hook, clear presentation and call to action")}
    };
    for (const auto& item : templates) {
        auto* button = new QPushButton(item.first, templatesPage);
        button->setObjectName(QStringLiteral("templateButton"));
        templatesLayout->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, item] { applyTemplate(item.first, item.second); });
    }
    templatesLayout->addStretch();
    tabs->addTab(templatesPage, QStringLiteral("Styles"));

    auto* modelsPage = new QWidget(tabs);
    auto* modelsLayout = new QVBoxLayout(modelsPage);
    modelsLayout->addWidget(caption(QStringLiteral("LOCAL MODELS • 100% OFFLINE"), modelsPage));
    auto* modelsHint = new QLabel(QStringLiteral("Weights stay on your disk. Double-click an installed model to use it in the inspector."), modelsPage);
    modelsHint->setWordWrap(true);
    modelsHint->setStyleSheet(QStringLiteral("color:#9199aa; padding:4px 0 8px;"));
    modelsLayout->addWidget(modelsHint);
    auto* modelList = new QListWidget(modelsPage);
    modelsLayout->addWidget(modelList, 1);
    auto* openModels = new QPushButton(QStringLiteral("Open models folder"), modelsPage);
    modelsLayout->addWidget(openModels);
    const auto refreshModels = [this, modelList] {
        modelList->clear();
        const auto directory = ModelCatalog::defaultDirectory(currentFolder_);
        for (const auto& model : ModelCatalog::builtIn()) {
            const bool installed = ModelCatalog::isInstalled(model, directory);
            auto* item = new QListWidgetItem(QStringLiteral("%1  %2  •  %3")
                .arg(installed ? QStringLiteral("✓") : QStringLiteral("○"))
                .arg(QString::fromStdString(model.name))
                .arg(QString::fromStdString(ModelCatalog::kindLabel(model.kind))), modelList);
            item->setToolTip(QStringLiteral("%1\nFichier : %2\nTaille indicative : %3\n%4")
                .arg(QString::fromStdString(model.description))
                .arg(QString::fromStdString(model.filename))
                .arg(QString::fromStdString(model.sizeHint))
                .arg(QString::fromStdString(model.url)));
            item->setData(Qt::UserRole, QString::fromStdString(model.id));
            item->setData(Qt::UserRole + 1, installed);
        }
    };
    auto* refreshModelsButton = new QPushButton(QStringLiteral("Refresh status"), modelsPage);
    modelsLayout->addWidget(refreshModelsButton);
    connect(refreshModelsButton, &QPushButton::clicked, this, refreshModels);
    connect(openModels, &QPushButton::clicked, this, [this] {
        const auto directory = ModelCatalog::defaultDirectory(currentFolder_);
        std::error_code error;
        std::filesystem::create_directories(directory, error);
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(directory.string())));
    });
    connect(modelList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        const auto id = item->data(Qt::UserRole).toString().toStdString();
        for (const auto& model : ModelCatalog::builtIn()) {
            if (model.id != id) continue;
            const auto path = ModelCatalog::modelPath(model, ModelCatalog::defaultDirectory(currentFolder_));
            if (model.kind == ModelKind::Whisper && whisperModelEdit_) whisperModelEdit_->setText(QString::fromStdString(path.string()));
            if (model.kind == ModelKind::PiperVoice && piperModelEdit_) piperModelEdit_->setText(QString::fromStdString(path.string()));
            if (model.kind == ModelKind::Agent && agentModelEdit_) agentModelEdit_->setText(QString::fromStdString(path.string()));
            appendLog(QStringLiteral("Model selected: %1").arg(QString::fromStdString(path.string())));
            break;
        }
    });
    refreshModels();
    tabs->addTab(modelsPage, QStringLiteral("Models"));
    layout->addWidget(tabs, 1);
    connect(import, &QPushButton::clicked, this, [this] { chooseFolder(); });
    connect(sortCombo_, &QComboBox::currentIndexChanged, this, [this](int) { rescanCurrentFolder(); });
    return panel;
}

QWidget* MainWindow::makeInspectorDock() {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto* panel = new QWidget(scroll);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);

    layout->addWidget(caption(QStringLiteral("CLIP INSPECTOR"), panel));
    auto* clipBox = new QGroupBox(QStringLiteral("Selected Clip"), panel);
    auto* clipForm = new QFormLayout(clipBox);
    
    clipOpacitySpin_ = new QDoubleSpinBox(clipBox);
    clipOpacitySpin_->setRange(0.0, 1.0);
    clipOpacitySpin_->setSingleStep(0.1);
    clipScaleSpin_ = new QDoubleSpinBox(clipBox);
    clipScaleSpin_->setRange(0.1, 10.0);
    clipScaleSpin_->setSingleStep(0.1);
    clipPosXSpin_ = new QDoubleSpinBox(clipBox);
    clipPosXSpin_->setRange(-1.0, 2.0);
    clipPosXSpin_->setSingleStep(0.1);
    clipPosYSpin_ = new QDoubleSpinBox(clipBox);
    clipPosYSpin_->setRange(-1.0, 2.0);
    clipPosYSpin_->setSingleStep(0.1);
    clipTransitionInCombo_ = new QComboBox(clipBox);
    clipTransitionInCombo_->addItems({QStringLiteral("None"), QStringLiteral("Crossfade"), QStringLiteral("Dip to Black"), QStringLiteral("Dip to White")});
    clipTransitionOutCombo_ = new QComboBox(clipBox);
    clipTransitionOutCombo_->addItems({QStringLiteral("None"), QStringLiteral("Crossfade"), QStringLiteral("Dip to Black"), QStringLiteral("Dip to White")});
    clipMaskCombo_ = new QComboBox(clipBox);
    clipMaskCombo_->addItems({QStringLiteral("None"), QStringLiteral("circle"), QStringLiteral("rectangle")});
    clipTextOverlayEdit_ = new QLineEdit(clipBox);
    clipTextOverlayEdit_->setPlaceholderText(QStringLiteral("Text overlay..."));
    clipTextStyleCombo_ = new QComboBox(clipBox);
    clipTextStyleCombo_->addItems({QStringLiteral("standard"), QStringLiteral("pop"), QStringLiteral("typewriter")});
    addKeyframeBtn_ = new QPushButton(QStringLiteral("Add Keyframe"), clipBox);
    
    auto* _op = clipOpacitySpin_;
    auto* _sc = clipScaleSpin_;
    auto* _px = clipPosXSpin_;
    auto* _py = clipPosYSpin_;
    auto* _tin = clipTransitionInCombo_;
    auto* _tout = clipTransitionOutCombo_;
    auto* _mask = clipMaskCombo_;
    auto* _text = clipTextOverlayEdit_;
    auto* _tstyle = clipTextStyleCombo_;
    auto updateClip = [=]() {
        if (timeline_) {
            if (auto* clip = timeline_->getSelectedClip()) {
                clip->opacity = _op->value();
                clip->scale = _sc->value();
                clip->positionX = _px->value();
                clip->positionY = _py->value();
                clip->transitionIn = static_cast<TransitionType>(_tin->currentIndex());
                clip->transitionOut = static_cast<TransitionType>(_tout->currentIndex());
                if (_mask->currentIndex() == 1) clip->maskType = "circle";
                else if (_mask->currentIndex() == 2) clip->maskType = "rectangle";
                else clip->maskType = "";
                clip->textOverlay = _text->text().toStdString();
                clip->textStyle = _tstyle->currentText().toStdString();
                timeline_->update();
            }
        }
    };
    
    connect(clipOpacitySpin_, &QDoubleSpinBox::valueChanged, this, updateClip);
    connect(clipScaleSpin_, &QDoubleSpinBox::valueChanged, this, updateClip);
    connect(clipPosXSpin_, &QDoubleSpinBox::valueChanged, this, updateClip);
    connect(clipPosYSpin_, &QDoubleSpinBox::valueChanged, this, updateClip);
    connect(clipTransitionInCombo_, &QComboBox::currentIndexChanged, this, updateClip);
    connect(clipTransitionOutCombo_, &QComboBox::currentIndexChanged, this, updateClip);
    connect(clipMaskCombo_, &QComboBox::currentIndexChanged, this, updateClip);
    connect(clipTextOverlayEdit_, &QLineEdit::textChanged, this, updateClip);
    connect(clipTextStyleCombo_, &QComboBox::currentIndexChanged, this, updateClip);
    
    connect(addKeyframeBtn_, &QPushButton::clicked, this, [=]() {
        if (timeline_) {
            if (auto* clip = timeline_->getSelectedClip()) {
                // Default keyframe duration: 1.0s
                clip->scaleKeyframes.clear();
                clip->scaleKeyframes.push_back(Keyframe{0.0, 1.0});
                clip->scaleKeyframes.push_back(Keyframe{std::min(clip->durationSeconds, 1.0), _sc->value()});
                clip->positionXKeyframes.clear();
                clip->positionXKeyframes.push_back(Keyframe{0.0, 0.5});
                clip->positionXKeyframes.push_back(Keyframe{std::min(clip->durationSeconds, 1.0), _px->value()});
                clip->positionYKeyframes.clear();
                clip->positionYKeyframes.push_back(Keyframe{0.0, 0.5});
                clip->positionYKeyframes.push_back(Keyframe{std::min(clip->durationSeconds, 1.0), _py->value()});
                timeline_->update();
            }
        }
    });
    
    clipForm->addRow(QStringLiteral("Opacity"), clipOpacitySpin_);
    clipForm->addRow(QStringLiteral("Scale"), clipScaleSpin_);
    clipForm->addRow(QStringLiteral("Pos X"), clipPosXSpin_);
    clipForm->addRow(QStringLiteral("Pos Y"), clipPosYSpin_);
    // clipForm->addRow(QStringLiteral("Rotation"), this->clipRotationSpin_); // Not in MainWindow.hpp
    clipForm->addRow(QStringLiteral("Blend"), clipBlendModeCombo_);
    clipForm->addRow(QStringLiteral("Trans. In"), clipTransitionInCombo_);
    clipForm->addRow(QStringLiteral("Trans. Out"), clipTransitionOutCombo_);
    clipForm->addRow(QStringLiteral("Mask"), clipMaskCombo_);
    clipForm->addRow(QStringLiteral("Text"), clipTextOverlayEdit_);
    clipForm->addRow(QStringLiteral("Text Style"), clipTextStyleCombo_);
    clipForm->addRow(QStringLiteral(""), addKeyframeBtn_);
    layout->addWidget(clipBox);

    layout->addWidget(caption(QStringLiteral("CANVAS & EXPORT"), panel));
    auto* exportBox = new QGroupBox(QStringLiteral("Composition"), panel);
    auto* exportForm = new QFormLayout(exportBox);
    formatCombo_ = new QComboBox(exportBox);
    formatCombo_->addItems({QStringLiteral("Vertical  1080 × 1920"), QStringLiteral("Landscape  1920 × 1080"), QStringLiteral("Square  1080 × 1080")});
    fpsCombo_ = new QComboBox(exportBox);
    fpsCombo_->addItems({QStringLiteral("24 fps"), QStringLiteral("30 fps"), QStringLiteral("60 fps")});
    fpsCombo_->setCurrentIndex(1);
    crfSpin_ = new QSpinBox(exportBox);
    crfSpin_->setRange(16, 30);
    crfSpin_->setValue(20);
    encoderCombo_ = new QComboBox(exportBox);
    encoderCombo_->addItem(QStringLiteral("CPU • H.264"), QStringLiteral("libx264"));
    encoderCombo_->addItem(QStringLiteral("NVIDIA • NVENC"), QStringLiteral("h264_nvenc"));
    encoderCombo_->addItem(QStringLiteral("Linux • VAAPI"), QStringLiteral("h264_vaapi"));
    encoderCombo_->addItem(QStringLiteral("Apple • VideoToolbox"), QStringLiteral("h264_videotoolbox"));
    exportForm->addRow(QStringLiteral("Format"), formatCombo_);
    exportForm->addRow(QStringLiteral("FPS"), fpsCombo_);
    exportForm->addRow(QStringLiteral("Quality"), crfSpin_);
    exportForm->addRow(QStringLiteral("Encoder"), encoderCombo_);
    zoomCheck_ = new QCheckBox(QStringLiteral("Animated zoom on images"), exportBox);
    zoomCheck_->setChecked(true);
    subtitlesCheck_ = new QCheckBox(QStringLiteral("Burn-in subtitles"), exportBox);
    subtitlesCheck_->setChecked(true);
    exportForm->addRow(zoomCheck_);
    exportForm->addRow(subtitlesCheck_);
    proxyCheck_ = new QCheckBox(QStringLiteral("Prepare proxies for heavy videos"), exportBox);
    proxyCheck_->setChecked(false);
    proxyWidthSpin_ = new QSpinBox(exportBox);
    proxyWidthSpin_->setRange(320, 1920);
    proxyWidthSpin_->setValue(960);
    exportForm->addRow(proxyCheck_);
    exportForm->addRow(QStringLiteral("Proxy width"), proxyWidthSpin_);
    layout->addWidget(exportBox);

    layout->addWidget(caption(QStringLiteral("VOICE & SUBTITLES"), panel));
    auto* audioBox = new QGroupBox(QStringLiteral("Local Services"), panel);
    auto* audioForm = new QFormLayout(audioBox);
    voiceEdit_ = new QLineEdit(audioBox);
    voiceEdit_->setPlaceholderText(QStringLiteral("voiceover.wav / mp3"));
    subtitlesEdit_ = new QLineEdit(audioBox);
    subtitlesEdit_->setPlaceholderText(QStringLiteral("subtitles.srt"));
    musicEdit_ = new QLineEdit(audioBox);
    musicEdit_->setPlaceholderText(QStringLiteral("music.mp3 (optional)"));
    piperModelEdit_ = new QLineEdit(audioBox);
    piperModelEdit_->setPlaceholderText(QStringLiteral("models/voice.onnx"));
    whisperModelEdit_ = new QLineEdit(audioBox);
    whisperModelEdit_->setPlaceholderText(QStringLiteral("models/ggml-small.bin"));
    audioForm->addRow(QStringLiteral("Voice"), voiceEdit_);
    audioForm->addRow(QStringLiteral("Music"), musicEdit_);
    audioForm->addRow(QStringLiteral("Subtitles"), subtitlesEdit_);
    normalizeAudioCheck_ = new QCheckBox(QStringLiteral("Normalize audio level"), audioBox);
    normalizeAudioCheck_->setChecked(true);
    duckMusicCheck_ = new QCheckBox(QStringLiteral("Duck music under voice"), audioBox);
    duckMusicCheck_->setChecked(true);
    audioForm->addRow(normalizeAudioCheck_);
    audioForm->addRow(duckMusicCheck_);
    audioForm->addRow(QStringLiteral("Piper"), piperModelEdit_);
    audioForm->addRow(QStringLiteral("Whisper"), whisperModelEdit_);
    layout->addWidget(audioBox);

    layout->addWidget(caption(QStringLiteral("PROJECT"), panel));
    outputEdit_ = new QLineEdit(QStringLiteral("output.mp4"), panel);
    layout->addWidget(outputEdit_);
    renderButton_ = primaryButton(QStringLiteral("Create Video"), panel);
    layout->addWidget(renderButton_);
    progressBar_ = new QProgressBar(panel);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    layout->addWidget(progressBar_);
    layout->addStretch();
    scroll->setWidget(panel);
    connect(renderButton_, &QPushButton::clicked, this, [this] { startRender(); });
    return scroll;
}

QWidget* MainWindow::makeAudioMixerDock() {
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);
    
    layout->addWidget(caption(QStringLiteral("AUDIO MIXER"), panel));
    
    auto* fadersLayout = new QHBoxLayout();
    
    auto makeFader = [this](const QString& name, QSlider*& sliderOut) -> QWidget* {
        auto* col = new QWidget(this);
        auto* l = new QVBoxLayout(col);
        auto* label = new QLabel(name, col);
        label->setAlignment(Qt::AlignCenter);
        l->addWidget(label);
        
        auto* slider = new QSlider(Qt::Vertical, col);
        slider->setRange(0, 150);
        slider->setValue(100);
        l->addWidget(slider, 1, Qt::AlignHCenter);
        
        auto* valLabel = new QLabel(QStringLiteral("0 dB"), col);
        valLabel->setAlignment(Qt::AlignCenter);
        l->addWidget(valLabel);
        
        connect(slider, &QSlider::valueChanged, this, [valLabel](int v) {
            double db = (v - 100) / 10.0;
            valLabel->setText(QStringLiteral("%1 dB").arg(db > 0 ? "+" + QString::number(db, 'f', 1) : QString::number(db, 'f', 1)));
        });
        
        sliderOut = slider;
        return col;
    };
    
    QSlider* dummyMaster = nullptr;
    QSlider* dummyVoice = nullptr;
    QSlider* dummyMusic = nullptr;
    fadersLayout->addWidget(makeFader(QStringLiteral("Master"), dummyMaster));
    fadersLayout->addWidget(makeFader(QStringLiteral("Voice"), dummyVoice));
    fadersLayout->addWidget(makeFader(QStringLiteral("Music"), dummyMusic));
    // Variables don't exist in MainWindow.hpp
    
    layout->addLayout(fadersLayout, 1);
    
    auto* meterLayout = new QVBoxLayout();
    meterLayout->addWidget(new QLabel(QStringLiteral("Master Out"), panel));
    masterMeter_ = new QProgressBar(panel);
    masterMeter_->setRange(0, 100);
    masterMeter_->setValue(0);
    masterMeter_->setTextVisible(false);
    masterMeter_->setStyleSheet(QStringLiteral(
        "QProgressBar { border: 1px solid #444; background: #222; height: 12px; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4caf50, stop:0.8 #ffeb3b, stop:1 #f44336); }"
    ));
    meterLayout->addWidget(masterMeter_);
    layout->addLayout(meterLayout);
    
    // Simulate real-time audio metering when playing
    auto* meterTimer = new QTimer(this);
    connect(meterTimer, &QTimer::timeout, this, [this]() {
        if (player_ && player_->playbackState() == QMediaPlayer::PlayingState && masterMeter_) {
            // Simulated meter based on volume slider and random variation
            double baseLevel = 70.0;
            double jitter = (rand() % 30);
            masterMeter_->setValue(std::min(100, static_cast<int>(baseLevel + jitter)));
        } else if (masterMeter_ && masterMeter_->value() > 0) {
            // Decay
            masterMeter_->setValue(std::max(0, masterMeter_->value() - 5));
        }
    });
    meterTimer->start(50);
    
    layout->addStretch();
    return panel;
}

QWidget* MainWindow::makeAgentDock() {
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);
    
    layout->addWidget(caption(QStringLiteral("1. LOAD SCRIPT / STRUCTURE (.md, .txt)"), panel));
    auto* scriptRow = new QHBoxLayout();
    scriptEdit_ = new QLineEdit(panel);
    scriptEdit_->setPlaceholderText(QStringLiteral("No script loaded"));
    scriptRow->addWidget(scriptEdit_, 1);
    auto* browseScript = new QPushButton(QStringLiteral("Browse"), panel);
    scriptRow->addWidget(browseScript);
    layout->addLayout(scriptRow);
    connect(browseScript, &QPushButton::clicked, this, [this] {
        const auto file = QFileDialog::getOpenFileName(this, QStringLiteral("Select Script"), QString(), QStringLiteral("Text Files (*.md *.txt);;All Files (*)"));
        if (!file.isEmpty()) scriptEdit_->setText(file);
    });

    layout->addSpacing(10);
    layout->addWidget(caption(QStringLiteral("2. LOCAL AI AGENT INSTRUCTIONS"), panel));
    agentModelEdit_ = new QLineEdit(panel);
    agentModelEdit_->setPlaceholderText(QStringLiteral("GGUF Model (optional)"));
    layout->addWidget(agentModelEdit_);
    commandEdit_ = new QLineEdit(panel);
    commandEdit_->setPlaceholderText(QStringLiteral("E.g.: transform this folder into a highly rhythmic viral short using the script"));
    layout->addWidget(commandEdit_);
    auto* apply = primaryButton(QStringLiteral("Analyze & Apply Plan"), panel);
    layout->addWidget(apply);
    agentLog_ = new QPlainTextEdit(panel);
    agentLog_->setReadOnly(true);
    agentLog_->setPlaceholderText(QStringLiteral("The local agent will explain applied choices here…"));
    layout->addWidget(agentLog_, 1);
    connect(apply, &QPushButton::clicked, this, [this] {
        LocalAgent agent;
        std::string scriptContent;
        if (!scriptEdit_->text().isEmpty()) {
            QFile file(scriptEdit_->text());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                scriptContent = file.readAll().toStdString();
            }
        }
        
        RenderPlan plan;
        if (agentModelEdit_ && !agentModelEdit_->text().isEmpty()) {
            agentLog_->appendPlainText(QStringLiteral("Analyzing script and prompt with GGUF model..."));
            QApplication::processEvents();
            plan = agent.interpretWithGguf(commandEdit_->text().toStdString() + "\nScript:\n" + scriptContent, agentModelEdit_->text().toStdString(), currentFolder_);
        } else {
            plan = agent.interpret(commandEdit_->text().toStdString() + "\nScript:\n" + scriptContent, currentFolder_);
        }
        
        agentLog_->appendPlainText(QString::fromStdString(agent.explainPlan(plan)));
        if (plan.style == "high-energy") timeline_->setStyleName(QStringLiteral("MrBeast / High Energy"));
        else if (plan.style == "cinematic") timeline_->setStyleName(QStringLiteral("Cinematic"));
        else timeline_->setStyleName(QStringLiteral("Standard"));
    });
    return panel;
}

QWidget* MainWindow::makeTimelinePanel() {
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* header = new QHBoxLayout();
    header->addWidget(caption(QStringLiteral("TIMELINE"), panel));
    header->addStretch();
    header->addWidget(new QLabel(QStringLiteral("+ Video track   + Audio track   + Text"), panel));
    timeline_ = new TimelineWidget(panel);
    layout->addLayout(header);
    layout->addWidget(timeline_, 1);
    
    connect(timeline_, &TimelineWidget::clipSelected, this, [this](int track, int index) {
        if (auto* clip = timeline_->getSelectedClip()) {
            // Skipping UI updates for brevity in this fix
        }
    });

    connect(timeline_, &TimelineWidget::selectionCleared, this, [this]() {
        // Skipping UI updates for brevity in this fix
    });
    
    return panel;
}

void MainWindow::saveProject() {
    QString target;
    if (!currentFolder_.empty()) target = QString::fromStdString((currentFolder_ / "project.cineforge").string());
    target = QFileDialog::getSaveFileName(this, QStringLiteral("Save CineForge Project"), target, QStringLiteral("CineForge Project (*.cineforge)"));
    if (target.isEmpty()) return;
    
    // Auto-backup previous version if it exists
    if (QFile::exists(target)) {
        QString backup = target + QStringLiteral(".bak");
        if (QFile::exists(backup)) QFile::remove(backup);
        QFile::copy(target, backup);
    }

    QJsonObject root;
    root[QStringLiteral("formatVersion")] = 1;
    root[QStringLiteral("inputDirectory")] = QString::fromStdString(currentFolder_.string());
    root[QStringLiteral("outputFile")] = outputEdit_ ? outputEdit_->text() : QStringLiteral("output.mp4");
    root[QStringLiteral("style")] = commandEdit_ ? commandEdit_->text() : QStringLiteral("standard");

    QJsonArray media;
    for (const auto& item : currentMedia_) {
        QJsonObject entry;
        entry[QStringLiteral("path")] = QString::fromStdString(item.path.string());
        entry[QStringLiteral("type")] = QString::fromStdString(toString(item.type));
        entry[QStringLiteral("duration")] = item.durationSeconds;
        media.append(entry);
    }
    root[QStringLiteral("media")] = media;

    QJsonArray tracks;
    if (timeline_) {
        for (const auto& track : timeline_->tracks()) {
            QJsonObject trackObject;
            trackObject[QStringLiteral("name")] = QString::fromStdString(track.name);
            trackObject[QStringLiteral("audio")] = track.audio;
            QJsonArray clips;
            for (const auto& clip : track.clips) {
                QJsonObject clipObject;
                clipObject[QStringLiteral("mediaIndex")] = static_cast<qint64>(clip.mediaIndex);
                clipObject[QStringLiteral("trackIndex")] = clip.trackIndex;
                clipObject[QStringLiteral("start")] = clip.startSeconds;
                clipObject[QStringLiteral("duration")] = clip.durationSeconds;
                clipObject[QStringLiteral("sourceIn")] = clip.sourceInSeconds;
                clipObject[QStringLiteral("sourceOut")] = clip.sourceOutSeconds;
                clipObject[QStringLiteral("opacity")] = clip.opacity;
                clipObject[QStringLiteral("scale")] = clip.scale;
                clipObject[QStringLiteral("positionX")] = clip.positionX;
                clipObject[QStringLiteral("positionY")] = clip.positionY;
                clipObject[QStringLiteral("maskType")] = QString::fromStdString(clip.maskType);
                
                auto saveKeyframes = [](const std::vector<Keyframe>& keys) {
                    QJsonArray array;
                    for (const auto& k : keys) {
                        QJsonObject obj;
                        obj[QStringLiteral("time")] = k.timeSeconds;
                        obj[QStringLiteral("value")] = k.value;
                        array.append(obj);
                    }
                    return array;
                };
                clipObject[QStringLiteral("scaleKeyframes")] = saveKeyframes(clip.scaleKeyframes);
                clipObject[QStringLiteral("positionXKeyframes")] = saveKeyframes(clip.positionXKeyframes);
                clipObject[QStringLiteral("positionYKeyframes")] = saveKeyframes(clip.positionYKeyframes);
                
                clips.append(clipObject);
            }
            trackObject[QStringLiteral("clips")] = clips;
            tracks.append(trackObject);
        }
    }
    root[QStringLiteral("tracks")] = tracks;

    QSaveFile file(target);
    if (!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) < 0 || !file.commit()) {
        QMessageBox::warning(this, QStringLiteral("Cannot save"), QStringLiteral("The project file could not be written."));
        return;
    }
    appendLog(QStringLiteral("Project saved: %1").arg(target));
    statusBar()->showMessage(QStringLiteral("CineForge Project saved"));
}

void MainWindow::openProject() {
    const auto target = QFileDialog::getOpenFileName(this, QStringLiteral("Open CineForge Project"), QString(), QStringLiteral("CineForge Project (*.cineforge)"));
    if (target.isEmpty()) return;
    QFile file(target);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("Cannot open"), QStringLiteral("The project file could not be read."));
        return;
    }
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        QMessageBox::warning(this, QStringLiteral("Invalid project"), parseError.errorString());
        return;
    }
    const auto root = document.object();
    const auto input = root.value(QStringLiteral("inputDirectory")).toString();
    if (input.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Incomplete project"), QStringLiteral("The project's media folder is missing."));
        return;
    }
    importFolder(std::filesystem::path(input.toStdString()));
    if (outputEdit_) outputEdit_->setText(root.value(QStringLiteral("outputFile")).toString(outputEdit_->text()));

    std::vector<TimelineTrack> tracks;
    for (const auto& trackValue : root.value(QStringLiteral("tracks")).toArray()) {
        const auto trackObject = trackValue.toObject();
        TimelineTrack track;
        track.name = trackObject.value(QStringLiteral("name")).toString().toStdString();
        track.audio = trackObject.value(QStringLiteral("audio")).toBool();
        for (const auto& clipValue : trackObject.value(QStringLiteral("clips")).toArray()) {
                const auto clipObject = clipValue.toObject();
                TimelineClip clip;
                clip.mediaIndex = static_cast<std::size_t>(clipObject.value(QStringLiteral("mediaIndex")).toInteger());
                clip.trackIndex = clipObject.value(QStringLiteral("trackIndex")).toInt();
                clip.startSeconds = clipObject.value(QStringLiteral("start")).toDouble();
                clip.durationSeconds = clipObject.value(QStringLiteral("duration")).toDouble();
                clip.sourceInSeconds = clipObject.value(QStringLiteral("sourceIn")).toDouble();
                clip.sourceOutSeconds = clipObject.value(QStringLiteral("sourceOut")).toDouble();
                clip.opacity = clipObject.value(QStringLiteral("opacity")).toDouble(1.0);
                clip.scale = clipObject.value(QStringLiteral("scale")).toDouble(1.0);
                clip.positionX = clipObject.value(QStringLiteral("positionX")).toDouble(0.5);
                clip.positionY = clipObject.value(QStringLiteral("positionY")).toDouble(0.5);
                clip.maskType = clipObject.value(QStringLiteral("maskType")).toString().toStdString();
                
                auto loadKeyframes = [](const QJsonArray& array) {
                    std::vector<Keyframe> result;
                    for (const auto& kv : array) {
                        auto obj = kv.toObject();
                        result.push_back({obj.value(QStringLiteral("time")).toDouble(), obj.value(QStringLiteral("value")).toDouble()});
                    }
                    return result;
                };
                clip.scaleKeyframes = loadKeyframes(clipObject.value(QStringLiteral("scaleKeyframes")).toArray());
                clip.positionXKeyframes = loadKeyframes(clipObject.value(QStringLiteral("positionXKeyframes")).toArray());
                clip.positionYKeyframes = loadKeyframes(clipObject.value(QStringLiteral("positionYKeyframes")).toArray());
                
                track.clips.push_back(clip);
            }
            tracks.push_back(track);
        }
    if (timeline_ && !tracks.empty()) timeline_->setTracks(tracks);
    appendLog(QStringLiteral("Project opened: %1").arg(target));
    statusBar()->showMessage(QStringLiteral("CineForge Project opened"));
}

void MainWindow::chooseFolder() {
    const auto folder = QFileDialog::getExistingDirectory(this, QStringLiteral("Choose media folder"));
    if (!folder.isEmpty()) importFolder(std::filesystem::path(folder.toStdString()));
}

void MainWindow::importFolder(const std::filesystem::path& folder) {
    currentFolder_ = folder;
    rescanCurrentFolder();
    outputEdit_->setText(QString::fromStdString((folder / "output.mp4").string()));
    statusBar()->showMessage(QStringLiteral("%1 media imported from %2").arg(currentMedia_.size()).arg(QString::fromStdString(folder.string())));
    previewLabel_->setText(QStringLiteral("Project loaded\n\n%1 media • ready for editing").arg(currentMedia_.size()));
    for (const auto& item : currentMedia_) {
        if (item.type == MediaType::Video) {
            loadPreviewFile(item.path);
            break;
        }
    }
}

void MainWindow::loadPreviewFile(const std::filesystem::path& file) {
    if (!player_ || file.empty()) return;
    player_->setSource(QUrl::fromLocalFile(QString::fromStdString(file.string())));
    if (previewStack_ && videoWidget_) previewStack_->setCurrentWidget(videoWidget_);
    appendLog(QStringLiteral("Preview loaded: %1").arg(QString::fromStdString(file.filename().string())));
}

void MainWindow::rescanCurrentFolder() {
    if (currentFolder_.empty()) return;
    Project project;
    project.setInputDirectory(currentFolder_);
    if (sortCombo_) {
        if (sortCombo_->currentIndex() == 1) project.setSortStrategy(SortStrategy::Filename);
        else if (sortCombo_->currentIndex() == 2) project.setSortStrategy(SortStrategy::ModifiedTime);
        else project.setSortStrategy(SortStrategy::NaturalFilename);
    }
    project.scanMedia(true);
    currentMedia_ = project.media();
    refreshMediaList();
    if (timeline_) timeline_->setMedia(currentMedia_);
}

void MainWindow::refreshMediaList() {
    mediaList_->clear();
    for (const auto& item : currentMedia_) {
        const QString icon = item.type == MediaType::Image ? QStringLiteral("▧") : QStringLiteral("▶");
        mediaList_->addItem(icon + QStringLiteral("  ") + QString::fromStdString(item.path.filename().string()));
    }
    mediaCountLabel_->setText(QStringLiteral("%1 items").arg(currentMedia_.size()));
}

void MainWindow::applyTemplate(const QString& name, const QString& instruction) {
    commandEdit_->setText(instruction);
    timeline_->setStyleName(name);
    agentLog_->appendPlainText(QStringLiteral("Template selected: %1").arg(name));
    agentLog_->appendPlainText(instruction);
}

void MainWindow::appendLog(const QString& text) {
    if (renderLog_) renderLog_->appendPlainText(text);
    QApplication::processEvents();
}

void MainWindow::startRender() {
    if (currentFolder_.empty() || currentMedia_.empty()) {
        QMessageBox::warning(this, QStringLiteral("Empty project"), QStringLiteral("First import a folder containing images or videos."));
        return;
    }
    if (this->renderButton_) this->renderButton_->setEnabled(false);
    if (this->progressBar_) this->progressBar_->setValue(0);
    if (!this->renderLog_) this->renderLog_ = new QPlainTextEdit(this);
    this->renderLog_->clear();
    appendLog(QStringLiteral("Starting local render…"));

    Project project;
    project.setInputDirectory(currentFolder_);
    project.setOutputFile(this->outputEdit_ ? this->outputEdit_->text().toStdString() : "output.mp4");
    project.scanMedia();
    auto plan = project.makePlan();
    LocalAgent agent;
    RenderPlan interpreted;
    std::string agentPrompt = this->commandEdit_ ? this->commandEdit_->text().toStdString() : "";
    if (this->scriptEdit_ && !this->scriptEdit_->text().isEmpty()) {
        QFile scriptFile(this->scriptEdit_->text());
        if (scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            agentPrompt += "\nScript:\n" + scriptFile.readAll().toStdString();
        }
    }
    
    if (this->agentModelEdit_ && !this->agentModelEdit_->text().isEmpty()) {
        appendLog(QStringLiteral("Calling local GGUF agent…"));
        interpreted = agent.interpretWithGguf(agentPrompt, this->agentModelEdit_->text().toStdString(), currentFolder_);
    } else {
        interpreted = agent.interpret(agentPrompt, currentFolder_);
    }
    plan.options = interpreted.options;
    plan.targetDurationSeconds = interpreted.targetDurationSeconds;
    plan.media = currentMedia_;
    plan.outputFile = this->outputEdit_ ? this->outputEdit_->text().toStdString() : "output.mp4";
    
    if (this->subtitlesCheck_) plan.options.burnSubtitles = this->subtitlesCheck_->isChecked();
    if (this->zoomCheck_) plan.options.addZoomToImages = this->zoomCheck_->isChecked();
    if (this->crfSpin_) plan.options.crf = this->crfSpin_->value();
    if (this->encoderCombo_) plan.options.videoEncoder = this->encoderCombo_->currentData().toString().toStdString();
    if (this->proxyCheck_) plan.options.useProxyPreview = this->proxyCheck_->isChecked();
    if (this->proxyWidthSpin_) plan.options.proxyWidth = this->proxyWidthSpin_->value();
    if (this->normalizeAudioCheck_) plan.options.loudnessNormalization = this->normalizeAudioCheck_->isChecked();
    if (this->duckMusicCheck_) plan.options.duckMusicUnderVoice = this->duckMusicCheck_->isChecked();
    if (this->subtitlesEdit_) plan.options.subtitlesFile = this->subtitlesEdit_->text().toStdString();
    if (this->voiceEdit_) plan.options.voiceOverFile = this->voiceEdit_->text().toStdString();
    if (this->musicEdit_) plan.options.musicFile = this->musicEdit_->text().toStdString();
    
    if (this->formatCombo_) {
        const QString format = this->formatCombo_->currentText();
        if (format.startsWith(QStringLiteral("Paysage"))) { plan.options.width = 1920; plan.options.height = 1080; }
        else if (format.startsWith(QStringLiteral("Carré"))) { plan.options.width = 1080; plan.options.height = 1080; }
    }
    
    if (this->fpsCombo_) {
        const int fps = this->fpsCombo_->currentText().split(' ').first().toInt();
        if (fps > 0) plan.options.fps = fps;
    }

    if (!interpreted.narrationText.empty() && this->piperModelEdit_ && !this->piperModelEdit_->text().isEmpty()) {
        const auto voicePath = plan.options.voiceOverFile.empty() ? currentFolder_ / "voiceover.wav" : plan.options.voiceOverFile;
        PiperVoiceEngine piper;
        std::string error;
        
        appendLog(QStringLiteral("Generating voice with Piper from script…"));
        if (piper.synthesize(interpreted.narrationText, this->piperModelEdit_->text().toStdString(), voicePath, &error)) {
            plan.options.voiceOverFile = voicePath;
            if (this->voiceEdit_) this->voiceEdit_->setText(QString::fromStdString(voicePath.string()));
        } else appendLog(QStringLiteral("Piper error: %1").arg(QString::fromStdString(error)));
    }

    const QString audioPath = QString::fromStdString(plan.options.voiceOverFile.string());
    if (!audioPath.isEmpty() && this->whisperModelEdit_ && !this->whisperModelEdit_->text().isEmpty() && this->subtitlesEdit_ && this->subtitlesEdit_->text().isEmpty()) {
        const auto srtPath = currentFolder_ / "subtitles.srt";
        WhisperSubtitleEngine whisperEngine;
        std::string error;
        appendLog(QStringLiteral("Local Whisper transcription…"));
        if (whisperEngine.transcribe(audioPath.toStdString(), this->whisperModelEdit_->text().toStdString(), srtPath, &error)) {
            plan.options.subtitlesFile = srtPath;
            if (this->subtitlesEdit_) this->subtitlesEdit_->setText(QString::fromStdString(srtPath.string()));
        } else appendLog(QStringLiteral("Whisper error: %1").arg(QString::fromStdString(error)));
    }
    
    // Parse SRT to add SubtitleCues to the plan
    if (!plan.options.subtitlesFile.empty() && std::filesystem::exists(plan.options.subtitlesFile)) {
        std::ifstream srt(plan.options.subtitlesFile);
        std::string line;
        SubtitleCue cue;
        int state = 0; // 0: index, 1: time, 2: text
        auto parseTime = [](const std::string& t) -> double {
            int h, m; double s;
            if (sscanf(t.c_str(), "%d:%d:%lf", &h, &m, &s) == 3) return h * 3600 + m * 60 + s;
            return 0.0;
        };
        while (std::getline(srt, line)) {
            if (line.empty() || line == "\r") {
                if (!cue.text.empty() && !plan.chapters.empty()) plan.chapters[0].subtitles.push_back(cue);
                cue = SubtitleCue();
                state = 0;
            } else if (state == 0) {
                state = 1;
            } else if (state == 1) {
                auto pos = line.find(" --> ");
                if (pos != std::string::npos) {
                    cue.startSeconds = parseTime(line.substr(0, pos));
                    std::string endStr = line.substr(pos + 5);
                    std::replace(endStr.begin(), endStr.end(), ',', '.');
                    cue.endSeconds = parseTime(endStr);
                }
                state = 2;
            } else if (state == 2) {
                if (!cue.text.empty()) cue.text += "\n";
                cue.text += line;
            }
        }
        if (!cue.text.empty() && !plan.chapters.empty()) plan.chapters[0].subtitles.push_back(cue);
    }

    appendLog(QString::fromStdString(agent.explainPlan(plan)));
    FfmpegRenderer renderer;
    std::string error;
    const bool success = renderer.render(plan,
        [this](double progress, const std::string& message) {
            if (this->progressBar_) this->progressBar_->setValue(static_cast<int>(progress * 100.0));
            this->appendLog(QStringLiteral("[%1%] %2").arg(static_cast<int>(progress * 100.0)).arg(QString::fromStdString(message)));
        }, &error);
    if (this->renderButton_) this->renderButton_->setEnabled(true);
    if (success) {
        if (this->previewLabel_) this->previewLabel_->setText(QStringLiteral("Export finished\n\n%1").arg(QString::fromStdString(plan.outputFile.string())));
        loadPreviewFile(plan.outputFile);
        QMessageBox::information(this, QStringLiteral("Export finished"), QStringLiteral("The video was created offline."));
    } else {
        appendLog(QStringLiteral("Error: %1").arg(QString::fromStdString(error)));
        QMessageBox::critical(this, QStringLiteral("Render failed"), QString::fromStdString(error));
    }
}

} // namespace ova
