#include "OperateDB.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

OperateDB &OperateDB::getInstance()
{
    static OperateDB instance;
    return instance;
}

void OperateDB::connectSQL()
{
    m_db.setHostName("localhost");
    m_db.setPort(3306);
    m_db.setUserName("root");
    m_db.setPassword("123456");
    m_db.setDatabaseName("mydb2504");
    if (m_db.open())
    {
        qDebug() << "数据库连接成功";
    }
    else
    {
        qDebug() << "数据库连接失败" << m_db.lastError().text();
    }
}

OperateDB::~OperateDB()
{
    m_db.close();
}

bool OperateDB::handleRegist(const char *caName, const char *caPwd)
{
    if (caName == NULL || caPwd == NULL)
    {
        return false;
    }
    QString sql = QString("select * from user_info where name = '%1'").arg(caName);
    qDebug() << "查找用户名是否存在 sql" << sql;
    QSqlQuery q;
    if (!q.exec(sql) || q.next())
    {
        return false;
    }
    sql = QString("insert into user_info(name, pwd) values('%1', '%2')").arg(caName).arg(caPwd);
    qDebug() << "新增一条用户名和密码 sql" << sql;
    return q.exec(sql);
}

bool OperateDB::handleLogin(const char *caName, const char *caPwd)
{
    if (caName == NULL || caPwd == NULL)
    {
        return false;
    }
    QString sql = QString("select * from user_info where name = '%1' and pwd = '%2'").arg(caName).arg(caPwd);
    qDebug() << "查找用户名和密码是否存在 sql" << sql;
    QSqlQuery q;
    if (!q.exec(sql) || !q.next())
    {
        return false;
    }
    sql = QString("update user_info set online = 1 where name = '%1'").arg(caName);
    qDebug() << "将用户的 online 字段置为 1 sql" << sql;
    return q.exec(sql);
}

void OperateDB::handleOffline(const char *caName)
{
    if (caName == NULL)
    {
        return ;
    }
    QString sql = QString("update user_info set online = 0 where name = '%1'").arg(caName);
    qDebug() << "将用户的 online 字段置为 0 sql" << sql;
    QSqlQuery q;
    bool ret = q.exec(sql);
    qDebug() << "handleOffline ret" << ret;
}

int OperateDB::handleFindUser(const char *caName)
{
    if (caName == NULL)
    {
        return -1;
    }
    QString sql = QString("select online from user_info where name = '%1'").arg(caName);
    qDebug() << "查找用户的 online 字段 sql" << sql;
    QSqlQuery q;
    if (!q.exec(sql))
    {
        return -1;
    }
    if (q.next())
    {
        return q.value(0).toInt();
    }
    return 2;
}

QStringList OperateDB::handleOnlineUser()
{
    QString sql = QString("select name from user_info where online = 1");
    QSqlQuery q;
    q.exec(sql);
    QStringList res;
    while (q.next())
    {
        res.append(q.value(0).toString());
    }
    return res;
}

int OperateDB::handleAddFriend(const char *caCurName, const char *caTarName)
{
    if (caCurName == NULL || caTarName == NULL)
    {
        return -1;
    }
    QString sql = QString(R"(
                          select * from friend where
                          (
                            user_id = (select id from user_info where name = '%1')
                            and
                            friend_id = (select id from user_info where name = '%2')
                          )
                          or
                          (
                            friend_id = (select id from user_info where name = '%1')
                            and
                            user_id = (select id from user_info where name = '%2')
                          )
                          )").arg(caCurName).arg(caTarName);
    qDebug() << "判断是否是好友 sql" << sql;
    QSqlQuery q;
    q.exec(sql);
    if (q.next())
    {
        return -2;
    }
    sql = QString("select online from user_info where name = '%1'").arg(caTarName);
    qDebug() << "查询用户是否在线 sql" << sql;
    q.exec(sql);
    if (q.next())
    {
        return q.value(0).toInt();
    }
    return -1;
}

bool OperateDB::handleAddFriendAgree(const char *caCurName, const char *caTarName)
{
    if (caCurName == NULL || caTarName == NULL)
    {
        return false;
    }
    QString sql = QString(R"(
                          insert into friend (user_id, friend_id)
                          select u1.id, u2.id
                          from user_info u1, user_info u2
                          where u1.name = '%1' and u2.name = '%2'"
                          )").arg(caCurName).arg(caTarName);
    QSqlQuery q;
    return q.exec(sql);
}

QStringList OperateDB::handleFlushFriend(const char *caCurName)
{
    QStringList res;
    if (caCurName == NULL)
    {
        return res;
    }
    QString sql = QString(R"(
                          select name from user_info
                          where id in
                          (
                            select user_id from friend where friend_id (select id from user_info where name = '%1)
                            union
                            select friend_id from friend where user_id (select id from user_info where name = '%1)
                          )
                          and online = 1
                          )").arg(caCurName);
    QSqlQuery q;
    q.exec(sql);
    while (q.next())
    {
        res.append(q.value(0).toString());
    }
    return res;
}

OperateDB::OperateDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
}
