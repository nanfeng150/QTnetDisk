#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "protocol.h"
#include "onlineuserwid.h"
#include "privatechatwid.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);

    QString getStrSearchName() const;
    void setStrSearchName(const QString &strSearchName);

    void setOnlineUsers(PDU* pdu);
    void updateFriendList(PDU *pdu);

    QListWidget *getPFriendLW() const;

    static Friend& returnFriendObj();//用于刷新

public slots:
    void showOrHideOnlineUserW();
    void searchUser();
    void flushFriendList();
    void deleteFriend();
    void privateChat();
    PrivateChatWid* searchPriChatWid(const char* chatName);
    void insertPriChatWidList(PrivateChatWid* priChat);
    void groupChatSendMsg();
    void updateGroupShowMsgTE(PDU* pdu);

signals:

private:
    QListWidget *m_pFriendLW = nullptr;
    QPushButton *m_pDelFriendPB = nullptr;
    QPushButton *m_pFlushFriendPB = nullptr;
    QPushButton *m_pSOrHOnlineUserPB = nullptr;
    QPushButton *m_pSearchUserPB = nullptr;
    QLineEdit *m_pGroupInputLE = nullptr;
    QPushButton *m_pGroupSendMsgPB = nullptr;
    QTextEdit *m_pGroupShowMsgTE = nullptr;
    QPushButton *m_pPrivateChatPB = nullptr;

    OnlineUserWid *m_pOnlineUserW = nullptr;
    QString m_strSearchName;

    QList<PrivateChatWid*> m_priChatWidList;
};

#endif // FRIEND_H
