#include "datatrackingbll.h"
#include <QDebug>
DataTrackingBLL::DataTrackingBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void DataTrackingBLL::init()
{
    dal.initTable(tableName,fieldList,false);
//    refreshLastID();
}

int DataTrackingBLL::addRecord(QString userName, QString equipmentNumber, QString testName, int operationID, int actionID, int isdetails, int remarkID, int resultID, QString detail)
{
//    newId++;
    QMap<QString,QString> mapList;
    mapList.insert(fieldList.at(USERNAME),userName);
    mapList.insert(fieldList.at(EQUIPMENT),equipmentNumber);
    mapList.insert(fieldList.at(TESTNAME),testName);
    mapList.insert(fieldList.at(OPERATIONID),QString::number(operationID));
    mapList.insert(fieldList.at(ISDETAILS),QString::number(isdetails));
    mapList.insert(fieldList.at(ACTIONID),QString::number(actionID));
//    mapList.insert(fieldList.at(ID),QString::number(newId));
    mapList.insert(fieldList.at(OPERATIONTIME),TableDAL::currentTime());
    mapList.insert(fieldList.at(RESULTID),QString::number(resultID));
    mapList.insert(fieldList.at(REMARK),QString::number(remarkID));
    mapList.insert(fieldList.at(DETAIL),detail);
    dal.appendRow(mapList,TableDAL::ASYNC);
    return newId;
}

int DataTrackingBLL::addRecord(QString userName, int operationID, int actionID, int isdetails)
{
//    newId++;
    QMap<QString,QString> mapList;
    mapList.insert(fieldList.at(USERNAME),userName);
    mapList.insert(fieldList.at(OPERATIONID),QString::number(operationID));
    mapList.insert(fieldList.at(DETAIL),QString::number(isdetails));
    mapList.insert(fieldList.at(ACTIONID),QString::number(actionID));
//    mapList.insert(fieldList.at(ID),QString::number(newId));
    mapList.insert(fieldList.at(OPERATIONTIME),TableDAL::currentTime());
    dal.appendRow(mapList,TableDAL::ASYNC);
    return newId;
}

void DataTrackingBLL::refreshLastID()
{
    newId = 0;
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(ID).toInt();
        if(curID>newId)
        {
            newId = curID;
        }
    }
}

