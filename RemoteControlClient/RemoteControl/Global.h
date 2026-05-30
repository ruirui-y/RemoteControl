#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWidget>
#include <functional>
#include <QRegularExpression>
#include <QStyle>
#include <memory>
#include <iostream>
#include <mutex>
#include <QString>
#include <QSettings>
#include <vector>

using namespace std;

// ========================================================
// 1. 核心工具与函数接口
// ========================================================
extern const int tipOffset;                                                 // 提示框的全局像素偏移量
extern function<void(QWidget*)> repolish;                                   // 刷新 QSS 样式的闭包函数
extern function<QString(QString)> xorString;                                // 基础字符串异或加密函数
extern QString GetLanguageFilePath(const QString& lang_code);               // 动态获取对应语言 .qm 文件的绝对路径

void InitGlobalPaths();                                                     // 初始化所有全局路径 (必须在 QApplication 之后调用)


// ========================================================
// 2. [系统配置] 路径与句柄
// ========================================================
extern QString ClientConfigPath;                                            // [文件] 客户端 config.json 绝对路径
extern QString LoginConfigPath;                                             // [文件] 登录凭证 login.json 绝对路径
extern QString AppConfigPath;                                               // [文件] UI偏好设置 app_settings.json 绝对路径
extern QString AndroidConfigPath;                                           // [目录] Android配置文件路径
extern QString GameSettingPath;                                             // [文件] 游戏包体配置 game_setting.json 绝对路径

// ========================================================
// 3. [国际化翻译] 路径
// ========================================================
extern QString TranslationsPath;                                            // [目录] 多语言 .qm 文件存放目录


// ========================================================
// 4. [应用程序与游戏 APK] 路径
// ========================================================
extern QString ClientApkPath;                                               // [文件] 客户端apk路径
extern QString MiNiWorldApkPath;                                            // [文件] 迷你世界apk路径
extern QString PvPApkPath;                                                  // [文件] PVP apk路径


// ========================================================
// 5. [游戏配置与脚本] 路径
// ========================================================
extern QString MiNiWorldConfigPath;                                         // [文件] 迷你世界配置文件路径
extern QString PvPConfigPath;                                               // [文件] PVP配置文件路径

extern QString MiNiWorldBatPath;                                            // [文件] 迷你世界启动脚本路径
extern QString PvPBatPath;                                                  // [文件] PVP启动脚本路径


// ========================================================
// 6. RAII 辅助工具类 (作用域结束自动执行)
// ========================================================
using DeferFunc = std::function<void()>;
class Defer
{
public:
    Defer(DeferFunc func) : m_func(func) {}
    ~Defer() { m_func(); }
private:
    DeferFunc m_func;
};

#endif // GLOBAL_H