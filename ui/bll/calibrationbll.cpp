#include "calibrationbll.h"
#include <QMetaEnum>

CalibrationBLL::CalibrationBLL(QObject *parent) : QObject(parent)
{
    QMetaEnum calibrationEnum = QMetaEnum::fromType<CalibrationBLL::CalibrationType>();
    for(int i = 0;i<calibrationEnum.keyCount();++i)
    {
        fieldList.append(calibrationEnum.key(i));
    }
}

void CalibrationBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}

QList<QSharedPointer<Row>> CalibrationBLL::getRowList()
{
    return dal.getRowList();
}

bool CalibrationBLL::save(QList<QVariantList> valueList)
{
    //删除表格
    dal.remove();
    for(int index = 0;index<valueList.length();index++)
    {
        QVariantList oneValue = valueList.at(index);
        dal.appendRow(oneValue,TableDAL::SYNC);
    }
    return true;
}
