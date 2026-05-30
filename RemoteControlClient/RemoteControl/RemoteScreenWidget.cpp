#include "RemoteScreenWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

RemoteScreenWidget::RemoteScreenWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true); // 需要捕获移动事件
    setFocusPolicy(Qt::StrongFocus); // 接收键盘事件
}

void RemoteScreenWidget::updateFrame(const QImage& img)
{
    m_frame = img;
    if (!m_frame.isNull()) {
        m_remoteSize = m_frame.size();
    }
    update(); // 触发重绘
}

void RemoteScreenWidget::clearFrame()
{
    m_frame = QImage();
    update();
}

void RemoteScreenWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    if (m_frame.isNull()) {
        painter.fillRect(rect(), QColor(10, 15, 20)); // 深色背景
        painter.setPen(QColor(120, 140, 160));
        painter.drawText(rect(), Qt::AlignCenter, tr("等待连接..."));
        return;
    }
    // 等比例缩放以填充控件，保持纵横比
    QSize widgetSize = size();
    QSize scaledSize = m_frame.size().scaled(widgetSize, Qt::KeepAspectRatio);
    int x = (widgetSize.width() - scaledSize.width()) / 2;
    int y = (widgetSize.height() - scaledSize.height()) / 2;
    painter.drawImage(QRect(x, y, scaledSize.width(), scaledSize.height()), m_frame);
}

QPoint RemoteScreenWidget::mapToRemote(const QPoint& localPos) const
{
    if (m_frame.isNull()) return QPoint(0, 0);
    QSize widgetSize = size();
    QSize scaledSize = m_frame.size().scaled(widgetSize, Qt::KeepAspectRatio);
    int offsetX = (widgetSize.width() - scaledSize.width()) / 2;
    int offsetY = (widgetSize.height() - scaledSize.height()) / 2;
    int imgX = localPos.x() - offsetX;
    int imgY = localPos.y() - offsetY;
    if (imgX < 0 || imgX >= scaledSize.width() || imgY < 0 || imgY >= scaledSize.height())
        return QPoint(-1, -1);
    // 映射到原始分辨率
    double ratioX = (double)m_frame.width() / scaledSize.width();
    double ratioY = (double)m_frame.height() / scaledSize.height();
    int remoteX = qBound(0, (int)(imgX * ratioX), m_frame.width());
    int remoteY = qBound(0, (int)(imgY * ratioY), m_frame.height());
    return QPoint(remoteX, remoteY);
}

void RemoteScreenWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint remote = mapToRemote(event->pos());
    if (remote.x() < 0) return;
    QJsonObject cmd;
    cmd["type"] = "mouse_move";
    cmd["x"] = remote.x();
    cmd["y"] = remote.y();
    emit controlCommandGenerated(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
}

void RemoteScreenWidget::mousePressEvent(QMouseEvent* event)
{
    QPoint remote = mapToRemote(event->pos());
    if (remote.x() < 0) return;
    QJsonObject cmd;
    cmd["type"] = "mouse_press";
    cmd["button"] = (int)event->button();
    cmd["x"] = remote.x();
    cmd["y"] = remote.y();
    emit controlCommandGenerated(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
}

void RemoteScreenWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QPoint remote = mapToRemote(event->pos());
    if (remote.x() < 0) return;
    QJsonObject cmd;
    cmd["type"] = "mouse_release";
    cmd["button"] = (int)event->button();
    cmd["x"] = remote.x();
    cmd["y"] = remote.y();
    emit controlCommandGenerated(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
}

void RemoteScreenWidget::wheelEvent(QWheelEvent* event)
{
    QPoint remote = mapToRemote(event->position().toPoint());
    if (remote.x() < 0) return;
    QJsonObject cmd;
    cmd["type"] = "mouse_wheel";
    cmd["delta"] = event->angleDelta().y();
    emit controlCommandGenerated(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
}

void RemoteScreenWidget::keyPressEvent(QKeyEvent* event)
{
    QJsonObject cmd;
    cmd["type"] = "key_press";
    cmd["key"] = event->key();
    cmd["modifiers"] = (int)event->modifiers();
    emit controlCommandGenerated(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
    event->accept();
}

void RemoteScreenWidget::keyReleaseEvent(QKeyEvent* event)
{
    QJsonObject cmd;
    cmd["type"] = "key_release";
    cmd["key"] = event->key();
    cmd["modifiers"] = (int)event->modifiers();
    emit controlCommandGenerated(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
    event->accept();
}