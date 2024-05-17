#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <QFont>
#include <QTableWidgetItem>
#include "mytcpserver.h"

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    loadConfig();
    DBOperate::getInstance().init();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
    ui->userinforTable->setRowCount(1000);
    m_recordTableRow = 0;
    ui->userinforTable->setColumnCount(4);
    QStringList table_header;
    table_header << "userid" << "username" << "password" << "online";
    ui->userinforTable->setHorizontalHeaderLabels(table_header);
    ui->userinforTable->setColumnWidth(0, 100);
    ui->userinforTable->setColumnWidth(1, 250);
    ui->userinforTable->setColumnWidth(2, 250);
    ui->userinforTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    m_queryLog = new queryLog;
    connect(ui->queryUserLog, &QPushButton::clicked, this, &TcpServer::on_queryLog_clicked);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();
        strData.replace("\r\n", " ");
        QStringList strList = strData.split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        QString widgetType = strList.at(2);
        m_widgetType = widgetType.mid(11, 1).toUShort();
        MyTcpServer::getInstance().setStrRootPath("G:/universityGraduatedWork");
    }
    else
    {
        QMessageBox::critical(this, "open config", "open config failed");
    }
}

quint16 TcpServer::getWidgetType()
{
    return m_widgetType;
}

void TcpServer::on_queryUserData_clicked()
{
    QSqlQuery query;
    query.exec("select * from userInfor");
    int row = 0;
    while(query.next())
    {
        QTableWidgetItem *item[4];
        QStringList userinfor;
        userinfor << query.value("ID").toString() << query.value("NAME").toString() << query.value("PASSWORD").toString() << query.value("ONLINE").toString();
        item[0] = new QTableWidgetItem(userinfor[0]);
        ui->userinforTable->setItem(row, 0, item[0]);
        ui->userinforTable->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[1] = new QTableWidgetItem(userinfor[1]);
        ui->userinforTable->setItem(row, 1, item[1]);
        ui->userinforTable->item(row, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[2] = new QTableWidgetItem(userinfor[2]);
        ui->userinforTable->setItem(row, 2, item[2]);
        ui->userinforTable->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[3] = new QTableWidgetItem(userinfor[3]);
        ui->userinforTable->setItem(row, 3, item[3]);
        ui->userinforTable->item(row, 3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        row++;
        m_recordTableRow++;
        if(m_recordTableRow >= ui->userinforTable->rowCount())
            ui->userinforTable->setRowCount(ui->userinforTable->rowCount() + 1000);
    }
}

void TcpServer::on_queryConditionUserData_clicked()
{
    conditionQuery::getInstance().show();
}

void TcpServer::on_queryLog_clicked()
{
    queryLog::getQueryLogInstance().show();
}
