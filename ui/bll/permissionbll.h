#ifndef PERMISSIONBLL_H
#define PERMISSIONBLL_H

#include "tabledal.h"
#include <QObject>

class PermissionBLL : public QObject
{
    Q_OBJECT
public:
    enum PermissionType
    {
        ID,
        PARENTID,
        CODE,
        NAME,
        INTRO,
        CATEGORY,
        CREATED,
        CREATOR,
        EDITED,
        EDITOR,
        DELETED
    };
    QStringList fieldList=
    {
        "id",
        "parent_id",
        "code",
        "name",
        "intro",
        "category",
        "created",
        "creator",
        "edited",
        "editor",
        "deleted"
    };
    static PermissionBLL *getInstance(QObject *parent = nullptr);


    void init();

    QStringList getCodeFromIDs(QList<int>idList);

    QStringList getNameFromIDs(QList<int>idList);

    QList<QSharedPointer<Row>> getAllPermissions();

    QList<int> getAllPermissionIDs();

    void addParentIDs(QList<int>&idList);


signals:


private:
    explicit PermissionBLL(QObject *parent = nullptr);
    static PermissionBLL *m_permissionBll;


    QString tableName = "permission";
    TableDAL dal;
};

#endif // PERMISSIONBLL_H
