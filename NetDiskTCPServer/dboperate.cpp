#include "dboperate.h"
#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <QDate>
#include <QThread>
DBOperate::DBOperate(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
}

DBOperate &DBOperate::getInstance()
{
    static DBOperate instance;
    return instance;
}

void DBOperate::init()
{
    m_db.setHostName("127.0.0.1");
    m_db.setUserName("root");
    m_db.setPassword("zcd15599210255");
    m_db.setPort(3306);
    m_db.setDatabaseName("qtfileserverdatabase");
    if(!m_db.open())
        QMessageBox::critical(nullptr, "数据库打开", "数据库打开失败");
}

DBOperate::~DBOperate()
{
    m_db.close();
}

bool DBOperate::handleRegist(const char *name, const char *pwd)
{
    if(nullptr == name || nullptr == pwd)
        return false;
    QString strQuery = QString("insert into userInfor(NAME, PASSWORD) values(\'%1\', \'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    bool insertDbRet = query.exec(strQuery);
    if(insertDbRet){
        QString sqlQuery = QString("select * from userInfor where NAME = \'%1\' ").arg(name);
        QSqlQuery logQuery;
        logQuery.exec(sqlQuery);
        while(logQuery.next()){
            QString id = logQuery.value("ID").toString();
            loggingDb(id, QString(name), REGIST);
        }
        recordLog::getLogInstace().recordLogToFile(QString(name), REGIST, "注册成功");
    }
    recordLog::getLogInstace().recordLogToFile(QString(name), REGIST, "注册失败");
    return  insertDbRet;
}

bool DBOperate::handleLogout(const char *name, const char *pwd)
{
    if(nullptr == name || nullptr == pwd)
        return false;
    QSqlQuery query;
    QString sqlQuery = QString("select * from userInfor where NAME = \'%1\' and PASSWORD = \'%2\' ").arg(name).arg(pwd);
    query.exec(sqlQuery);
    bool logoutRet = false;
    while(query.next()){
        QString userName =  query.value("NAME").toString();
        QString userPassword =  query.value("PASSWORD").toString();
        logoutRet = userName == QString(name) && userPassword == QString(pwd);
    }

    if(logoutRet){
        QString logSqlQuery = QString("select * from userInfor where NAME = \'%1\' ").arg(name);
        QSqlQuery logQuery;
        logQuery.exec(logSqlQuery);
        while(logQuery.next()){
            QString id = logQuery.value("ID").toString();
            loggingDb(id, QString(name), LOGOUT);
        }
        recordLog::getLogInstace().recordLogToFile(QString(name), LOGOUT, "注销成功");
        QString strQuery = QString("delete from userInfor where NAME = \'%1\' and PASSWORD = \'%2\' ").arg(name).arg(pwd);
        return query.exec(strQuery);
    }
    recordLog::getLogInstace().recordLogToFile(QString(name), LOGOUT, "注销失败");
    return false;
}

bool DBOperate::handleLogin(const char *name, const char *pwd)
{
    if(nullptr == name || nullptr == pwd)
    {
        return false;
    }

    QString sqlQuery = QString("select * from userInfor where NAME = \'%1\' and PASSWORD = \'%2\' and ONLINE = 0 ").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(sqlQuery);
    bool queryLoginUserRet = false;

    while(query.next()){
        queryLoginUserRet = query.value("NAME").toString() == QString(name) && query.value("PASSWORD").toString() == QString(pwd);
    }
    if(queryLoginUserRet)
    {
        m_operateDbUser = QString(name);
        QString logSqlQuery = QString("select * from userInfor where NAME = \'%1\' ").arg(name);
        QSqlQuery logQuery;
        logQuery.exec(logSqlQuery);
        while(logQuery.next()){
            QString id = logQuery.value("ID").toString();
            loggingDb(id, QString(name), LOGIN);
        }
        recordLog::getLogInstace().recordLogToFile(QString(name), LOGIN, "登录成功");
        sqlQuery = QString("update userInfor set ONLINE = 1 where NAME = \'%1\' and PASSWORD = \'%2\' ").arg(name).arg(pwd);
        return query.exec(sqlQuery);
    }
    recordLog::getLogInstace().recordLogToFile(QString(name), LOGIN, "登录失败");
    return false;
}


bool DBOperate::handleOffline(const char *name)
{
    if(nullptr == name)
        return false;
    qDebug() << "name:" << name;
    recordLog::getLogInstace().recordLogToFile(QString(name), OFFLINE, "下线成功");
    QString logSqlQuery = QString("select * from userInfor where NAME = \'%1\' ").arg(name);
    QSqlQuery logQuery;
    logQuery.exec(logSqlQuery);
    while(logQuery.next()){
        QString id = logQuery.value("ID").toString();
        loggingDb(id, QString(name), OFFLINE);
    }

    QString strQuery = QString("update userInfor set online = 0 where NAME = \'%1\' ").arg(name);
    QSqlQuery query;
    return query.exec(strQuery);
}

QStringList DBOperate::handleOnlineUsers()
{
    recordLog::getLogInstace().recordLogToFile(m_operateDbUser, QUERYALLUSER, "查询成功");

    QString strQuery = QString("select NAME from userInfor where online = 1 ");
    QStringList result;
    result.clear();

    QSqlQuery query;
    query.exec(strQuery);
    while(query.next())
    {
        result.append(query.value("NAME").toString());
    }

    return result;
}

int DBOperate::handleSearchUser(const char *name)
{
    if(nullptr == name)
    {
        return 2;
    }

    recordLog::getLogInstace().recordLogToFile(m_operateDbUser, SEARCHUSER, "查询用户");
    QString strQuery = QString("select online from userInfor where NAME = \'%1\' ").arg(name);
    QSqlQuery query;
    query.exec(strQuery);
    if(query.next())
    {
        return query.value(0).toInt();
    }
    else
    {
        return 2;
    }
}

int DBOperate::handleAddFriend(const char *addedName, const char *sourceName)
{
    if(nullptr == addedName || nullptr == sourceName)
    {
        return 4;
    }

    recordLog::getLogInstace().recordLogToFile(m_operateDbUser, ADDFRIEND, "添加好友");
    QString strQuery = QString("select * from friendinfo "
                               "where (ID = (select ID from userInfor where NAME = \'%1\') and "
                               "FRIENDID = (select ID from userInfor where NAME = \'%2\')) or "
                               "(ID = (select ID from userInfor where NAME = \'%3\') and "
                               "FRIENDID = (select ID from userInfor where NAME = \'%4\'))")
            .arg(sourceName).arg(addedName).arg(addedName).arg(sourceName);
    QSqlQuery query;
    query.exec(strQuery);
    if(query.next())
    {
        return 3;
    }
    else
    {
        return handleSearchUser(addedName);
    }
}

bool DBOperate::handleAddFriendAgree(const char *addedName, const char *sourceName)
{
    if(nullptr == addedName || nullptr == sourceName)
    {
        qDebug() << "handleAddFriendAgree: name is NULL";
        return false;
    }

    int sourceUserId = -1, addedUserId = -1;

    addedUserId = getIdByUserName(addedName);
    sourceUserId = getIdByUserName(sourceName);
    recordLog::getLogInstace().recordLogToFile(m_operateDbUser, ADDFRIEND, "添加好友成功");
    QString strQuery = QString("insert into friendInfo values(%1, %2) ").arg(sourceUserId).arg(addedUserId);
    QSqlQuery query;
    return query.exec(strQuery);
}

int DBOperate::getIdByUserName(const char *name)
{
    if(nullptr == name)
        return -1;
    QString strQuery = QString("select ID from userInfor where NAME = \'%1\' ").arg(name);
    QSqlQuery query;
    query.exec(strQuery);
    if(query.next())
        return query.value(0).toInt();
    else return -1;
}

QStringList DBOperate::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();

    if (nullptr == name)
    {
        return strFriendList;
    }

    recordLog::getLogInstace().recordLogToFile(m_operateDbUser, FLUSHFRIENDLIST, "刷新好友列表");

    QString strQuery = QString("select ID from userInfor where NAME = \'%1\' and ONLINE = 1 ").arg(name);
    QSqlQuery query;
    int iId = -1;
    query.exec(strQuery);
    if (query.next())
    {
        iId = query.value(0).toInt();
    }

    strQuery = QString("select NAME, online from userInfor "
                       "where ID in "
                       "((select FRIENDID from friendinfo "
                       "where ID = %1) "
                       "union "
                       "(select ID from friendinfo "
                       "where FRIENDID = %2))").arg(iId).arg(iId);
    query.exec(strQuery);
    while(query.next())
    {
        char friName[32];
        char friOnline[4];
        strncpy(friName, query.value(0).toString().toStdString().c_str(), 32);
        strncpy(friOnline, query.value(1).toString().toStdString().c_str(), 4);
        strFriendList.append(friName);
        strFriendList.append(friOnline);
    }

    return strFriendList;
}

bool DBOperate::handleDeleteFriend(const char *deletedName, const char *sourceName)
{
    if(deletedName == nullptr || sourceName == nullptr)
    {
        return false;
    }

    recordLog::getLogInstace().recordLogToFile(m_operateDbUser, DELETEFRIEND, QString("删除好友%1成功").arg(deletedName));

    int iDelId = -1;
    int iSouId = -1;
    QString strQuery = QString("select ID from userInfor where NAME in (\'%1\', \'%2\') ").arg(deletedName).arg(sourceName);
    QSqlQuery query;

    query.exec(strQuery);
    if(query.next())
    {
        iDelId = query.value(0).toInt();
    }
    if(query.next())
    {
        iSouId = query.value(0).toInt();
    }

    strQuery = QString("delete from friendInfo where ID in (\'%1\', \'%2\') and FRIENDID in (\'%3\', \'%4\') ")
            .arg(iDelId).arg(iSouId).arg(iDelId).arg(iSouId);

    return query.exec(strQuery);
}

void DBOperate::loggingDb(QString ID, QString name, QString operateType)
{
    QString curYear = QString::number(QDate::currentDate().year()) + ".";
    QString curMonth =QString::number(QDate::currentDate().month()) + ".";
    QString curDay = QString::number(QDate::currentDate().day());
    QString curDate = curYear + curMonth + curDay + " ";
    QString strQuery = QString("insert into serverlog values(%1, \'%2\', \'%3\', \'%4\')").arg(ID).arg(name).arg(curDate + QTime::currentTime().toString()).arg(operateType);
    QSqlQuery query;
    query.exec(strQuery);
}
