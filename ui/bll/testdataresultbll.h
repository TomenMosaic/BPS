#ifndef TESTDATARESULTBLL_H
#define TESTDATARESULTBLL_H

#include "tabledal.h"
#include <QObject>

class TestDataResultBLL : public QObject
{
    Q_OBJECT
public:
    enum TestDataResultType
    {
        ID,
        TESTID,
        Latency,//滞后时间
        gasPremeation,
        humid,
        flow,
        reportPath,
        diffusionCoefficient,//扩散系数
        solubilityCoefficient,//溶解度系数
        gasPermeabilityCoefficient,//气体透过系数
        REPORTPIC,
        DATETIME
    };
    QStringList m_fieldList =
    {
        "id",
        "test_id",
        "count",
        "result1",
        "result2",
        "result3",
        "result4",
        "result5",
        "result6",
        "result7",
        "reportPic",
        "data_time"
    };
    Q_ENUM(TestDataResultType)
    explicit TestDataResultBLL(QObject *parent = nullptr);

    QSharedPointer<Row> getRecord(int testID);

    void addRecord(int testID,double gasPremeation,double humid,double flow);
    void addRecord(int testID,double gasPremeation,double humid,double flow,double latency,double diffusion,double solubility,double gasPermeability);

    void init();
    void addRecord(QMap<TestDataResultType, QString> mapList);
    int addReportPic(int testID, QVariant variant,QString reportPath);
    QSharedPointer<Row> getReportPic(int testID);

private:
    QString m_tableName = "test_data_result";
    TableDAL m_tableDAL;
};

#endif // TESTDATARESULTBLL_H
