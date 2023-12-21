#include "rolepolicybll.h"
#include <QDebug>
#include "log.h"
RolePolicyBLL::RolePolicyBLL(QObject *parent) : QObject(parent)
{
    init();
}

void RolePolicyBLL::init()
{
    m_dal.initTable(tableName,fieldList,true);
    for(int index = 0;index<m_dal.getRowCount();index++)
    {
        QSharedPointer<Row> row = m_dal.getRow(index);
        curRowList.insert(row->data(fieldList.at(RolePolicyBLL::roleID)).toInt(),row);
    }

}

QMap<int, QSharedPointer<Row>> RolePolicyBLL::getRolePolicy(QList<int> roleIDs)
{
    QMap<int, QSharedPointer<Row>>resultList;
    foreach(int roleID,roleIDs)
    {
        if(!curRowList.contains(roleID))
        {
            addRolePolicy(roleID);
            resultList.insert(roleID,curRowList.value(roleID));
        }
        else
        {
            resultList.insert(roleID,curRowList.value(roleID));
        }
    }
    curRowList=resultList;
    return resultList;
}

void RolePolicyBLL::addRolePolicy(int roleID)
{
    QMap<QString,QString> mapList = {{"roleID",QString::number(roleID)},
                                     {"useTime","60"},{"defaultTimes","3"},
                                     {"lockTime","00:10:00"}};
    m_dal.appendRow(mapList,TableDAL::SYNC);

    qDebug()<<Q_FUNC_INFO<<__LINE__<<" roleID:"<<roleID;
    init();
}

QSharedPointer<Row> RolePolicyBLL::getRolePolicy(int roleID)
{
    qDebug()<<Q_FUNC_INFO<<__LINE__<<" roleID:"<<roleID;
    if(curRowList.contains(roleID))
    {
        return curRowList.value(roleID);
    }
    else
    {
        addRolePolicy(roleID);
        qDebug()<<Q_FUNC_INFO<<__LINE__<<" row valueList:"<<curRowList.keys();
        return curRowList.value(roleID);
    }
}

bool RolePolicyBLL::savePolicy(int roleID, int useTime, int defaultTime, QString lockTime)
{
    for(int index =0;index<m_dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = m_dal.getRow(index);
        if(curRow->data(fieldList.at(RolePolicyBLL::roleID)).toInt()==roleID)
        {
            curRow->setData(RolePolicyBLL::useTime,useTime);
            curRow->setData(RolePolicyBLL::defaultTimes,defaultTime);
            curRow->setData(RolePolicyBLL::lockTime,lockTime);
            m_dal.update(curRow);
            return true;
        }
    }
    CLOG_ERROR(QString("save policy failed:%1").arg(roleID).toUtf8());
    return false;
}

void RolePolicyBLL::initRoleIDs(QList<int> roleIDs)
{
    getRolePolicy(roleIDs);
    QList<QSharedPointer<Row>>rowList = m_dal.getRowList();
    QList<QSharedPointer<Row>>useRowList = curRowList.values();
    for(int index = 0;index<rowList.length();index++)
    {
        if(!useRowList.contains(rowList.at(index)))
            m_dal.removeRow(rowList.at(index));
    }
}
