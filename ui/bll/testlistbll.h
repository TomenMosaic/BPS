#ifndef TESTLISTBLL_H
#define TESTLISTBLL_H

#include "tabledal.h"
#include "GlobalVar.h"
#include <QObject>
#include <QJsonObject>
//保存每组实验的结果，每次开始一组实验时插入记录，结束时修改实验组的结束时间
class TestListBLL : public QObject
{
    Q_OBJECT
public:
    enum ReportState{
        Ready = 0,
        Run = 1,
        TestFinished =2,
        AddSignature =3,
        BuildReport = 4,
        DefaultError = 9
    };
    enum TestListType
    {
        ID,
        TESTNAME,
        TESTTIME,
        TESTISCOMPLETE,
        TESTCOMPLETETIME,
        ACCOUNT,
        TESTUNIT,
        REMARKS,
        PRINTED,
        RUNMODE,
        RUNACTION,
        CAVCOUNT,
        TOTALCOUNT,
        PARAMID,
        REPORTID,
        FLOWID,
        TASKID,
        ISUSE
    };
    QStringList fieldList=
    {
        "id",
        "test_name",
        "test_time",
        "test_is_complete",
        "test_complete_time",
        "account",
        "test_unit",
        "remarks",
        "printed",
        "run_mode",
        "run_action",
        "cavCount",
        "totalCount",
        "paramId",
        "reportId",
        "flowId",
        "taskId",
        "is_use"
    };

    static TestListBLL *getInstance(QObject *parent = nullptr);


    //初始化
    void init();
    //插入记录
    int addRecord(QMap<TestListType,QString> valMap);
    //获取对应的实验记录
    QSharedPointer<Row>getRecord(int testID);
    //获取对应的实验记录
    QSharedPointer<Row>getRecord(QString testName);
    //完成实验
    bool finishTest(int testID,bool success);
    //完成报告生成
    bool finishReport(int testID);

    //当前实验组是否完成
    bool isFinish();
    //获取当前实验
    QSharedPointer<Row>getCurrentTset();
    //结束当前实验组的实验
//    bool finishTest(int totalCount);
    //切换到下一实验组
//    int nextTest(QString userName,int reportID,int settingID, int runAction = 1);
    //设置生成报告标记

    int lastID();

    bool setReportID(int reportID);

    bool removeTest(int testID);

    bool blockTest(int testID);
//    bool blockTest();
//    //清除本次实验
//    bool removeCurTest();
//    //获取系统设置ID
//    int getSettingID(int testID =-1);
    //获取报告生成的参数内容
    QJsonObject getReportData(int testID);

    int getReportID(int testID = -1);

    QString testName(int testID = -1);

    bool getParam(QString testName,int& flowID,int& reportID);

    QString testTime(int testID = -1);

    QList<QSharedPointer<Row>> query(QDateTime beginTime,QDateTime endTime);

    QList<QStringList>query(QDateTime beginTime, QDateTime endTime, QString userName, QString testName,QList<int>idList={
            ID,ACCOUNT,TESTNAME,TESTTIME,TESTCOMPLETETIME,TESTISCOMPLETE
    });
//    bool finishTest(int testID,int testCount);

private:
     explicit TestListBLL(QObject *parent = nullptr);
    void freshLastID();
signals:

private:
    static TestListBLL *m_testListBll;
    int newId = 0;
    QSharedPointer<Row>currentTest = nullptr;
    QString tableName = "test_list";
    TableDAL dal;
};

#endif // TESTLISTBLL_H
