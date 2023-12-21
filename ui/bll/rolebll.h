#ifndef ROLEBLL_H
#define ROLEBLL_H

#include "tabledal.h"
#include <QObject>

class RoleBLL : public QObject
{
    Q_OBJECT
public:
    enum RoleType
    {
        ID,
        ParentID,
        CODE,
        NAME,
        INTRO,
        CREATED,
        CREATOR,
        EDITED,
        EDITOR,
        DELETED
    };
    QStringList fieldList =
    {
        "id",
        "parent_id",
        "code",
        "name",
        "intro",
        "created",
        "creator",
        "edited",
        "editor",
        "deleted"
    };
    Q_ENUM(RoleType)
    static RoleBLL *getInstance(QObject *parent = nullptr);


    void init();

    int addRole(QMap<RoleType,QVariant>roleVal);

    bool removeRole(int id);

    QList<QStringList> getAllRole(QList<RoleType> tyleList);

    QList<QSharedPointer<Row>> getAllRole();

    QList<QSharedPointer<Row>> getRoles(QList<int> roleIDs);

    bool reName(int roleID,QString newName,QString editor = "");

    bool updateRole(int roleID,QMap<RoleType,QString> roleData);

    QStringList getCodeListFromIDs(QList<int>idList);
signals:
private:
    explicit RoleBLL(QObject *parent = nullptr);


private:
    static RoleBLL *m_roleBll;

    TableDAL dal;
    QString tableName = "role";
};

#endif // ROLEBLL_H
