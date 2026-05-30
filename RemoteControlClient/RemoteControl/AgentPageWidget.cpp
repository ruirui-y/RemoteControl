#include "AgentPageWidget.h"
#include "TCPMgr.h"
#include "common.pb.h"
#include "server_msg.pb.h"
#include "Global.h"                  // AppConfigPath 等
#include "JsonTool.h"
#include "ThreadPool.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QIntValidator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QGuiApplication>
#include <QBuffer>
#include <QDateTime>
#include <QThread>
#include <QDebug>

AgentPageWidget::AgentPageWidget(QWidget* parent)
    : QWidget(parent)
    , m_tcpMgr(ThreadPool::Instance()->GetTCPMgr())
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("AgentPageWidget");
    BuildUI();
    LoadSettings();
    BindSignals();
}

AgentPageWidget::~AgentPageWidget()
{
    if (m_agentRunning) {
        StopAgent();
    }
}

// ---------------------------------------------------------
// 界面构建（完全模仿 SettingWidget 风格）
// ---------------------------------------------------------
void AgentPageWidget::BuildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 30, 40, 30);
    root->setSpacing(20);

    // 大标题
    QLabel* mainTitle = new QLabel(tr("本机被控"), this);
    mainTitle->setObjectName("settingMainTitle");   // 复用 SettingWidget 标题样式
    root->addWidget(mainTitle);

    // ========== 分组 1：连接配置 ==========
    QFrame* connGroup = new QFrame(this);
    connGroup->setObjectName("settingGroupFrame");
    QVBoxLayout* connLayout = new QVBoxLayout(connGroup);
    connLayout->setContentsMargins(20, 20, 20, 20);
    connLayout->setSpacing(15);

    QLabel* connTitle = new QLabel(tr("连接配置"), connGroup);
    connTitle->setObjectName("settingGroupTitle");
    connLayout->addWidget(connTitle);

    // 服务器地址
    QHBoxLayout* serverLayout = new QHBoxLayout();
    QLabel* serverLbl = new QLabel(tr("服务器地址:"), connGroup);
    m_serverEdit = new QLineEdit(connGroup);
    m_serverEdit->setObjectName("agentInput");
    m_serverEdit->setFixedSize(200, 35);
    m_serverEdit->setPlaceholderText("127.0.0.1");
    serverLayout->addWidget(serverLbl);
    serverLayout->addWidget(m_serverEdit);
    serverLayout->addStretch();
    connLayout->addLayout(serverLayout);

    // 端口
    QHBoxLayout* portLayout = new QHBoxLayout();
    QLabel* portLbl = new QLabel(tr("端口:"), connGroup);
    m_portEdit = new QLineEdit(connGroup);
    m_portEdit->setObjectName("agentInput");
    m_portEdit->setFixedSize(120, 35);
    m_portEdit->setPlaceholderText("5486");
    m_portEdit->setValidator(new QIntValidator(1, 65535, this));
    portLayout->addWidget(portLbl);
    portLayout->addWidget(m_portEdit);
    portLayout->addStretch();
    connLayout->addLayout(portLayout);

    // 本机名称
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLbl = new QLabel(tr("本机名称:"), connGroup);
    m_nameEdit = new QLineEdit(connGroup);
    m_nameEdit->setObjectName("agentInput");
    m_nameEdit->setFixedSize(200, 35);
    m_nameEdit->setPlaceholderText("My-PC");
    nameLayout->addWidget(nameLbl);
    nameLayout->addWidget(m_nameEdit);
    nameLayout->addStretch();
    connLayout->addLayout(nameLayout);

    root->addWidget(connGroup);

    // ========== 分组 2：截图参数 ==========
    QFrame* captureGroup = new QFrame(this);
    captureGroup->setObjectName("settingGroupFrame");
    QVBoxLayout* captureLayout = new QVBoxLayout(captureGroup);
    captureLayout->setContentsMargins(20, 20, 20, 20);
    captureLayout->setSpacing(15);

    QLabel* captureTitle = new QLabel(tr("截图参数"), captureGroup);
    captureTitle->setObjectName("settingGroupTitle");
    captureLayout->addWidget(captureTitle);

    // 截图质量滑块
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    QLabel* qualityLbl = new QLabel(tr("截图质量:"), captureGroup);
    m_qualitySlider = new QSlider(Qt::Horizontal, captureGroup);
    m_qualitySlider->setObjectName("agentSlider");
    m_qualitySlider->setRange(10, 100);
    m_qualitySlider->setValue(70);
    m_qualitySlider->setFixedWidth(200);
    m_qualityValue = new QLabel("70%", captureGroup);
    m_qualityValue->setObjectName("agentSliderValue");
    qualityLayout->addWidget(qualityLbl);
    qualityLayout->addWidget(m_qualitySlider);
    qualityLayout->addWidget(m_qualityValue);
    qualityLayout->addStretch();
    captureLayout->addLayout(qualityLayout);

    // FPS 限制滑块
    QHBoxLayout* fpsLayout = new QHBoxLayout();
    QLabel* fpsLbl = new QLabel(tr("FPS 限制:"), captureGroup);
    m_fpsSlider = new QSlider(Qt::Horizontal, captureGroup);
    m_fpsSlider->setObjectName("agentSlider");
    m_fpsSlider->setRange(1, 30);
    m_fpsSlider->setValue(5);
    m_fpsSlider->setFixedWidth(200);
    m_fpsValue = new QLabel("5 fps", captureGroup);
    m_fpsValue->setObjectName("agentSliderValue");
    fpsLayout->addWidget(fpsLbl);
    fpsLayout->addWidget(m_fpsSlider);
    fpsLayout->addWidget(m_fpsValue);
    fpsLayout->addStretch();
    captureLayout->addLayout(fpsLayout);

    root->addWidget(captureGroup);

    // ========== 分组 3：运行状态 ==========
    QFrame* statusGroup = new QFrame(this);
    statusGroup->setObjectName("settingGroupFrame");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setContentsMargins(20, 20, 20, 20);
    statusLayout->setSpacing(10);

    QLabel* statusTitle = new QLabel(tr("运行状态"), statusGroup);
    statusTitle->setObjectName("settingGroupTitle");
    statusLayout->addWidget(statusTitle);

    QHBoxLayout* statusRow = new QHBoxLayout();
    m_statusLabel = new QLabel(tr("● 未启动"), statusGroup);
    m_statusLabel->setObjectName("agentStatusText");
    m_statusLabel->setProperty("agentStatus", "stopped");

    m_frameCountLabel = new QLabel(tr("已发送: 0 帧"), statusGroup);
    m_frameCountLabel->setObjectName("agentStatusText");

    statusRow->addWidget(m_statusLabel);
    statusRow->addWidget(m_frameCountLabel);
    statusRow->addStretch();
    statusLayout->addLayout(statusRow);

    root->addWidget(statusGroup);

    // 开始/停止按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_startBtn = new QPushButton(tr("开始被控"), this);
    m_startBtn->setObjectName("agentStartBtn");
    m_startBtn->setFixedSize(160, 45);
    m_startBtn->setCursor(Qt::PointingHandCursor);
    btnLayout->addWidget(m_startBtn);
    btnLayout->addStretch();
    root->addLayout(btnLayout);

    root->addStretch(1);
}

// ---------------------------------------------------------
// 信号槽绑定
// ---------------------------------------------------------
void AgentPageWidget::BindSignals()
{
    // 质量滑块数值显示
    connect(m_qualitySlider, &QSlider::valueChanged, this, [this](int val) {
        m_qualityValue->setText(QString("%1%").arg(val));
        });
    // FPS 滑块数值显示
    connect(m_fpsSlider, &QSlider::valueChanged, this, [this](int val) {
        m_fpsValue->setText(QString("%1 fps").arg(val));
        });

    // 开始/停止按钮
    connect(m_startBtn, &QPushButton::clicked, this, [this]() {
        if (m_agentRunning) {
            StopAgent();
        }
        else {
            StartAgent();
        }
        });

    // 接收远程控制指令（从 TCPMgr 信号）
    connect(m_tcpMgr, &TCPMgr::SigControlCmd, this, [this](const QByteArray& json) {
        // v1.0 仅记录日志，后续可在此模拟键鼠
        qDebug() << "[Agent] 收到控制指令:" << json;
        });
}

// ---------------------------------------------------------
// 配置读写
// ---------------------------------------------------------
void AgentPageWidget::LoadSettings()
{
    QJsonDocument doc;
    JsonTool::Instance()->readJsonFile(AppConfigPath, doc);
    QJsonObject obj = doc.object();

    m_serverEdit->setText(obj.value("AgentServer").toString("127.0.0.1"));
    m_portEdit->setText(obj.value("AgentPort").toString("5486"));
    m_nameEdit->setText(obj.value("AgentName").toString("My-PC"));
    m_qualitySlider->setValue(obj.value("AgentQuality").toInt(70));
    m_fpsSlider->setValue(obj.value("AgentFPS").toInt(5));
}

void AgentPageWidget::SaveSettings()
{
    QJsonDocument doc;
    JsonTool::Instance()->readJsonFile(AppConfigPath, doc);
    QJsonObject obj = doc.object();

    obj["AgentServer"] = m_serverEdit->text();
    obj["AgentPort"] = m_portEdit->text();
    obj["AgentName"] = m_nameEdit->text();
    obj["AgentQuality"] = m_qualitySlider->value();
    obj["AgentFPS"] = m_fpsSlider->value();

    doc.setObject(obj);
    JsonTool::Instance()->writeJsonFile(AppConfigPath, doc);
}

// ---------------------------------------------------------
// 启动/停止被控核心逻辑
// ---------------------------------------------------------
void AgentPageWidget::StartAgent()
{
    // 保存当前设置
    SaveSettings();

    // 初始化截图定时器
    if (!m_captureTimer) {
        m_captureTimer = new QTimer(this);
        connect(m_captureTimer, &QTimer::timeout, this, &AgentPageWidget::onCaptureTimeout);
    }

    int fps = m_fpsSlider->value();
    m_captureTimer->start(1000 / fps);

    m_agentRunning = true;
    m_startBtn->setText(tr("停止被控"));
    m_statusLabel->setText(tr("● 运行中"));
    m_statusLabel->setProperty("agentStatus", "running");
    m_frameCount = 0;
    m_frameCountLabel->setText(tr("已发送: 0 帧"));

    // 禁用参数输入
    m_serverEdit->setEnabled(false);
    m_portEdit->setEnabled(false);
    m_nameEdit->setEnabled(false);
    m_qualitySlider->setEnabled(false);
    m_fpsSlider->setEnabled(false);

    // 刷新样式
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

void AgentPageWidget::StopAgent()
{
    if (m_captureTimer) {
        m_captureTimer->stop();
    }

    m_agentRunning = false;
    m_startBtn->setText(tr("开始被控"));
    m_statusLabel->setText(tr("● 已停止"));
    m_statusLabel->setProperty("agentStatus", "stopped");

    // 启用参数输入
    m_serverEdit->setEnabled(true);
    m_portEdit->setEnabled(true);
    m_nameEdit->setEnabled(true);
    m_qualitySlider->setEnabled(true);
    m_fpsSlider->setEnabled(true);

    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

void AgentPageWidget::onCaptureTimeout()
{
    // 1. 截图
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;
    QPixmap pixmap = screen->grabWindow(0);   // 全屏幕截图

    // 2. 压缩为 JPEG
    QByteArray jpegData;
    QBuffer buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPEG", m_qualitySlider->value());

    // 3. 构造 Protobuf 消息
    ServerApi::ScreenFrame frame;
    frame.set_frame_index(m_frameCount++);
    frame.set_capture_ms(QDateTime::currentMSecsSinceEpoch());
    frame.set_width(pixmap.width());
    frame.set_height(pixmap.height());
    frame.set_jpeg_data(jpegData.toStdString());

    // 4. 通过 TCPMgr 发送
    m_tcpMgr->SendProtoMsg(ServerApi::ID_SCREEN_FRAME, frame);

    // 5. 更新 UI 帧计数（注意跨线程调用，但 Timer 在主线程，直接更新即可）
    m_frameCountLabel->setText(tr("已发送: %1 帧").arg(m_frameCount));
}

// ---------------------------------------------------------
// 页面显示/隐藏时不做额外操作（用户可继续运行或手动停止）
// ---------------------------------------------------------
void AgentPageWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
}

void AgentPageWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    // 隐藏时不自动停止，保持后台运行
}