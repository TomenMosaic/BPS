#include "demarcationschemebll.h"
#include <QMetaEnum>
#include <QDebug>
DemarcaSchemeBLL::DemarcaSchemeBLL(QObject *parent) : QObject(parent)
{
    QMetaEnum enumList = QMetaEnum::fromType<DemarcaSchemeBLL::SchemeType>();
    for(int i = 0;i<enumList.keyCount();++i)
    {
        fieldList.append(enumList.key(i));
    }
    this->init();
}

void DemarcaSchemeBLL::init()
{
    dal.initTable(tableName,fieldList,true);
    freshLastID();
}

QList<QSharedPointer<Row>> DemarcaSchemeBLL::getRowList()
{
    return dal.getRowList();
}

QStringList DemarcaSchemeBLL::getNameList()
{
    QStringList nameList = TableDAL::toStringList(dal.getColCell(schemeName));
    return nameList;
}

int DemarcaSchemeBLL::addRecord(QString newName, QString userName, QString time, int cavityNum)
{
    newId ++;
    QMap<QString,QString> valMap;
    valMap.insert(fieldList.at(id),QString::number(newId));
    valMap.insert(fieldList.at(schemeName),newName);
    valMap.insert(fieldList.at(account),userName);
    valMap.insert(fieldList.at(DemarcaSchemeBLL::time),time);
    valMap.insert(fieldList.at(cavityType),QString::number(cavityNum));
    bool ok = dal.appendRow(valMap);
    if(ok)
        return  newId;
    else
        return -1;
}

void DemarcaSchemeBLL::setCavityDefault(int noID,int cavityNum)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>curRow = rowList.at(rowIndex);
        if(curRow->data(cavityType).toInt()==cavityNum)
        {
            curRow->setData(cavityType,0);
            dal.update(curRow);
        }
        if(curRow->data(id).toInt()==noID)
        {
            curRow->setData(cavityType,cavityNum);
            dal.update(curRow);
        }
        //        else
        //        {
        //            curRow->setData(cavityType,false);
        //            dal.update(curRow);
        //        }
    }
}

bool DemarcaSchemeBLL::removeRow(int noID)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>curRow = rowList.at(rowIndex);
        if(curRow->data(id)==noID)
        {
            dal.removeRow(curRow,TableDAL::ASYNC);
            return true;
        }
    }
    return false;
}

bool DemarcaSchemeBLL::reName(QString newName, int noID)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>curRow = rowList.at(rowIndex);
        if(curRow->data(id)==noID)
        {
            curRow->setData(schemeName,newName);
            dal.update(curRow);
            return true;
        }
    }
    return false;
}

int DemarcaSchemeBLL::getDefaultID(int cavityNum)
{
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    if(!rowList.isEmpty())
    {
        for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
        {
            QSharedPointer<Row>curRow = rowList.at(rowIndex);
            if(curRow->data(cavityType).toInt()==cavityNum)
                return curRow->data(id).toInt();
        }
    }
    return -1;
}

void DemarcaSchemeBLL::freshLastID()
{
    newId = 0;
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(id).toInt();
        newId = qMax(curID,newId);
    }
}
