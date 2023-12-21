#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include "testdataresultbll.h"
#include "Global.h"
#include "kalmanfilter.h"
#include "testdatarawbll.h"
#include "trackdetail.h"
#include "judgesetupbll.h"
#include "decalibrationalgorithm.h"
#include <QDateTime>
#include <demarcationbll.h>
#include <demarcationschemebll.h>
//! 计算方差
template<typename ForwardIt>
inline double ComputeVariance(ForwardIt first, ForwardIt last)
{
    using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
    auto size = std::distance(first, last);
    if (size <= 1)
    { // 样本数量小于或等于1时，直接返回0
        return 0;
    }

    double avg = std::accumulate(first, last, 0.0) / size; // 计算均值
    double variance(0);
    std::for_each(first, last, [avg, &variance](const ValueType &num) { variance += (num - avg) * (num - avg); });
    return variance / size;
}

//! 计算标准差
template<typename ForwardIt>
inline double ComputeStandardDeviation(ForwardIt first, ForwardIt last)
{
    return std::sqrt(ComputeVariance(first, last));
}
struct FlowDetail
{
    enum ErrorMsg
    {
        none,
        temeratureControlOverTime = 11061,//实验过程中温度变化超过范围
        deviceOffLine = 11062,//设备离线
        vacuumingOutTime = 11063,  //抽真空超时
        downCavityOutRange = 11064 //下腔压力超过量程
    };
    enum FlowState
    {
        Ready,
        Running,
        Finished,
        Error
    };
    bool operator == (const FlowDetail &other)const
    {
        return flowType==other.flowType;
    }
    bool operator == (const FlowType &other)const
    {
        return flowType==other;
    }
    FlowDetail();
    FlowDetail(FlowType flowType);

    FlowDetail(int flowTypeID,QString newTime,QString newTrap,int flowVal);
    FlowType flowType = FlowType::unStart;
    uint overTime = 0;
    QStringList trap;
    QDateTime beginTime;
    QDateTime endTime;
    int flowValue = 0;
    FlowState state = Ready;
    friend QDebug& operator<<(QDebug out, const FlowDetail& info)
    {
        out <<"  flowType:"<<info.flowType
           <<"  overTime:"<<info.overTime
          <<"  trap:"<<info.trap
         <<"  beginTime:"<<info.beginTime
        <<"  endTime:"<<info.endTime
        <<"  state:"<<info.state;
        return out;
    }
    ErrorMsg error = none;
};

struct RealData
{
      QDateTime dateTime;
      double flowVal = 0.0;
      double temperature = 0.0;
      double humid = 0.0;
      double voltage = 0.0;
      double sensorTemperature = 0.0;
};
struct Record
{
    QDateTime dateTime;
    double flowVal = 0.0;
    double temperature = 0.0;
    double humid = 0.0;
    double voltage = 0.0;
    double sensorTemperature = 0.0;
    double transmittance = 0.0;
};
class Worker : public QThread
{
    Q_OBJECT
public slots:

public:
    explicit Worker(Cavity cavity,QObject *parent = nullptr);

    void init(TestProcessMode mode,int testID,QString testName,double area,double volume,double thickness,double temprature);
    //停止实验
    void stopRunning();

    void run() override;

    //获取上下腔压差
    double getDeltaPUpDown(double upPressure,double downPressure);
    //获取气体透过率，单位时间压差变化值pa/h、时间、温度、上下腔压差
    float calculateGasTransmittance(double pressureDiffChange, double temperature, double deltaPUpDown);

    QList<FlowDetail> getFlowList() const;

    int getTestID() const;

    FlowDetail::ErrorMsg getErrorMsg(FlowType flowType);

    QString getTestName() const;

    ~Worker();
private:

    void calculateKd(const QVector<double>& xData, const QVector<double>& yList, double &k, double &d);

    double calculateR(const QVector<double>& xData, const QVector<double>& yData, double K, double d);


    void addRecord(double voltage, double gasTransmissionRate,
                   double temperature, double humid, double flow, double ambientTemperature, QDateTime dateTime);

    double getAvgValue(QList<double> valueList);

    double celsiusToKelvin(double celsius);

    bool isSuccess(double &lastResult);

    QList<double> getRecordList(QDateTime m_beginTime,QDateTime endTime);

    void getGasPermeability(double &latency,double &diffusion,double &solubility,double &gasPermeability);
signals:
    bool cmdRead(Instruct instruct,Cavity cavity);
    bool cmdSend(Instruct instruct,Cavity cavity,QVariant value);
    void flowBegin(FlowType flowType);
    void flowFinish(FlowType flowType,bool success,bool  isManual = false);
    void testFinish(Cavity cavity,bool ok);
    void dataUpdate(Cavity cavity,QMap<RealDataType,QVariant> data,QDateTime date);
    void updateCaculateData(Cavity cavity,double humid,double gasTransmissionRate,
                            double temperature,double flowType,double voltage,double ambientTemperature,QDateTime dateTime);
    void errorMsg(Cavity cavity,FlowDetail::ErrorMsg error);
private:
    bool m_finish = false;
    bool m_runFlag = false;

    bool findMinPressureDiff = false;
    //实验开始的下腔压力
    double beginDownPressure  = 0.0;
    //温度超过30min不在0.5范围内，判断实验失败
    int m_tempretaureSecs = 60*30;
    // 超过温度
    double m_outRangeTemprature = 0.5;
    //开始超过时间
    QDateTime m_temperatureOutTime;
    //下腔量程
    int m_downCavityRange = 0.0;

    double m_beginDownPressure = 0.0;//记录抽真空前下腔压力
    //实验模式
    TestProcessMode m_mode;
    //温控误差
    double temperatureControleError = 0.5;
    //实验温度偏差时间
    double temperatureErrorTime = 30;
    //

    double lastResult = 0.0;
    //自动判断
    bool autoJust = false;
    //相对偏差
    double m_relativeDeviation = 0;
    //绝对偏差
    double m_absoluteValue = 0;

    //方差
    int m_standardDeviation = 0;
    DemarcationBLL *m_demarcationBll = nullptr;
    DemarcaSchemeBLL *m_demarcaSchemeBll =nullptr;
    DecalibrationAlgorithm *m_decalibrationAlgorithm = nullptr;
    JudgeSetupBLL *m_judgetSetupBll = nullptr;
    TestDataResultBLL *m_testDataResultBll = nullptr;
    TestDataRawBLL *m_testDataRawBll = nullptr;
    QString m_testName;
    int m_testID = -1;
    QDateTime m_timeColor;
    QDateTime m_currentTime;
    QDateTime m_begin;
    int m_collectionTime = 60;//60s记录一次数据
    int m_caculateCycleTime = 60;//60min
    int m_beginTime = 0;//开始计算时间
    int m_sameTime = 3;

    //实时数据记录时间（1min1条数据）//时间，压差变化量、气体透过率、温度、上腔压力、下腔压力、环境温度、
    QList<Record>recordList;
    //记录下腔压力，用来计算扩散系数
    QList<double> m_downPressureList;
    //用来保存每秒数据//温度、上腔压力、下腔压力、环境温度
    QList<RealData>realDatas;
    QMutex m_mutex;
    QWaitCondition m_condition;
    double m_thickness = 1.0*0.001;//cm
    double m_temperature  = 23;//℃
    double m_area = 50.24*0.0001;//m2
    double m_volume = 50000.0;//cm3
    Cavity m_cavity;
    QList<FlowDetail> m_flowList;
};

#endif // WORKERTHREAD_H
