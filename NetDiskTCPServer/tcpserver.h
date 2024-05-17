#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include <QFile>
#include "conditionquery.h"
#include "recordlog.h"
#include "querylog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpServer; }
QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer();
    void loadConfig();
    quint16 getWidgetType();
private slots:
    void on_queryUserData_clicked();

    void on_queryConditionUserData_clicked();

    void on_queryLog_clicked();

private:
    Ui::TcpServer *ui;
    QString m_strIP;
    quint16 m_usPort;
    quint16 m_widgetType;
    conditionQuery *m_queryConditionDb = nullptr;
    queryLog *m_queryLog = nullptr;
    quint16 m_recordTableRow;

};
#endif // TCPSERVER_H
