#include "datatrackingtreedetailbll.h"

DataTrackingTreeDetailBLL::DataTrackingTreeDetailBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void DataTrackingTreeDetailBLL::addTreeRecord(int dataTrackID, QString value, int code, bool valueTranslate, int dataType,int parentCode)
{
    QMap<QString,QString>mapList;
    mapList.insert(fieldList.at(DATATRACKINGID),QString::number(dataTrackID));
    mapList.insert(fieldList.at(VALUE),value);
    mapList.insert(fieldList.at(CODE),QString::number(code));
    mapList.insert(fieldList.at(VALUETRANSLATE),QString::number(valueTranslate));
    mapList.insert(fieldList.at(DATATYPE),QString::number(dataType));
    mapList.insert(fieldList.at(PARENDCODE),QString::number(parentCode));
    dal.appendRow(mapList,TableDAL::ASYNC);
}

void DataTrackingTreeDetailBLL::init()
{
    dal.initTable(tableName,fieldList,false);
    dal.refreshLastID();
}
