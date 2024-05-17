#include "friend.h"

#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QIcon>
#include <QByteArray>
#include "privatechatwid.h"


Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pFriendLW = new QListWidget;
    m_pGroupInputLE = new QLineEdit;
    m_pGroupShowMsgTE = new QTextEdit;
    m_pDelFriendPB = new QPushButton("删除好友");
    m_pDelFriendPB->setIcon(QIcon(":/icon/deleteFriend .png"));
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pFlushFriendPB->setIcon(QIcon(":/icon/updateFriend .png"));
    m_pSOrHOnlineUserPB = new QPushButton("在线用户");
    m_pSOrHOnlineUserPB->setIcon(QIcon(":/icon/onlineFriend.png"));
    m_pSearchUserPB = new QPushButton("查找用户");
    m_pSearchUserPB->setIcon(QIcon(":/icon/findUser.png"));
    m_pGroupSendMsgPB = new QPushButton("发送");
    m_pGroupSendMsgPB->setIcon(QIcon(":/icon/sendMsg.png"));
    m_pPrivateChatPB = new QPushButton("好友私聊");
    m_pPrivateChatPB->setIcon(QIcon(":/icon/talkFriend.png"));

    m_pOnlineUserW = new OnlineUserWid;
    m_pOnlineUserW->setWindowIcon(QIcon(":/icon/fileServer.png"));
    m_pOnlineUserW->setWindowTitle("onlineUser");

    QVBoxLayout *pLeftRightVBL = new QVBoxLayout;
    pLeftRightVBL -> addWidget(m_pPrivateChatPB);
    pLeftRightVBL -> addWidget(m_pDelFriendPB);
    pLeftRightVBL -> addWidget(m_pFlushFriendPB);
    pLeftRightVBL -> addWidget(m_pSOrHOnlineUserPB);
    pLeftRightVBL -> addWidget(m_pSearchUserPB);
    QHBoxLayout *pRightDownHBL = new QHBoxLayout;
    pRightDownHBL -> addWidget(m_pGroupInputLE);
    pRightDownHBL -> addWidget(m_pGroupSendMsgPB);
    QVBoxLayout *pRightVBL = new QVBoxLayout;
    pRightVBL -> addWidget(m_pGroupShowMsgTE);
    pRightVBL -> addLayout(pRightDownHBL);
    QHBoxLayout *pMainHBL = new QHBoxLayout;
    pMainHBL -> addWidget(m_pFriendLW);
    pMainHBL -> addLayout(pLeftRightVBL);
    pMainHBL -> addLayout(pRightVBL);

    m_pOnlineUserW -> hide();

    setLayout(pMainHBL);

    connect(m_pSOrHOnlineUserPB, SIGNAL(clicked(bool)), this, SLOT(showOrHideOnlineUserW()));

    connect(m_pSearchUserPB, SIGNAL(clicked(bool)), this, SLOT(searchUser()));

    connect(m_pFlushFriendPB, SIGNAL(clicked(bool)), this, SLOT(flushFriendList()));

    connect(m_pDelFriendPB, SIGNAL(clicked(bool)), this, SLOT(deleteFriend()));

    connect(m_pPrivateChatPB, SIGNAL(clicked(bool)),this, SLOT(privateChat()));

    connect(m_pGroupSendMsgPB, SIGNAL(clicked(bool)), this, SLOT(groupChatSendMsg()));
}

void Friend::setOnlineUsers(PDU* pdu)
{
    if(nullptr == pdu)
    {
        return ;
    }
    m_pOnlineUserW->setOnlineUsers(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(nullptr == pdu)
    {
        return ;
    }
    uint uiSize = pdu -> uiMsgLen / 36;
    char caName[32] = {'\0'};
    char caOnline[4] = {'\0'};

    m_pFriendLW -> clear();
    for(uint i = 0; i < uiSize; ++ i)
    {
        memcpy(caName, (char*)(pdu -> caMsg) + i * 36, 32);
        memcpy(caOnline, (char*)(pdu -> caMsg) + 32 + i * 36, 4);

        QListWidgetItem *item = new QListWidgetItem(QString("%1\t%2").arg(caName).arg(strcmp(caOnline, "1") == 0?"在线":"离线"));
        item->setIcon(QIcon(":/icon/onlineUser.png"));
        m_pFriendLW -> addItem(item);
    }
}

void Friend::showOrHideOnlineUserW()
{
    if(m_pOnlineUserW -> isHidden())
    {
        m_pOnlineUserW -> show();
        PDU *pdu = mkPDU(0);
        pdu -> uiMsgType = ENUM_MSG_TYPE_ONLINE_USERS_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        m_pOnlineUserW -> hide();
    }
}

void Friend::searchUser()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名：");
    if(!m_strSearchName.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu -> uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
        memcpy((char*)pdu -> caData, m_strSearchName.toStdString().c_str(), m_strSearchName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Friend::flushFriendList()
{
    QString strName = TcpClient::getInstance().getStrLoginName();

    PDU* pdu = mkPDU(0);
    pdu -> uiMsgType = ENUM_MSG_TYPE_FLSUH_FRIEND_REQUEST;
    strncpy(pdu -> caData, strName.toStdString().c_str(), strName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::deleteFriend()
{
    if(nullptr == m_pFriendLW -> currentItem())
    {
        QMessageBox::information(this, "删除好友", "error no check friend!!!");
        return ;
    }
    QString checkUserName = m_pFriendLW -> currentItem() -> text();
    checkUserName = checkUserName.split("\t")[0];
    QString loginName = TcpClient::getInstance().getStrLoginName();

    qDebug() << checkUserName;
    PDU* pdu = mkPDU(0);
    pdu -> uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    strncpy(pdu -> caData, checkUserName.toStdString().c_str(), 32);
    strncpy(pdu -> caData + 32, loginName.toStdString().c_str(), 32);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::privateChat()
{
    if(nullptr == m_pFriendLW -> currentItem())
    {
        QMessageBox::warning(this, "私聊", "没有选中私聊对象");
        return ;
    }
    QString friName = m_pFriendLW -> currentItem() -> text();
    friName = friName.split("\t")[0];
    QString loginName = TcpClient::getInstance().getStrLoginName();

    PrivateChatWid *priChat = searchPriChatWid(friName.toStdString().c_str());

    if(priChat == nullptr)
    {
        priChat = new PrivateChatWid;
        priChat -> setStrChatName(friName);
        priChat -> setStrLoginName(loginName);
        priChat -> setPriChatTitle(friName.toStdString().c_str());
        priChat -> setWindowIcon(QIcon(":/icon/fileServer.png"));
        m_priChatWidList.append(priChat);
    }
    if(priChat->isHidden())
    {
        priChat->show();
    }
    if(priChat -> isMinimized())
    {
        priChat->showNormal();
    }
}

PrivateChatWid *Friend::searchPriChatWid(const char *chatName)
{
    for (PrivateChatWid *ptr:m_priChatWidList)
    {
        if(ptr->strChatName() == chatName)
        {
            return ptr;
        }
    }
    return nullptr;
}

void Friend::insertPriChatWidList(PrivateChatWid *priChat)
{
    m_priChatWidList.append(priChat);
}

void Friend::groupChatSendMsg()
{
    QString strMsg = m_pGroupInputLE -> text();
    if(strMsg.isEmpty())
    {
       QMessageBox::warning(this, "群聊", "发送信息不能为空！");
       return ;
    }
    m_pGroupInputLE->clear();
    m_pGroupShowMsgTE->append(QString("%1 : %2").arg(TcpClient::getInstance().getStrLoginName()).arg(strMsg));

    QByteArray  sendGroupMsg = strMsg.toUtf8();

    PDU* pdu = mkPDU(sendGroupMsg.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
    strncpy(pdu -> caData, TcpClient::getInstance().getStrLoginName().toStdString().c_str(), 32);
    strncpy((char*)(pdu -> caMsg), sendGroupMsg.toStdString().c_str(), sendGroupMsg.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::updateGroupShowMsgTE(PDU *pdu)
{
    QString strMsg = QString("%1 : %2").arg(pdu->caData).arg((char*)pdu->caMsg);
    m_pGroupShowMsgTE -> append(strMsg);
}

QListWidget *Friend::getPFriendLW() const
{
    return m_pFriendLW;
}

Friend &Friend::returnFriendObj()
{
    static Friend instance;
    return instance;
}

QString Friend::getStrSearchName() const
{
    return m_strSearchName;
}

void Friend::setStrSearchName(const QString &strSearchName)
{
    m_strSearchName = strSearchName;
}
