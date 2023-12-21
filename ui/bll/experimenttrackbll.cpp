#include "experimenttrackbll.h"

ExperimentTrackBLL::ExperimentTrackBLL(QObject *parent) : QObject(parent)
{
    QMetaEnum calibrationEnum = QMetaEnum::fromType<ExperimentTrackBLL::ExperimentTracType>();
    for(int i = 0;i<calibrationEnum.keyCount();++i)
    {
        fieldList.append(calibrationEnum.key(i));
    }
}

QList<QSharedPointer<Row> > ExperimentTrackBLL::query(QDateTime beginTime, QDateTime endTime, QString userName, QString testName, int beginNum, int endNum)
{
    TableDAL dal;
    QStringList conditions;
//    QVariant val = g_sqlconfig->getRecord("key4");
//    fieldList = val.toStringList();
//    if(fieldList.isEmpty())
//    {
//        fieldList = QStringList{"id","user_name","test_name","operation_id","operation_time","action_id","remark"};
//        g_sqlconfig->addRecord("key4",fieldList);
//    }
    QString condition = QString("operation_time between '%1 00:00:00'and '%2 23:59:59'").arg(beginTime.toString("yyyy-MM-dd")).arg(endTime.toString("yyyy-MM-dd"));
    conditions.append(condition);
    if(!testName.isEmpty())
    {
        conditions.append(QString("test_name like '%%1%'").arg(testName));
    }
    if(!userName.isEmpty())
    {
        conditions.append(QString("user_name like '%%1%'").arg(userName));
    }
    conditions.append("TRIM(test_name)!='' order by id desc");
    dal.initTable("data_tracking",fieldList,conditions,beginNum,endNum,true);
    return dal.getRowList();
}

//QList<QStringList> ExperimentTrackBLL::query(QDateTime beginTime, QDateTime endTime, QString userName, QString testName,int beginNum,int endNum)
//{
//    TableDAL dal;
//    QStringList conditions;
//    QStringList fieldList;
//    QVariant val = g_sqlconfig->getRecord("key4");
//    fieldList = val.toStringList();
//    if(fieldList.isEmpty())
//    {
//        fieldList = QStringList{"id","user_name","test_name","operation_id","operation_time","action_id","remark"};
//        g_sqlconfig->addRecord("key4",fieldList);
//    }
//    QString condition = QString("operation_time between '%1 00:00:00'and '%2 23:59:59'").arg(beginTime.toString("yyyy-MM-dd")).arg(endTime.toString("yyyy-MM-dd"));
//    conditions.append(condition);
//    if(!testName.isEmpty())
//    {
//        conditions.append(QString("test_name like '%%1%'").arg(testName));
//    }
//    if(!userName.isEmpty())
//    {
//        conditions.append(QString("user_name like '%%1%'").arg(userName));
//    }
//    conditions.append("TRIM(test_name)!='' order by operation_time desc");
//    dal.initTable("data_tracking",fieldList,conditions,beginNum,endNum,true);
//    return dal.getStrList();
//}

int ExperimentTrackBLL::queryCount(QDateTime beginTime, QDateTime endTime, QString userName, QString testName)
{
    TableDAL dal;
    QStringList conditions;
    QVariant val = g_sqlconfig->getRecord("key4");
    QString condition = QString("operation_time between '%1 00:00:00'and '%2 23:59:59'").arg(beginTime.toString("yyyy-MM-dd")).arg(endTime.toString("yyyy-MM-dd"));
    conditions.append(condition);
    {
        conditions.append(QString("test_name like '%%1%'").arg(testName));
    }
    if(!userName.isEmpty())
    {
        conditions.append(QString("user_name like '%%1%'").arg(userName));
    }
    conditions.append("TRIM(test_name)!=''");
    if(!testName.isEmpty())
    {
        conditions.append(QString("test_name like '%%1%'").arg(testName));
    }
    return dal.queryCount("data_tracking",conditions);
}
