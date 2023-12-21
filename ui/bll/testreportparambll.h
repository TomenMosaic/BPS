#ifndef TESTREPORTPARAMBLL_H
#define TESTREPORTPARAMBLL_H

#include "tabledal.h"
#include "GlobalVar.h"
#include <QJsonObject>
#include <QObject>
#include <QShowEvent>
class TestReportParamBLL : public QObject
{
    Q_OBJECT
public:
    enum TestReportParamType
    {
        SAMPLETYPE,//样品类型
        AREA,//面积
        Thickness,//样品厚度
        ChamberVolume,//低压室体积
        TEMPERATURE,//温度
        TESTUNIT,//测试单位
        SAMPLENAME,//样品名称
        TESTSTANDARD,//测试标准
        SAMPLESOURCE,//样品来源
        SAMPLEBATCHNUMBER,//检品批号
        SAMPLESPECIFICATION,//样品规格
        ID,
        ShemeId,
        cavityType,
    };
    QStringList fieldList =
    {
        "SampleType",
        "Area",
        "Thickness",
        "ChamberVolume",
        "Temperature",
        "TestUnit",
        "SampleName",
        "TestStandard",
        "SampleSource",
        "SampleBatchNumber",
        "SampleSpecification",
        "id",
        "ShemeId",
        "cavityType",
    };
    Q_ENUM(TestReportParamType)
    explicit TestReportParamBLL(QObject *parent = nullptr);

    void init();

    bool isEmpty();

    QVariantList getLastReportParam();

    QSharedPointer<Row>getReportData(int reportID);

    QMap<TestReportParamType,QString> getReportMap(int reportID);

    QMap<TestReportParamType,QString> getLastReportMap();
    //插入报告，并重新查询，并获取ID
    int addReportParam(QMap<TestReportParamType,QString>dataMap);
    //判断报告ID是否存在
    bool hasReport(int reportID);
    //获取最新的报告ID
    int getLastReportID();

    QJsonObject getReportParam(int reportID);

    QSharedPointer<Row>getSchemeRecord(int schemeID,Cavity cavity);

    void updateReportParam(int schemeID,int cavityId = -1);
private:
    void refreshLastID();


signals:

private:
    int newId = 0;
    QString tableName = "test_report_param";
    TableDAL dal;
};

#endif // TESTREPORTPARAMBLL_H
