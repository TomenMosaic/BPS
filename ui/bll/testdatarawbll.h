#ifndef TESTDATARAWBLL_H
#define TESTDATARAWBLL_H

#include "tabledal.h"
#include <QObject>
//保存实验的实时数据
class TestDataRawBLL : public QObject
{
    Q_OBJECT
public:

    enum TestDataRawType
    {
        id,
        testid,
        cavity,
        index,
        dateTime,
        temperature,
        humid,
        flow,
        voltage,
        transmittance,
        environmentTemprate,
        DATAUNIT
    };
    QStringList fieldList =
    {
        "id",
        "test_id",
        "count",
        "index",
        "data_time",
        "data",
        "reserve1",
        "reserve2",
        "reserve3",
        "reserve4",
        "reserve5",
        "data_unit"
    };

    explicit TestDataRawBLL(QObject *parent = nullptr);

    void init();
    //插入实时数据记录
    void addRecord(QMap<TestDataRawType, QString> mapList);

    QList<QSharedPointer<Row>> getRecord(int testID);
    //查询某次实验实时数据记录
    QMap<TestDataRawBLL::TestDataRawType,QVariantList> getRealData(int testID,int cavity);


    QList<QMap<TestDataRawType, QString> > getRealDataForRow(int testID,int cavity);

    //当存在单次实验开始后，程序异常关闭（实验未结束），软件未统计到该次实验的结果，此时需要移除此次实验实时记录
    void setRealDataMaxCount(int testID,int maxTestCount);

signals:


private:
    QString tableName = "test_data_raw";
    TableDAL dal;
};

#endif // TESTDATARAWBLL_H
