#include "systemtrackbll.h"
#include "Global.h"
#include "tabledal.h"
SystemtrackBLL::SystemtrackBLL(QObject *parent)
    : QObject{parent}
{

}

QList<QStringList> SystemtrackBLL::query(QDateTime beginTime, QDateTime endTime, QString userName, int beginNum, int endNum)
{
    TableDAL dal;
    QStringList conditions;
    QStringList fieldList;
    QVariant val = g_sqlconfig->getRecord("key1");
    fieldList = val.toStringList();
    if(fieldList.isEmpty())
    {
        fieldList = QStringList{"id","user_name","operation_id","operation_time","action_id","isdetails","detail"};
        g_sqlconfig->addRecord("key1",fieldList);
    }
    fieldList = QStringList{"id","user_name","operation_id","operation_time","action_id","isdetails","detail"};
    QString condition = QString("operation_time between '%1 00:00:00'and '%2 23:59:59'").arg(beginTime.toString("yyyy-MM-dd")).arg(endTime.toString("yyyy-MM-dd"));
    conditions.append(condition);
    if(!userName.isEmpty())
    {
        conditions.append(QString("user_name like '%%1%'").arg(userName));
    }
    conditions.append("TRIM(test_name)='' order by operation_time desc");

    dal.initTable("data_tracking",fieldList,conditions,beginNum,endNum,true);
    return dal.getStrList();
}

int SystemtrackBLL::queryCount(QDateTime beginTime, QDateTime endTime, QString userName)
{
    TableDAL dal;
    QStringList conditions;
    QString condition = QString("operation_time between '%1 00:00:00'and '%2 23:59:59'").arg(beginTime.toString("yyyy-MM-dd")).arg(endTime.toString("yyyy-MM-dd"));
    conditions.append(condition);
    if(!userName.isEmpty())
    {
        conditions.append(QString("user_name like '%%1%'").arg(userName));
    }
    conditions.append("TRIM(test_name)=''order by operation_time desc");

    int count = dal.queryCount("data_tracking",conditions);
    return count;
}

QList<QStringList> SystemtrackBLL::getDetail(int roleID, int detailType)
{
    QString tableName = "";
    QStringList conditions;
    QStringList fieldList;
    TableDAL dal;
    conditions.append(QString("data_tracking_id = %1").arg(roleID));
    if(detailType==1)
    {
        tableName = "data_tracking_details";
        QVariant val = g_sqlconfig->getRecord("key2");
        fieldList = val.toStringList();
        if(fieldList.isEmpty())
        {
            fieldList = QStringList{"name_id","original_value","modify_value"};
            g_sqlconfig->addRecord("key2",fieldList);
        }
    }
    else
    {
        tableName = "data_tracking_treedetails";
        fieldList = QStringList{
                "value","code","value_translate","parent_code","data_type"
    };
    }
    dal.initTable(tableName,fieldList,conditions,true);
    return dal.getStrList();

}
