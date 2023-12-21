#ifndef USERBLL_H
#define USERBLL_H

#include <QObject>
#include "qdatetime.h"
#include "tabledal.h"
class UserBLL : public QObject
{
    Q_OBJECT
public:
    enum UserType
    {
        USERID,
        USERNAME, //名称
        PASSWORD, //密码
        RSREMEBERPWD,//记住密码
        REGISETERTIME,//注册时间
        MODIFYTIME, //修改密码时间
        DEFAULTTIMES,//密码输入错误次数
        ISUSE, //是否启用
        TRYLOGINTIME//尝试登录时间
    };
    QStringList fieldList =
    {
        "userID",
        "userName",
        "userPassword",
        "rememberPassword",
        "registerTime",
        "modifyTime",
        "defaultTimes",
        "isuse",
        "tryloginTime"
    };
    static UserBLL *getInstance(QObject *parent = nullptr);

    bool changePwd(QString userName,QString password,QString newPassword);

    bool login(QString userName,QString password,bool remeberPwd);

    bool checkPwd(QString userName,QString password);

    int registerUser(QString userName, QString password, int userState =1);

    ~UserBLL();

    QList<int> getIDList();

    QStringList getUserNameList();

    QStringList getPasswordList();

    QList<bool> getRemeberPWDList();

    void init();

    void setCurrentUser(QSharedPointer<Row>currentUser);

    QVariant getCurrentVar(UserType type);

    QList<QSharedPointer<Row>> getRowList();

    QSharedPointer<Row> getUser(int userID);

    void updateUser(QSharedPointer<Row>user);

    void removeUser(QSharedPointer<Row>user);
private:
    explicit UserBLL(QObject *parent = nullptr);

signals:

private:
    static UserBLL *m_userBll;

    QSharedPointer<Row>currentUser = nullptr;

    TableDAL dal;
};

#endif // USERBLL_H
