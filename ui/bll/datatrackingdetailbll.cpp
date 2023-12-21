#include "datatrackingdetailbll.h"

DataTrackingDetailBLL::DataTrackingDetailBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

int DataTrackingDetailBLL::addDetail(int dataTrakingID, int nameID, QString originalVal, QString modifyVal, QString nameVar)
{
    int lastID = dal.getLastID()+1;
    QMap<QString,QString>mapList;
    mapList.insert(fieldList.at(DATATRACKID),QString::number(dataTrakingID));
    mapList.insert(fieldList.at(NAMEID),QString::number(nameID));
    mapList.insert(fieldList.at(ORIGINALVALUE),originalVal);
    mapList.insert(fieldList.at(MODIFYVALUE),modifyVal);
    mapList.insert(fieldList.at(NAMEVAR),nameVar);
    mapList.insert(fieldList.at(ID),QString::number(lastID));
    dal.appendRow(mapList,TableDAL::ASYNC);
    return lastID;
}

void DataTrackingDetailBLL::init()
{
    dal.initTable(tableName,fieldList,false);
    dal.refreshLastID();
}
