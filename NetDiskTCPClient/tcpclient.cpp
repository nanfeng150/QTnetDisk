#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <QRegularExpression>
#include <QIcon>
#include "protocol.h"
#include "uploadfileprogress.h"

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();

    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));

    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveMsg()));

    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);

    ui->regist_pb->setToolTip(tr("账户名为英文(5-20位),账户密码为数字(5-10位),请严格遵守规定!!!"));
    ui->logout_pb->setToolTip((tr("您确定注销该账号吗?")));

    m_fileOperate = new FileSystem;
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();

        strData.replace("\r\n", " ");
        QStringList strList = strData.split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
    }
    else
    {
        QMessageBox::critical(this, "open config", "open config failed"); // 严重错误
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    instance.setWindowTitle("网盘客户端");
    instance.setWindowIcon(QIcon(":/icon/fileServer.png"));
    return instance;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
    //qDebug() << "连接服务器成功!!!";
}

void TcpClient::receiveMsg()
{
    TransFile *transFile = OperateWidget::getInstance().getPFileSystem()->getDownloadFileInfo();
    if(transFile->bTransform)
    {
        uint uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        char* baBuffer = new char[uiMsgLen];
        m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
        memcpy(baBuffer, (char*)pdu -> caMsg, pdu -> uiMsgLen);//读取密文
        QByteArray encryptText(const_cast<char*>(baBuffer), uiMsgLen);
        int isFillByte = 0;
        int iRecvFileSize = 0;
        sscanf(pdu -> caData, "%d %d", &isFillByte, &iRecvFileSize);
        AES aes;
        QByteArray decryptText = aes.printfData(encryptText);//解密
        transFile->file.write(decryptText, decryptText.length());
        transFile -> iReceivedSize += iRecvFileSize;
        if(uploadFileProgress::getProgress().isHidden() || uploadFileProgress::getProgress().isMinimized())
            uploadFileProgress::getProgress().showNormal();

        uploadFileProgress::getProgress().setProgressValue(((qreal)transFile->iReceivedSize / (qreal)transFile->iTotalSize) * 100);

        if(transFile->iReceivedSize == transFile->iTotalSize)
        {
            QMessageBox::information(this, "下载文件", "下载文件成功！");
            transFile->file.close();
            transFile->file.setFileName("");
            transFile->bTransform = false;
            transFile->iTotalSize = 0;
            transFile->iReceivedSize = 0;
            uploadFileProgress::getProgress().hide();
            //处理空包,清理套接字缓冲区
            QByteArray emptyPackage =  m_tcpSocket.readAll();
            //qDebug() << "文件下载完成";
        }
        else if(transFile->iReceivedSize > transFile->iTotalSize)
        {
            QMessageBox::warning(this, "下载文件", "下载文件失败！");
            transFile->file.close();
            transFile->file.setFileName("");
            transFile->bTransform = false;
            transFile->iTotalSize = 0;
            transFile->iReceivedSize = 0;
        }
        free(pdu);
        pdu = nullptr;
        delete []baBuffer;
        baBuffer = nullptr;
        return ;
    }

    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));


    switch(pdu -> uiMsgType)
    {

    case ENUM_MSG_TYPE_REGIST_RESPOND:
    {
        if(0 == strcmp(pdu -> caData, REGIST_OK))
        {
            QMessageBox::information(this, "注册", REGIST_OK);
        }
        else if(0 == strcmp(pdu -> caData, REGIST_FAILED))
        {
            QMessageBox::warning(this, "注册", REGIST_FAILED);
        }
        break;
    }

    case ENUM_MSG_TYPE_LOGOUT_RESPOND:
    {
        if(0 == strcmp(pdu -> caData, LOGOUT_OK))
        {
            QMessageBox::information(this, "注销", LOGOUT_OK);
        }
        else if(0 == strcmp(pdu -> caData, LOGOUT_FAILED))
        {
            QMessageBox::warning(this, "注销", LOGOUT_FAILED);
        }
        break;
    }

    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(0 == strcmp(pdu -> caData, LOGIN_OK))
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu -> caData + 32, 32);
            m_strRootPath = QString((char*)pdu -> caMsg);
            m_strCurPath = m_strRootPath;
            setStrLoginName(QString(caName));

            OperateWidget::getInstance().setUserLabel(caName);
            OperateWidget::getInstance().show();

            OperateWidget::getInstance().getPFriend() -> flushFriendList();
            m_isLoginFlushFile = true;
            this -> hide();
        }
        else if(0 == strcmp(pdu -> caData, LOGIN_FAILED))
        {
            QMessageBox::warning(this, "登录", LOGIN_FAILED);
        }
        break;
    }

    case ENUM_MSG_TYPE_ONLINE_USERS_RESPOND:
    {
        OperateWidget::getInstance().getPFriend() -> setOnlineUsers(pdu);
        break;
    }

    case ENUM_MSG_TYPE_SEARCH_USER_RESPOND:
    {
        if(0 == strcmp(SEARCH_USER_OK, pdu -> caData))
        {
            QMessageBox::information(this, "查找", OperateWidget::getInstance().getPFriend()->getStrSearchName() + " " + SEARCH_USER_OK);
        }
        else if(0 == strcmp(SEARCH_USER_OFFLINE, pdu -> caData))
        {
            QMessageBox::information(this, "查找", OperateWidget::getInstance().getPFriend()->getStrSearchName() +  " " + SEARCH_USER_OFFLINE);
        }
        else if(0 == strcmp(SEARCH_USER_EMPTY, pdu -> caData))
        {
            QMessageBox::warning(this, "查找", OperateWidget::getInstance().getPFriend()->getStrSearchName() +  " " + SEARCH_USER_EMPTY);
        }
        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
    {
        QMessageBox::information(this, "添加好友", pdu -> caData);
        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char sourceName[32];
        strncpy(sourceName, pdu -> caData + 32, 32);
        int ret = QMessageBox::information(this, "好友申请", QString("%1 想添加您为好友，是否同意？").arg(sourceName),
                                           QMessageBox::Yes, QMessageBox::No);

        PDU* resPdu = mkPDU(0);

        strncpy(resPdu -> caData, pdu -> caData, 32);
        strncpy(resPdu -> caData + 32, pdu -> caData + 32, 32);
        if(ret == QMessageBox::Yes)
        {
            resPdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        }
        else
        {
            resPdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REJECT;
        }
        m_tcpSocket.write((char*)resPdu, resPdu -> uiPDULen);
        delete resPdu;
        resPdu = nullptr;
        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
    {
        QMessageBox::information(this, "添加好友", QString("%1 已同意您的好友申请！").arg(pdu -> caData));
        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_REJECT:
    {
        QMessageBox::information(this, "添加好友", QString("%1 已拒绝您的好友申请！").arg(pdu -> caData));
        break;
    }

    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
    {
        OperateWidget::getInstance().getPFriend()->updateFriendList(pdu);
        if(m_isLoginFlushFile){
            FileSystem::operateFileOrDirUpFileListWidget();
            m_isLoginFlushFile = false;
        }
        break;
    }

    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
    {
        QMessageBox::information(this, "删除好友", pdu -> caData);
        break;
    }

    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {
        char sourceName[32];
        strncpy(sourceName, pdu -> caData + 32, 32);
        QMessageBox::information(this, "删除好友", QString("%1 已删除与您的好友关系！").arg(sourceName));
        break;
    }

    case ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND:
    {
        if(strcmp(PRIVATE_CHAT_OFFLINE, pdu -> caData) == 0)
        {
            QMessageBox::information(this, "私聊", PRIVATE_CHAT_OFFLINE);
        }
        break;
    }

    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        char sourceName[32];
        strncpy(sourceName, pdu -> caData + 32, 32);
        PrivateChatWid *priChatW = OperateWidget::getInstance().getPFriend()->searchPriChatWid(sourceName);
        if(nullptr == priChatW)
        {
            priChatW = new PrivateChatWid;
            priChatW -> setStrChatName(sourceName);
            priChatW -> setStrLoginName(m_strLoginName);
            priChatW -> setPriChatTitle(sourceName);
            OperateWidget::getInstance().getPFriend()->insertPriChatWidList(priChatW);
        }
        priChatW->updateShowMsgTE(pdu);
        priChatW->show();
        if(priChatW->isMinimized())
        {
            priChatW->showNormal();
        }
        break;
    }

    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        OperateWidget::getInstance().getPFriend()->updateGroupShowMsgTE(pdu);
        break;
    }

    case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
    {
        QMessageBox::information(this, "创建文件夹", pdu -> caData);
        FileSystem::operateFileOrDirUpFileListWidget();
        break;
    }

    case ENUM_MSG_TYPE_FLUSH_DIR_RESPOND:
    {
        OperateWidget::getInstance().getPFileSystem()->updateFileList(pdu);
        break;
    }

    case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
    {
        QMessageBox::information(this, "删除文件", pdu -> caData);
        FileSystem::operateFileOrDirUpFileListWidget();
        break;
    }

    case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
    {
        QMessageBox::information(this, "重命名文件", pdu -> caData);
        FileSystem::operateFileOrDirUpFileListWidget();
        break;
    }

    case ENUM_MSG_TYPE_ENTRY_DIR_RESPOND:
    {
        if(strcmp(ENTRY_DIR_OK, pdu -> caData) == 0)
        {
            OperateWidget::getInstance().getPFileSystem() -> updateFileList(pdu);
            QString entryPath = OperateWidget::getInstance().getPFileSystem()->strTryEntryDir();
            if(!entryPath.isEmpty())
            {
                m_strCurPath = entryPath;
                OperateWidget::getInstance().getPFileSystem()->clearStrTryEntryDir();
            }
        }
        else
        {
            QMessageBox::warning(this, "进入文件夹", pdu -> caData);
        }
        break;
    }

    case ENUM_MSG_TYPE_PRE_DIR_RESPOND:
    {
        if(strcmp(PRE_DIR_OK, pdu -> caData) == 0)
        {
            OperateWidget::getInstance().getPFileSystem() -> updateFileList(pdu);
            QString entryPath = OperateWidget::getInstance().getPFileSystem()->strTryEntryDir();
            if(!entryPath.isEmpty())
            {
                m_strCurPath = entryPath;
                OperateWidget::getInstance().getPFileSystem()->clearStrTryEntryDir();
            }
        }
        else
        {
            QMessageBox::warning(this, "上一文件夹", pdu -> caData);
        }
        break;
    }

    case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
    {
        if(strcmp(UPLOAD_FILE_START, pdu -> caData) == 0)
        {
            OperateWidget::getInstance().getPFileSystem()->startTimer();
        }
        else if(strcmp(UPLOAD_FILE_OK, pdu -> caData) == 0)
        {
            QMessageBox::information(this, "上传文件", pdu -> caData);
            FileSystem::operateFileOrDirUpFileListWidget();
        }
        else if(strcmp(UPLOAD_FILE_FAILED, pdu -> caData) == 0)
        {
            QMessageBox::warning(this, "上传文件", pdu -> caData);
        }
        break;
    }

    case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
    {
        if(strcmp(DOWNLOAD_FILE_START, pdu -> caData) == 0)
        {
            TransFile *transFile = OperateWidget::getInstance().getPFileSystem()->getDownloadFileInfo();
            qint64 ifileSize = 0;
            char strFileName[32];
            sscanf((char*)pdu -> caMsg, "%s %lld", strFileName, &ifileSize);

            if(strlen(strFileName) > 0 && transFile->file.open(QIODevice::WriteOnly))
            {
                transFile->bTransform = true;
                transFile->iTotalSize = ifileSize;
                transFile->iReceivedSize = 0;
            }
            else
            {
                QMessageBox::warning(this, "下载文件", "下载文件失败！");
            }
        }
        else if(strcmp(DOWNLOAD_FILE_OK, pdu -> caData) == 0)
        {
            QMessageBox::information(this, "下载文件", pdu -> caData);
        }
        else if(strcmp(DOWNLOAD_FILE_FAILED, pdu -> caData) == 0)
        {
            QMessageBox::warning(this, "下载文件", pdu -> caData);
        }
        break;
    }

    case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
    {
        QMessageBox::information(this, "移动文件", pdu -> caData);
        FileSystem::operateFileOrDirUpFileListWidget();
        break;
    }

    case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: // 分享文件响应
    {
        QMessageBox::information(this, "分享文件", pdu -> caData);
        break;
    }

    case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
    {
        char caFileName[32];
        char caSouName[32];
        int iFilePathLen = pdu -> uiMsgLen;
        char caFilePath[iFilePathLen];

        memcpy(caSouName, pdu -> caData, 32);
        memcpy(caFileName, pdu -> caData + 32, 32);
        QString strShareNote = QString("%1想要分享%2文件给您，\n是否接收？").arg(caSouName).arg(caFileName);
        QMessageBox::StandardButton sbShareNote = QMessageBox::question(this, "分享文件", strShareNote);
        if(sbShareNote == QMessageBox::No)
        {
            break;
        }

        memcpy(caFilePath, (char*)pdu -> caMsg, iFilePathLen);
        QString strRootDir = m_strRootPath;
        PDU *resPdu = mkPDU(iFilePathLen + strRootDir.size() + 1);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
        sprintf(resPdu -> caData, "%d %d", iFilePathLen, strRootDir.size());
        sprintf((char*)resPdu -> caMsg, "%s %s", caFilePath, strRootDir.toStdString().c_str());
        qDebug() << (char*)resPdu -> caMsg;
        m_tcpSocket.write((char*)resPdu, resPdu -> uiPDULen);
        free(resPdu);
        resPdu = nullptr;
        break;
    }

    case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
    {
        FileSystem::operateFileOrDirUpFileListWidget();
        QMessageBox::information(this, "分享文件", pdu -> caData);
        break;
    }

    default:
        break;
    }

    free(pdu);
    pdu = nullptr;
}

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui -> name_le -> text();
    QString strPwd = ui -> pwd_le -> text();

    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu -> uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;

        memcpy(pdu -> caData, strName.toStdString().c_str(), 32);
        memcpy(pdu -> caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu -> uiPDULen);

        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::critical(this, "登录", "登录失败：用户名或密码为空！");
    }
}

void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();

    bool isCanRegist = false;
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        QRegularExpression name_expression(QString("^[a-zA-Z]{5,20}$"));
        if(!name_expression.match(strName).hasMatch()){
            QMessageBox::critical(this, "注册", "注册失败：用户名不合法！");
            return;
        }
        QRegularExpression pwd_expression(QString("^[0-9]{5,10}$"));
        if(!pwd_expression.match(strPwd).hasMatch()){
            QMessageBox::critical(this, "注册", "注册失败：密码不合法！");
            return;
        }
        isCanRegist = true;
    }
    if(isCanRegist)
    {
        PDU *pdu = mkPDU(0);
        pdu -> uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;

        memcpy(pdu -> caData, strName.toStdString().c_str(), 32);
        memcpy(pdu -> caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu -> uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else
    {
        QMessageBox::critical(this, "注册", "注册失败：用户名或密码为空！");
    }
}

void TcpClient::on_logout_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        PDU *pdu = mkPDU(0);
        pdu -> uiMsgType = ENUM_MSG_TYPE_LOGOUT_REQUEST;
        memcpy(pdu -> caData, strName.toStdString().c_str(), 32);
        memcpy(pdu -> caData + 32, strPwd.toStdString().c_str(), 32);

        m_tcpSocket.write((char*)pdu, pdu -> uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
    else{
        QMessageBox::critical(this, "注册", "注册失败：用户名或密码为空！");
    }

}

QString TcpClient::getStrRootPath() const
{
    return m_strRootPath;
}

void TcpClient::setStrRootPath(const QString &strRootPath)
{
    m_strRootPath = strRootPath;
}

QString TcpClient::getStrCurPath() const
{
    return m_strCurPath;
}

void TcpClient::setStrCurPath(const QString &strCurPath)
{
    m_strCurPath = strCurPath;
}

QString TcpClient::getStrLoginName() const
{
    return m_strLoginName;
}

void TcpClient::setStrLoginName(const QString &strName)
{
    m_strLoginName = strName;
}

QTcpSocket& TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}
