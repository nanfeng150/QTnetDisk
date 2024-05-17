#ifndef CONDITIONQUERY_H
#define CONDITIONQUERY_H

#include <QWidget>
#include <QTableWidget>
#include <QMessageBox>

namespace Ui {
class conditionQuery;
}

class conditionQuery : public QWidget
{
    Q_OBJECT

public:
    explicit conditionQuery(QWidget *parent = nullptr);
    ~conditionQuery();
    static conditionQuery &getInstance();
    void initLogTable();
    void inputTooltip(QString toolTipinfor);

public slots:
    void on_nameQueryBtn_clicked();
    void on_idQUeryBtn_clicked();
    void on_logQueryBtn_clicked();
private:
    Ui::conditionQuery *ui;
    QString m_strOpearateUserName;
    QString m_strOpearateUserID;
};

#endif
