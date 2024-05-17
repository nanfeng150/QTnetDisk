#include "conditionquery.h"
#include "ui_conditionquery.h"
#include <QSqlQuery>
#include <QDebug>
#include <QTableWidgetItem>
#include <QInputDialog>

conditionQuery::conditionQuery(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::conditionQuery)
{
    ui->setupUi(this);

    initLogTable();

    connect(ui->idQueryBtn, &QPushButton::clicked, this, &conditionQuery::on_idQUeryBtn_clicked);
    connect(ui->queryLogBtn, &QPushButton::clicked, this, &conditionQuery::on_logQueryBtn_clicked);
}

conditionQuery::~conditionQuery()
{
    delete ui;
}

conditionQuery &conditionQuery::getInstance()
{
    static conditionQuery conditionquery;
    return conditionquery;
}

void conditionQuery::initLogTable()
{
    ui->logTable->setColumnCount(4);
    ui->logTable->setRowCount(1000);
    QStringList table_header;
    table_header << "userid" << "username" << "time" << "operateType";
    ui->logTable->setHorizontalHeaderLabels(table_header);
    ui->logTable->setColumnWidth(0, 100);
    ui->logTable->setColumnWidth(1, 250);
    ui->logTable->setColumnWidth(2, 250);
    ui->logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

void conditionQuery::inputTooltip(QString toolTipinfor)
{
    QMessageBox::warning(this, "查询日志", toolTipinfor);
}

void conditionQuery::on_nameQueryBtn_clicked()
{
    ui->logTable->clear();
    initLogTable();
    m_strOpearateUserName = QInputDialog::getText(this, "搜索", "用户名：");
    QSqlQuery query;
    QString sqlQuery = "select * from serverlog where NAME=\'" + m_strOpearateUserName + "\'";
    query.exec(sqlQuery);
    int row = 0;
    bool isQueryDate = false;
    while(query.next())
    {
        isQueryDate = true;
        QTableWidgetItem *item[4];
        QStringList userinfor;
        userinfor << query.value("ID").toString() << query.value("NAME").toString() << query.value("TIME").toString() << query.value("OPERATE").toString();
        item[0] = new QTableWidgetItem(userinfor[0]);
        ui->logTable->setItem(row, 0, item[0]);
        ui->logTable->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);//内容居中
        item[1] = new QTableWidgetItem(userinfor[1]);
        ui->logTable->setItem(row, 1, item[1]);
        ui->logTable->item(row, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[2] = new QTableWidgetItem(userinfor[2]);
        ui->logTable->setItem(row, 2, item[2]);
        ui->logTable->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[3] = new QTableWidgetItem(userinfor[3]);
        ui->logTable->setItem(row, 3, item[3]);
        ui->logTable->item(row, 3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        row++;
    }
    if(!isQueryDate)
        inputTooltip("没有该用户信息");
}

void conditionQuery::on_idQUeryBtn_clicked()
{
    ui->logTable->clear();
    initLogTable();
    m_strOpearateUserID = QInputDialog::getText(this, "搜索", "ID：");
    QString sqlQuery = "select * from serverlog where ID=" + m_strOpearateUserID;
    QSqlQuery query;
    query.exec(sqlQuery);
    int row = 0;
    bool isQueryDate = false;
    while(query.next())
    {
        isQueryDate = true;
        QTableWidgetItem *item[4];
        QStringList userinfor;
        userinfor << query.value("ID").toString() << query.value("NAME").toString() << query.value("TIME").toString() << query.value("OPERATE").toString();
        item[0] = new QTableWidgetItem(userinfor[0]);
        ui->logTable->setItem(row, 0, item[0]);
        ui->logTable->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[1] = new QTableWidgetItem(userinfor[1]);
        ui->logTable->setItem(row, 1, item[1]);
        ui->logTable->item(row, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[2] = new QTableWidgetItem(userinfor[2]);
        ui->logTable->setItem(row, 2, item[2]);
        ui->logTable->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[3] = new QTableWidgetItem(userinfor[3]);
        ui->logTable->setItem(row, 3, item[3]);
        ui->logTable->item(row, 3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        row++;
    }
    if(!isQueryDate)
        inputTooltip("没有该用户信息");
}

void conditionQuery::on_logQueryBtn_clicked()
{
    ui->logTable->clear();
    initLogTable();
    QSqlQuery query;
    query.exec("select * from serverlog");
    int row = 0;
    while(query.next())
    {
        QTableWidgetItem *item[4];
        QStringList userinfor;
        userinfor << query.value("ID").toString() << query.value("NAME").toString() << query.value("TIME").toString() << query.value("OPERATE").toString();
        item[0] = new QTableWidgetItem(userinfor[0]);
        ui->logTable->setItem(row, 0, item[0]);
        ui->logTable->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);//内容居中
        item[1] = new QTableWidgetItem(userinfor[1]);
        ui->logTable->setItem(row, 1, item[1]);
        ui->logTable->item(row, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[2] = new QTableWidgetItem(userinfor[2]);
        ui->logTable->setItem(row, 2, item[2]);
        ui->logTable->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item[3] = new QTableWidgetItem(userinfor[3]);
        ui->logTable->setItem(row, 3, item[3]);
        ui->logTable->item(row, 3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        row++;
    }
}
