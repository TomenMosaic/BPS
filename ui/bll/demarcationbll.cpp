#include "demarcationbll.h"
#include <QMetaEnum>
#include <QDebug>
DemarcationBLL::DemarcationBLL(QObject *parent) : QObject(parent)
{
    QMetaEnum enumList = QMetaEnum::fromType<DemarcationBLL::DemarcationType>();
    for(int i = 0;i<enumList.keyCount();++i)
    {
        fieldList.append(enumList.key(i));
    }
    this->init();
}

void DemarcationBLL::init()
{
    dal.initTable(tableName,fieldList,true);
    freshLastID();
}



QList<QSharedPointer<Row> > DemarcationBLL::getRowListByID(int schemeID)
{
    QList<QSharedPointer<Row> > rowList;
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(DemarcationBLL::schemeID).toInt()==schemeID)
            rowList.append(curRow);
    }
    return rowList;
}

void DemarcationBLL::removeScheme(int schemeID)
{
    for(int index = dal.getRowCount()-1;index>=0;index--)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(DemarcationBLL::schemeID).toInt()==schemeID)
            dal.removeRow(curRow);
    }
}

void DemarcationBLL::removeRow(int id)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(DemarcationBLL::id).toInt()==id)
            dal.removeRow(curRow);
    }
}

int DemarcationBLL::addRecord(QString operationTime, double transmittance, double pressureVariation, double shiftValue, int schemeID)
{
    int lastID = dal.getLastID();
    lastID++;
    QMap<QString,QString>valMap;
    valMap.insert(fieldList.at(DemarcationBLL::id),QString::number(lastID));
    valMap.insert(fieldList.at(DemarcationBLL::operationTime),operationTime);
    valMap.insert(fieldList.at(DemarcationBLL::transmittance),QString::number(transmittance));
    valMap.insert(fieldList.at(DemarcationBLL::pressureVariation),QString::number(pressureVariation));
    valMap.insert(fieldList.at(DemarcationBLL::shifting),QString::number(shiftValue));
    valMap.insert(fieldList.at(DemarcationBLL::schemeID),QString::number(schemeID));
    dal.appendRow(valMap);
    return lastID;
}

bool DemarcationBLL::modifyRecord(double transmittance, double pressureVariation, double shiftValue, int id)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(DemarcationBLL::id).toInt()==id)
        {
            curRow->setData(DemarcationBLL::transmittance,transmittance);
            curRow->setData(DemarcationBLL::pressureVariation,pressureVariation);
            curRow->setData(DemarcationBLL::shifting,shiftValue);

            dal.update(curRow);
            return true;
        }
    }
    return false;
}

void DemarcationBLL::freshLastID()
{
    newId = 0;
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(id).toInt();
        newId = qMax(curID,newId);
    }
}


