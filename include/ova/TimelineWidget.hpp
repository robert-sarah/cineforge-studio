#pragma once

#include "ova/Project.hpp"

#include <QWidget>
#include <vector>

namespace ova {

class TimelineWidget final : public QWidget {
public:
    explicit TimelineWidget(QWidget* parent = nullptr);

    void setMedia(const std::vector<MediaItem>& media);
    void setStyleName(const QString& styleName);
    void setCurrentTime(double seconds);
    void setTracks(const std::vector<TimelineTrack>& tracks);
    void setSnapEnabled(bool enabled) noexcept { snapEnabled_ = enabled; }
    void cutSelectedAtPlayhead();
    void deleteSelectedClip();
    const std::vector<TimelineTrack>& tracks() const noexcept { return tracks_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    double timeAtX(int x) const;
    int xAtTime(double seconds) const;
    int clipAt(const QPoint& point) const;
    double snapTime(double seconds) const;
    void rebuildDefaultTimeline();

    std::vector<MediaItem> media_;
    std::vector<TimelineTrack> tracks_;
    QString styleName_ = QStringLiteral("MrBeast / High Energy");
    double currentTime_ = 0.0;
    double pixelsPerSecond_ = 105.0;
    int selectedClip_ = -1;
    bool dragging_ = false;
    bool snapEnabled_ = true;
    double dragOffset_ = 0.0;
};

} // namespace ova
