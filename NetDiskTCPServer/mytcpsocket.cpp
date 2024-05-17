#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QDebug>
QByteArray stringToBytearray(QString toString);
MyTcpSocket::MyTcpSocket()
{
    m_uploadFile = new TransFile;
    m_uploadFile->bTransform = false;
    m_pDownloadFile = new QFile;
    m_pTimer = new QTimer;

    connect(this, SIGNAL(readyRead()), this, SLOT(receiveMsg()));
    connect(this, SIGNAL(disconnected()), this, SLOT(handleClientOffline()));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(handledownloadFileData()));

    /*fileThread* downloadFileDataThread = new fileThread;
    QThread *downloadFileThread = new QThread;
    downloadFileDataThread -> moveToThread(downloadFileThread);
    downloadFileThread -> start();

    connect(this, &MyTcpSocket::startDownloadFile, this, &MyTcpSocket::handledownloadFileData);
    connect(downloadFileDataThread, &fileThread::downloadFileOver, this, [&](MyTcpSocket* socket){
        this -> setSocketDescriptor(MyTcpServer::getInstance().getSocketMap()[socket]);
    });*/
}

MyTcpSocket &MyTcpSocket::getSocketInstance()
{
    static MyTcpSocket tcpSocketObject;
    return tcpSocketObject;
}

void MyTcpSocket::setStrName(QString loginName)
{
    m_strName = loginName;
}

QString MyTcpSocket::getStrName()
{
    return m_strName;
}

PDU* handleRegistRequest(PDU* pdu)
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};
    strncpy(caName, pdu -> caData, 32);
    strncpy(caPwd, pdu -> caData + 32, 32);
    bool insertrDataRet = DBOperate::getInstance().handleRegist(caName, caPwd);

    PDU *resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
    if(insertrDataRet)
    {
        strcpy(resPdu -> caData, REGIST_OK);

        QDir dir;
        dir.mkdir(QString("%1/%2").arg(MyTcpServer::getInstance().getStrRootPath()).arg(caName));
    }
    else strcpy(resPdu -> caData, REGIST_FAILED);
    return resPdu;
}

PDU* handleLogoutRequest(PDU* pdu){
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};
    strncpy(caName, pdu -> caData, 32);
    strncpy(caPwd, pdu -> caData + 32, 32);
    bool deleteDataRet = DBOperate::getInstance().handleLogout(caName, caPwd);
    PDU *resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_LOGOUT_RESPOND;
    if(deleteDataRet)
    {
        strcpy(resPdu -> caData, LOGOUT_OK);

        QString userFilePath = MyTcpServer::getInstance().getStrRootPath() + "/" + QString(caName);
        QDir deleteDir;
        if(!deleteDir.exists(userFilePath)){
            //qDebug() << "路径不存在!!!";
        }
        else{
            QFileInfo fileInfo(userFilePath);
            if(fileInfo.isDir()){
                deleteDir.setPath(userFilePath);
                deleteDir.removeRecursively();
            }
        }
    }
    if(!deleteDataRet)
    {
        strcpy(resPdu -> caData, LOGOUT_FAILED);
    }
    return resPdu;
}

PDU* handleLoginRequest(PDU* pdu, QString& m_strName)
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};

    strncpy(caName, pdu -> caData, 32);
    strncpy(caPwd, pdu -> caData + 32, 32);
    bool ret = DBOperate::getInstance().handleLogin(caName, caPwd);

    PDU *resPdu = nullptr;
    if(ret)
    {
        QString strUserRootPath = QString("%1/%2")
                .arg(MyTcpServer::getInstance().getStrRootPath()).arg(caName);
        resPdu = mkPDU(strUserRootPath.size() + 1);
        memcpy(resPdu -> caData, LOGIN_OK, 32);
        memcpy(resPdu -> caData + 32, caName, 32);

        m_strName = caName;
        MyTcpSocket::getSocketInstance().setStrName(m_strName);

        strncpy((char*)resPdu -> caMsg, strUserRootPath.toStdString().c_str(), strUserRootPath.size() + 1);
    }
    else
    {
        resPdu = mkPDU(0);
        strcpy(resPdu -> caData, LOGIN_FAILED);
    }
    resPdu -> uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
    return resPdu;
}

PDU* handleOnlineUsersRequest()
{
    QStringList strList = DBOperate::getInstance().handleOnlineUsers();
    uint uiMsgLen = strList.size() * 32;

    PDU *resPdu = mkPDU(uiMsgLen);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_ONLINE_USERS_RESPOND;
    for(int i = 0; i < strList.size(); ++ i)
    {
        memcpy((char*)(resPdu -> caMsg) + 32 * i, strList[i].toStdString().c_str(), strList[i].size());
    }

    return resPdu;
}

PDU* handleSearchUserRequest(PDU* pdu)
{
    char caName[32] = {'\0'};
    strncpy(caName, pdu -> caData, 32);
    int ret = DBOperate::getInstance().handleSearchUser(caName);

    PDU *resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
    if(ret == 1)
    {
        strcpy(resPdu -> caData, SEARCH_USER_OK);
    }
    else if(ret == 0)
    {
        strcpy(resPdu -> caData, SEARCH_USER_OFFLINE);
    }
    else if(ret == 2)
    {
        strcpy(resPdu -> caData, SEARCH_USER_EMPTY);
    }

    return resPdu;
}

PDU* handleAddFriendRequest(PDU* pdu)
{
    char addedName[32] = {'\0'};
    char sourceName[32] = {'\0'};

    strncpy(addedName, pdu -> caData, 32);
    strncpy(sourceName, pdu -> caData + 32, 32);

    int iSearchUserStatus = DBOperate::getInstance().handleAddFriend(addedName, sourceName);

    PDU* resPdu = nullptr;

    switch (iSearchUserStatus) {
    case FRIEND_INDWELL_OFFLINE:
    {
        resPdu = mkPDU(0);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strcpy(resPdu -> caData, ADD_FRIEND_OFFLINE);
        break;
    }
    case FRIEND_INDWELL_ONLINE:
    {
        MyTcpServer::getInstance().forwardMsg(addedName, pdu);

        resPdu = mkPDU(0);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strcpy(resPdu -> caData, ADD_FRIEND_OK);
        break;
    }
    case FRIEND_NO_INDWELL:
    {
        resPdu = mkPDU(0);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strcpy(resPdu -> caData, ADD_FRIEND_EMPTY);
        break;
    }
    case FRIEND_IS_FRIEND:
    {
        resPdu = mkPDU(0);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strcpy(resPdu -> caData, ADD_FRIEND_EXIST);
        break;
    }
    case FRIEND_REQUEST_ERROR:
    {
        resPdu = mkPDU(0);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        strcpy(resPdu -> caData, UNKNOWN_ERROR);
        break;
    }
    default:
        break;
    }

    return resPdu;
}

void handleAddFriendAgree(PDU* pdu)
{
    char addedName[32] = {'\0'};
    char sourceName[32] = {'\0'};

    strncpy(addedName, pdu -> caData, 32);
    strncpy(sourceName, pdu -> caData + 32, 32);

    DBOperate::getInstance().handleAddFriendAgree(addedName, sourceName);

    MyTcpServer::getInstance().forwardMsg(sourceName, pdu);
}

void handleAddFriendReject(PDU* pdu)
{
    char sourceName[32] = {'\0'};
    strncpy(sourceName, pdu -> caData + 32, 32);
    recordLog::getLogInstace().recordLogToFile(sourceName, ADDFRIEND, "添加好友失败");
    MyTcpServer::getInstance().forwardMsg(sourceName, pdu);
}

PDU* handleFlushFriendRequest(PDU* pdu)
{
    char caName[32] = {'\0'};
    strncpy(caName, pdu -> caData, 32);
    QStringList strList = DBOperate::getInstance().handleFlushFriend(caName);
    uint uiMsgLen = strList.size() / 2 * 36;
    PDU* resPdu = mkPDU(uiMsgLen);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
    for(int i = 0; i * 2 < strList.size(); ++ i)
    {
        strncpy((char*)(resPdu -> caMsg) + 36 * i, strList.at(i * 2).toStdString().c_str(), 32);
        strncpy((char*)(resPdu -> caMsg) + 36 * i + 32, strList.at(i * 2 + 1).toStdString().c_str(), 4);
    }

    return resPdu;
}

PDU* handleDeleteFriendRequest(PDU* pdu)
{
    char deletedName[32] = {'\0'};
    char sourceName[32] = {'\0'};

    strncpy(deletedName, pdu -> caData, 32);
    strncpy(sourceName, pdu -> caData + 32, 32);
    bool ret = DBOperate::getInstance().handleDeleteFriend(deletedName, sourceName);

    PDU *resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
    if(ret)
    {
        strncpy(resPdu -> caData, DEL_FRIEND_OK, 32);
    }
    else
    {
        strncpy(resPdu -> caData, DEL_FRIEND_FAILED, 32);
    }

    MyTcpServer::getInstance().forwardMsg(deletedName, pdu);

    return resPdu;
}

PDU*  handlePrivateChatRequest(PDU* pdu)
{
    char chatedName[32] = {'\0'};
    char sourceName[32] = {'\0'};

    strncpy(chatedName, pdu -> caData, 32);
    strncpy(sourceName, pdu -> caData + 32, 32);

    PDU* resPdu = nullptr;
    recordLog::getLogInstace().recordLogToFile(sourceName, "好友私聊", QString("与%1私聊").arg(chatedName));

    bool ret = MyTcpServer::getInstance().forwardMsg(chatedName, pdu);

    if(!ret)
    {
        resPdu = mkPDU(0);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND;
        strcpy(resPdu -> caData, PRIVATE_CHAT_OFFLINE);
    }

    return resPdu;
}

void handleGroupChatRequest(PDU* pdu)
{
    QStringList strList = DBOperate::getInstance().handleFlushFriend(pdu->caData);
    char chatRequestName[32] = {'\0'};
    strncpy(chatRequestName, pdu->caData, 32);
    recordLog::getLogInstace().recordLogToFile(chatRequestName, "群聊", "消息群聊");
    for(QString strName:strList)
    {
        MyTcpServer::getInstance().forwardMsg(strName, pdu);
    }
}

PDU* handleCreateDirRequest(PDU* pdu)
{
    char caDirName[32];
    char caCurPath[pdu -> uiMsgLen];
    strncpy(caDirName, pdu -> caData, 32);
    strncpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    QString strDir = QString("%1/%2").arg(caCurPath).arg(caDirName);
    QDir dir;
    PDU *resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;

    if(dir.exists(caCurPath))
    {
        if(dir.exists(strDir))
        {
            strncpy(resPdu -> caData, CREATE_DIR_EXIST, 32);
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "创建文件夹", " 文件夹已经存在，创建失败");
        }
        else
        {
            dir.mkdir(strDir);
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "创建文件夹", " 文件夹创建成功");
            strncpy(resPdu -> caData, CREATE_DIR_OK, 32);
        }
    }
    else
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "创建文件夹", " 路径不存在，创建失败");
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }

    return resPdu;
}

PDU* handleFlushDirRequest(PDU* pdu)
{
    char caCurDir[pdu -> uiMsgLen];
    memcpy(caCurDir, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    QDir dir;
    PDU* resPdu = nullptr;
    if(!dir.exists(caCurDir))
    {
        resPdu = mkPDU(0);
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "刷新文件", " 文件夹不存在，刷新失败");
    }
    else
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "刷新文件", " 刷新成功");
        dir.setPath(caCurDir);
        QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot); // 获取当前目录下所有文件包括文件夹
        int iFileNum = fileInfoList.size();
        resPdu = mkPDU(sizeof(FileInfo) * iFileNum);
        FileInfo *pFileInfo = nullptr;
        strncpy(resPdu -> caData, FLUSH_DIR_OK, 32);
        for(int i = 0; i < iFileNum; ++ i)
        {
            pFileInfo = (FileInfo*)(resPdu -> caMsg) + i;

            QByteArray byteFileName = fileInfoList[i].fileName().toUtf8();
            memcpy(pFileInfo -> caName, byteFileName.toStdString().c_str(), byteFileName.size());
            pFileInfo -> bIsDir = fileInfoList[i].isDir();
            pFileInfo -> uiSize = fileInfoList[i].size();
            QDateTime dtLastTime = fileInfoList[i].lastModified();
            QString strLastTime = dtLastTime.toString("yyyy/MM/dd hh:mm");
            memcpy(pFileInfo -> caTime, strLastTime.toStdString().c_str(), strLastTime.size());
        }
    }
    resPdu -> uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_RESPOND;

    return resPdu;
}

PDU* handleDelFileOrDirRequest(PDU* pdu)
{
    PDU* resPdu = mkPDU(0);
    char strDelPath[pdu -> uiMsgLen];
    memcpy(strDelPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);

    QDir dir;

    resPdu -> uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
    if(!dir.exists(strDelPath))
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "删除文件夹或文件操作", " 路径" + QString(strDelPath) + "不存在，删除失败");
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }
    else
    {
        bool ret = false;
        QFileInfo fileInfo(strDelPath);
        if(fileInfo.isDir())
        {
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "删除文件夹", "删除" + QString(strDelPath) + "成功");
            dir.setPath(strDelPath);
            ret = dir.removeRecursively();
        }
        else if(fileInfo.isFile())
        {
            char waitDelFileName[64] = {'\0'};
            memcpy(waitDelFileName, (char*)pdu->caData, 64);
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "删除文件", "删除" + QString(waitDelFileName) + "成功");
            ret = dir.remove(strDelPath);
        }
        if(ret)
        {
            strncpy(resPdu -> caData, DELETE_FILEORDIR_OK, 32);
        }
        else
        {
            strncpy(resPdu -> caData, DELETE_FILEORDIR_FAILED, 32);
        }
    }

    return resPdu;
}

PDU* handleRenameFileRequest(PDU* pdu)
{
    PDU* resPdu = mkPDU(0);
    char caCurPath[pdu -> uiMsgLen];
    char caOldName[32] = {'\0'};
    char caNewName[32] = {'\0'};
    memcpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    strncpy(caOldName, pdu -> caData, 32);
    strncpy(caNewName, pdu -> caData + 32, 32);

    QDir dir;
    resPdu -> uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
    dir.setPath(caCurPath);
    if(dir.rename(caOldName, caNewName))
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "文件重命名", QString(caOldName) + "更名为" + QString(caNewName) + "成功");
        strncpy(resPdu -> caData, RENAME_FILE_OK, 32);
    }
    else
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "文件重命名", QString(caOldName) + "更名为" + QString(caNewName) + "失败");
        strncpy(resPdu -> caData, RENAME_FILE_FAILED, 32);
    }

    return resPdu;
}

PDU* handleEntryDirRequest(PDU* pdu)
{
    char strEntryPath[pdu -> uiMsgLen];
    memcpy(strEntryPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    PDU* resPdu = nullptr;
    QDir dir(strEntryPath);

    if(!dir.exists())
    {
        resPdu = mkPDU(0);
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }
    else
    {
        QFileInfo fileInfo(strEntryPath);
        if(!fileInfo.isDir())
        {
            resPdu = mkPDU(0);
            strncpy(resPdu -> caData, ENTRY_DIR_FAILED, 32);
        }
        else
        {
            resPdu = handleFlushDirRequest(pdu);
        }
    }
    resPdu -> uiMsgType = ENUM_MSG_TYPE_ENTRY_DIR_RESPOND;
    if(strcmp(resPdu -> caData, FLUSH_DIR_OK) == 0)
    {
        strncpy(resPdu -> caData, ENTRY_DIR_OK, 32);
    }
    else
    {
        strncpy(resPdu -> caData, ENTRY_DIR_FAILED, 32);
    }

    return resPdu;
}

PDU* handlePreDirRequest(PDU* pdu)
{
    char strPrePath[pdu -> uiMsgLen];
    memcpy(strPrePath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    PDU* resPdu = nullptr;
    QDir dir(strPrePath);

    if(!dir.exists())
    {
        resPdu = mkPDU(0);
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }
    else
    {
        resPdu = handleFlushDirRequest(pdu);
    }
    resPdu -> uiMsgType = ENUM_MSG_TYPE_PRE_DIR_RESPOND;
    if(strcmp(resPdu -> caData, FLUSH_DIR_OK) == 0)
    {
        strncpy(resPdu -> caData, PRE_DIR_OK, 32);
    }
    else
    {
        strncpy(resPdu -> caData, PRE_DIR_FAILED, 32);
    }

    return resPdu;
}

PDU* handleUploadFileRequest(PDU* pdu, TransFile* transFile)
{
    char caCurPath[pdu -> uiMsgLen];
    char caFileName[32] = {'\0'};
    qint64 fileSize = 0;

    strncpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    sscanf(pdu -> caData, "%s %lld", caFileName, &fileSize);
    QString strFilePath = QString("%1/%2").arg(caCurPath).arg(caFileName);

    PDU* resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

    transFile->file.setFileName(strFilePath);
    if(transFile->file.open(QIODevice::WriteOnly))
    {
        transFile->bTransform = true;
        transFile->iTotalSize = fileSize;
        transFile->iReceivedSize = 0;

        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "上传文件", "开始上传文件" + QString(caFileName));
        memcpy(resPdu -> caData, UPLOAD_FILE_START, 32);
    }
    else
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "上传文件", QString(caFileName) + "文件不存在，上传失败");
        memcpy(resPdu -> caData, UPLOAD_FILE_FAILED, 32);
    }

    return resPdu;
}

PDU* handleDownloadFileRequest(PDU* pdu, QFile *fDownloadFile, QTimer *pTimer)
{
    char caFileName[32] = {'\0'};
    char caCurPath[pdu -> uiMsgLen];
    memcpy(caFileName, pdu -> caData, 32);
    memcpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    QString strDownloadFilePath = QString("%1/%2").arg(caCurPath).arg(caFileName);
    fDownloadFile->setFileName(strDownloadFilePath);
    qint64 fileSize = fDownloadFile -> size();

    PDU *resPdu = nullptr;
    if(fDownloadFile->open(QIODevice::ReadOnly))
    {
        resPdu = mkPDU(32 + sizeof (qint64) + 5);

        resPdu -> uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
        strncpy(resPdu -> caData, DOWNLOAD_FILE_START, 32);
        sprintf((char*)resPdu -> caMsg, "%s %lld", caFileName, fileSize);
        pTimer -> start(1000);
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "下载文件", "开始下载文件" + QString(caFileName));
    }
    else
    {
        resPdu = mkPDU(0);
        resPdu -> uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
        strncpy(resPdu -> caData, DOWNLOAD_FILE_FAILED, 32);
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "下载文件", "下载" + QString(caFileName) + "文件失败,文件打开失败");
    }

    return resPdu;
}

PDU* handleMoveFileRequest(PDU* pdu)
{
    char caMoveFileName[32];
    int iOldDirSize = 0;
    int iDesDirSize = 0;
    sscanf(pdu -> caData, "%s %d %d", caMoveFileName, &iDesDirSize, &iOldDirSize);
    char caOldDir[iOldDirSize + 1 + 32];
    char caDesDir[iDesDirSize + 1 + 32];
    memset(caOldDir, '\0', iOldDirSize + 1 + 32);
    memset(caDesDir, '\0', iDesDirSize + 1 + 32);
    sscanf((char*)pdu -> caMsg, "%s %s", caDesDir, caOldDir);

    QFileInfo fileInfo(caDesDir);
    PDU* resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;

    if(!fileInfo.isDir())
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "移动文件", "非法操作文件移动");
        strncpy(resPdu -> caData, MOVE_FILE_FAILED, 32);
        return resPdu;
    }

    strcat(caOldDir, "/");
    strcat(caOldDir, caMoveFileName);
    strcat(caDesDir, "/");
    strcat(caDesDir, caMoveFileName);
    if (QFile::rename(caOldDir, caDesDir))
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "移动文件", "移动文件" +
                                                   QString(caMoveFileName) + "从" + QString(caOldDir) + "到" + QString(caDesDir));
        strncpy(resPdu -> caData, MOVE_FILE_OK, 32);
    }
    else
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "移动文件", "移动文件失败");
        strncpy(resPdu -> caData, MOVE_FILE_FAILED, 32);
    }

    return resPdu;
}

PDU* handleShareFileRequest(PDU* pdu, QString strSouName)
{
    int iUserNum = 0;
    char caFileName[32];
    sscanf(pdu -> caData, "%s %d", caFileName, &iUserNum);

    const int iFilePathLen = pdu->uiMsgLen - iUserNum * 32;
    char caFilePath[iFilePathLen];
    PDU* resPdu = mkPDU(iFilePathLen);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
    memcpy(resPdu -> caData, strSouName.toStdString().c_str(), strSouName.size());
    memcpy(resPdu -> caData + 32, caFileName, 32);
    memcpy(caFilePath, (char*)(pdu -> caMsg) + 32 * iUserNum, iFilePathLen);
    memcpy((char*)resPdu -> caMsg, caFilePath, iFilePathLen);

    recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "分享文件", "分享文件" + QString(caFileName));

    char caDesName[32];
    for(int i = 0; i < iUserNum; ++ i)
    {
        memcpy(caDesName, (char*)(pdu -> caMsg) + 32 * i, 32);
        MyTcpServer::getInstance().forwardMsg(caDesName, resPdu);
    }
    free(resPdu);
    resPdu = nullptr;

    resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
    strncpy(resPdu -> caData, SHARE_FILE_OK, 32);

    return resPdu;
}

bool copyDir(QString strOldPath, QString strNewPath)
{
    int ret = true;
    QDir dir;

    dir.mkdir(strNewPath);
    dir.setPath(strOldPath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    QString strOldFile;
    QString strNewFile;
    for(QFileInfo fileInfo : fileInfoList)
    {
        strOldFile = QString("%1/%2").arg(strOldPath).arg(fileInfo.fileName());
        strNewFile = QString("%1/%2").arg(strNewPath).arg(fileInfo.fileName());
        if(fileInfo.isFile())
        {
            ret = ret && QFile::copy(strOldFile, strNewFile);
        }
        else if(fileInfo.isDir())
        {
            ret = ret && copyDir(strOldFile, strNewFile);
        }
    }

    return ret;
}

PDU* handleShareFileNoteRespond(PDU *pdu)
{
    int iOldPathLen = 0;
    int iNewPathLen = 0;
    sscanf(pdu -> caData, "%d %d", &iOldPathLen, &iNewPathLen);
    char caOldPath[iOldPathLen];
    char caNewDir[iNewPathLen];
    sscanf((char*)pdu -> caMsg, "%s %s", caOldPath, caNewDir);

    char *pIndex = strrchr(caOldPath, '/');
    QString strNewPath = QString("%1/%2").arg(caNewDir).arg(pIndex + 1);
    QFileInfo fileInfo(caOldPath);
    bool ret = false;
    if(fileInfo.isFile())
    {
        ret = QFile::copy(caOldPath, strNewPath);
    }
    else if(fileInfo.isDir())
    {
        ret = copyDir(caOldPath, strNewPath);
    }
    else
    {
        ret = false;
    }

    PDU* resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
    if(ret)
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "分享文件", "分享文件成功");
        memcpy(resPdu -> caData, SHARE_FILE_OK, 32);
    }
    else
    {
        recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "分享文件", "分享文件失败");
        memcpy(resPdu -> caData, SHARE_FILE_FAILED, 32);
    }

    return resPdu;
}

void MyTcpSocket::receiveMsg()
{
    if(m_uploadFile->bTransform)
    {
        uint uiPDULen = 0;
        this -> read((char*)&uiPDULen, sizeof(uint));
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        char* baBuffer = new char[uiMsgLen];
        this -> read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
        memcpy(baBuffer, (char*)pdu -> caMsg, pdu -> uiMsgLen);//读取密文
        QByteArray encryptText(const_cast<char*>(baBuffer), uiMsgLen);
        int isFillByte = 0;
        int iRecvFileSize = 0;
        sscanf(pdu -> caData, "%d %d", &isFillByte, &iRecvFileSize);
        AES aes;
        QByteArray decryptText = aes.printfData(encryptText);//解密
        m_uploadFile->file.write(decryptText, decryptText.length());
        m_uploadFile->iReceivedSize += iRecvFileSize;
        PDU* resPdu = nullptr;

        if(m_uploadFile->iReceivedSize == m_uploadFile->iTotalSize)
        {
            m_uploadFile->file.close();
            m_uploadFile->bTransform = false;

            resPdu = mkPDU(0);
            resPdu -> uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strncpy(resPdu -> caData, UPLOAD_FILE_OK, 32);
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "上传文件","文件上传成功");
            //处理空包,将缓冲区清理干净
            QByteArray emptyPackage  = this -> readAll();
            //qDebug() << "文件上传完成";
        }
        else if(m_uploadFile -> iReceivedSize > m_uploadFile->iTotalSize)
        {
            m_uploadFile->file.close();
            m_uploadFile->bTransform = false;

            resPdu = mkPDU(0);
            resPdu -> uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strncpy(resPdu -> caData, UPLOAD_FILE_FAILED, 32);
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "上传文件","文件上传失败");
        }

        if(nullptr != resPdu)
        {
            this -> write((char*)resPdu, resPdu -> uiPDULen);
            free(resPdu);
            resPdu = nullptr;
        }
        free(pdu);
        pdu = nullptr;
        delete []baBuffer;
        baBuffer = nullptr;
        return ;
    }

    uint uiPDULen = 0;
    this -> read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this -> read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));

    PDU* resPdu = nullptr;
    switch(pdu -> uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_REQUEST:{
        resPdu = handleRegistRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:{
        resPdu = handleLoginRequest(pdu, m_strName);
        break;
    }
    case ENUM_MSG_TYPE_LOGOUT_REQUEST:{
        resPdu = handleLogoutRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_ONLINE_USERS_REQUEST:{
        resPdu = handleOnlineUsersRequest();
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:{
        resPdu = handleSearchUserRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:{
        resPdu = handleAddFriendRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:{
        handleAddFriendAgree(pdu);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REJECT:{
        handleAddFriendReject(pdu);
        break;
    }
    case ENUM_MSG_TYPE_FLSUH_FRIEND_REQUEST:{
        resPdu = handleFlushFriendRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:{
        resPdu = handleDeleteFriendRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:{
        resPdu = handlePrivateChatRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:{
        handleGroupChatRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:{
        resPdu = handleCreateDirRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_DIR_REQUEST:{
        resPdu = handleFlushDirRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:{
        resPdu = handleDelFileOrDirRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:{
        resPdu = handleRenameFileRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_ENTRY_DIR_REQUEST:{
        resPdu = handleEntryDirRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_PRE_DIR_REQUEST:{
        resPdu = handlePreDirRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:{
        resPdu = handleUploadFileRequest(pdu, m_uploadFile);
        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:{
        resPdu = handleDownloadFileRequest(pdu, m_pDownloadFile, m_pTimer);
        break;
    }
    case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:{
        resPdu = handleMoveFileRequest(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:{
        resPdu = handleShareFileRequest(pdu, m_strName);
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:{
        resPdu = handleShareFileNoteRespond(pdu);
        break;
    }
    default:
        break;
    }

    if(nullptr != resPdu)
    {
        this -> write((char*)resPdu, resPdu -> uiPDULen);
        free(resPdu);
        resPdu = nullptr;
    }
    free(pdu);
    pdu = nullptr;
}

void MyTcpSocket::handleClientOffline()
{
    DBOperate::getInstance().handleOffline(m_strName.toStdString().c_str());
    qDebug() << "m_strName:" << m_strName;
    emit offline(this);
}

void MyTcpSocket::handledownloadFileData()
{
    m_pTimer->stop();
    char *pBuffer = new char[4096];
    qint64 iActualSize = 0; 
    while(true)
    {
        iActualSize = m_pDownloadFile->read(pBuffer, 4096);

        AES aes;
        QByteArray encryptText = aes.scanfData(QByteArray(const_cast<char*>(pBuffer), iActualSize));//加密
        if(iActualSize > 0 && iActualSize <= 4096){
            PDU *resPdu = mkPDU(encryptText.length());
            resPdu -> uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_BYTE_TIPS;
            int fillByteNum = padding::getPKCS7PaddedLength(iActualSize, 16);
            int isFillByte = (0 == fillByteNum) ? 0 : 1;
            sprintf(resPdu -> caData, "%d %d", isFillByte, iActualSize);
            memcpy((char*)resPdu -> caMsg, encryptText.data(), encryptText.length());
            this -> write((char*)resPdu, resPdu -> uiPDULen);
            pBuffer[4096] = {'\0'};
            free(resPdu);
            resPdu = nullptr;
            SYNCED_EXEC_DELAY_TIME(10);
        }

        else if (iActualSize == 0)//发送文件成功后给请求端发送一个空包，防止堵塞
        {
            SYNCED_EXEC_DELAY_TIME(10);
            char emptyPackage[10] = {'e', 'm', 'p', 't', 'y', 'p', 'a', 'c', 'k', '\0'};
            this -> write(emptyPackage, 10);
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "下载文件", "下载文件成功");
            break;
        }
        else
        {
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "下载文件", "发送文件数据给客户端出错");
           break;
        }
    }

    m_pDownloadFile -> close();
    delete [] pBuffer;
    pBuffer = nullptr;
    m_pDownloadFile->setFileName("");
    /* qDebug() << "main threadId:" << QThread::currentThreadId();
    emit startDownloadFile(m_pDownloadFile, this);*/
}

QByteArray stringToBytearray(QString toString){
    QByteArray byteArray;
    bool ok;
    for(int i = 0; i < toString.size(); i++){
        byteArray.append((char)toString.mid(i, 2).toInt(&ok, 16));
    }
    return byteArray;
}


