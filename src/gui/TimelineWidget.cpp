#include "ova/TimelineWidget.hpp"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace ova {
namespace {
constexpr int kHeaderHeight = 30;
constexpr int kLabelWidth = 112;
constexpr int kTrackHeight = 52;
constexpr double kGridStep = 1.0;
}

TimelineWidget::TimelineWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(kHeaderHeight + kTrackHeight * 3 + 18);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void TimelineWidget::setMedia(const std::vector<MediaItem>& media) {
    media_ = media;
    rebuildDefaultTimeline();
    undoStack_.clear();
    redoStack_.clear();
    update();
}

void TimelineWidget::setStyleName(const QString& styleName) {
    styleName_ = styleName;
    update();
}

void TimelineWidget::setCurrentTime(double seconds) {
    currentTime_ = std::max(0.0, seconds);
    update();
}

void TimelineWidget::setTracks(const std::vector<TimelineTrack>& tracks) {
    tracks_ = tracks;
    if (tracks_.empty()) rebuildDefaultTimeline();
    selectedTrack_ = tracks_.empty() || tracks_[0].clips.empty() ? -1 : 0;
    selectedClip_ = tracks_.empty() || tracks_[0].clips.empty() ? -1 : 0;
    currentTime_ = 0.0;
    update();
}

void TimelineWidget::rebuildDefaultTimeline() {
    tracks_.clear();
    tracks_.push_back(TimelineTrack{"V1  •  VIDEO", false, {}});
    tracks_.push_back(TimelineTrack{"V2  •  TEXTE / OVERLAYS", false, {}});
    tracks_.push_back(TimelineTrack{"A1  •  AUDIO", true, {}});
    double cursor = 0.0;
    for (std::size_t i = 0; i < media_.size(); ++i) {
        const auto& media = media_[i];
        const double duration = media.durationSeconds > 0.0
            ? media.durationSeconds
            : (media.type == MediaType::Image ? 3.0 : 5.0);
        tracks_[0].clips.push_back(TimelineClip{i, 0, cursor, duration, 0.0, duration});
        cursor += duration;
    }
    selectedTrack_ = tracks_[0].clips.empty() ? -1 : 0;
    selectedClip_ = tracks_[0].clips.empty() ? -1 : 0;
    currentTime_ = 0.0;
}

double TimelineWidget::timeAtX(int x) const {
    return std::max(0.0, (x - kLabelWidth) / pixelsPerSecond_);
}

int TimelineWidget::xAtTime(double seconds) const {
    return kLabelWidth + static_cast<int>(std::lround(seconds * pixelsPerSecond_));
}

double TimelineWidget::snapTime(double seconds) const {
    if (!snapEnabled_) return std::max(0.0, seconds);
    return std::max(0.0, std::round(seconds / kGridStep) * kGridStep);
}

int TimelineWidget::trackAt(const QPoint& point) const {
    if (point.y() < kHeaderHeight) return -1;
    const int trackIndex = (point.y() - kHeaderHeight) / kTrackHeight;
    if (trackIndex < 0 || trackIndex >= static_cast<int>(tracks_.size())) return -1;
    return trackIndex;
}

int TimelineWidget::clipAt(const QPoint& point) const {
    const int trackIndex = trackAt(point);
    if (trackIndex < 0) return -1;
    const auto& clips = tracks_[trackIndex].clips;
    const double time = timeAtX(point.x());
    for (int i = 0; i < static_cast<int>(clips.size()); ++i) {
        const auto& clip = clips[i];
        if (time >= clip.startSeconds && time <= clip.startSeconds + clip.durationSeconds) return i;
    }
    return -1;
}

void TimelineWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#14161c"));

    const int contentWidth = width() - kLabelWidth;
    painter.fillRect(0, 0, width(), kHeaderHeight, QColor("#222630"));
    painter.fillRect(0, kHeaderHeight, kLabelWidth, height(), QColor("#1b1e26"));
    painter.setPen(QColor("#333844"));
    painter.drawLine(kLabelWidth, 0, kLabelWidth, height());

    painter.setFont(QFont(QStringLiteral("Inter"), 9));
    const double visibleSeconds = std::max(1.0, contentWidth / pixelsPerSecond_);
    for (double t = 0.0; t <= visibleSeconds + 1.0; t += kGridStep) {
        const int x = xAtTime(t);
        painter.setPen(QColor("#383d49"));
        painter.drawLine(x, kHeaderHeight, x, height());
        painter.setPen(QColor("#a5adbd"));
        painter.drawText(x + 4, 19, QStringLiteral("%1s").arg(static_cast<int>(t)));
    }

    for (int track = 0; track < static_cast<int>(tracks_.size()); ++track) {
        const int y = kHeaderHeight + track * kTrackHeight;
        painter.fillRect(0, y, kLabelWidth, kTrackHeight, track % 2 ? QColor("#20232b") : QColor("#1d2027"));
        painter.setPen(QColor("#c1c7d4"));
        painter.drawText(12, y + 30, QString::fromStdString(tracks_[track].name));
        painter.setPen(QColor("#2e333e"));
        painter.drawLine(0, y + kTrackHeight - 1, width(), y + kTrackHeight - 1);

        const auto& clips = tracks_[track].clips;
        for (int index = 0; index < static_cast<int>(clips.size()); ++index) {
            const auto& clip = clips[index];
            const int x = xAtTime(clip.startSeconds);
            const int w = std::max(18, xAtTime(clip.startSeconds + clip.durationSeconds) - x);
            const bool selected = track == selectedTrack_ && index == selectedClip_;
            const QColor base = selected ? QColor("#5b55df") : (track == 0 ? QColor("#304a72") : QColor("#3d5e4d"));
            painter.setPen(selected ? QColor("#aaa5ff") : QColor("#526b95"));
            painter.setBrush(base);
            painter.drawRoundedRect(QRect(x + 2, y + 7, w - 4, kTrackHeight - 14), 5, 5);
            painter.setPen(QColor("#f2f4fa"));
            const auto label = clip.mediaIndex < media_.size()
                ? QString::fromStdString(media_[clip.mediaIndex].path.filename().string())
                : QStringLiteral("Clip");
            painter.drawText(QRect(x + 9, y + 20, std::max(0, w - 16), 18), Qt::TextSingleLine, label);
            painter.setPen(QColor(255, 255, 255, 80));
            for (int thumb = x + 7; thumb < x + w - 8; thumb += 52) painter.drawLine(thumb, y + 10, thumb, y + kTrackHeight - 10);
        }
    }

    const int playheadX = xAtTime(currentTime_);
    painter.setPen(QPen(QColor("#ff5d8f"), 2));
    painter.drawLine(playheadX, 0, playheadX, height());
    painter.setBrush(QColor("#ff5d8f"));
    painter.drawPolygon(QPolygon{QPoint(playheadX - 6, 0), QPoint(playheadX + 6, 0), QPoint(playheadX, 8)});
    painter.setPen(QColor("#ff9fbd"));
    painter.drawText(playheadX + 7, 19, QStringLiteral("%1s").arg(QString::number(currentTime_, 'f', 1)));
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    if (event->pos().y() < kHeaderHeight) {
        currentTime_ = timeAtX(event->pos().x());
        update();
        return;
    }
    const int track = trackAt(event->pos());
    const int index = clipAt(event->pos());
    if (track >= 0 && index >= 0) {
        rememberEdit();
        selectedTrack_ = track;
        selectedClip_ = index;
        const auto& clip = tracks_[track].clips[index];
        dragOffset_ = timeAtX(event->pos().x()) - clip.startSeconds;
        dragging_ = true;
        currentTime_ = timeAtX(event->pos().x());
    } else {
        currentTime_ = timeAtX(event->pos().x());
        selectedTrack_ = -1;
        selectedClip_ = -1;
    }
    update();
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!dragging_ || selectedTrack_ < 0 || selectedTrack_ >= static_cast<int>(tracks_.size()) ||
        selectedClip_ < 0 || selectedClip_ >= static_cast<int>(tracks_[selectedTrack_].clips.size())) return;
    auto& clip = tracks_[selectedTrack_].clips[selectedClip_];
    clip.startSeconds = snapTime(timeAtX(event->pos().x()) - dragOffset_);
    currentTime_ = clip.startSeconds + dragOffset_;
    update();
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) dragging_ = false;
}

void TimelineWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    const int track = trackAt(event->pos());
    const int index = clipAt(event->pos());
    if (track >= 0 && index >= 0) {
        selectedTrack_ = track;
        selectedClip_ = index;
        currentTime_ = timeAtX(event->pos().x());
        cutSelectedAtPlayhead();
    }
}

void TimelineWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        pixelsPerSecond_ *= event->angleDelta().y() > 0 ? 1.15 : 0.87;
        pixelsPerSecond_ = std::clamp(pixelsPerSecond_, 35.0, 420.0);
        update();
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

void TimelineWidget::cutSelectedAtPlayhead() {
    if (selectedTrack_ < 0 || selectedTrack_ >= static_cast<int>(tracks_.size()) ||
        selectedClip_ < 0 || selectedClip_ >= static_cast<int>(tracks_[selectedTrack_].clips.size())) return;
    rememberEdit();
    auto& clips = tracks_[selectedTrack_].clips;
    auto clip = clips[selectedClip_];
    const double relative = currentTime_ - clip.startSeconds;
    if (relative <= 0.15 || relative >= clip.durationSeconds - 0.15) return;
    TimelineClip right = clip;
    right.startSeconds = currentTime_;
    right.durationSeconds = clip.durationSeconds - relative;
    right.sourceInSeconds = clip.sourceInSeconds + relative;
    clip.durationSeconds = relative;
    clip.sourceOutSeconds = clip.sourceInSeconds + relative;
    clips[selectedClip_] = clip;
    clips.insert(clips.begin() + selectedClip_ + 1, right);
    ++selectedClip_;
    update();
}

void TimelineWidget::deleteSelectedClip() {
    if (selectedTrack_ < 0 || selectedTrack_ >= static_cast<int>(tracks_.size()) ||
        selectedClip_ < 0 || selectedClip_ >= static_cast<int>(tracks_[selectedTrack_].clips.size())) return;
    rememberEdit();
    auto& clips = tracks_[selectedTrack_].clips;
    clips.erase(clips.begin() + selectedClip_);
    selectedClip_ = std::min(selectedClip_, static_cast<int>(clips.size()) - 1);
    if (selectedClip_ < 0) selectedTrack_ = -1;
    update();
}

void TimelineWidget::rememberEdit() {
    undoStack_.push_back(tracks_);
    if (undoStack_.size() > 100) undoStack_.erase(undoStack_.begin());
    redoStack_.clear();
}

void TimelineWidget::undo() {
    if (undoStack_.empty()) return;
    redoStack_.push_back(tracks_);
    tracks_ = undoStack_.back();
    undoStack_.pop_back();
    selectedTrack_ = tracks_.empty() || tracks_[0].clips.empty() ? -1 : 0;
    selectedClip_ = selectedTrack_ < 0 ? -1 : 0;
    update();
}

void TimelineWidget::redo() {
    if (redoStack_.empty()) return;
    undoStack_.push_back(tracks_);
    tracks_ = redoStack_.back();
    redoStack_.pop_back();
    selectedTrack_ = tracks_.empty() || tracks_[0].clips.empty() ? -1 : 0;
    selectedClip_ = selectedTrack_ < 0 ? -1 : 0;
    update();
}

void TimelineWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelectedClip();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_S) {
        cutSelectedAtPlayhead();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

} // namespace ova
