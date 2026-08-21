#include "ova/TimelineWidget.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <algorithm>

namespace ova {

TimelineWidget::TimelineWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(245);
    setMouseTracking(true);
    setAutoFillBackground(false);
}

void TimelineWidget::setMedia(const std::vector<MediaItem>& media) {
    media_ = media;
    const int width = std::max(900, static_cast<int>(media_.size() * 330 + 190));
    setMinimumWidth(width);
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

double TimelineWidget::timeAtX(int x) const {
    return std::max(0.0, (x - 150.0) / pixelsPerSecond_);
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    currentTime_ = timeAtX(event->position().x());
    update();
    QWidget::mousePressEvent(event);
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        currentTime_ = timeAtX(event->position().x());
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void TimelineWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#181a20"));

    const int left = 150;
    const int rulerHeight = 32;
    const QColor grid("#30333d");
    const QColor muted("#8e94a3");
    const QColor text("#e6e8ef");

    painter.fillRect(0, 0, width(), rulerHeight, QColor("#20232b"));
    painter.fillRect(0, rulerHeight, left, height() - rulerHeight, QColor("#20232b"));
    painter.setPen(QPen(grid, 1));
    for (int second = 0; second < 120; ++second) {
        const int x = left + static_cast<int>(second * pixelsPerSecond_);
        if (x > width()) break;
        painter.drawLine(x, rulerHeight, x, height());
        painter.setPen(muted);
        painter.drawText(x + 5, 20, QStringLiteral("%1:%2")
            .arg(second / 60, 2, 10, QChar('0'))
            .arg(second % 60, 2, 10, QChar('0')));
        painter.setPen(QPen(grid, 1));
    }

    const QStringList trackNames = {QStringLiteral("VIDEO 01"), QStringLiteral("OVERLAYS"), QStringLiteral("AUDIO / VOICE"), QStringLiteral("SUBTITLES")};
    for (int row = 0; row < trackNames.size(); ++row) {
        const int y = rulerHeight + row * 49;
        painter.setPen(QPen(grid, 1));
        painter.drawLine(0, y, width(), y);
        painter.setPen(muted);
        painter.drawText(20, y + 29, trackNames.at(row));
    }

    double cursor = 0.0;
    int clipIndex = 0;
    for (const auto& item : media_) {
        const double duration = item.type == MediaType::Image ? 3.0 : 5.0;
        const int x = left + static_cast<int>(cursor * pixelsPerSecond_);
        const int w = std::max(90, static_cast<int>(duration * pixelsPerSecond_) - 5);
        const int y = rulerHeight + 7;
        const QColor color = item.type == MediaType::Image ? QColor("#7057d9") : QColor("#2f9e8f");
        painter.setBrush(color);
        painter.setPen(QPen(color.lighter(135), 1));
        painter.drawRoundedRect(x, y, w, 35, 5, 5);
        painter.setPen(text);
        const QString filename = QString::fromStdString(item.path.filename().string());
        painter.drawText(x + 9, y + 22, painter.fontMetrics().elidedText(filename, Qt::ElideRight, w - 18));
        painter.setPen(QColor("#ffffff80"));
        painter.drawText(x + 9, y + 33, QStringLiteral("%1  •  %2s").arg(++clipIndex).arg(duration, 0, 'f', 1));
        cursor += duration;
    }

    painter.setBrush(QColor("#f04969"));
    painter.setPen(Qt::NoPen);
    const int playheadX = left + static_cast<int>(currentTime_ * pixelsPerSecond_);
    painter.drawRect(playheadX - 1, 0, 3, height());
    QPolygon triangle;
    triangle << QPoint(playheadX - 7, 0) << QPoint(playheadX + 7, 0) << QPoint(playheadX, 10);
    painter.drawPolygon(triangle);

    painter.setPen(QColor("#aeb4c2"));
    painter.drawText(left + 15, height() - 12, QStringLiteral("Style actif : %1").arg(styleName_));
}

} // namespace ova
