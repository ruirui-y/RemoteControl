#include "SettingWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>
#include "Global.h"         
#include "JsonTool.h"       
#include "CinemaMessageBox.h"

SettingWidget::SettingWidget(QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StyledBackground, true);
	setObjectName("SettingWidget");
	BuildUI();
	BindSignals();
}

SettingWidget::~SettingWidget()
{
}

// ---------------------------------------------------------
// 模块 1：界面构建
// ---------------------------------------------------------
void SettingWidget::BuildUI()
{
    // 根布局采用垂直布局
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 30, 40, 30);
    root->setSpacing(20);

    // 1. 大标题
    QLabel* mainTitle = new QLabel(tr("系统设置"), this);
    mainTitle->setObjectName("settingMainTitle");
    root->addWidget(mainTitle);

    // ==========================================================
    // 分组 1：常规设置 (General Settings)
    // ==========================================================
    QFrame* generalGroup = new QFrame(this);
    generalGroup->setObjectName("settingGroupFrame");
    QVBoxLayout* generalLayout = new QVBoxLayout(generalGroup);
    generalLayout->setContentsMargins(20, 20, 20, 20);
    generalLayout->setSpacing(15);

    // 小标题
    QLabel* generalTitle = new QLabel(tr("常规设置"), generalGroup);
    generalTitle->setObjectName("settingGroupTitle");
    generalLayout->addWidget(generalTitle);

    // ---------------------------------------------------------
    // 设置项 1：界面语言
    // ---------------------------------------------------------
    QHBoxLayout* langLayout = new QHBoxLayout();
    QLabel* langLbl = new QLabel(tr("界面语言:"), generalGroup);

    m_langBox = new QComboBox(generalGroup);
    m_langBox->setObjectName("settingComboBox");
    m_langBox->setFixedSize(160, 35);
    m_langBox->setCursor(Qt::PointingHandCursor);
    m_langBox->addItem("简体中文", "zh");
    m_langBox->addItem("English", "en");

    // 读取当前系统的语言配置，设置下拉框的默认选中项
    QJsonDocument doc;
    JsonTool::Instance()->readJsonFile(AppConfigPath, doc);
    QString currentLang = doc.object()["Language"].toString("zh");
    int langIndex = m_langBox->findData(currentLang);
    if (langIndex != -1) {
        m_langBox->setCurrentIndex(langIndex);
    }

    langLayout->addWidget(langLbl);
    langLayout->addWidget(m_langBox);
    langLayout->addStretch();

    generalLayout->addLayout(langLayout);

    // ---------------------------------------------------------
    // 设置项 2：界面皮肤
    // ---------------------------------------------------------
    QHBoxLayout* skinLayout = new QHBoxLayout();
    QLabel* skinLbl = new QLabel(tr("界面皮肤:"), generalGroup);

    m_skinBox = new QComboBox(generalGroup);
    m_skinBox->setObjectName("settingComboBox");
    m_skinBox->setFixedSize(160, 35);
    m_skinBox->setCursor(Qt::PointingHandCursor);
    m_skinBox->addItem(tr("深海蔚蓝 (默认)"), "ocean");
    m_skinBox->addItem(tr("深翠夜幕"), "aurora");
    m_skinBox->addItem(tr("琥珀深林"), "amber");
    m_skinBox->addItem(tr("猩红纪元"), "crimson");
    m_skinBox->addItem(tr("星穹紫夜"), "violet");
    m_skinBox->addItem(tr("霜雪银灰"), "frost");
    m_skinBox->addItem(tr("余烬暗红"), "embers");

    // 读取当前系统的皮肤配置，设置下拉框的默认选中项
    QString currentSkin = doc.object()["Skin"].toString("default");
    int skinIndex = m_skinBox->findData(currentSkin);
    if (skinIndex != -1) {
        m_skinBox->setCurrentIndex(skinIndex);
    }

    skinLayout->addWidget(skinLbl);
    skinLayout->addWidget(m_skinBox);
    skinLayout->addStretch();

    generalLayout->addLayout(skinLayout);
    // ==========================================================

    // 将"常规设置"分组加入根布局
    root->addWidget(generalGroup);

    // 最底下加一个弹簧，把所有的设置块往上顶
    root->addStretch(1);
}

// ---------------------------------------------------------
// 模块 2：信号与槽绑定
// ---------------------------------------------------------
void SettingWidget::BindSignals()
{
    // 绑定语言切换事件
    connect(m_langBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        // 1. 获取选中的语言代码 ("zh" 或 "en")
        QString langCode = m_langBox->itemData(index).toString();

        // 2. 读取现有的系统配置文件
        QJsonDocument doc;
        JsonTool::Instance()->readJsonFile(AppConfigPath, doc);
        QJsonObject obj = doc.object();

        // 3. 修改 Language 字段并写回
        obj["Language"] = langCode;
        doc.setObject(obj);
        JsonTool::Instance()->writeJsonFile(AppConfigPath, doc);

        // 4. 弹出重启生效提示
        CinemaMessageBox::ShowInfo(this, tr("提示"), tr("语言设置已保存，将在下次启动软件时生效。"));
        });

    // 绑定皮肤切换事件
    connect(m_skinBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        // 1. 获取选中的皮肤代码 ("default" / "aurora" / "amber")
        QString skinCode = m_skinBox->itemData(index).toString();

        // 2. 读取现有的系统配置文件
        QJsonDocument doc;
        JsonTool::Instance()->readJsonFile(AppConfigPath, doc);
        QJsonObject obj = doc.object();

        // 3. 修改 Skin 字段并写回
        obj["Skin"] = skinCode;
        doc.setObject(obj);
        JsonTool::Instance()->writeJsonFile(AppConfigPath, doc);

        // 4. 弹出重启生效提示
        CinemaMessageBox::ShowInfo(this, tr("提示"), tr("皮肤设置已保存，将在下次启动软件时生效。"));
        });
}