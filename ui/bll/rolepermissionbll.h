#ifndef ROLEPERMISSIONBLL_H
#define ROLEPERMISSIONBLL_H

#include "tabledal.h"
#include <QObject>

class RolePermissionBLL : public QObject
{
    Q_OBJECT
public:
    enum RolePermissionType
    {
        ID,
        ROLEID,
        PERMISSIONID,
        CREATED,
        CREATOR,
        EDITED,
        EDITOR,
        DELETED
    };
    QStringList fieldList =
    {
        "id",
        "role_id",
        "permission_id",
        "created",
        "creator",
        "edited",
        "editor",
        "deleted"
    };
    static RolePermissionBLL *getInstance(QObject *parent = nullptr);


    void init();

    void addPermission(int roleID,QList<int> permissionIDs);

    void removePermission(int roleID,QList<int> permissionIDs);

    QList<int> getRolePermissions(int roleID);

    void resetPermission(int roleID,QList<int>permissionIDs);

    void removePermission(int roleID);
signals:
private:
    static RolePermissionBLL *m_rolePermissionBll;
private:
    explicit RolePermissionBLL(QObject *parent = nullptr);

    QString tableName = "role_permission";
    TableDAL dal;
};

#endif // ROLEPERMISSIONBLL_H
