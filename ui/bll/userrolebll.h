#ifndef USERROLEBLL_H
#define USERROLEBLL_H

#include "tabledal.h"
#include <QObject>

class UserRoleBLL : public QObject
{
    Q_OBJECT
public:
    enum UserRoleType
    {
        ID,
        USERID,
        ROLEID,
        CREATED,
        CREATOR,
        EDITED,
        EDITOR,
        DELETED
    };
    QStringList fieldList =
    {
        "id",
        "user_id",
        "role_id",
        "created",
        "creator",
        "edited",
        "editor",
        "deleted"
    };
    static UserRoleBLL *getInstance(QObject *parent = nullptr);


    void addRole(int userID,int roleID);

    void removeRole(int userID,int roleID);

    QList<int> getRoleList(int userID);

    void removeUser(int userID);

    QList<int> getUserListForRole(int roleID);

    void init();

signals:

private:
private:
    explicit UserRoleBLL(QObject *parent = nullptr);

    static UserRoleBLL *m_userRole;

    QString tableName = "user_role";
    TableDAL dal;
};

#endif // USERROLEBLL_H
