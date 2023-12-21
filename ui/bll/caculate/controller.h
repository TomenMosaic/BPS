#ifndef CONTROLTHREAD_H
#define CONTROLTHREAD_H

#include <QObject>
#include <Global.h>
#include "worker.h"
#include "calibrationalgorithms.h"
#include "testreportparambll.h"
#include "processsettingbll.h"
#include "testlistbll.h"
#include "datatrackingbll.h"
class Controller : public QObject
{
    Q_OBJECT
public:

    static Controller *getInstance(QObject *parent =nullptr);

    QList<FlowType> getCurrentFlowList() const;


    FlowType getCurFlowType() const;

    ~Controller();
    //获取当前流程剩余时间
    int getCurFlowLeftTime();
    //获取总剩余时间
    int getAllLeftTime();

    bool getIsRunning() const;

    void initFlowList(QList<FlowDetail> flowList);

public slots:
    bool start(QList<std::tuple<bool, int, double,int>> runFlags);

    void stop();

    void nextFlow(ChangeFlowMode mode = ChangeFlowMode::manualMode);

signals:
    bool cmdSend(Instruct instruct,Cavity cavity,QVariant value);

    bool cmdRead(Instruct instruct,Cavity cavity);

    void updateCaculateData(Cavity cavity,double pressureDiff,double gasTransmissionRate,
                            double temperature,double upPressure,double downPressure,double ambientTemperature,QDateTime date);

    void dataUpdate(Cavity cavity,QMap<RealDataType,QVariant> data,QDateTime date);

    void flowFinish(FlowType flowtype,bool success);
    void flowBegin(FlowType flowType);
    void testFinish(Cavity cavity);
    void errorMsg(Cavity cavity,FlowDetail::ErrorMsg error);
private slots:
    void onTestFinish(Cavity cavity,bool success);
    void onTimeOut();
    void onReciveData(Instruct instruct,Cavity cavity,QVariant value,QVariant modifyValue);
private:
    QString getTestName(FlowType flowType);
    explicit Controller(QObject *parent = nullptr);
    void dealTraps();
private:
    QString curTestName = "";
    int flowID = -1;
    QMap<Cavity,QSharedPointer<Row>>reportDataMap;
    QTimer *m_timer;
    TestProcessMode mode = TestProcessMode::thinFilmMode;
    bool isRunning = false;
    FlowDetail curFlow;
    QList<FlowDetail> m_flowList;
    DataTrackingBLL *m_dataTrackingBll = nullptr;
    TestListBLL *m_testListBll = nullptr;
    ProcessSettingBLL* m_processSettingBll = nullptr;
    TestReportParamBLL *m_testReportParamBll = nullptr;
    QMap<Cavity,Worker*> m_workerList;
    static Controller *m_controller;
};

#endif // CONTROLTHREAD_H
