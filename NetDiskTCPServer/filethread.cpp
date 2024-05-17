#include "filethread.h"
#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QThread>
#include <QDebug>
fileThread::fileThread(QObject *parent) : QObject(parent)
{

}

void fileThread::downloadFileData(QFile *downloadFile, MyTcpSocket* socket)
{
    MyTcpSocket *newSocket = new MyTcpSocket;
    newSocket -> setSocketDescriptor(MyTcpServer::getInstance().getSocketMap()[socket]);
    qDebug() << "currentThreadID:" << QThread::currentThreadId();
    char *pBuffer = new char[4096];
    qint64 iActualSize = 0;
    while(true)
    {
        iActualSize = downloadFile->read(pBuffer, 4096);

        AES aes;
        QByteArray encryptText = aes.scanfData(QByteArray(const_cast<char*>(pBuffer), iActualSize));//加密
        if(iActualSize > 0 && iActualSize <= 4096){
            PDU *resPdu = mkPDU(encryptText.length());
            resPdu -> uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_BYTE_TIPS;
            int fillByteNum = padding::getPKCS7PaddedLength(iActualSize, 16);
            int isFillByte = (0 == fillByteNum) ? 0 : 1;
            sprintf(resPdu -> caData, "%d %d", isFillByte, iActualSize);
            memcpy((char*)resPdu -> caMsg, encryptText.data(), encryptText.length());
            newSocket -> write((char*)resPdu, resPdu -> uiPDULen);
            pBuffer[4096] = {'\0'};
            free(resPdu);
            resPdu = nullptr;
            SYNCED_EXEC_DELAY_TIME(20);
        }

        else if (iActualSize == 0)//发送文件成功后给请求端发送一个空包，防止堵塞
        {
            SYNCED_EXEC_DELAY_TIME(10);
            char emptyPackage[10] = {'e', 'm', 'p', 't', 'y', 'p', 'a', 'c', 'k', '\0'};
            newSocket -> write(emptyPackage, 10);
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "下载文件", "下载文件成功");
            break;
        }
        else
        {
            recordLog::getLogInstace().recordLogToFile(MyTcpSocket::getSocketInstance().getStrName(), "下载文件", "发送文件数据给客户端出错");
            break;
        }
    }

    downloadFile -> close();
    delete [] pBuffer;
    pBuffer = nullptr;
    downloadFile->setFileName("");
    emit downloadFileOver(newSocket);
}

