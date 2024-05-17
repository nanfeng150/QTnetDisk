#include "sharedfilefriendlist.h"
#include <QDebug>
#include <QIcon>
#include "operatewidget.h"
#include "tcpclient.h"

sharedFileFriendList::sharedFileFriendList(QWidget *parent) : QWidget(parent)
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancleSelectPB = new QPushButton("清空");
    m_pSelectAllPB -> setIcon(QIcon(":/icon/selectAll.png"));
    m_pCancleSelectPB -> setIcon(QIcon(":/icon/cancleSelect.png"));

    m_pAffirmPB = new QPushButton("确认");
    m_pCanclePB = new QPushButton("取消");
    m_pAffirmPB -> setIcon(QIcon(":/icon/ok.png"));
    m_pCanclePB -> setIcon(QIcon(":/icon/cancle.png"));

    m_pFriendsSA = new QScrollArea; // 展示好友区
    m_pFriendsWid = new QWidget; // 所有好友窗口
    m_pFriendsVBL = new QVBoxLayout(m_pFriendsWid);
    m_pFriendsBG = new QButtonGroup(m_pFriendsWid);
    m_pFriendsBG->setExclusive(false);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addStretch();
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancleSelectPB);

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pAffirmPB);
    pDownHBL->addWidget(m_pCanclePB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pFriendsSA);
    pMainVBL->addLayout(pDownHBL);

    setLayout(pMainVBL);

    connect(m_pSelectAllPB, SIGNAL(clicked(bool)),
            this, SLOT(selectAll()));
    connect(m_pCancleSelectPB, SIGNAL(clicked(bool)),
            this, SLOT(cancleSelect()));
    connect(m_pAffirmPB, SIGNAL(clicked(bool)),
            this, SLOT(affirmShare()));
    connect(m_pCanclePB, SIGNAL(clicked(bool)),
            this, SLOT(cancleShare()));
}

void sharedFileFriendList::updateFriendList(QListWidget *pFriendList)
{
    if(nullptr == pFriendList)
    {
        return;
    }

    QList<QAbstractButton*> preFriendList = m_pFriendsBG->buttons();
    for(QAbstractButton* pItem : preFriendList)
    {
        m_pFriendsVBL->removeWidget(pItem);
        m_pFriendsBG->removeButton(pItem);
        delete pItem;
        pItem = nullptr;
    }

    QCheckBox *pCB = nullptr;
    for(int i = 0; i < pFriendList->count(); i ++)
    {

        if(pFriendList->item(i)->text().endsWith("离线"))
            continue;
        QString shareFileToFriendName = pFriendList->item(i)->text().split('\t')[0];
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendsVBL->addWidget(pCB);
        m_pFriendsBG->addButton(pCB);
    }
    m_pFriendsSA->setWidget(m_pFriendsWid);
}

void sharedFileFriendList::selectAll()
{
    QList<QAbstractButton*> friendsButtons = m_pFriendsBG->buttons();
    for(QAbstractButton* pItem : friendsButtons)
        pItem->setChecked(true);
}

void sharedFileFriendList::cancleSelect()
{
    QList<QAbstractButton*> friendsButtons = m_pFriendsBG->buttons();
    for(QAbstractButton* pItem : friendsButtons)
        pItem->setChecked(false);
}

void sharedFileFriendList::affirmShare()
{
    QByteArray strFileName = OperateWidget::getInstance().getPFileSystem()->getStrSharedFileName().toUtf8();
    QByteArray strFilePath = OperateWidget::getInstance().getPFileSystem()->getStrSharedFilePath().toUtf8();

    QList<QAbstractButton*> abList = m_pFriendsBG->buttons();
    QList<QString> userList;

    for(int i = 0; i < abList.count(); ++ i)
    {
        if(abList[i]->isChecked())
        {
            userList.append(abList[i]->text().split('\t')[0]);
        }
    }

    int iUserNum = userList.count();
    PDU* pdu = mkPDU(strFilePath.size() + (iUserNum * 32) + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu -> caData, "%s %d", strFileName.toStdString().c_str(), iUserNum);
    for(int i = 0; i < iUserNum; ++ i)
    {
        strncpy((char*)(pdu -> caMsg) + 32 * i, userList[i].toStdString().c_str(), userList[i].size());
        qDebug() << QString((char*)(pdu -> caMsg) + 32 * i);
    }
    memcpy((char*)(pdu -> caMsg) + 32 * iUserNum, strFilePath.toStdString().c_str(), strFilePath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
    this -> hide();
}

void sharedFileFriendList::cancleShare()
{
    this -> hide();
}
