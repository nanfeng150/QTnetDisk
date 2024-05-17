#ifndef RECORDLOG_H
#define RECORDLOG_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTime>
#include <QDate>
#include <QTextStream>
#include <QDir>
class recordLog
{
public:
    recordLog();
    ~recordLog();
    static recordLog& getLogInstace();
    bool createLogFileOrInputLog(QString operateTime, QString operateUse, QString operateType, QString operateLog);
    QString getCurrentTime();
    bool recordLogToFile(QString operateUse, QString operateType, QString operateLog);

};

#endif // RECORDLOG_H
