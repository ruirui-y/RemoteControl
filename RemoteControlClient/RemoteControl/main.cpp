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
    // 1. 核心与基础枚举
    // =========================================================================================
    qRegisterMetaType<ServerApi::MsgId>("ServerApi::MsgId");
    qRegisterMetaType<ServerApi::FileType>("ServerApi::FileType");                  // 底层引擎文件类型过滤枚举

    // =========================================================================================
    // 2. 登录与基础模块
    // =========================================================================================
    qRegisterMetaType<ServerApi::LoginRsp>("ServerApi::LoginRsp");                  // 登录响应

    // =========================================================================================
    // 3. 远程控制模块
    // =========================================================================================
    qRegisterMetaType<ServerApi::ScreenFrame>("ServerApi::ScreenFrame");            // 屏幕帧数据
    qRegisterMetaType<ServerApi::ControlCmd>("ServerApi::ControlCmd");              // 控制指令
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