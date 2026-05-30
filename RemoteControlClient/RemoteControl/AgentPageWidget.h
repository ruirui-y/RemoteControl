#ifndef AGENTPAGEWIDGET_H
#define AGENTPAGEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QScreen>

class TCPMgr;

class AgentPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AgentPageWidget(QWidget* parent = nullptr);
    ~AgentPageWidget();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void BuildUI();
    void BindSignals();
    void LoadSettings();        // 从配置文件读取参数
    void SaveSettings();        // 保存当前参数
    void StartAgent();          // 开始被控
    void StopAgent();           // 停止被控
    void onCaptureTimeout();    // 定时截图并发送

    // UI 组件
    QLineEdit* m_serverEdit = nullptr;  // 服务器地址
    QLineEdit* m_portEdit = nullptr;  // 端口
    QLineEdit* m_nameEdit = nullptr;  // 本机名称
    QSlider* m_qualitySlider = nullptr; // 截图质量 (10-100)
    QLabel* m_qualityValue = nullptr;  // 质量数值显示
    QSlider* m_fpsSlider = nullptr;  // FPS (1-30)
    QLabel* m_fpsValue = nullptr;  // FPS 数值显示

    QLabel* m_statusLabel = nullptr;  // 连接状态文本
    QLabel* m_frameCountLabel = nullptr; // 已发送帧数
    QPushButton* m_startBtn = nullptr;  // 开始/停止按钮

    // 运行状态
    TCPMgr* m_tcpMgr = nullptr;
    QTimer* m_captureTimer = nullptr;
    bool         m_agentRunning = false;
    int          m_frameCount = 0;
};

#endif // AGENTPAGEWIDGET_H