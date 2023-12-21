#include "testdatadetailbll.h"
#include "log.h"
#include <QDebug>

TestDataDetailBLL::TestDataDetailBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void TestDataDetailBLL::init()
{
    dal.initTable(tableName,fieldList,false);
    useCount = 0;
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>row = dal.getRow(index);
        if(row->data(ISUSE).toInt()==1)
        {
            useCount++; 
        }
    }
    maxID = dal.getRowCount();

}

void TestDataDetailBLL::reloadTest(int testID)
{
    QStringList conditions;
    conditions.append(QString("%1 = '%2'").arg(fieldList.at(TESTID)).arg(QString::number(testID)));
    dal.initTable(tableName,fieldList,conditions,true);
    useCount = 0;
    maxID = 0;
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>row = dal.getRow(index);
        if(row->data(ISUSE).toInt()==1)
        {
            useCount++;
        }
        maxID++;
    }
}

QList<QVariantList> TestDataDetailBLL::getDetailRecords()
{
    QList<int> colList =
    {
        COUNT,
        RESERVE1,
        RESERVE2,
        RESERVE3,
        RESERVE4,
        RESERVE5,
        RESERVE6,
        RESERVE7,
        RESERVE8,
        RESERVE9
    };
    return dal.getVarList(colList);
}

int TestDataDetailBLL::getUseCount()
{
    return useCount;
}

int TestDataDetailBLL::getMaxID()
{
    return maxID;
}

QVariantList TestDataDetailBLL::getOneDetailRecord(int recordIndex)
{
    QList<int> colList =
    {
        //        COUNT,
        RESERVE1,
        RESERVE2,
        RESERVE3,
        RESERVE4,
        RESERVE5,
        RESERVE6,
        RESERVE7,
        RESERVE8,
        RESERVE9
    };
    int useIndex = 1;
    for(int testIndex = 0;testIndex<dal.getRowCount();testIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(testIndex);
        if(row->data(TestDataDetailType::ISUSE).toInt()==0)
            continue;
        if(useIndex == recordIndex)
        {
            return row->toVariantList(colList);
        }
        useIndex++;

    }
    CLOG_INFO(QString("testRecord is empty").toUtf8());
    return QVariantList();
}

void TestDataDetailBLL::addDetailResult(QMap<TestDataDetailType, QString> mapList)
{
    QMap<QString,QString> dataMap;
    auto it = mapList.begin();
    for(;it!=mapList.end();it++)
    {
        dataMap.insert(fieldList.at(it.key()),it.value());
    }
    dataMap.insert(fieldList.at(ISUSE),QString::number(1));//置数据有效标志
    dataMap.insert(fieldList.at(DATATIME),TableDAL::currentTime());
    dal.appendRow(dataMap,TableDAL::SYNC);
    init();
}

bool TestDataDetailBLL::removeCurTest()
{
    for(int index = dal.getRowCount()-1;index>=0;index--)
    {
        QSharedPointer<Row>row = dal.getRow(index);
        if(row->data(ISUSE).toInt()==1)
        {
            row->setData(ISUSE,0); //置数据无效标志
            dal.update(row,TableDAL::SYNC);
            init();
            return true;
        }
    }
    return false;
}
