#ifndef CONTROLPAGEWIDGET_H
#define CONTROLPAGEWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "RemoteScreenWidget.h"

class TCPMgr;

class ControlPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPageWidget(QWidget* parent = nullptr);
    ~ControlPageWidget();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void BuildUI();
    void BindSignals();

    // 界面组件
    QComboBox* m_deviceBox = nullptr;                                                   // 远端设备下拉列表
    QPushButton* m_connectBtn = nullptr;  // 连接/断开按钮
    RemoteScreenWidget* m_screenView = nullptr; // 远程屏幕显示组件
    QLabel* m_latencyLabel = nullptr; // 延迟显示
    QLabel* m_fpsLabel = nullptr; // 帧率显示
    QLabel* m_statusLabel = nullptr; // 连接状态

    // 网络与数据
    TCPMgr* m_tcpMgr = nullptr;  // 全局单例，不负责生命周期
    bool            m_connected = false;
    QTimer          m_refreshTimer;           // 用于定期刷新设备列表（可选）
};

#endif // CONTROLPAGEWIDGET_H