#ifndef DBOPERATE_H
#define DBOPERATE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include "recordlog.h"

#define REGIST "注册"
#define LOGIN "登录"
#define LOGOUT "注销"
#define OFFLINE "用户下线"
#define QUERYALLUSER "查询在线用户"
#define SEARCHUSER "搜索好友"
#define ADDFRIEND "添加好友"
#define FLUSHFRIENDLIST "刷新好友列表"
#define DELETEFRIEND "删除好友"

class DBOperate : public QObject
{
    Q_OBJECT
public:
    explicit DBOperate(QObject *parent = nullptr);
    static DBOperate& getInstance();
    void init();
    ~DBOperate();

    bool handleRegist(const char *name, const char *pwd);
    bool handleLogin(const char *name, const char *pwd);
    bool handleLogout(const char *name, const char *pwd);
    bool handleOffline(const char *name);
    QStringList handleOnlineUsers();
    int handleSearchUser(const char *name);// 处理查找用户，0存在不在线，1存在并在线，2不存在
    int handleAddFriend(const char *addedName, const char *sourceName);
    bool handleAddFriendAgree(const char *addedName, const char *sourceName);
    int getIdByUserName(const char *name);
    QStringList handleFlushFriend(const char *name);
    bool handleDeleteFriend(const char *deletedName, const char *sourceName);
    void loggingDb(QString ID, QString name, QString operateType);

private:
    QSqlDatabase m_db;
    QString m_operateDbUser;

};

#endif
