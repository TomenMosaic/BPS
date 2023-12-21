#include "testreportparambll.h"
#include "qmetaobject.h"
#include <QDebug>
TestReportParamBLL::TestReportParamBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void TestReportParamBLL::init()
{
    dal.initTable(tableName,fieldList,true);
    refreshLastID();
}

bool TestReportParamBLL::isEmpty()
{
    return dal.getRowCount()>0?false:true;
}

QVariantList TestReportParamBLL::getLastReportParam()
{
    int maxID = 0;
    QSharedPointer<Row>maxRow = nullptr;
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int id = row->data(ID).toInt();
        if(id >=maxID)
        {
            maxID = id;
            maxRow = row;
        }
    }
    QVariantList valueList;
    if(maxRow)
        valueList = maxRow->toVariantList();
    return valueList;
}

QSharedPointer<Row> TestReportParamBLL::getReportData(int reportID)
{
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>curRow = dal.getRow(rowIndex);
        if(curRow->data(ID).toInt()==reportID)
            return curRow;
    }
    return nullptr;
}

QMap<TestReportParamBLL::TestReportParamType, QString> TestReportParamBLL::getReportMap(int reportID)
{
    QMap<TestReportParamBLL::TestReportParamType,QString> dataMap;

    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        if(row->data(ID).toInt()==reportID)
        {
            QMetaEnum TestReportParamEnum = QMetaEnum::fromType<TestReportParamBLL::TestReportParamType>();
            for(int index = 0;index<TestReportParamEnum.keyCount();index++)
            {
                TestReportParamType key = TestReportParamType(TestReportParamEnum.value(index));
                dataMap.insert(key,VarToString(row->data(key)));
            }
        }
    }
    return dataMap;
}

QMap<TestReportParamBLL::TestReportParamType, QString> TestReportParamBLL::getLastReportMap()
{
    QMap<TestReportParamBLL::TestReportParamType,QString> dataMap;
    int maxID = 0;
    QSharedPointer<Row>maxRow = nullptr;
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int id = row->data(ID).toInt();
        if(id >=maxID)
        {
            maxID = id;
            maxRow = row;
        }
    }
    if(maxRow)
    {
        QMetaEnum TestReportParamEnum = QMetaEnum::fromType<TestReportParamBLL::TestReportParamType>();
        for(int index = 0;index<TestReportParamEnum.keyCount();index++)
        {
            TestReportParamType key = TestReportParamType(TestReportParamEnum.value(index));
            dataMap.insert(key,VarToString(maxRow->data(key)));
        }
    }
    return dataMap;
}

int TestReportParamBLL::addReportParam(QMap<TestReportParamType, QString> dataMap)
{
    QMap<QString,QString> fieldMap;
    for(auto it = dataMap.begin();it!=dataMap.end();it++)
    {
        fieldMap.insert(fieldList.at(int(it.key())),it.value());
    }
//    qDebug()<<Q_FUNC_INFO<<__LINE__<<"fieldMap:"<<fieldMap;
    //同步刷新
    dal.appendRow(fieldMap,TableDAL::SYNC);
    init();
    return newId;
}

bool TestReportParamBLL::hasReport(int reportID)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>row = dal.getRow(index);
        if(row->data(ID)==reportID)
            return true;
    }
    return false;
}

int TestReportParamBLL::getLastReportID()
{
    return newId;
}

QJsonObject TestReportParamBLL::getReportParam(int reportID)
{
    QJsonObject reportObj;
    QList<TestReportParamBLL::TestReportParamType>reportList =
    {
        SAMPLENAME,//样品名称
        SAMPLESOURCE,//样品来源
        SAMPLEBATCHNUMBER,
        TESTUNIT,//测试单位
        TESTSTANDARD,//测试标准
        TEMPERATURE,//温度
        SAMPLESPECIFICATION//样品规格
    };
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(ID).toInt();
        if(curID==reportID)
        {
            foreach(TestReportParamBLL::TestReportParamType type,reportList)
            {
                reportObj[fieldList.at(type)] = row->data(type).toJsonValue();
            }
        }
    }
    return reportObj;
}

QSharedPointer<Row>TestReportParamBLL::getSchemeRecord(int schemeID, Cavity cavity)
{
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>curRow = dal.getRow(rowIndex);
        if(curRow->data(ShemeId).toInt()==schemeID&&Cavity(curRow->data(cavityType).toInt())==cavity)
        {
            return curRow;
        }
    }
    return nullptr;
}

void TestReportParamBLL::updateReportParam(int schemeID, int cavityId)
{
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>curRow = dal.getRow(rowIndex);
        if(curRow->data(ShemeId).toInt()==schemeID&&(cavityId==-1||curRow->data(cavityType).toInt()==cavityId))
        {
            curRow->setData(ShemeId,-1);
            dal.update(curRow,TableDAL::SYNC);
        }
    }

}

void TestReportParamBLL::refreshLastID()
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
