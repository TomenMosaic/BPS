#include "testdataresultbll.h"
#include "log.h"
#include "Global.h"

TestDataResultBLL::TestDataResultBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

QSharedPointer<Row> TestDataResultBLL::getRecord(int testID)
{
    for(int rowIndex = 0 ;rowIndex<m_tableDAL.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>curRow = m_tableDAL.getRow(rowIndex);
        if(curRow->data(TESTID).toInt()==testID)
            return curRow;
    }
    this->init();
    for(int rowIndex = 0 ;rowIndex<m_tableDAL.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>curRow = m_tableDAL.getRow(rowIndex);
        if(curRow->data(TESTID).toInt()==testID)
            return curRow;
    }
    return QSharedPointer<Row>();
}

void TestDataResultBLL::addRecord(int testID,double gasPremeation,double humid,double flow)
{
    for(int index = 0;index<m_tableDAL.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = m_tableDAL.getRow(index);
        if(curRow->data(TestDataResultType::TESTID).toInt()==testID)
        {
            m_tableDAL.removeRow(curRow);
        }
    }
    QMap<QString,QString> dataMap;
    dataMap.insert(m_fieldList.value(TestDataResultType::TESTID),QString::number(testID));
    dataMap.insert(m_fieldList.value(TestDataResultType::gasPremeation),QString::number(gasPremeation));
    dataMap.insert(m_fieldList.value(TestDataResultType::flow),QString::number(flow));
    dataMap.insert(m_fieldList.value(TestDataResultType::humid),QString::number(humid));
    dataMap.insert(m_fieldList.value(TestDataResultType::DATETIME),TableDAL::currentTime());
    m_tableDAL.appendRow(dataMap, TableDAL::ASYNC);
}

void TestDataResultBLL::addRecord(int testID,double gasPremeation,double humid,double flow,double latency,double diffusion,double solubility,double gasPermeability)
{
    for(int index = 0;index<m_tableDAL.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = m_tableDAL.getRow(index);
        if(curRow->data(TestDataResultType::TESTID).toInt()==testID)
        {
            m_tableDAL.removeRow(curRow);
        }
    }
    QMap<QString,QString> dataMap;
    dataMap.insert(m_fieldList.value(TestDataResultType::TESTID),QString::number(testID));
    dataMap.insert(m_fieldList.value(TestDataResultType::gasPremeation),QString::number(gasPremeation));
    dataMap.insert(m_fieldList.value(TestDataResultType::humid),QString::number(humid));
    dataMap.insert(m_fieldList.value(TestDataResultType::flow),QString::number(flow));
    dataMap.insert(m_fieldList.value(TestDataResultType::DATETIME),TableDAL::currentTime());
    dataMap.insert(m_fieldList.value(TestDataResultType::diffusionCoefficient),QString::number(diffusion));
    dataMap.insert(m_fieldList.value(TestDataResultType::solubilityCoefficient),QString::number(solubility));
    dataMap.insert(m_fieldList.value(TestDataResultType::gasPermeabilityCoefficient),QString::number(gasPermeability));
    dataMap.insert(m_fieldList.value(TestDataResultType::Latency),QString::number(latency));
    m_tableDAL.appendRow(dataMap, TableDAL::ASYNC);
}

void TestDataResultBLL::init()
{
    m_tableDAL.initTable(m_tableName, m_fieldList, true);
}

void TestDataResultBLL::addRecord(QMap<TestDataResultType, QString> mapList)
{
    if(!mapList.contains(TestDataResultType::TESTID))
        return;
    for(int index = 0;index<m_tableDAL.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = m_tableDAL.getRow(index);
        if(curRow->data(TestDataResultType::TESTID).toInt()==mapList.value(TestDataResultType::TESTID))
        {
            m_tableDAL.removeRow(curRow);
        }
    }
    QMap<QString,QString> dataMap;
    auto it = mapList.begin();
    for(;it!=mapList.end();it++)
    {
        dataMap.insert(m_fieldList.at(it.key()), it.value());
    }
    m_tableDAL.appendRow(dataMap, TableDAL::ASYNC);
}

int TestDataResultBLL::addReportPic(int testID, QVariant variant, QString reportPath)
{
    qDebug()<<Q_FUNC_INFO<<__LINE__<<"begin"<<testID;
    QMap<QString,QVariant>mapList;
    mapList.insert(m_fieldList.at(TestDataResultType::REPORTPIC),variant);
    mapList.insert(m_fieldList.at(TestDataResultType::TESTID),testID);
    mapList.insert(m_fieldList.at(TestDataResultType::reportPath),reportPath);
     mapList.insert(m_fieldList.at(TestDataResultType::DATETIME),TableDAL::currentTime());
    for(int rowIndex = 0;rowIndex<m_tableDAL.getRowCount();rowIndex++)
    {
        QSharedPointer<Row> curRow = m_tableDAL.getRow(rowIndex);
        if(curRow->data(TestDataResultType::TESTID).toInt()==testID)
        {
            mapList.insert(m_fieldList.value(TestDataResultType::gasPremeation),curRow->data(TestDataResultType::gasPremeation));
            mapList.insert(m_fieldList.value(TestDataResultType::humid),curRow->data(TestDataResultType::humid));
            mapList.insert(m_fieldList.value(TestDataResultType::flow),curRow->data(TestDataResultType::flow));
//            m_tableDAL.removeRow(curRow);
        }
    }

    bool ok = m_tableDAL.appendRow(mapList);
    if(ok)
        return m_tableDAL.getLastID();
    else
        return -1;
//    QMap<QString,QVariant>mapList;
//    mapList.insert(m_fieldList.at(TestDataResultType::TESTID),testID);
//    mapList.insert(m_fieldList.at(TestDataResultType::REPORTPIC),variant);
//    mapList.insert(m_fieldList.at(TestDataResultType::DATETIME),TableDAL::currentTime());
//    bool ok = m_tableDAL.appendRow(mapList);
//    if(ok)
//        return m_tableDAL.getLastID();
//    else
//        return -1;
}

QSharedPointer<Row> TestDataResultBLL::getReportPic(int testID)
{
    QSharedPointer<Row> result;
    for(int index = 0;index<m_tableDAL.getRowCount();index++)
    {
       QSharedPointer<Row> curRow = m_tableDAL.getRow(index);
        if(curRow->data(TESTID).toInt()==testID)
            result = curRow;
    }
    return result;
}
