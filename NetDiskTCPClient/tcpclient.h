#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "operatewidget.h"
#include "rsakey.h"
#include "aes.h"
QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();

    static TcpClient& getInstance();
    QTcpSocket& getTcpSocket();

    QString getStrLoginName() const;
    void setStrLoginName(const QString &strName);

    QString getStrCurPath() const;
    void setStrCurPath(const QString &strCurPath);

    QString getStrRootPath() const;
    void setStrRootPath(const QString &strRootPath);

public slots:
    void showConnect();
    void receiveMsg();

private slots:
    void on_regist_pb_clicked();
    void on_login_pb_clicked();
    void on_logout_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;
    QTcpSocket m_tcpSocket;

    QString m_strLoginName;
    QString m_strRootPath;
    QString m_strCurPath;

    FileSystem *m_fileOperate = nullptr;

    bool m_isLoginFlushFile;
};
#endif // TCPCLIENT_H
