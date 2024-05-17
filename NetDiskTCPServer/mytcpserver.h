#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>
#include <QList>
#include <QMap>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
private:
    MyTcpServer();
public:
    static MyTcpServer& getInstance();
    void incomingConnection(qintptr socketDescriptor) override;
    bool forwardMsg(const QString caDesName, PDU *pdu);

    QString getStrRootPath() const;
    void setStrRootPath(const QString &strRootPath);

    QMap<MyTcpSocket*, qintptr> getSocketMap();
public slots:
    void deleteSocket(MyTcpSocket *waitDelectSocket);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
    QString m_strRootPath;
    QMap<MyTcpSocket*, qintptr> m_socketMap;
};

#endif // MYTCPSERVER_H
