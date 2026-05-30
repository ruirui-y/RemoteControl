#ifndef STRUCT_H
#define STRUCT_H

#include <QString>
#include <QHostAddress>
#include <QByteArray>

// TCP 持久化数据
struct DeviceInfo {
    QString device_id;                                                          // 设备唯一标识符
    QString device_nick;                                                        // 设备昵称
    QString team_name;                                                          // 设备所属分组名称
    QString play_time;                                                          // 设备可玩时段 (如 "09:00-18:00")
};

// 用于 UDP 瞬时状态更新的数据
struct DeviceStatus {
    QString id;
    int battery;
    bool online;
};

/* UDP信息 */
struct UDPMessage
{
public:
	uint16_t ID;						// 消息ID
	uint16_t MsgLen;
	QString Msg;						// 发送时使用
	QByteArray Data;					// 数据
	QHostAddress Addr;
	int Port;
};

/* 发送消息 */
struct UDPSendMessage : public UDPMessage
{

};

/* 接收消息 */
struct UDPRecvMessage : public UDPMessage
{

};

#endif