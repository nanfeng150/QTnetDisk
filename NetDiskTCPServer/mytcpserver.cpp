#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket -> setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);
    m_socketMap[pTcpSocket] = socketDescriptor;

    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket *)), this, SLOT(deleteSocket(MyTcpSocket *)));
}

bool MyTcpServer::forwardMsg(const QString caDesName, PDU *pdu)
{
    if(caDesName.isEmpty() || pdu == nullptr)
    {
        return false;
    }

    for(int i = 0; i < m_tcpSocketList.size(); ++ i)
    {
        if(caDesName == m_tcpSocketList.at(i) -> getStrName())
        {
            m_tcpSocketList.at(i)->write((char*)pdu, pdu -> uiPDULen);
            return true;
        }
    }
    return false;
}

void MyTcpServer::deleteSocket(MyTcpSocket *waitDelectSocket)
{
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for(; iter != m_tcpSocketList.end(); iter ++)
    {
        if(waitDelectSocket == *iter)
        {
            (*iter) -> deleteLater();
            *iter = nullptr;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
}

QString MyTcpServer::getStrRootPath() const
{
    return m_strRootPath;
}

void MyTcpServer::setStrRootPath(const QString &strRootPath)
{
    m_strRootPath = strRootPath;
}

QMap<MyTcpSocket *, qintptr> MyTcpServer::getSocketMap()
{
    return m_socketMap;
}
