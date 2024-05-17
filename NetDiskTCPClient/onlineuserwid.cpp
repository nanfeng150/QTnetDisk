#include "onlineuserwid.h"
#include "ui_onlineuserwid.h"
#include <QDebug>
#include "tcpclient.h"
#include <QMessageBox>

OnlineUserWid::OnlineUserWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineUserWid)
{
    ui->setupUi(this);
}

OnlineUserWid::~OnlineUserWid()
{
    delete ui;
}

void OnlineUserWid::setOnlineUsers(PDU *pdu)
{
    if(NULL == pdu)
    {
        return ;
    }

    uint uiSize = pdu -> uiMsgLen / 32;
    char caTmp[32] = {'\0'};

    ui -> onlineuser_lw -> clear();
    for(uint i = 0; i < uiSize; ++ i)
    {
        memcpy(caTmp, (char*)(pdu -> caMsg) + 32 * i, 32);
        if(strcmp(caTmp, TcpClient::getInstance().getStrLoginName().toStdString().c_str()) == 0)
            continue;
        QListWidgetItem *item = new QListWidgetItem(QString(caTmp));
        item->setIcon(QIcon(":/icon/onlineUser.png"));
        ui -> onlineuser_lw -> addItem(item);
    }
}

void OnlineUserWid::on_addfriend_pb_clicked()
{
    QListWidgetItem *pItem = ui -> onlineuser_lw -> currentItem();
    if(nullptr == pItem)
    {
        QMessageBox::warning(this, "添加好友", "请选择要添加的好友！");
        return ;
    }

    QString strAddName = pItem->text();
    QString strLoginName = TcpClient::getInstance().getStrLoginName();
    PDU* pdu = mkPDU(0);

    pdu -> uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData, strAddName.toStdString().c_str(), strAddName.size());
    memcpy(pdu->caData + 32, strLoginName.toStdString().c_str(), strLoginName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu -> uiPDULen);
    free(pdu);
    pdu = nullptr;
}
