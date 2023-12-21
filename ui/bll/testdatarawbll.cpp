#include "testdatarawbll.h"
#include "Global.h"
TestDataRawBLL::TestDataRawBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void TestDataRawBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}

void TestDataRawBLL::addRecord(QMap<TestDataRawType, QString> mapList)
{
    QMap<QString,QString> dataMap;
    for( auto it = mapList.begin();it!=mapList.end();it++)
    {
        dataMap.insert(fieldList.at(it.key()),it.value());
    }
    dal.appendRow(dataMap,TableDAL::ASYNC);
}

QList<QSharedPointer<Row> > TestDataRawBLL::getRecord(int testID)
{
    QStringList conditions;
    conditions.append(QString("%1 = '%2'").arg(fieldList.at(testid)).arg(QString::number(testID)));
    dal.initTable(tableName,fieldList,conditions,true);
    return dal.getRowList();
}

QMap<TestDataRawBLL::TestDataRawType,QVariantList> TestDataRawBLL::getRealData(int testID, int count)
{
    QList<TestDataRawBLL::TestDataRawType>colList =
    {
        TestDataRawType::temperature,
        TestDataRawType::humid,
        TestDataRawType::flow,
        TestDataRawType::voltage,
        TestDataRawType::transmittance,
        TestDataRawType::environmentTemprate
    };
    QStringList conditions;
    conditions.append(QString("%1 = '%2'").arg(fieldList.at(testid)).arg(QString::number(testID)));
    conditions.append(QString("%1 = '%2'").arg(fieldList.at(cavity)).arg(QString::number(count)));
    dal.initTable(tableName,fieldList,conditions,true);
    QMap<TestDataRawBLL::TestDataRawType,QVariantList>dataMap;
    for(int colIndex = 0;colIndex<colList.length();colIndex++)
    {
        dataMap.insert(colList.at(colIndex),dal.getColCell(int(colList.at(colIndex))));
    }
    return dataMap;
}

QList<QMap<TestDataRawBLL::TestDataRawType, QString> > TestDataRawBLL::getRealDataForRow(int testID, int count)
{
    QList<TestDataRawBLL::TestDataRawType>colList =
    {
        TestDataRawType::temperature,
        TestDataRawType::humid,
        TestDataRawType::flow,
        TestDataRawType::voltage,
        TestDataRawType::transmittance,
        TestDataRawType::environmentTemprate
    };
    QStringList conditions;
    conditions.append(QString("%1 = '%2'").arg(fieldList.at(testID)).arg(QString::number(testID)));
    conditions.append(QString("%1 = '%2'").arg(fieldList.at(count)).arg(QString::number(count)));
    dal.initTable(tableName,fieldList,conditions,true);
    QList<QMap<TestDataRawBLL::TestDataRawType, QString>>dataList;
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        QMap<TestDataRawBLL::TestDataRawType, QString> map;
        for(int colIndex = 0;colIndex<colList.length();colIndex++)
        {
            map.insert(colList.at(colIndex),VarToString(row->data(colList.at(colIndex))));
        }
        dataList.append(map);
    }
    return dataList;
}

void TestDataRawBLL::setRealDataMaxCount(int testID, int maxTestCount)
{
    QStringList conditions;
    conditions.append(QString("%1 = '%2'").arg(fieldList.at(testID)).arg(QString::number(testID)));
    conditions.append(QString("%1 > '%2'").arg(fieldList.at(cavity)).arg(QString::number(maxTestCount)));
    dal.remove(conditions);
    CLOG_INFO(QString("remove testDataRaw where conditions:%1").arg(conditions.join(" and ")).toUtf8());
}
