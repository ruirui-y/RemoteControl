#ifndef REMOTESCREENWIDGET_H
#define REMOTESCREENWIDGET_H

#include <QWidget>
#include <QImage>

class RemoteScreenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteScreenWidget(QWidget* parent = nullptr);
    void updateFrame(const QImage& img);
    void clearFrame();

signals:
    void controlCommandGenerated(const QByteArray& jsonCmd); // 可后续连接 TCPMgr 发送指令

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    QPoint mapToRemote(const QPoint& localPos) const;
    QImage m_frame;
    QSize  m_remoteSize;                                // 真实远端分辨率（可从第一帧中获取）
};

#endif // REMOTESCREENWIDGET_H