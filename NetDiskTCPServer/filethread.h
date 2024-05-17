#ifndef FILETHREAD_H
#define FILETHREAD_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QMap>
#include "protocol.h"
#include "padding.h"
#include "recordlog.h"
#include "aes.h"
class MyTcpSocket;
class MyTcpServer;

class fileThread : public QObject
{
    Q_OBJECT
public:
    explicit fileThread(QObject *parent = nullptr);

public slots:
    void downloadFileData(QFile*, MyTcpSocket*);

signals:
    void downloadFileOver(MyTcpSocket*);

};

#endif // FILETHREAD_H
