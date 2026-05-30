#ifndef USERMGR_H
#define USERMGR_H

#include <QObject>
#include <QString>
#include <QReadWriteLock>
#include "singletion.h" 

class UserMgr : public QObject, public Singleton<UserMgr>
{
    Q_OBJECT
        friend class Singleton<UserMgr>;

public:
    ~UserMgr();

    // --- 线程安全的 Setter ---
    void SetUserName(const QString& user_name);
    void SetPassword(const QString& password);
    void SetToken(const QString& token);
    void SetId(const int32_t& id);

    // 👑 核心业务数据 Setter
    void SetUserDeviceCounts(const int32_t& counts);
    void SetGameName(const QString& game_name);

    // 一次性设置所有信息
    void SetUserInfo(const QString& user_name, const QString& password,
        const QString& token, const int32_t& id,
        const int32_t& user_device_counts, const QString& game_name);

    // --- 线程安全的 Getter ---
    QString GetUserName() const;
    QString GetPassword() const;
    QString GetToken() const;
    int32_t GetId() const;

    // 👑 核心业务数据 Getter
    int32_t GetUserDeviceCounts() const;
    QString GetGameName() const;
    bool GetIsMiNiWorldGame() const;

    // 清空用户信息
    void ClearUser();

private:
    UserMgr(QObject* parent = nullptr);

private:
    QString user_name_;
    QString password_;
    QString token_;
    int32_t id_;

    int32_t user_device_counts_;      // 用户设备数量
    QString game_name_;               // 游戏名称 (用于区分打包和渲染逻辑)

    // 读写锁
    mutable QReadWriteLock rw_lock_;
};

#endif // USERMGR_H