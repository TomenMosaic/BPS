#ifndef TESTSETTINGPARAMBLL_H
#define TESTSETTINGPARAMBLL_H

#include "tabledal.h"
#include <QJsonObject>
#include <QObject>

class TestSettingParamBLL : public QObject
{
    Q_OBJECT
public:
    enum TestSettingParam
    {
        ID,
        TOTALCOUNT,
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
    QStringList fieldList =
    {
        "id",
        "TotalCount",
        "distancecorrection",
        "forcecorrection",
        "fracturecondition",
        "gaugelength",
        "gobacktype",
        "judgetype",
        "speed",
        "speedcorrection",
        "thickness",
        "width",
        "experimentType"
    };
    Q_ENUM(TestSettingParam)
    explicit TestSettingParamBLL(QObject *parent = nullptr);

    void init();
    //判断系统设置参数是否为空
    bool hasParam();
    //插入系统参数
    void addParam(QMap<TestSettingParamBLL::TestSettingParam, QVariant> mapList);
    //获取最新的参数ID
    int  getLastID();
    //获取最新的参数
    QMap<TestSettingParam,QVariant> getCurrentEdit();
    //获取指定的参数
    QMap<TestSettingParam,QVariant> getSettingParam(int settingID);

    QJsonObject getReportData(int settingID);
private:
    void refreshLastID();


signals:


private:
    QSharedPointer<Row>currentRow = nullptr;
    int newId = 0;
    TableDAL dal;
    QString tableName = "test_setting_param";
};

#endif // TESTSETTINGPARAMBLL_H
