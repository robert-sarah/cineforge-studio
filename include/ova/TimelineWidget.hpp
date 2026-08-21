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

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    double timeAtX(int x) const;
    std::vector<MediaItem> media_;
    QString styleName_ = QStringLiteral("MrBeast / High Energy");
    double currentTime_ = 0.0;
    double pixelsPerSecond_ = 105.0;
};

} // namespace ova
