#include "unitlistbll.h"
#include <QMetaEnum>
#include <QDebug>
UnitListBLL::UnitListBLL(QObject *parent) : QObject(parent)
{
    QMetaEnum calibrationEnum = QMetaEnum::fromType<UnitListBLL::UnitType>();
    for(int i = 0;i<calibrationEnum.keyCount();++i)
    {
        fieldList.append(calibrationEnum.key(i));
    }
    init();
}

void UnitListBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}

QList<QSharedPointer<Row>> UnitListBLL::getRowList(int unitID)
{
    QList<QSharedPointer<Row>> rowList;
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(UnitType::unitID).toInt()==unitID)
        {
            rowList.append(curRow);
        }
    }
    return rowList;
}

QList<int> UnitListBLL::getUnitList()
{
    QList<int>unitList;
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        int curUnit = curRow->data(UnitType::unitID).toInt();
        if(!unitList.contains(curUnit))
            unitList.append(curUnit);
    }
    return unitList;
}

double UnitListBLL::getUnitCovertValue(int id)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(UnitListBLL::id).toInt()==id)
        {
            bool ok = true;
            double lastResult = curRow->data(UnitListBLL::unitconverter).toDouble(&ok);
            return lastResult;
        }
    }
    return 1;
}

QSharedPointer<Row> UnitListBLL::getRow(int id)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(UnitListBLL::id).toInt()==id)
        {
            return curRow;
        }
    }
    return nullptr;
}
