#include "recordlog.h"
#include <QDebug>
recordLog::recordLog()
{

}

recordLog::~recordLog()
{

}

recordLog &recordLog::getLogInstace()
{
    static recordLog logObject;
    return logObject;
}

bool recordLog::createLogFileOrInputLog(QString operateTime, QString operateUse, QString operateType, QString operateLog)
{
    QString currentPath = QDir::currentPath();
    QString currentDate = QString::number(QDate::currentDate().year()) + "_" + QString::number(QDate::currentDate().month()) + "_" +
                          QString::number(QDate::currentDate().day()) + "_";
    QString currentFilePath = currentPath + "/log_" + currentDate + "input" + ".txt";
    QFile file(currentFilePath);
    bool isOpenFile = file.open(QIODevice::WriteOnly | QIODevice::Append);
    if(isOpenFile){
        QTextStream coutStraem(&file);
        coutStraem.setCodec("utf-8");
        QString logMsg = operateTime + " " + operateUse + " " + operateType + " " + operateLog + "\n";
        coutStraem << logMsg;
        file.close();
        return true;
    }
    else
        return false;
}

QString recordLog::getCurrentTime()
{
    QString curYear = QString::number(QDate::currentDate().year()) + ".";
    QString curMonth =QString::number(QDate::currentDate().month()) + ".";
    QString curDay = QString::number(QDate::currentDate().day());
    QString curDate = curYear + curMonth + curDay + " ";
    QString currentTime = curDate + QTime::currentTime().toString();
    return currentTime;
}

bool recordLog::recordLogToFile(QString operateUse, QString operateType, QString operateLog)
{
    if(createLogFileOrInputLog(getCurrentTime(), operateUse, operateType, operateLog))
        return true;
    return false;
}
