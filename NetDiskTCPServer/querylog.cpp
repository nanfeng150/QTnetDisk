#include "querylog.h"
#include "ui_querylog.h"
#include <QByteArray>
#include <QDebug>
#include <QTimer>
queryLog::queryLog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::queryLog)
{
    ui->setupUi(this);
    ui->timeLabel->setText(QDate::currentDate().toString());
    connect(ui->queryLogBtn, &QPushButton::clicked, this, &queryLog::queryFileTolog);
}

queryLog::~queryLog()
{
    delete ui;
}

queryLog &queryLog::getQueryLogInstance()
{
    static queryLog queryLogObject;
    return queryLogObject;
}

void queryLog::queryFileTolog()
{
    ui->queryLogEdit->clear();
    QString currentPath = QDir::currentPath();
    QString currentDate = QString::number(QDate::currentDate().year()) + "_" + QString::number(QDate::currentDate().month()) + "_" +
                          QString::number(QDate::currentDate().day()) + "_";
    QString currentFilePath = currentPath + "/log_" + currentDate + "input" + ".txt";
    QFile file(currentFilePath);
    if(file.open(QIODevice::ReadOnly)){
        while(!file.atEnd()){
            QByteArray userOperateMsg = file.readLine();
            QString strUserOperateMsg = userOperateMsg.split('\n')[0];

            ui->queryLogEdit->append(strUserOperateMsg);
        }
        file.close();
    }
    else
        qDebug() << "日志文件打开失败!!!";
}
