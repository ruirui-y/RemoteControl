#include "ControlPageWidget.h"
#include "TCPMgr.h"
#include "common.pb.h"
#include "server_msg.pb.h"
#include "ThreadPool.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QDebug>
#include <QDateTime>
#include <QImage>
#include <QShowEvent>
#include <QHideEvent>

ControlPageWidget::ControlPageWidget(QWidget* parent)
    : QWidget(parent)
    , m_tcpMgr(ThreadPool::Instance()->GetTCPMgr())
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("ControlPageWidget");
    BuildUI();
    BindSignals();
}

ControlPageWidget::~ControlPageWidget()
{
}

// ---------------------------------------------------------
// 界面构建
// ---------------------------------------------------------
void ControlPageWidget::BuildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 30, 40, 30);
    root->setSpacing(20);

    // 大标题
    QLabel* mainTitle = new QLabel(tr("远程控制"), this);
    mainTitle->setObjectName("controlMainTitle");
    root->addWidget(mainTitle);

    // ========== 操作栏分组 ==========
    QFrame* controlBar = new QFrame(this);
    controlBar->setObjectName("controlBarFrame");
    QHBoxLayout* barLayout = new QHBoxLayout(controlBar);
    barLayout->setContentsMargins(20, 15, 20, 15);
    barLayout->setSpacing(15);

    QLabel* deviceLabel = new QLabel(tr("远端设备:"), controlBar);
    deviceLabel->setObjectName("controlBarLabel");

    m_deviceBox = new QComboBox(controlBar);
    m_deviceBox->setObjectName("controlDeviceCombo");
    m_deviceBox->setFixedSize(200, 35);
    m_deviceBox->setCursor(Qt::PointingHandCursor);

    m_connectBtn = new QPushButton(tr("连接设备"), controlBar);
    m_connectBtn->setObjectName("controlConnectBtn");
    m_connectBtn->setFixedSize(120, 35);
    m_connectBtn->setCursor(Qt::PointingHandCursor);

    barLayout->addWidget(deviceLabel);
    barLayout->addWidget(m_deviceBox);
    barLayout->addWidget(m_connectBtn);
    barLayout->addStretch();

    root->addWidget(controlBar);

    // ========== 远程屏幕显示区域 ==========
    QFrame* screenFrame = new QFrame(this);
    screenFrame->setObjectName("controlScreenFrame");
    QVBoxLayout* screenLayout = new QVBoxLayout(screenFrame);
    screenLayout->setContentsMargins(0, 0, 0, 0);

    m_screenView = new RemoteScreenWidget(screenFrame);
    m_screenView->setObjectName("controlScreenView");
    // 设置最小尺寸保证可见
    m_screenView->setMinimumSize(640, 480);
    screenLayout->addWidget(m_screenView);

    root->addWidget(screenFrame, 1); // stretch=1 让屏幕区域占据剩余空间

    // ========== 状态栏分组 ==========
    QFrame* statusBar = new QFrame(this);
    statusBar->setObjectName("controlStatusBar");
    QHBoxLayout* statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(20, 10, 20, 10);
    statusLayout->setSpacing(30);

    m_statusLabel = new QLabel(tr("● 未连接"), statusBar);
    m_statusLabel->setObjectName("controlStatusText");

    m_latencyLabel = new QLabel(tr("延迟: -- ms"), statusBar);
    m_latencyLabel->setObjectName("controlStatusText");

    m_fpsLabel = new QLabel(tr("FPS: --"), statusBar);
    m_fpsLabel->setObjectName("controlStatusText");

    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_latencyLabel);
    statusLayout->addWidget(m_fpsLabel);
    statusLayout->addStretch();

    root->addWidget(statusBar);
}

// ---------------------------------------------------------
// 信号绑定
// ---------------------------------------------------------
void ControlPageWidget::BindSignals()
{
    // 连接/断开按钮
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        if (m_connected) {
            // 断开连接
            m_connected = false;
            m_connectBtn->setText(tr("连接设备"));
            m_statusLabel->setText(tr("● 未连接"));
            m_statusLabel->setProperty("status", "offline");
            m_screenView->clearFrame(); // 清空画面
            // 此处可通知服务器取消订阅，v1.0 忽略
        }
        else {
            // 尝试连接
            QString deviceId = m_deviceBox->currentData().toString();
            if (deviceId.isEmpty()) {
                return;
            }
            // v1.0 直接认为连接成功，开始接收流
            m_connected = true;
            m_connectBtn->setText(tr("断开连接"));
            m_statusLabel->setText(tr("● 已连接"));
            m_statusLabel->setProperty("status", "online");
            // 此时屏幕流由 TCP 路由自动刷新
        }
        // 强制刷新样式
        m_statusLabel->style()->unpolish(m_statusLabel);
        m_statusLabel->style()->polish(m_statusLabel);
        });

    // 连接 TCPMgr 的信号
    connect(m_tcpMgr, &TCPMgr::SigScreenFrame, this, [this](const QImage& img, qint64 latency, double fps) {
        // 更新界面（此 lambda 在主线程执行）
        m_latencyLabel->setText(tr("延迟: %1 ms").arg(latency));
        m_fpsLabel->setText(tr("FPS: %1").arg(fps, 0, 'f', 1));
        m_screenView->updateFrame(img);
        });

    // 连接指令发送（原 RemoteScreenWidget 事件产生的指令需要发到 TCPMgr）
    connect(m_screenView, &RemoteScreenWidget::controlCommandGenerated, this, [this](const QByteArray& json) {
        ServerApi::ControlCmd cmd;
        cmd.set_json_cmd(json.toStdString());
        m_tcpMgr->SendProtoMsg(ServerApi::ID_CONTROL_CMD, cmd);
        });
}


// ---------------------------------------------------------
// 页面显示/隐藏时管理路由
// ---------------------------------------------------------
void ControlPageWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // 此处可重新注册路由（若之前注销了）
}

void ControlPageWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    // 页面隐藏时可选择注销路由，避免其他页面显示远程画面
    // UnregisterTCPRoutes();
}