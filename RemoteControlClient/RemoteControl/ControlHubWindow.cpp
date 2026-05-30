#include "ControlHubWindow.h"
#include "TitleBar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QDebug>
#include <QPainter>
#include <QListWidget>
#include <QButtonGroup>
#include "GameItem.h"
#include "SettingWidget.h"
#include "UserMgr.h"
#include "ControlPageWidget.h"
#include "AgentPageWidget.h"

// 假设你有一个定义侧边栏宽度的宏，如果没有可以在这里写死比如 200
#ifndef HZ_LIST_WIDTH
#define HZ_LIST_WIDTH 200
#endif

ControlHubWindow::ControlHubWindow(QWidget* parent) : QWidget(parent)
{
    resize(1100, 800);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent); // 我来负责整窗绘制，更高效

    // 1.根布局(垂直布局)
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // 2.顶部栏
    m_title = new TitleBar(this);
    m_title->SetMode(TitleMode::Hub);
    m_title->SetUserName(UserMgr::Instance()->GetUserName());
    root->addWidget(m_title);

    // 3.主窗口(水平布局)
    auto* center = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout(center);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);

    center->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ==========================================================
    // 4. 左侧边栏 (剥离了 MiniWorld 概念，替换为中控模块名)
    // ==========================================================
    m_gameItems.insert(tr("远程控制"), ":/icons/remote_control.png");   // 索引 0
    m_gameItems.insert(tr("本机被控"), ":/icons/local_agent.png");     // 索引 1
    m_gameItems.insert(tr("系统设置"), ":/MiNi/Images/MiNiWorld/Setting.png");

    m_leftList_c = new QListWidget(center);
    m_leftList_c->setObjectName("LeftSidebar");                                        // 为左侧栏赋予唯一 ID
    QVBoxLayout* listLayout = new QVBoxLayout(m_leftList_c);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(0);
    m_leftList_c->setFixedWidth(HZ_LIST_WIDTH);

    // 按钮组
    m_leftList_btns = new QButtonGroup(m_leftList_c);
    m_leftList_btns->setExclusive(true);                                                // 互斥选中

    AddGameItem(tr("远程控制"));  // 按钮 id: 2
    AddGameItem(tr("本机被控"));  // 按钮 id: 0

    // 添加弹簧把上面三个标签顶上去
    listLayout->addStretch(1);

    // 添加底部的设置按钮
    AddGameItem(tr("系统设置"));  // 按钮 id: 3

    // 默认选中第一个“游戏安装”
    m_leftList_btns->button(0)->setChecked(true);

    hbox->addWidget(m_leftList_c, 0);

    // ==========================================================
    // 5. 右边堆栈窗口 (页面管理器)
    // ==========================================================
    m_stack = new QStackedLayout();
    m_stack->setContentsMargins(0, 0, 0, 0);
    m_stack->setSpacing(0);

    ControlPageWidget* controlPage = new ControlPageWidget(center);                                      // 控制端
    AgentPageWidget* agentPage = new AgentPageWidget(center);                                          // 被控端
    SettingWidget* setting = new SettingWidget(center);

    m_stack->addWidget(controlPage);
    m_stack->addWidget(agentPage);
    m_stack->addWidget(setting);

    // 绑定侧边栏切换信号
    connect(m_leftList_btns, QOverload<int>::of(&QButtonGroup::buttonClicked),
        m_stack, &QStackedLayout::setCurrentIndex);

    hbox->addLayout(m_stack, 1);
    root->addWidget(center, 1);
}

void ControlHubWindow::AddGameItem(QString name)
{
    GameItem* item = new GameItem(name, m_gameItems.value(name), m_leftList_c);
    item->setCheckable(true);
    item->setChecked(true);
    // 自动以当前 count 作为其 ID（0, 1, 2, 3...）
    m_leftList_btns->addButton(item, m_leftList_btns->buttons().count());
    m_leftList_c->layout()->addWidget(item);
}