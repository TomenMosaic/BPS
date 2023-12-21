#include "worker.h"
#include "kalmanfilter.h"
#include "medianfilter.h"
#include <QDateTime>
#include "filtermanage.h"

Worker::Worker(Cavity cavity, QObject *parent) : QThread(parent)
{
    if(!m_testDataRawBll)
        m_testDataRawBll = new TestDataRawBLL(this);
    m_demarcationBll= new DemarcationBLL(this);
    m_demarcaSchemeBll = new DemarcaSchemeBLL(this);
    m_testDataResultBll = new TestDataResultBLL(this);
    m_decalibrationAlgorithm = new DecalibrationAlgorithm(this);
    m_cavity = cavity;
    m_judgetSetupBll = new JudgeSetupBLL(this);

    connect(this,SIGNAL(cmdSend(Instruct,Cavity,QVariant)),g_dev,SLOT(writeData(Instruct,Cavity,QVariant)),Qt::BlockingQueuedConnection);
    connect(this,SIGNAL(cmdRead(Instruct,Cavity)),g_dev,SLOT(readData(Instruct,Cavity)),Qt::BlockingQueuedConnection);
}

void Worker::init(TestProcessMode mode, int testID, QString testName, double area, double volume, double thickness, double temprature)
{
    m_demarcaSchemeBll->init();
    m_demarcationBll->init();
    m_mode = mode;
    if(g_config->contains("filterValue"))
    {
        int filterValue = g_config->getRecord("filterValue").toInt();
        FilterManage::getInstance()->setRange(filterValue);
        FilterManage::getInstance()->setCoefficient(0.1);
    }
    int schemeID = m_demarcaSchemeBll->getDefaultID(m_cavity);
    QList<QSharedPointer<Row>> rowList = m_demarcationBll->getRowListByID(schemeID);
    QVector<double> xValues;
    QVector<double> yValues;
    for(int index = 0;index<rowList.length();index++)
    {
        QSharedPointer<Row>curRow = rowList.at(index);
        double shiftVal = curRow->data(DemarcationBLL::shifting).toDouble();
        xValues.append(curRow->data(DemarcationBLL::pressureVariation).toDouble()+shiftVal);
        yValues.append(curRow->data(DemarcationBLL::transmittance).toDouble());
    }
    m_decalibrationAlgorithm->setValueList(xValues,yValues);
    m_testName = testName;
    m_testID = testID;
    m_mutex.lock();
    m_thickness = thickness*0.0001;//将单位mm转化成cm
    m_area = area * 0.0001;//将单位cm2转化成m2
    m_volume = volume;
    m_temperature = temprature;
    if(g_config->contains("tempretaureSecs"))
        m_tempretaureSecs = qMin(g_config->getRecord("tempretaureSecs").toInt(),60);
    if(g_config->contains("outRangeTemprature"))
        m_outRangeTemprature = qMin(g_config->getRecord("outRangeTemprature").toDouble(),0.1);
    if(g_config->contains("downCavityRange"))
        m_downCavityRange = qMin(g_config->getRecord("downCavityRange").toInt(),0);

    m_runFlag = true;
    m_finish = false;
    m_judgetSetupBll->init();
    QSharedPointer<Row> row = m_judgetSetupBll->getCavityEdit(m_cavity);
    if(row)
    {
        m_caculateCycleTime = row->data(JudgeSetupBLL::gatherTimeInterval).toInt();
        m_sameTime = qMax(row->data(JudgeSetupBLL::gatherNum).toInt(),2);
        m_relativeDeviation = qMax(row->data(JudgeSetupBLL::relativeDeviation).toDouble(),0.0);
        m_absoluteValue = qMax(row->data(JudgeSetupBLL::absoluteDeviation).toDouble(),0.0);
        m_standardDeviation = qMax(row->data(JudgeSetupBLL::standardDeviation).toDouble(),0.0);
        autoJust = row->data(JudgeSetupBLL::autoJudgeCol).toBool();
        m_beginTime = qMax(row->data(JudgeSetupBLL::beginTime).toInt(),0);
    }
    m_begin = QDateTime::currentDateTime();
    m_timeColor = m_begin;
    this->start();
    m_mutex.unlock();
    m_condition.wakeOne();

}


void Worker::stopRunning()
{
    m_mutex.lock();
    m_finish = true;
    m_mutex.unlock();
    m_condition.wakeOne();
}

void Worker::run()
{
    m_runFlag = true;
    while(m_runFlag)
    {
        m_mutex.lock();
        m_condition.wait(&m_mutex, 1000); // 等待1秒钟
        m_mutex.unlock();
        m_currentTime = QDateTime::currentDateTime();
        double curHumid = FilterManage::getInstance()->readData(m_cavity,RealDataType::humidType);
        double curTemerature = FilterManage::getInstance()->readData(m_cavity,RealDataType::temperateType);
        double curFlow = FilterManage::getInstance()->readData(m_cavity,RealDataType::flowType);
        double curVoltage = FilterManage::getInstance()->readData(Cavity::none,RealDataType::voltage);
        double curSensorTemperature = FilterManage::getInstance()->readData(Cavity::Environment,RealDataType::temperateType);
        realDatas.append(RealData{m_currentTime,curFlow,curTemerature,curHumid
                                  ,curVoltage,curSensorTemperature});
        if(realDatas.isEmpty())
            continue;
        else
        {
            if(m_begin.secsTo(m_currentTime)%20==0)
            {
                m_downPressureList.append(curVoltage);
            }
            double gasPremeation = 0;
            if(m_decalibrationAlgorithm->isUserful())
                gasPremeation = m_decalibrationAlgorithm->getCalibrationResult(curVoltage);
            bool finish = false;
            int curSecs = m_begin.secsTo(m_currentTime);
            if(curSecs<=9000&&curSecs%90==0)
            {
                int curNum = curSecs/100;
                FilterManage::getInstance()->setCoefficient(0.1+curNum*0.01);
            }
            emit updateCaculateData(m_cavity,curHumid,gasPremeation,curTemerature,curFlow,curVoltage,curSensorTemperature,m_currentTime);
            if(m_timeColor.secsTo(m_currentTime)>=m_collectionTime)
            {
                qDebug()<<Q_FUNC_INFO<<__LINE__<<"m_timeColor:"<<m_timeColor;
                recordList.append(Record{m_timeColor.addSecs(m_collectionTime/2),curFlow,curTemerature,
                                         curHumid,curVoltage,curSensorTemperature,gasPremeation});
                addRecord(curVoltage,gasPremeation,curTemerature,curHumid,curFlow,curSensorTemperature,m_currentTime);
                m_timeColor = m_currentTime;
                QDateTime lastTime = m_currentTime.addSecs(-1*m_caculateCycleTime*60);
                QList<double> valueList =  getRecordList(lastTime,m_currentTime);
                if(autoJust)
                {
                    if(m_begin.secsTo(m_currentTime)>=(m_sameTime*m_caculateCycleTime+m_beginTime)*60)
                    {
                        finish = isSuccess(lastResult);
                    }
                }
            }
            if(finish||m_finish)
            {
                double latency=0, diffusion=0, solubility=0, gasPermeability=0;
                getGasPermeability(latency,diffusion,solubility,gasPermeability);
                if(!recordList.isEmpty())
                {

                    emit updateCaculateData(m_cavity,recordList.last().humid,recordList.last().transmittance
                                            ,recordList.last().temperature,recordList.last().flowVal,
                                            recordList.last().voltage,recordList.last().sensorTemperature,m_currentTime);

                    m_testDataResultBll->addRecord(m_testID,recordList.last().transmittance,recordList.last().humid,
                                                   recordList.last().flowVal,latency,diffusion,solubility,gasPermeability);
                }
                else
                {
                    m_testDataResultBll->addRecord(m_testID,gasPremeation,curHumid,curFlow,latency,diffusion,solubility,gasPermeability);
//                    emit dataUpdate(m_cavity,{{RealDataType::humidType,curHumid},
//                                              {RealDataType::flowType,curFlow},
//                                              {RealDataType::temperateType,curTemerature},
//                                              {RealDataType::voltage,curVoltage},
//                                              {RealDataType::sensorTemperature,curSensorTemperature},
//                                              {RealDataType::transmittance,gasPremeation}},m_currentTime);

                    emit updateCaculateData(m_cavity,curHumid,gasPremeation,curTemerature,curFlow,curVoltage,curSensorTemperature,m_currentTime);
                }
                recordList.clear();
                m_downPressureList.clear();
                m_runFlag = false;
                qDebug()<<Q_FUNC_INFO<<__LINE__<<"set runFlag:"<<false;
            }
        }
    }
}


double Worker::getDeltaPUpDown(double upPressure, double downPressure)
{
    return upPressure*1000-downPressure;
}

float Worker::calculateGasTransmittance(double pressureDiffChange,  double temperature, double deltaPUpDown)
{
    assert(m_area>0);
    if(deltaPUpDown==0||m_area==0||celsiusToKelvin(temperature)==0)
        return 0;
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<" thickness:"<<m_thickness;
    //    double unitpressureDiffChange = double(pressureDiffChange*3600)/overTime;
    float lastResult= 0.0;
    if(m_mode==TestProcessMode::thinFilmMode&&m_decalibrationAlgorithm->isUserful())
        lastResult = m_decalibrationAlgorithm->getCalibrationResult(pressureDiffChange);
    else
        lastResult = (pressureDiffChange*m_volume*273.15*24)/(m_area*celsiusToKelvin(temperature)*deltaPUpDown);
    return lastResult;
}

double Worker::getAvgValue(QList<double> valueList)
{
    if(valueList.isEmpty())
        return 0.0;
    else
    {
        double sum = 0;
        sum=  std::accumulate(valueList.begin(), valueList.end(), sum);
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"sum:"<<sum;
        double mean =  sum / valueList.size(); //均值
        return mean;
    }
}

double Worker::celsiusToKelvin(double celsius)
{
    return celsius + 273.15;
}



bool Worker::isSuccess(double &lastResult)
{
    QList<double>results;
    for(int index = 0;index<m_sameTime;index++)
    {
        QDateTime lastTime = m_currentTime.addSecs((-index-1)*m_caculateCycleTime*60);
        QDateTime endTime = lastTime.addSecs(m_caculateCycleTime*60);
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"lastTime:"<<lastTime.toString("yyyy-MM-dd hh:mm:ss")
               <<"endTime:"<<endTime.toString("yyyy-MM-dd hh:mm:ss")
              <<"currentTime:"<<m_currentTime.toString("yyyy-MM-dd hh:mm:ss");
        QList<double>records = getRecordList(lastTime,endTime);
        qDebug()<<Q_FUNC_INFO<<__LINE__<<" records:"<<records<<" recordList length:"<<recordList.length();
        if(records.isEmpty())
            return false;
        double oneresult = getAvgValue(records);
        results.append(oneresult);
    }
    double max = *std::max_element(results.begin(),results.end());
    double min = *std::min_element(results.begin(),results.end());
    double curAbsoluteValue = max-min;
    double currelativeDeviation = double(curAbsoluteValue/min)*100;
    double curstandardDeviation = ComputeStandardDeviation(results.begin(),results.end());
    lastResult = getAvgValue(results);
    qDebug()<<Q_FUNC_INFO<<__LINE__<<" results:"<<results<<"  max:"<<max<<"  min:"<<min<<"  curAbsoluteValue:"<<curAbsoluteValue
           <<"  standardDeviation:"<<curstandardDeviation<<"   lastResult:"<<lastResult;
    emit dataUpdate(m_cavity,{ {RealDataType::absoluteValueSignal     ,curAbsoluteValue}
                               ,{RealDataType::relativeDeviationSignal,currelativeDeviation}
                               ,{RealDataType::standardDeviationSignal,curstandardDeviation}},m_currentTime);

    //绝对偏差
    QStringList resultStrs;
    foreach(double result,results)
    {
        resultStrs.append(QString::number(result));
    }
    if(curAbsoluteValue<=m_absoluteValue||currelativeDeviation<=m_relativeDeviation||curstandardDeviation<=m_standardDeviation)
    {
        CLOG_INFO(QString("End expriment results:%1").arg(resultStrs.join("-")).toUtf8());
        return true;
    }
    return false;
}

QList<double> Worker::getRecordList(QDateTime beginTime, QDateTime endTime)
{
    QList<double>dataList;
    for(int index = 0;index<recordList.length();index++)
    {
        QDateTime curTime = recordList.at(index).dateTime;
        if(beginTime.secsTo(curTime)>=0&&curTime.secsTo(endTime)>=0)
        {
            dataList.append(recordList.at(index).transmittance);
        }
    }
    return dataList;
}

void Worker::getGasPermeability(double &latency, double &diffusion, double &solubility, double &gasPermeability)
{
    if(m_downPressureList.length()<2)
    {
        latency = 0;
        diffusion = 0;
        solubility = 0;
        gasPermeability = 0;
    }
    else
    {
        int timeRange =  100;
        if(lastResult<=10)
        {
            timeRange = 60;//20min取一个点
        }
        else if(10<=lastResult&&lastResult<40)
        {
            timeRange = 30;//10min取一个点
        }
        else if(lastResult>=40)
        {
            timeRange = 1;//20s取一个点
        }
        int length = m_downPressureList.length();

        for(int index = 0;index+timeRange*5<length;index++)
        {
            QVector<double> yvalueList = {m_downPressureList[index],
                                          m_downPressureList[index+timeRange],
                                          m_downPressureList[index+2*timeRange],
                                          m_downPressureList[index+3*timeRange],
                                          m_downPressureList[index+4*timeRange]};
            QVector<double> xValueList = {double(index*20),double((index+timeRange*1)*20),double((index+timeRange*2)*20),double((index+timeRange*3)*20),double((index+timeRange*4)*20)};
            double k = 0;
            double b = 0;
            calculateKd(xValueList,yvalueList,k,b);
            double R = calculateR(xValueList,yvalueList,k,b);
            double curlatency = -b/k;
            if(R>0.8)
                latency = qMax(curlatency,latency);
            qDebug()<<Q_FUNC_INFO<<__LINE__<<"curlatency:"<<curlatency;
        }
        gasPermeability = lastResult * 1.16*qPow(10,-12);
        if(latency==0)
            diffusion = 0;
        else
            diffusion = m_thickness*m_thickness/(6*latency);
        if(diffusion>0)
            solubility= gasPermeability/diffusion;
        else
            solubility = 0;
    }
}


QString Worker::getTestName() const
{
    return m_testName;
}

Worker::~Worker()
{

}


void Worker::calculateKd(const QVector<double> &xList, const QVector<double> &yList, double &k, double &d)
{
    MatrixXd B(xList.length(),2);
    MatrixXd l(yList.length(),1);
    for(int xIndex = 0;xIndex<xList.length();xIndex++)
    {
        B(xIndex,0) = xList.at(xIndex);
        B(xIndex,1) = 1;
        l(xIndex,0) = yList.at(xIndex);
    }
    MatrixXd I=MatrixXd::Identity(xList.length(),yList.length());//单位矩阵
    MatrixXd Z=((B.transpose()*I*B).inverse())*B.transpose()*I*l;
    MatrixXd V=B*Z-l;
    k=Z(0,0);//求出k、d值
    d=Z(1,0);
}

double Worker::calculateR(const QVector<double> &xData, const QVector<double> &yData, double K, double d)
{
    int n = xData.size();

    // 计算y的平均值
    double meanY = 0;
    for (int i = 0; i < n; ++i) {
        meanY += yData[i];
    }
    meanY /= n;

    // 计算拟合直线的y值
    QVector<double> fittedY(n);
    for (int i = 0; i < n; ++i) {
        fittedY[i] = K * xData[i] + d;
    }

    // 计算SST（总平方和）
    double SST = 0;
    for (int i = 0; i < n; ++i) {
        SST += qPow(yData[i] - meanY, 2);
    }

    // 计算SSR（回归平方和）
    double SSR = 0;
    for (int i = 0; i < n; ++i) {
        SSR += qPow(fittedY[i] - meanY, 2);
    }

    // 计算SSE（残差平方和）
    double SSE = 0;
    for (int i = 0; i < n; ++i) {
        SSE += qPow(yData[i] - fittedY[i], 2);
    }

    // 计算R值
    double R = qSqrt(SSR / SST);

    return R;
}


void Worker::addRecord(double voltage, double gasTransmissionRate, double temperature, double humid, double flow, double ambientTemperature, QDateTime dateTime)
{
    QMap<TestDataRawBLL::TestDataRawType,QString> mapList;
    mapList.insert(TestDataRawBLL::testid,QString::number(m_testID));
    mapList.insert(TestDataRawBLL::cavity,QString::number(m_cavity));
    mapList.insert(TestDataRawBLL::dateTime,TableDAL::currentTime());
    mapList.insert(TestDataRawBLL::temperature,QString::number(temperature,'f',4));
    mapList.insert(TestDataRawBLL::humid,QString::number(humid,'f',4));
    mapList.insert(TestDataRawBLL::flow,QString::number(flow,'f',4));
    mapList.insert(TestDataRawBLL::voltage,QString::number(voltage,'f',4));
    mapList.insert(TestDataRawBLL::transmittance,QString::number(gasTransmissionRate,'f',4));
    mapList.insert(TestDataRawBLL::environmentTemprate,QString::number(ambientTemperature,'f',4));
    m_testDataRawBll->addRecord(mapList);

}

int Worker::getTestID() const
{
    return m_testID;
}

FlowDetail::FlowDetail()
{

}

FlowDetail::FlowDetail(FlowType flowType)
{
    this->flowType = flowType;
}

FlowDetail::FlowDetail(int flowTypeID, QString newTime, QString newTrap, int flowVal)
{
    flowType = FlowType(flowTypeID);
    QStringList newTimeList = newTime.split(":");
    QList<int> timeInts = intsListFromStrs(newTimeList);
    assert(timeInts.length()==3);
    overTime = timeInts[0]*3600+timeInts[1]*60+timeInts[2];
    flowValue = flowVal;
    if(!newTrap.trimmed().isEmpty())
        trap = newTrap.split(",");
}
