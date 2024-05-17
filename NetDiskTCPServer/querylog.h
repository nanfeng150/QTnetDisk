#ifndef QUERYLOG_H
#define QUERYLOG_H

#include <QWidget>
#include <QString>
#include <QFile>
#include <QTime>
#include <QDate>
#include <QTextStream>
#include <QDir>

namespace Ui {
class queryLog;
}

class queryLog : public QWidget
{
    Q_OBJECT

public:
    explicit queryLog(QWidget *parent = nullptr);
    ~queryLog();
    static queryLog& getQueryLogInstance();

public slots:
    void queryFileTolog();


private:
    Ui::queryLog *ui;
};

#endif // QUERYLOG_H
