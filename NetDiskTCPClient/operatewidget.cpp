#include "operatewidget.h"
#include <QListWidgetItem>
#include <QIcon>
#include <QFont>
#include <QSize>
OperateWidget::OperateWidget(QWidget *parent) : QWidget(parent)
{
    m_pUserLabel = new QLabel;
    m_pListWidget = new QListWidget(this);

    QIcon icon_friend(":/icon/friend.png");
    QIcon icon_file(":/icon/file.png");
    QListWidgetItem *itemIcon[2] = {new QListWidgetItem("好友"), new QListWidgetItem("文件")};
    itemIcon[0]->setIcon(icon_friend);
    itemIcon[1]->setIcon(icon_file);
    m_pListWidget->addItem(itemIcon[0]);
    m_pListWidget->addItem(itemIcon[1]);
    m_pListWidget->setUniformItemSizes(true);

    m_pFriend = new Friend;
    m_pFileSystem = new FileSystem;

    m_pStackWidget = new QStackedWidget;
    m_pStackWidget -> addWidget(m_pFriend);
    m_pStackWidget -> addWidget(m_pFileSystem);

    QHBoxLayout *pBodyHBL = new QHBoxLayout;
    pBodyHBL -> addWidget(m_pListWidget);
    pBodyHBL -> addWidget(m_pStackWidget);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL -> addWidget(m_pUserLabel);
    pMainVBL -> addLayout(pBodyHBL);
    setLayout(pMainVBL);

    connect(m_pListWidget, SIGNAL(currentRowChanged(int)), m_pStackWidget, SLOT(setCurrentIndex(int)));
}

OperateWidget &OperateWidget::getInstance()
{
    static OperateWidget instance;
    instance.setWindowTitle("网盘客户端");
    instance.setWindowIcon(QIcon(":/icon/fileServer.png"));
    instance.setMinimumSize(QSize(800, 650));
    instance.setMaximumSize(QSize(1000, 800));
    return instance;
}

Friend *OperateWidget::getPFriend() const
{
    return m_pFriend;
}

FileSystem *OperateWidget::getPFileSystem() const
{
    return m_pFileSystem;
}


void OperateWidget::setUserLabel(const char *name)
{
    m_pUserLabel->setText(name);
}

