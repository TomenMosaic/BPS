#include "testsettingparambll.h"
#include "qmetaobject.h"
#include "GlobalVar.h"

TestSettingParamBLL::TestSettingParamBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void TestSettingParamBLL::init()
{
    dal.initTable(tableName,fieldList,true);
    refreshLastID();
}

bool TestSettingParamBLL::hasParam()
{
    return dal.getRowCount()==0?false:true;
}

void TestSettingParamBLL::addParam(QMap<TestSettingParam, QVariant> mapList)
{
    //    int lastID = dal.getLastID()+1;
    if(mapList.contains(TestSettingParam::ID))
        mapList.remove(TestSettingParam::ID);
    QMap<QString,QString> dataMap;
    for(auto it = mapList.begin();it!=mapList.end();it++)
    {
        QString field = fieldList.at(it.key());
        QString value = VarToString(it.value());
        dataMap.insert(field,value);
    }
    //    dataMap.insert(fieldList.at(ID),QString::number(lastID));
    dal.appendRow(dataMap,TableDAL::SYNC);
    init();
}

int TestSettingParamBLL::getLastID()
{
    return newId;
}

QMap<TestSettingParamBLL::TestSettingParam, QVariant> TestSettingParamBLL::getSettingParam(int settingID)
{
    //查找需要的配置参数列
    QSharedPointer<Row> settingRow = nullptr;
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = dal.getRow(index);
        if(curRow->data(ID).toInt()==settingID)
        {
            settingRow = curRow;
            break;
        }
    }
    //并获取对应的参数
    QMap<TestSettingParam,QVariant> dataMap;
    if(settingRow)
    {
        QMetaEnum testSettingParamEnum = QMetaEnum::fromType<TestSettingParamBLL::TestSettingParam>();
        for(int index = 0;index<testSettingParamEnum.keyCount();index++)
        {
            int col = testSettingParamEnum.value(index);
            dataMap.insert(TestSettingParam(col),settingRow->data(col));
        }
    }

    return dataMap;
}

QJsonObject TestSettingParamBLL::getReportData(int settingID)
{
    QJsonObject settingObj;
    QList<TestSettingParamBLL::TestSettingParam>reportList =
    {
        DISTANCECORRECTION,
        FORCECORRECTION,
        FRACTURECONDITION,
        GUAGELENGTH,
        GOBACKTYPE,
        JUDGETYPE,
        SPEED,
        SPEEDCORRECTION,
        THICKNESS,
        WIDTH,
        EXPERIMENTTYPE
    };
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(ID).toInt();
        if(curID==settingID)
        {
            foreach(TestSettingParamBLL::TestSettingParam type,reportList)
            {
                settingObj[fieldList.at(type)] = row->data(type).toJsonValue();
            }
        }
    }
    return settingObj;
}

QMap<TestSettingParamBLL::TestSettingParam, QVariant> TestSettingParamBLL::getCurrentEdit()
{
    QMap<TestSettingParam,QVariant> dataMap;
    if(currentRow)
    {
        QMetaEnum testSettingParamEnum = QMetaEnum::fromType<TestSettingParamBLL::TestSettingParam>();
        for(int index = 0;index<testSettingParamEnum.keyCount();index++)
        {
            int col = testSettingParamEnum.value(index);
            dataMap.insert(TestSettingParam(col),currentRow->data(col));
        }
    }
    return dataMap;
}

void TestSettingParamBLL::refreshLastID()
{
    newId = 0;
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(ID).toInt();
        if(curID>newId)
        {
            currentRow = row;
            newId = curID;
        }
    }
}
