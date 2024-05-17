#include "privatechatwid.h"
#include "tcpclient.h"
#include "ui_privatechatwid.h"

#include <QMessageBox>
#include <QByteArray>
#include "protocol.h"

PrivateChatWid::PrivateChatWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChatWid)
{
    ui->setupUi(this);
}

PrivateChatWid::~PrivateChatWid()
{
    delete ui;
}

QString PrivateChatWid::strChatName() const
{
    return m_strChatName;
}

void PrivateChatWid::setStrChatName(const QString &strChatName)
{
    m_strChatName = strChatName;
}

void PrivateChatWid::on_sendMsg_pb_clicked()
{
    QString strSendMsg = ui -> inputMsg_le -> text();
    if(strSendMsg.isEmpty())
    {
        QMessageBox::warning(this, "私聊", "发送消息不能为空！");
        return ;
    }

    ui -> inputMsg_le -> clear();
    ui -> showMsg_te -> append(QString("%1: %2").arg(m_strLoginName).arg(strSendMsg));

    QByteArray msgArray = strSendMsg.toUtf8();
    PDU *pdu = mkPDU(msgArray.size() + 1);
    pdu -> uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
    strncpy(pdu -> caData, m_strChatName.toStdString().c_str(), 32);
    strncpy(pdu -> caData + 32, m_strLoginName.toStdString().c_str(), 32);
    strncpy((char*)pdu -> caMsg, msgArray.toStdString().c_str(), msgArray.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}

QString PrivateChatWid::strLoginName() const
{
    return m_strLoginName;
}

void PrivateChatWid::setStrLoginName(const QString &strLoginName)
{
    m_strLoginName = strLoginName;
}

void PrivateChatWid::updateShowMsgTE(PDU *pdu)
{
    if(NULL == pdu)
    {
        return ;
    }
    char caSendName[32] = {'\0'};
    strncpy(caSendName, pdu -> caData + 32, 32);
    QString strMsg = QString("%1: %2").arg(caSendName).arg((char*)pdu -> caMsg);
    ui -> showMsg_te -> append(strMsg);
}

void PrivateChatWid::setPriChatTitle(const char *caTitle)
{
    ui -> priChatName_l -> setText(caTitle);
}
