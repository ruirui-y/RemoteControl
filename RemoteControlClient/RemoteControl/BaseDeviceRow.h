#ifndef BASE_DEVICE_ROW_H
#define BASE_DEVICE_ROW_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include "Struct.h"

class QFrame;
class QToolButton;

class BaseDeviceRow : public QWidget
{
    Q_OBJECT

public:
    explicit BaseDeviceRow(const QString& id, QWidget* parent = nullptr);

    // 基础信息设置
    virtual void SetInfo(const DeviceInfo& info);

    // 在线状态
    void SetOnline(bool online);

    // 电量显示 (PvP 继承类也会用到)
    void SetBattery(int level);

    // 🔥 新增 Getter 方法，供容器层遍历查询
    QString GetDeviceId() const { return m_id; }
    QString GetDeviceName() const { return m_nameEdit ? m_nameEdit->text().trimmed() : ""; }
    bool IsSelected() const { return m_cb ? m_cb->isChecked() : false; }
    bool IsOnline() const { return m_isOnline; }

    virtual DeviceInfo GetDeviceInfo() const {
        DeviceInfo info;
        info.device_id = m_id;
        info.device_nick = GetDeviceName();
        return info;
    }

protected:
    void BuildUI();
    void BindSlots();
    void OnEditFinish();

protected:
    QString        m_id;                                                // 设备唯一ID
    QPixmap        m_baseIconPixmap;                                    // 图标原始缓存
    bool           m_isOnline = false;                                  // 在线状态

    QCheckBox* m_cb;                                                    // 选中框
    QFrame* m_lamp;                                                     // 在线状态灯
    QToolButton* m_deviceIcon;                                          // 设备图标按钮
    QLineEdit* m_nameEdit;                                              // 昵称编辑框
    QString m_originalName;                                             // 云服务器原始昵称
    QToolButton* m_removeBtn;                                           // 删除按钮

    QHBoxLayout* m_mainLayout;                                          // 布局管理器
};

#endif