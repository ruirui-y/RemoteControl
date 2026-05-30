#include "GameItem.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QMouseEvent>
#include "Macro.h"

GameItem::GameItem(QString name, QString iconPath, QWidget* parent)
	: m_name(name), m_iconPath(iconPath), QToolButton(parent)
{
	BuildUI();
	setCursor(Qt::PointingHandCursor);
	setObjectName("GameItem");
}

GameItem::~GameItem()
{}

QSize GameItem::sizeHint() const
{
	return QSize(HZ_LIST_WIDTH, 60);
}

void GameItem::BuildUI()
{
	setText(m_name);
	setIcon(QIcon(m_iconPath));
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	setCheckable(true);
	setAutoExclusive(true);																		// 同父级目录下互斥选中
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}