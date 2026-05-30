#include "mainWindow.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QTranslator>
#include <QLocale>
#include "Macro.h"
#include "GameItem.h"
#include "LogRecord.h"
#include "Enum.h"
#include "ThreadPool.h"
#include "common.pb.h"
#include "server_msg.pb.h"
#include "JsonTool.h"


void LoadStyle(QApplication* app)
{
    QJsonDocument userDoc;
    JsonTool::Instance()->readJsonFile(AppConfigPath, userDoc);
    QString skinName = userDoc.object()["Skin"].toString("ocean");    // 默认 ocean

    QFile templateFile("./StyleSheet/template.qss");
    if (!templateFile.open(QFile::ReadOnly)) {
        qDebug() << "Failed to open template QSS!";
        return;
    }
    QString qss = QString::fromUtf8(templateFile.readAll());
    templateFile.close();

    QFile themesFile("./StyleSheet/themes.json");
    if (!themesFile.open(QFile::ReadOnly)) {
        qDebug() << "Failed to open themes.json!";
        return;
    }
    QJsonDocument themesDoc = QJsonDocument::fromJson(themesFile.readAll());
    themesFile.close();

    QJsonObject allThemes = themesDoc.object();
    QJsonObject theme = allThemes[skinName].toObject();

    if (theme.isEmpty()) {
        qDebug() << "Skin not found:" << skinName << ", fallback to ocean";
        theme = allThemes["ocean"].toObject();
    }

    for (auto it = theme.begin(); it != theme.end(); ++it) {
        QString placeholder = "{{" + it.key() + "}}";
        QString value = it.value().toString();
        qss.replace(placeholder, value);
    }

    app->setStyleSheet(qss);
    qDebug() << "Load Style Success:" << skinName;
}

void RegisterMetaTypes()
{
    // =========================================================================================
    // 1. 👑 核心业务结构体 (解决跨线程 UI 渲染报错的关键)
    // =========================================================================================
    qRegisterMetaType<DeviceInfo>("DeviceInfo");                                    // 单个设备信息
    qRegisterMetaType<QVector<DeviceInfo>>("QVector<DeviceInfo>");                  // 设备信息列表 (解决 Cannot queue arguments 报错)

    // =========================================================================================
    // 2. 核心与基础枚举
    // =========================================================================================
    qRegisterMetaType<ServerApi::MsgId>("ServerApi::MsgId");
    qRegisterMetaType<ServerApi::FileType>("ServerApi::FileType");                  // 底层引擎文件类型过滤枚举

    // =========================================================================================
    // 3. 登录与基础模块
    // =========================================================================================
    qRegisterMetaType<ServerApi::LoginRsp>("ServerApi::LoginRsp");                  // 登录响应

    // =========================================================================================
    // 4. 🥽 VR 设备管理模块相关响应 (👑 最新增补)
    // =========================================================================================
    qRegisterMetaType<ServerApi::AddUserVrDeviceRsp>("ServerApi::AddUserVrDeviceRsp");         // 添加设备响应
    qRegisterMetaType<ServerApi::DelUserVrDeviceRsp>("ServerApi::DelUserVrDeviceRsp");         // 删除设备响应
    qRegisterMetaType<ServerApi::UpdateDeviceNickRsp>("ServerApi::UpdateDeviceNickRsp");       // 更新昵称响应
    qRegisterMetaType<ServerApi::UpdateDeviceTeamRsp>("ServerApi::UpdateDeviceTeamRsp");       // 更新队伍响应
    qRegisterMetaType<ServerApi::UpdateDevicePlayTimeRsp>("ServerApi::UpdateDevicePlayTimeRsp"); // 更新时间响应
    qRegisterMetaType<ServerApi::GetVrDeviceListRsp>("ServerApi::GetVrDeviceListRsp");         // 获取设备列表响应
    qRegisterMetaType<ServerApi::VrDeviceSummary>("ServerApi::VrDeviceSummary");               // 单个设备摘要(PB)

    // =========================================================================================
    // 5. 💰 支付与商业化模块
    // =========================================================================================
    qRegisterMetaType<ServerApi::GetWalletRsp>("ServerApi::GetWalletRsp");          // 钱包余额与历史统计响应
    qRegisterMetaType<ServerApi::GetGoodsRsp>("ServerApi::GetGoodsRsp");            // 充值套餐列表响应
    qRegisterMetaType<ServerApi::CreateOrderRsp>("ServerApi::CreateOrderRsp");      // 订单创建与二维码下发响应
    qRegisterMetaType<ServerApi::OrderNotifyPush>("ServerApi::OrderNotifyPush");    // 支付成功异步推送通知
    qRegisterMetaType<ServerApi::GetFlowRsp>("ServerApi::GetFlowRsp");              // 资金流水列表响应

    // 💡 提示：如果信号里传递了列表内的单条数据或结构体，也需要注册以确保跨线程安全
    qRegisterMetaType<ServerApi::GoodsInfo>("ServerApi::GoodsInfo");                // 单条充值套餐结构体
    qRegisterMetaType<ServerApi::PointFlowRecord>("ServerApi::PointFlowRecord");    // 单条资金流水记录
}

void LoadAppLanguage()
{
    // 1. 读取复用的 LoginConfig.json
    QJsonDocument doc;
    JsonTool::Instance()->readJsonFile(AppConfigPath, doc);
    QJsonObject obj = doc.object();

    // 2. 提取语言配置。如果文件为空或没有 "Language" 字段，默认 fallback 到 "zh"
    QString langCode = "zh";
    if (!obj.isEmpty() && obj.contains("Language")) {
        langCode = obj["Language"].toString();
    }

    // 3. 执行加载逻辑
    if (langCode == "en") 
    {
        // ==========================================================
        // 1. 挂载英文词典 (翻译你的业务代码)
        // ==========================================================
        static QTranslator s_translator;
        QString fullPath = GetLanguageFilePath("en");
        qDebug() << "[Language] Loading:" << fullPath;
        if (QFile::exists(fullPath) && s_translator.load(fullPath)) {
            qApp->installTranslator(&s_translator);
        }

        // ==========================================================
        // 👑 2. 强行把整个应用程序的底层文化环境设为美国英语！
        // 这会瞬间把日历、时间格式全部变成英文原生状态！
        // ==========================================================
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

        qDebug() << "[Language] 成功切换为全英文环境";
    }
}

int main(int argc, char *argv[])
{
    // =================================================================
    // 强心剂 1：强制抛弃 DirectShow，使用 Windows 现代媒体引擎 (WMF)
    // 这会让 Qt 使用和 Windows 自带播放器一模一样的底层解码和渲染管线！
    // =================================================================
    qputenv("QT_MULTIMEDIA_PREFERRED_PLUGINS", "windowsmediafoundation");

    // =================================================================
    // 强心剂 2：开启 Qt 的全局高分屏与抗锯齿缩放支持
    // 防止 Windows 系统级的野蛮拉伸导致像素狗牙
    // =================================================================
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    InitGlobalPaths();                                                                  // 初始化全局路径

    RegisterMetaTypes();                                                                // 注册跨线程通信类型

    // 加载语言
    LoadAppLanguage();

    // 加载样式表
    LoadStyle(&app);

    // 开始记录日志
    LogRecord::startRecord("Log.txt");

    // 启动线程池
    ThreadPool::Instance()->Start(4);

    mainWindow window;

    app.setWindowIcon(QIcon(":/MiNi/Images/MiNiWorld/Login.jpg"));

    app.exec();

    // 停止线程池
    ThreadPool::Instance()->Stop();

    return 0;
}