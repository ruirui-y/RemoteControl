#include "BaseDeviceRow.h"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFrame>
#include <QToolButton>
#include <QPainter>
#include <QIcon>
#include "ThreadPool.h"
#include "CinemaMessageBox.h"


BaseDeviceRow::BaseDeviceRow(const QString& id, QWidget* parent)
    : QWidget(parent), m_id(id)
{
    BuildUI();
}

void BaseDeviceRow::BuildUI()
{
    setFixedHeight(40);
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(8, 4, 8, 4);
    lay->setSpacing(8);

    // 勾选框
    m_cb = new QCheckBox(this);
    m_cb->setObjectName("DeviceCheckBox");
    m_cb->setCursor(Qt::PointingHandCursor);
    m_cb->setToolTip(tr("选中/取消选中该设备"));

    // 在线指示灯 (默认设置为离线 false)
    m_lamp = new QFrame(this);
    m_lamp->setObjectName("DeviceOnlineLamp");
    m_lamp->setFixedSize(14, 14);
    m_lamp->setProperty("isOnline", false);                                     // 设置初始自定义属性

    // 设备图标/电量展示
    m_deviceIcon = new QToolButton(this);
    m_deviceIcon->setObjectName("DeviceIconBtn");
    m_baseIconPixmap = QPixmap(":/demand/Images/MiNiWorld/DeviceIcon.png");
    m_deviceIcon->setIcon(QIcon(m_baseIconPixmap));
    m_deviceIcon->setFixedSize(28, 28);
    m_deviceIcon->setAutoRaise(true);
    m_deviceIcon->setCursor(Qt::PointingHandCursor);                            // 鼠标手型
    m_deviceIcon->setToolTip(tr("当前设备离线"));                                 // 默认提示

    // 设备名称
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setObjectName("DeviceNameEdit");
    m_nameEdit->setToolTip(tr("点击修改设备名称"));

    // 移除按钮
    m_removeBtn = new QToolButton(this);
    m_removeBtn->setObjectName("RemoveBtn");
    m_removeBtn->setIcon(QIcon(":/demand/Images/MiNiWorld/RemoveDevice.png"));

    // 将移除按钮热区拉宽到 80px，和上方表头严格对齐
    m_removeBtn->setFixedSize(80, 28);

    m_removeBtn->setAutoRaise(true);
    m_removeBtn->setCursor(Qt::PointingHandCursor);
    m_removeBtn->setToolTip(tr("移除此设备"));

    lay->addWidget(m_cb);
    lay->addWidget(m_lamp);
    lay->addWidget(m_deviceIcon);
    lay->addWidget(m_nameEdit, 1);
    lay->addWidget(m_removeBtn);

    BindSlots();
}

void BaseDeviceRow::BindSlots()
{
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &BaseDeviceRow::OnEditFinish);

    TCPMgr* tcp = ThreadPool::Instance()->GetTCPMgr();
    UDPBroadCaster* udp = ThreadPool::Instance()->GetUdpBroadcaster();

    // 1. 删除设备请求：二次确认拦截 -> 组装 Protobuf 并发送
    connect(m_removeBtn, &QToolButton::clicked, this, [this, tcp]()
        {
            // 👑 弹出确认窗口拦截误触
            if (CinemaMessageBox::ShowQuestion(this, tr("移除设备"), tr("确定要移除该头显设备吗？\n此操作将不可逆转。")))
            {
                ServerApi::DelUserVrDeviceReq req;
                req.set_device_id(m_id.toStdString());
                tcp->SendProtoMsg(ServerApi::ID_DEL_USER_VR_DEVICE_REQ, req);
            }
        });

    // 2. 绑定 UDP 更新状态信号
    connect(udp, &UDPBroadCaster::SigStatusUpdate, this, [this](const DeviceStatus& status) 
        {
            if (status.id == m_id) 
            {
                SetOnline(status.online);
                SetBattery(status.battery);
            }
        });
}

void BaseDeviceRow::OnEditFinish()
{
    QString newName = m_nameEdit->text().trimmed();

    // 1. 拦截无效修改（没改动，或者改成了空）
    if (newName == m_originalName || newName.isEmpty())
    {
        m_nameEdit->setText(m_originalName); // 恢复原状
        m_nameEdit->clearFocus();
        return;
    }

    // ==========================================================
    // 👑 悲观 UI 核心：进入 Loading/锁定 状态
    // ==========================================================
    m_nameEdit->setEnabled(false);                   // 锁定输入框，防止用户在等待期间疯狂连点
    m_nameEdit->setStyleSheet("color: #999999;");    // 文字变灰，暗示“正在同步中...”

    // 2. 跨线程安全投递 UDP 请求给头显
    UDPBroadCaster* udp = ThreadPool::Instance()->GetUdpBroadcaster();
    ThreadPool::Instance()->PostTask(udp, [id = m_id, newName](UDPBroadCaster* u) 
        {
            u->ChangeClientText(id, newName);
        });

    // 3. 启动超时回滚机制（例如 5 秒）
    // 如果 5 秒后，TCP 还没有触发全量刷新来调用 SetInfo，说明 UDP 丢包了或头显没理你
    QTimer::singleShot(5000, this, [this, newName]() {
        // 怎么判断是否超时？如果输入框还是 disabled 状态，说明 SetInfo 没被调用过！
        if (!m_nameEdit->isEnabled() && m_nameEdit->text() == newName)
        {
            // 恢复 UI 可用状态
            m_nameEdit->setEnabled(true);
            m_nameEdit->setStyleSheet("");

            // 🌟 核心：回滚到最初的真理名字！斩断假更新！
            m_nameEdit->setText(m_originalName);

            // 弹出气泡提示用户
            CinemaMessageBox::ShowError(this, tr("昵称修改"), tr("修改超时：头显设备未响应"));
        }
        });
}

void BaseDeviceRow::SetInfo(const DeviceInfo& info)
{
    m_id = info.device_id;

    // 1. 备份真理
    m_originalName = info.device_nick;

    // 2. 覆盖 UI 文本
    m_nameEdit->setText(info.device_nick);

    // 3. 解除锁定状态（恢复可用，清除置灰样式）
    m_nameEdit->setEnabled(true);
    m_nameEdit->setStyleSheet("");
}

void BaseDeviceRow::SetOnline(bool online)
{
    m_isOnline = online;

    // 👑 通过修改自定义属性并刷新样式表，将颜色控制权交还给 QSS
    m_lamp->setProperty("isOnline", online);
    m_lamp->style()->unpolish(m_lamp);
    m_lamp->style()->polish(m_lamp);

    if (!online)
    {
        m_deviceIcon->setIcon(QIcon(m_baseIconPixmap));
        m_deviceIcon->setToolTip(tr("当前设备离线"));
    }
    else
    {
        m_deviceIcon->setToolTip(tr("设备已连接，电量获取中..."));
    }
}

void BaseDeviceRow::SetBattery(int level)
{
    if (!m_isOnline)
        return;

    level = qBound(0, level, 100);
    if (level == 0) return;

    // 👑 动态更新悬浮提示框显示精确电量
    m_deviceIcon->setToolTip(tr("设备电量: %1%").arg(level));

    QPixmap pix(m_baseIconPixmap.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.drawPixmap(0, 0, m_baseIconPixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);

    int h = pix.height();
    int fill = h * level / 100;

    // 0-20红，21-40黄，41-100绿
    p.fillRect(0, h - fill, pix.width(), fill, (level <= 20) ? "#FF4D4F" : ((level <= 40) ? "#FFC107" : "#37D978"));
    p.end();

    m_deviceIcon->setIcon(QIcon(pix));
}