#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QFile>
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>
#include "protocol.h"
#include "dboperate.h"
#include "rsakey.h"
#include "aes.h"
#include "padding.h"
#include "filethread.h"
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();

    static MyTcpSocket &getSocketInstance();

    void setStrName(QString loginName);
    QString getStrName();


signals:
    void offline(MyTcpSocket *socket);
    void startDownloadFile(QFile*, MyTcpSocket*);//发出开始下载文件的信号
public slots:
    void receiveMsg();
    void handleClientOffline();
    void handledownloadFileData();


private:
    QString m_strName;

    TransFile* m_uploadFile;
    QFile *m_pDownloadFile;

    QTimer *m_pTimer;
};

#endif // MYTCPSOCKET_H
