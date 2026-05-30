#ifndef ENUM_H
#define ENUM_H

enum ReqID_VR
{
    ID_VR_DEVICE_ID_REQUEST                 = 600,              // VR设备ID请求
    ID_VR_DEVICE_ID_RESPONSE                = 601,              // VR设备ID响应
    ID_VR_START_GAME_REQUEST                = 602,              // VR游戏启动请求
    ID_VR_START_GAME_RESPONSE               = 603,              // VR游戏启动响应
    ID_VR_STOP_GAME_REQUEST                 = 604,              // VR游戏停止请求
    ID_VR_STOP_GAME_RESPONSE                = 605,              // VR游戏停止响应
    ID_HEART_BEAT_REQUEST                   = 606,			    // 心跳请求
    ID_HEART_BEAT_RESPONSE                  = 607,			    // 心跳回复
    ID_VR_BROCAST_MSG_REQUEST               = 608,			    // VR广播消息请求
    ID_VR_BROCAST_MSG_RESPONSE              = 609,			    // VR广播消息回复
    ID_VR_MODIFY_USER_NICK_REQUEST          = 610,              // VR修改昵称请求
    ID_VR_MODIFY_USER_NICK_RESPONSE         = 611,              // VR修改昵称响应
    ID_VR_MODIFY_USER_TEAM_REQUEST          = 612,              // VR修改队伍请求
    ID_VR_MODIFY_USER_TEAM_RESPONSE         = 613,              // VR修改队伍响应
    ID_VR_MODIFY_USER_PLAY_TIME_REQUEST     = 614,              // VR修改游戏时间请求
    ID_VR_MODIFY_USER_PLAY_TIME_RESPONSE    = 615,              // VR修改游戏时间响应
};

enum ErrorCodes
{
    SUCCESS                                 = 0,
    LOGIN_USER_EXIT_ERR                     = 1,                // 登录失败
    LOGIN_PWD_ERR                           = 2,                // 密码错误
    LOGIN_USER_NOT_EXIST_ERR                = 3,                // 用户不存在
    JSON_ERR                                = 4,                // JSON解析失败
    REMOVE_DEVICE_ERR                       = 5,                // 删除设备失败
    ADD_DEVICE_ERR                          = 6,                // 添加设备失败
};

#endif // ENUM_H