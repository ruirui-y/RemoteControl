#include "Global.h"
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QProcess>

// ========================================================
// 1. 常量与不依赖 QApplication 的全局函数
// ========================================================
const int tipOffset = 5;

// 刷新样式的闭包函数
function<void(QWidget*)> repolish = [](QWidget* Widget)
    {
        Widget->style()->unpolish(Widget);
        Widget->style()->polish(Widget);
    };

/* 密码加密 */
function<QString(QString)> xorString = [](QString input)
    {
        QString result = input;
        int length = input.length();
        length %= 255;
        for (int i = 0; i < length; ++i)
        {
            result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ static_cast<ushort>(length)));
        }
        return result;
    };


// ========================================================
// 2. 全局变量定义 (分配内存，初始置空)
// ========================================================
QString ClientConfigPath;
QString LoginConfigPath;
QString AppConfigPath;
QString AndroidConfigPath;
QString GameSettingPath;

QString TranslationsPath;

QString ClientApkPath;
QString MiNiWorldApkPath;
QString PvPApkPath;

QString MiNiWorldConfigPath;
QString PvPConfigPath;

QString MiNiWorldBatPath;
QString PvPBatPath;


// 获取语言文件路径 (此函数在运行时调用，所以可以放在这里)
QString GetLanguageFilePath(const QString& lang_code)
{
    return QDir(TranslationsPath).filePath(QString("DemandStation_%1.qm").arg(lang_code));
}


// ========================================================
// 3. 👑 核心封装：全局路径初始化函数
// ========================================================
void InitGlobalPaths()
{
    // 获取当前工作路径和可执行文件 (.exe) 所在的绝对路径
    QString currentDir = QDir::currentPath();
    QString appDir = currentDir;
    // QString appDir = QCoreApplication::applicationDirPath();

    // --------------------------------------------------------
    // [JSON 系统配置模块]
    // --------------------------------------------------------
    ClientConfigPath = QDir(appDir).filePath("ClientInstall/Configs/config.json");
    LoginConfigPath = QDir(appDir).filePath("ClientInstall/Configs/login.json");
    AppConfigPath = QDir(appDir).filePath("ClientInstall/Configs/app_settings.json");
    GameSettingPath = QDir(appDir).filePath("ClientInstall/Configs/game_setting.json");
    AndroidConfigPath = "/sdcard/Documents/config/";

    // --------------------------------------------------------
    // [多语言翻译模块]
    // --------------------------------------------------------
    TranslationsPath = QDir(appDir).filePath("ClientInstall/Translations");

    // --------------------------------------------------------
    // [应用程序与游戏 APK] 路径
    // --------------------------------------------------------
    ClientApkPath = QDir(appDir).filePath("ClientInstall/APKS/DemandClinet.apk");
    MiNiWorldApkPath = QDir(appDir).filePath("ClientInstall/APKS/MiNiWorld.apk");
    PvPApkPath = QDir(appDir).filePath("ClientInstall/APKS/PvP_VR.apk");

    // --------------------------------------------------------
    // [游戏配置与脚本] 路径
    // --------------------------------------------------------
    MiNiWorldConfigPath = QDir(appDir).filePath("Pack/MiNiWorld/Windows/MiNiWorld/Configs/config.json");
    PvPConfigPath = QDir(appDir).filePath("Pack/PvP/Windows/PvP_VR/Configs/config.json");

    MiNiWorldBatPath = currentDir + "/Pack/MiNiWorld/Windows/Start.bat";
    PvPBatPath = currentDir + "/Pack/PvP/Windows/Start.bat";

    qDebug() << u8"[Global] 🌍 全局环境与路径字典初始化完毕！引擎挂载点:" << appDir;
}