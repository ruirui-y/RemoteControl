#include "UserMgr.h"
#include <QReadLocker>
#include <QWriteLocker>
#include "Macro.h"

UserMgr::UserMgr(QObject* parent) : QObject(parent)
{
    id_ = -1;
    user_device_counts_ = 0;
}

UserMgr::~UserMgr() {}

void UserMgr::SetUserName(const QString& user_name) {
    QWriteLocker locker(&rw_lock_); user_name_ = user_name;
}
void UserMgr::SetPassword(const QString& password) {
    QWriteLocker locker(&rw_lock_); password_ = password;
}
void UserMgr::SetToken(const QString& token) {
    QWriteLocker locker(&rw_lock_); token_ = token;
}
void UserMgr::SetId(const int32_t& id) {
    QWriteLocker locker(&rw_lock_); id_ = id;
}
void UserMgr::SetUserDeviceCounts(const int32_t& counts) {
    QWriteLocker locker(&rw_lock_); user_device_counts_ = counts;
}
void UserMgr::SetGameName(const QString& game_name) {
    QWriteLocker locker(&rw_lock_); game_name_ = game_name;
}

void UserMgr::SetUserInfo(const QString& user_name, const QString& password, const QString& token,
    const int32_t& id, const int32_t& user_device_counts, const QString& game_name)
{
    QWriteLocker locker(&rw_lock_);
    user_name_ = user_name;
    password_ = password;
    token_ = token;
    id_ = id;
    user_device_counts_ = user_device_counts;
    game_name_ = game_name;
}

QString UserMgr::GetUserName() const {
    QReadLocker locker(&rw_lock_); return user_name_;
}
QString UserMgr::GetPassword() const {
    QReadLocker locker(&rw_lock_); return password_;
}
QString UserMgr::GetToken() const {
    QReadLocker locker(&rw_lock_); return token_;
}
int32_t UserMgr::GetId() const {
    QReadLocker locker(&rw_lock_); return id_;
}
int32_t UserMgr::GetUserDeviceCounts() const {
    QReadLocker locker(&rw_lock_); return user_device_counts_;
}
QString UserMgr::GetGameName() const {
    QReadLocker locker(&rw_lock_); return game_name_;
}

bool UserMgr::GetIsMiNiWorldGame() const
{
    QReadLocker locker(&rw_lock_);
    if (game_name_ == MINI_WORLD_GAME_NAME) {
        return true;
    }
    return false;
}

void UserMgr::ClearUser()
{
    QWriteLocker locker(&rw_lock_);
    user_name_.clear();
    password_.clear();
    token_.clear();
    game_name_.clear();
    id_ = -1;
    user_device_counts_ = 0;
}