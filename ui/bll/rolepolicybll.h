#ifndef ROLEPOLICYBLL_H
#define ROLEPOLICYBLL_H

#include <QObject>
#include "tabledal.h"
class RolePolicyBLL : public QObject
{
    Q_OBJECT
public:
    enum RolePolicyType
    {
        id,
        roleID,
        useTime,
        defaultTimes,
        lockTime
    };
    QStringList fieldList =
    {
        "id",
        "roleID",
        "useTime",
        "defaultTimes",
        "lockTime"
    };
    explicit RolePolicyBLL(QObject *parent = nullptr);

    void init();

    QMap<int,QSharedPointer<Row>> getRolePolicy(QList<int> roleIDs);

    void addRolePolicy(int roleID);

    QSharedPointer<Row> getRolePolicy(int roleID);

    bool savePolicy(int roleID,int useTime,int defaultTime,QString lockTime);

    void initRoleIDs(QList<int> roleIDs);
private:
    QMap<int,QSharedPointer<Row>> curRowList;
    QString tableName = "role_policy";
    TableDAL m_dal;
signals:

};

#endif // ROLEPOLICYBLL_H
