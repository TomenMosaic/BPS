#include "schemebll.h"
#include <QMetaEnum>
#include <QDebug>
SchemeBLL::SchemeBLL(QObject *parent) : QObject(parent)
{
    QMetaEnum enumList = QMetaEnum::fromType<SchemeBLL::SchemeType>();
    for(int i = 0;i<enumList.keyCount();++i)
    {
        fieldList.append(enumList.key(i));
    }
}

void SchemeBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}

QSharedPointer<Row> SchemeBLL::getScheme(int schemeID)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(id).toInt()==schemeID)
            return curRow;
    }
    return QSharedPointer<Row>();
}

QList<QSharedPointer<Row>> SchemeBLL::getRowList()
{
    return dal.getRowList();
}

QStringList SchemeBLL::getNameList()
{
    QStringList nameList = TableDAL::toStringList(dal.getColCell(schemeName));
    return nameList;
}

int SchemeBLL::addRecord(QString newName, QString userName)
{
    int lastID = dal.getLastID();
    lastID++;
    QMap<QString,QString> valMap;
    valMap.insert(fieldList.at(id),QString::number(lastID));
    valMap.insert(fieldList.at(schemeName),newName);
    valMap.insert(fieldList.at(account),userName);
    valMap.insert(fieldList.at(time),TableDAL::currentTime());
    valMap.insert(fieldList.at(isdefault),QString::number(lastID==1?true:false));
    dal.appendRow(valMap);
    return  lastID;
}

void SchemeBLL::setDefault(int noID)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>curRow = rowList.at(rowIndex);
        if(curRow->data(id).toInt()==noID)
        {
            curRow->setData(isdefault,true);
            dal.update(curRow);
        }
        else
        {
            curRow->setData(isdefault,false);
            dal.update(curRow);
        }
    }
}

bool SchemeBLL::removeRow(int noID)
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

bool SchemeBLL::reName(QString newName, int noID)
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

int SchemeBLL::getDefaultID()
{
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    if(!rowList.isEmpty())
    {
        for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
        {
            QSharedPointer<Row>curRow = rowList.at(rowIndex);
            if(curRow->data(isdefault).toBool())
                return curRow->data(id).toInt();
        }
    }
    return -1;
}
