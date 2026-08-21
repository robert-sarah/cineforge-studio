#include "ova/MainWindow.hpp"

#include "ova/Engines.hpp"
#include "ova/LocalServices.hpp"
#include "ova/ModelCatalog.hpp"
#include "ova/Project.hpp"
#include "ova/TimelineWidget.hpp"

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
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
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
    setWindowTitle(QStringLiteral("CineForge Studio  •  Éditeur IA Hors Ligne"));
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

    auto* mediaDock = new QDockWidget(QStringLiteral("MÉDIAS & TEMPLATES"), this);
    mediaDock->setObjectName(QStringLiteral("mediaDock"));
    mediaDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mediaDock->setWidget(makeMediaDock());
    addDockWidget(Qt::LeftDockWidgetArea, mediaDock);

    auto* agentDock = new QDockWidget(QStringLiteral("AGENT LOCAL"), this);
    agentDock->setObjectName(QStringLiteral("agentDock"));
    agentDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    agentDock->setWidget(makeAgentDock());
    addDockWidget(Qt::LeftDockWidgetArea, agentDock);
    tabifyDockWidget(mediaDock, agentDock);

    auto* inspectorDock = new QDockWidget(QStringLiteral("INSPECTEUR"), this);
    inspectorDock->setObjectName(QStringLiteral("inspectorDock"));
    inspectorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    inspectorDock->setWidget(makeInspectorDock());
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock);

    auto* toolbar = addToolBar(QStringLiteral("Édition"));
    toolbar->setMovable(false);
    auto* importAction = toolbar->addAction(QStringLiteral("Importer"));
    auto* saveAction = toolbar->addAction(QStringLiteral("Sauvegarder"));
    toolbar->addSeparator();
    auto* undoAction = toolbar->addAction(QStringLiteral("Annuler"));
    auto* redoAction = toolbar->addAction(QStringLiteral("Rétablir"));
    toolbar->addSeparator();
    auto* exportAction = toolbar->addAction(QStringLiteral("Exporter la vidéo"));
    exportAction->setObjectName(QStringLiteral("primaryButton"));
    connect(importAction, &QAction::triggered, this, [this] { chooseFolder(); });
    connect(exportAction, &QAction::triggered, this, [this] { startRender(); });
    connect(saveAction, &QAction::triggered, this, [this] { appendLog(QStringLiteral("Projet sauvegardé localement (prochaine version : fichier .ova).")); });
    connect(undoAction, &QAction::triggered, this, [this] { appendLog(QStringLiteral("Annulation : prête pour les opérations de timeline.")); });
    connect(redoAction, &QAction::triggered, this, [this] { appendLog(QStringLiteral("Rétablissement : prête pour les opérations de timeline.")); });

    auto* fileMenu = menuBar()->addMenu(QStringLiteral("Fichier"));
    fileMenu->addAction(importAction);
    fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Quitter"), this, &QWidget::close);
    menuBar()->addMenu(QStringLiteral("Édition"));
    menuBar()->addMenu(QStringLiteral("Effets"));
    menuBar()->addMenu(QStringLiteral("Audio"));
    menuBar()->addMenu(QStringLiteral("Aide"));

    statusBar()->showMessage(QStringLiteral("Prêt • moteur local • aucun média envoyé en ligne"));
}

QWidget* MainWindow::makePreview() {
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* header = new QHBoxLayout();
    header->addWidget(caption(QStringLiteral("PRÉVISUALISATION"), container));
    header->addStretch();
    auto* status = new QLabel(QStringLiteral("●  HORS LIGNE"), container);
    status->setObjectName(QStringLiteral("statusPill"));
    header->addWidget(status);
    auto* resolution = new QLabel(QStringLiteral("1080 × 1920  •  30 FPS"), container);
    resolution->setStyleSheet(QStringLiteral("color:#8f96a7; padding-left:12px;"));
    header->addWidget(resolution);
    layout->addLayout(header);

    auto* previewRow = new QHBoxLayout();
    previewLabel_ = new QLabel(QStringLiteral("Aucune prévisualisation\n\nImporte un dossier média pour commencer"), container);
    previewLabel_->setObjectName(QStringLiteral("previewScreen"));
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setMinimumHeight(360);
    previewRow->addWidget(previewLabel_, 1);
    layout->addLayout(previewRow, 1);

    auto* controls = new QHBoxLayout();
    auto* rewind = new QPushButton(QStringLiteral("◀  5s"), container);
    auto* play = new QPushButton(QStringLiteral("▶  Lecture"), container);
    auto* forward = new QPushButton(QStringLiteral("5s  ▶"), container);
    controls->addStretch();
    controls->addWidget(rewind);
    controls->addWidget(play);
    controls->addWidget(forward);
    controls->addStretch();
    connect(play, &QPushButton::clicked, this, [this, play] {
        play->setText(play->text().startsWith(QStringLiteral("▶")) ? QStringLiteral("Ⅱ  Pause") : QStringLiteral("▶  Lecture"));
        appendLog(QStringLiteral("Prévisualisation : le lecteur sera relié au rendu GPU dans l'étape suivante."));
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
    auto* import = primaryButton(QStringLiteral("＋  Importer un dossier"), mediaPage);
    mediaLayout->addWidget(import);
    sortCombo_ = new QComboBox(mediaPage);
    sortCombo_->addItems({QStringLiteral("Ordre naturel (01, 02, 10)"), QStringLiteral("Nom alphabétique"), QStringLiteral("Date de modification")});
    sortCombo_->setToolTip(QStringLiteral("Détermine l'ordre des plans ajoutés automatiquement à la timeline."));
    mediaLayout->addWidget(sortCombo_);
    mediaCountLabel_ = new QLabel(QStringLiteral("0 élément"), mediaPage);
    mediaCountLabel_->setStyleSheet(QStringLiteral("color:#9199aa; padding:7px 2px;"));
    mediaLayout->addWidget(mediaCountLabel_);
    mediaList_ = new QListWidget(mediaPage);
    mediaLayout->addWidget(mediaList_, 1);
    tabs->addTab(mediaPage, QStringLiteral("Médias"));

    auto* templatesPage = new QWidget(tabs);
    auto* templatesLayout = new QVBoxLayout(templatesPage);
    templatesLayout->addWidget(caption(QStringLiteral("CHOISIR UN STYLE"), templatesPage));
    const QList<QPair<QString, QString>> templates = {
        {QStringLiteral("⚡  MrBeast / High Energy"), QStringLiteral("Crée une vidéo verticale ultra dynamique style MrBeast avec des zooms rapides, des cuts serrés, des sous-titres impactants et un rythme viral")},
        {QStringLiteral("🎬  Cinématique"), QStringLiteral("Crée un montage cinématique avec format paysage, rythme lent, zooms doux et ambiance premium")},
        {QStringLiteral("📱  Shorts / TikTok"), QStringLiteral("Crée un short vertical très rapide avec sous-titres contrastés, cuts fréquents et accroche immédiate")},
        {QStringLiteral("🎓  Tutoriel"), QStringLiteral("Crée un tutoriel vertical clair avec sous-titres lisibles, rythme régulier et hiérarchie visuelle")}
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
    modelsLayout->addWidget(caption(QStringLiteral("MODÈLES LOCAUX • 100 % HORS LIGNE"), modelsPage));
    auto* modelsHint = new QLabel(QStringLiteral("Les poids restent sur ton disque. Double-clique un modèle installé pour l'utiliser dans l'inspecteur."), modelsPage);
    modelsHint->setWordWrap(true);
    modelsHint->setStyleSheet(QStringLiteral("color:#9199aa; padding:4px 0 8px;"));
    modelsLayout->addWidget(modelsHint);
    auto* modelList = new QListWidget(modelsPage);
    modelsLayout->addWidget(modelList, 1);
    auto* openModels = new QPushButton(QStringLiteral("Ouvrir le dossier des modèles"), modelsPage);
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
    auto* refreshModelsButton = new QPushButton(QStringLiteral("Actualiser les statuts"), modelsPage);
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
            appendLog(QStringLiteral("Modèle sélectionné : %1").arg(QString::fromStdString(path.string())));
            break;
        }
    });
    refreshModels();
    tabs->addTab(modelsPage, QStringLiteral("Modèles"));
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

    layout->addWidget(caption(QStringLiteral("CANVAS & EXPORT"), panel));
    auto* exportBox = new QGroupBox(QStringLiteral("Composition"), panel);
    auto* exportForm = new QFormLayout(exportBox);
    formatCombo_ = new QComboBox(exportBox);
    formatCombo_->addItems({QStringLiteral("Vertical  1080 × 1920"), QStringLiteral("Paysage  1920 × 1080"), QStringLiteral("Carré  1080 × 1080")});
    fpsCombo_ = new QComboBox(exportBox);
    fpsCombo_->addItems({QStringLiteral("24 fps"), QStringLiteral("30 fps"), QStringLiteral("60 fps")});
    fpsCombo_->setCurrentIndex(1);
    crfSpin_ = new QSpinBox(exportBox);
    crfSpin_->setRange(16, 30);
    crfSpin_->setValue(20);
    exportForm->addRow(QStringLiteral("Format"), formatCombo_);
    exportForm->addRow(QStringLiteral("Images/s"), fpsCombo_);
    exportForm->addRow(QStringLiteral("Qualité"), crfSpin_);
    zoomCheck_ = new QCheckBox(QStringLiteral("Zoom animé sur les images"), exportBox);
    zoomCheck_->setChecked(true);
    subtitlesCheck_ = new QCheckBox(QStringLiteral("Incruster les sous-titres"), exportBox);
    subtitlesCheck_->setChecked(true);
    exportForm->addRow(zoomCheck_);
    exportForm->addRow(subtitlesCheck_);
    layout->addWidget(exportBox);

    layout->addWidget(caption(QStringLiteral("VOIX & SOUS-TITRES"), panel));
    auto* audioBox = new QGroupBox(QStringLiteral("Services locaux"), panel);
    auto* audioForm = new QFormLayout(audioBox);
    voiceEdit_ = new QLineEdit(audioBox);
    voiceEdit_->setPlaceholderText(QStringLiteral("voiceover.wav / mp3"));
    subtitlesEdit_ = new QLineEdit(audioBox);
    subtitlesEdit_->setPlaceholderText(QStringLiteral("subtitles.srt"));
    scriptEdit_ = new QLineEdit(audioBox);
    scriptEdit_->setPlaceholderText(QStringLiteral("Texte de narration Piper"));
    piperModelEdit_ = new QLineEdit(audioBox);
    piperModelEdit_->setPlaceholderText(QStringLiteral("models/voice.onnx"));
    whisperModelEdit_ = new QLineEdit(audioBox);
    whisperModelEdit_->setPlaceholderText(QStringLiteral("models/ggml-small.bin"));
    audioForm->addRow(QStringLiteral("Voix"), voiceEdit_);
    audioForm->addRow(QStringLiteral("Sous-titres"), subtitlesEdit_);
    audioForm->addRow(QStringLiteral("Script"), scriptEdit_);
    audioForm->addRow(QStringLiteral("Piper"), piperModelEdit_);
    audioForm->addRow(QStringLiteral("Whisper"), whisperModelEdit_);
    layout->addWidget(audioBox);

    layout->addWidget(caption(QStringLiteral("PROJET"), panel));
    outputEdit_ = new QLineEdit(QStringLiteral("output.mp4"), panel);
    layout->addWidget(outputEdit_);
    renderButton_ = primaryButton(QStringLiteral("Créer la vidéo"), panel);
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

QWidget* MainWindow::makeAgentDock() {
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(caption(QStringLiteral("DONNE UNE INSTRUCTION À L’AGENT"), panel));
    commandEdit_ = new QLineEdit(panel);
    commandEdit_->setPlaceholderText(QStringLiteral("Ex. : transforme ce dossier en short viral très rythmé"));
    layout->addWidget(commandEdit_);
    auto* apply = primaryButton(QStringLiteral("Appliquer le plan"), panel);
    layout->addWidget(apply);
    agentLog_ = new QPlainTextEdit(panel);
    agentLog_->setReadOnly(true);
    agentLog_->setPlaceholderText(QStringLiteral("L'agent local expliquera ici les choix appliqués…"));
    layout->addWidget(agentLog_, 1);
    connect(apply, &QPushButton::clicked, this, [this] {
        LocalAgent agent;
        const auto plan = agent.interpret(commandEdit_->text().toStdString(), currentFolder_);
        agentLog_->appendPlainText(QString::fromStdString(agent.explainPlan(plan)));
        if (plan.style == "high-energy") timeline_->setStyleName(QStringLiteral("MrBeast / High Energy"));
        else if (plan.style == "cinematic") timeline_->setStyleName(QStringLiteral("Cinématique"));
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
    header->addWidget(new QLabel(QStringLiteral("+ Piste vidéo   + Piste audio   + Texte"), panel));
    timeline_ = new TimelineWidget(panel);
    layout->addLayout(header);
    layout->addWidget(timeline_);
    return panel;
}

void MainWindow::chooseFolder() {
    const auto folder = QFileDialog::getExistingDirectory(this, QStringLiteral("Choisir le dossier média"));
    if (!folder.isEmpty()) importFolder(std::filesystem::path(folder.toStdString()));
}

void MainWindow::importFolder(const std::filesystem::path& folder) {
    currentFolder_ = folder;
    rescanCurrentFolder();
    outputEdit_->setText(QString::fromStdString((folder / "output.mp4").string()));
    statusBar()->showMessage(QStringLiteral("%1 média(s) importé(s) depuis %2").arg(currentMedia_.size()).arg(QString::fromStdString(folder.string())));
    previewLabel_->setText(QStringLiteral("Projet chargé\n\n%1 média(s) • prêt pour le montage").arg(currentMedia_.size()));
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
    mediaCountLabel_->setText(QStringLiteral("%1 élément(s)").arg(currentMedia_.size()));
}

void MainWindow::applyTemplate(const QString& name, const QString& instruction) {
    commandEdit_->setText(instruction);
    timeline_->setStyleName(name);
    agentLog_->appendPlainText(QStringLiteral("Template sélectionné : %1").arg(name));
    agentLog_->appendPlainText(instruction);
}

void MainWindow::appendLog(const QString& text) {
    if (renderLog_) renderLog_->appendPlainText(text);
    QApplication::processEvents();
}

void MainWindow::startRender() {
    if (currentFolder_.empty() || currentMedia_.empty()) {
        QMessageBox::warning(this, QStringLiteral("Projet vide"), QStringLiteral("Importe d'abord un dossier contenant des images ou des vidéos."));
        return;
    }
    renderButton_->setEnabled(false);
    progressBar_->setValue(0);
    if (!renderLog_) renderLog_ = new QPlainTextEdit(this);
    renderLog_->clear();
    appendLog(QStringLiteral("Démarrage du rendu local…"));

    Project project;
    project.setInputDirectory(currentFolder_);
    project.setOutputFile(outputEdit_->text().toStdString());
    project.scanMedia();
    auto plan = project.makePlan();
    LocalAgent agent;
    const auto interpreted = agent.interpret(commandEdit_->text().toStdString(), currentFolder_);
    plan.options = interpreted.options;
    plan.media = currentMedia_;
    plan.outputFile = outputEdit_->text().toStdString();
    plan.options.burnSubtitles = subtitlesCheck_->isChecked();
    plan.options.addZoomToImages = zoomCheck_->isChecked();
    plan.options.crf = crfSpin_->value();
    plan.options.subtitlesFile = subtitlesEdit_->text().toStdString();
    plan.options.voiceOverFile = voiceEdit_->text().toStdString();
    const QString format = formatCombo_->currentText();
    if (format.startsWith(QStringLiteral("Paysage"))) { plan.options.width = 1920; plan.options.height = 1080; }
    else if (format.startsWith(QStringLiteral("Carré"))) { plan.options.width = 1080; plan.options.height = 1080; }
    const int fps = fpsCombo_->currentText().split(' ').first().toInt();
    if (fps > 0) plan.options.fps = fps;

    if (!scriptEdit_->text().isEmpty() && !piperModelEdit_->text().isEmpty()) {
        const auto voicePath = plan.options.voiceOverFile.empty() ? currentFolder_ / "voiceover.wav" : plan.options.voiceOverFile;
        PiperVoiceEngine piper;
        std::string error;
        appendLog(QStringLiteral("Génération de la voix avec Piper…"));
        if (piper.synthesize(scriptEdit_->text().toStdString(), piperModelEdit_->text().toStdString(), voicePath, &error)) {
            plan.options.voiceOverFile = voicePath;
            voiceEdit_->setText(QString::fromStdString(voicePath.string()));
        } else appendLog(QStringLiteral("Piper : %1").arg(QString::fromStdString(error)));
    }

    const QString audioPath = QString::fromStdString(plan.options.voiceOverFile.string());
    if (!audioPath.isEmpty() && !whisperModelEdit_->text().isEmpty() && subtitlesEdit_->text().isEmpty()) {
        const auto srtPath = currentFolder_ / "subtitles.srt";
        WhisperSubtitleEngine whisper;
        std::string error;
        appendLog(QStringLiteral("Transcription Whisper locale…"));
        if (whisper.transcribe(audioPath.toStdString(), whisperModelEdit_->text().toStdString(), srtPath, &error)) {
            plan.options.subtitlesFile = srtPath;
            subtitlesEdit_->setText(QString::fromStdString(srtPath.string()));
        } else appendLog(QStringLiteral("Whisper : %1").arg(QString::fromStdString(error)));
    }

    appendLog(QString::fromStdString(agent.explainPlan(plan)));
    FfmpegRenderer renderer;
    std::string error;
    const bool success = renderer.render(plan,
        [this](double progress, const std::string& message) {
            progressBar_->setValue(static_cast<int>(progress * 100.0));
            appendLog(QStringLiteral("[%1%] %2").arg(static_cast<int>(progress * 100.0)).arg(QString::fromStdString(message)));
        }, &error);
    renderButton_->setEnabled(true);
    if (success) {
        previewLabel_->setText(QStringLiteral("Export terminé\n\n%1").arg(QString::fromStdString(plan.outputFile.string())));
        QMessageBox::information(this, QStringLiteral("Export terminé"), QStringLiteral("La vidéo a été créée hors ligne."));
    } else {
        appendLog(QStringLiteral("Erreur : %1").arg(QString::fromStdString(error)));
        QMessageBox::critical(this, QStringLiteral("Échec du rendu"), QString::fromStdString(error));
    }
}

} // namespace ova
