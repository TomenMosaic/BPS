#ifndef TESTDATADETAILBLL_H
#define TESTDATADETAILBLL_H

#include "tabledal.h"
#include <QObject>
//单次实验结束后插入一条记录
class TestDataDetailBLL : public QObject
{
    Q_OBJECT
public:
    enum TestDataDetailType
    {
        ID,
        TESTID,
        COUNT,
        DATATIME,
        DATA,
        RESERVE1,
        RESERVE2,
        RESERVE3,
        RESERVE4,
        RESERVE5,
        RESERVE6,
        RESERVE7,
        RESERVE8,
        RESERVE9,
        ISUSE
    };
    QStringList fieldList =
    {
        "id",
        "test_id",
        "count",
        "data_time",
        "data",
        "reserve1",
        "reserve2",
        "reserve3",
        "reserve4",
        "reserve5",
        "reserve6",
        "reserve7",
        "reserve8",
        "reserve9",
        "is_use"
    };
    Q_ENUM(TestDataDetailType)
    explicit TestDataDetailBLL(QObject *parent = nullptr);

    void init();
    //重新加载实验
    void reloadTest(int testID);
    //查询某次实验组的全部实验记录
    QList<QVariantList> getDetailRecords();
    //获取某次实验组的行数
    int getUseCount();
    //获取最大的ID，用于插入记录
    int getMaxID();
    //获取某次有用的实验记录（与useCount结合使用）
    QVariantList getOneDetailRecord(int recordIndex);
    //插入单次实验结果
    void addDetailResult(QMap<TestDataDetailType,QString>mapList);
    //清除最后一次实验
    bool removeCurTest();
signals:


private:
    int maxID = 0;
    int useCount = 0;
    QString tableName = "test_data_detail";
    TableDAL dal;
};

#endif // TESTDATADETAILBLL_H
