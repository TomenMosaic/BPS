#include "controller.h"
#include "filtermanage.h"
Controller *Controller::m_controller = nullptr;

Controller *Controller::getInstance(QObject *parent)
{
    if(m_controller == nullptr)
    {
        Controller::m_controller = new Controller(parent);
    }
    return  Controller::m_controller;
}

Controller::Controller(QObject *parent) : QObject(parent)
{
    curFlow.flowType = FlowType::unStart;
    curFlow.state = FlowDetail::Ready;
    m_flowList.append(curFlow);
    m_dataTrackingBll = new DataTrackingBLL(this);
    m_processSettingBll = ProcessSettingBLL::getInstance(this);
    m_testReportParamBll = new TestReportParamBLL(this);
    m_testListBll = TestListBLL::getInstance(this);
    m_timer = new QTimer(this);
    int cavityNum = g_dev->getCavityNum();
    qRegisterMetaType<FlowType>("FlowType");
    qRegisterMetaType<QMap<RealDataType,QVariant>>("QMap<RealDataType,QVariant>");
    for(int index =0 ;index<cavityNum;index++)
    {
        Cavity cavity = Cavity(cavityA+index);
        Worker *worker = new Worker(cavity,this);
        m_workerList.insert(cavity,worker);
        connect(worker,SIGNAL(flowFinish(FlowType,bool,bool)),this,SIGNAL(flowFinish(FlowType,bool)),Qt::QueuedConnection);
        connect(worker,SIGNAL(flowBegin(FlowType)),this,SIGNAL(flowBegin(FlowType)),Qt::QueuedConnection);
        connect(worker,SIGNAL(updateCaculateData(Cavity,double,double,double,double,double,double,QDateTime)),this,SIGNAL(updateCaculateData(Cavity,double,double,double,double,double,double,QDateTime)));
        connect(worker,SIGNAL(testFinish(Cavity,bool)),this,SIGNAL(testFinish(Cavity)));
        connect(worker,SIGNAL(dataUpdate(Cavity,QMap<RealDataType,QVariant>,QDateTime)),this,SIGNAL(dataUpdate(Cavity,QMap<RealDataType,QVariant>,QDateTime)));
        connect(worker,SIGNAL(errorMsg(Cavity,FlowDetail::ErrorMsg)),this,SIGNAL(errorMsg(Cavity,FlowDetail::ErrorMsg)));
        qRegisterMetaType<FlowDetail::ErrorMsg>("FlowDetail::ErrorMsg");
    }
    connect(g_dev,SIGNAL(valueChange(Instruct,Cavity,QVariant,QVariant)),this,SLOT(onReciveData(Instruct,Cavity,QVariant,QVariant)),Qt::QueuedConnection);
    m_timer->start(1000);

    connect(m_timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    connect(this,SIGNAL(cmdRead(Instruct,Cavity)),g_dev,SLOT(readData(Instruct,Cavity)));
    connect(this,SIGNAL(cmdSend(Instruct,Cavity,QVariant)),g_dev,SLOT(writeData(Instruct,Cavity,QVariant)));

}

void Controller::dealTraps()
{
    cmdSend(Instruct::flowControl,Cavity::none,QByteArray("\x00\x00\x00\x00",4));
}

bool Controller::getIsRunning() const
{
    return isRunning;
}

void Controller::initFlowList(QList<FlowDetail> flowList)
{
    curTestName = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    m_dataTrackingBll->addRecord(g_power->userName(),"",curTestName,10000,12053,0);

    m_flowList = flowList;
    curFlow = m_flowList.at(0);
    nextFlow(autoMode);
}

bool Controller::start(QList<std::tuple<bool, int, double, int> > runFlags)
{
    if(runFlags.length()==0)
        return false;
    else
    {
        mode = TestProcessMode(std::get<1>(runFlags.first()));
        //        flowID = m_processSettingBll->getProcessNum(mode);
        //        if(flowID==-1)
        //        {
        //            CLOG_ERROR("TEST MODE flow is empty");
        //            return false;
        //        }
        //        else
        //        {
        m_testReportParamBll->init();
        reportDataMap.clear();
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"runFlags"<<runFlags.length();
        QList<QSharedPointer<Row>> processList =  m_processSettingBll->getProcessDetail(mode);
        QList<FlowDetail> flowList;
        for(int processIndex = 0;processIndex<processList.length();processIndex++)
        {
            QSharedPointer<Row>processRow = processList.at(processIndex);
            int flow = processRow->data(ProcessSettingBLL::flow).toInt();
            if((flow>=4&&std::get<0>(runFlags.at(flow-4))==true)||flow<4)
            {
                if(processRow->data(ProcessSettingBLL::enable).toInt())
                {
                    FlowDetail flowDetail(processRow->data(ProcessSettingBLL::flow).toInt(),
                                          processRow->data(ProcessSettingBLL::maxTime).toString(),
                                          processRow->data(ProcessSettingBLL::valueDetails).toString(),
                                          processRow->data(ProcessSettingBLL::flowValue).toInt());
                    if(flowDetail.flowType>=4)
                    {
                        int reportID = std::get<3>(runFlags.at(flow-4));
                        QSharedPointer<Row>reportData = m_testReportParamBll->getReportData(reportID);
                        if(reportData==nullptr)
                        {
                            qDebug()<<Q_FUNC_INFO<<__LINE__<<"unused reportID:"<<reportID;
                            return false;
                        }
                        else
                        {
                            reportDataMap.insert(Cavity(cavityA+flow-4),reportData);
                        }
                    }
                    flowList.append(flowDetail);
                }
            }
        }
        flowList.push_front(FlowDetail(unStart));
        flowList.push_back(FlowDetail(stopTest));
        initFlowList(flowList);
        //        }
    }
}

QList<FlowType> Controller::getCurrentFlowList() const
{
    QList<FlowType>flowList;
    for(int flowIndex = 0;flowIndex<m_flowList.length();flowIndex++)
    {
        flowList.append(m_flowList.at(flowIndex).flowType);
    }
    return flowList;
}

FlowType Controller::getCurFlowType() const
{
    return curFlow.flowType;
}

Controller::~Controller()
{
    m_timer->stop();
    qDebug()<<Q_FUNC_INFO<<__LINE__<<"release controller";
    for(auto it= m_workerList.begin();it!=m_workerList.end();it++)
    {
        Worker *worker = it.value();
        if(worker->isRunning())
        {
            worker->stopRunning();
            if(worker->wait(500))
            {
                worker->quit();
                worker->terminate();
                worker->deleteLater();
            }
        }
    }
    m_workerList.clear();
}

int Controller::getCurFlowLeftTime()
{
    if(curFlow.flowType==FlowType::stopTest||curFlow.flowType==FlowType::unStart)
    {
        return 0;
    }
    else
    {
        return curFlow.overTime-(curFlow.beginTime.secsTo(QDateTime::currentDateTime()));
    }
}

int Controller::getAllLeftTime()
{
    int leftTime = getCurFlowLeftTime();

    int flowId = m_flowList.indexOf(curFlow);
    for(int index = flowId+1;index<m_flowList.length();index++)
    {
        leftTime+=m_flowList.at(index).overTime;
    }
    return leftTime;
}

void Controller::stop()
{
    QByteArray postData = DevManage::convertIntToArr(FlowType::unStart,1)+DevManage::byteArrFormInts({},3);
    cmdSend(Instruct::flowControl,Cavity::none,postData);
    if(curFlow.flowType>=cavityTestA&&curFlow.flowType<stopTest)
    {
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"stopTest:"<<curFlow.flowType;
        m_dataTrackingBll->addRecord(g_power->userName(),"",getTestName(curFlow.flowType),10000,11042,0);
        m_workerList.value(Cavity(curFlow.flowType-cavityTestA+cavityA))->stopRunning();
        int resultID = 10040;
        if(curFlow.error!=FlowDetail::none)
        {
            resultID = int(curFlow.error);
        }
    }
    else
    {
        m_dataTrackingBll->addRecord(g_power->userName(),"",getTestName(curFlow.flowType),10000,11042,0,int(curFlow.flowType));

    }
    m_dataTrackingBll->addRecord(g_power->userName(),"",curTestName,10000,11013,0);

    curFlow = m_flowList.last();
    emit flowBegin(FlowType::stopTest);
}

void Controller::nextFlow(ChangeFlowMode mode)
{
    if(mode == ChangeFlowMode::manualMode)
    {
        m_dataTrackingBll->addRecord(g_power->userName(),"",getTestName(curFlow.flowType),10000,11043,0);
    }
    int index = m_flowList.indexOf(curFlow);
    if(index==-1||index==m_flowList.length()-1)
    {
        int cavityNum  = g_dev->getCavityNum();
        for(int index = 0;index<cavityNum;index++)
        {
            Cavity cavity = Cavity(index+cavityA);
            cmdSend(Instruct::cavityTrafficSetup,cavity,0);
        }
        curFlow.state = FlowDetail::Finished;
        curFlow.endTime = QDateTime::currentDateTime();
        return;
    }
    else
    {
        if(curFlow.flowType<cavityTestA&&curFlow.flowType!=FlowType::unStart)
        {
            int resultID = 10040;
            if(curFlow.error!=FlowDetail::none)
            {
                resultID = int(curFlow.error);
            }
            m_dataTrackingBll->addRecord(g_power->userName(),"",curTestName,10000,11038,0,int(curFlow.flowType),resultID);

        }
        else if(curFlow.flowType!=FlowType::unStart&&curFlow.flowType!=FlowType::stopTest)
        {
            int resultID = 10040;
            if(curFlow.error!=FlowDetail::none)
            {
                resultID = int(curFlow.error);
            }
            m_dataTrackingBll->addRecord(g_power->userName(),"",m_workerList.value(Cavity(curFlow.flowType-cavityTestA+cavityA))->getTestName(),10000,11038,0,int(curFlow.flowType),resultID);
            m_workerList.value(Cavity(curFlow.flowType-cavityTestA+cavityA))->stopRunning();
        }
        curFlow.endTime = QDateTime::currentDateTime();
        emit flowFinish(curFlow.flowType,true);

        curFlow = m_flowList.at(index+1);
        curFlow.beginTime = QDateTime::currentDateTime();
        emit flowBegin(curFlow.flowType);

        //发送控温
        if(curFlow.flowType==temperateControl)
        {
            for(auto it = reportDataMap.begin();it!=reportDataMap.end();it++)
            {
                QSharedPointer<Row> reportData = it.value();
                double temperature = reportData->data(TestReportParamBLL::TEMPERATURE).toDouble();
                cmdSend(Instruct::temperatureSetup,it.key(),temperature);
                cmdSend(Instruct::temperatureControl,it.key(),1);
                cmdSend(Instruct::humidControl,it.key(),1);
            }
            cmdSend(Instruct::temperatureControl,Cavity::Environment,1);

        }
        else if(curFlow.flowType>=cavityTestA&&curFlow.flowType!=stopTest)
        {
            Cavity cavity = Cavity(curFlow.flowType-cavityTestA+cavityA);
            QSharedPointer<Row> reportData = reportDataMap.value(cavity);
            double thickness = reportData->data(TestReportParamBLL::Thickness).toDouble();
            double chamberVolume = reportData->data(TestReportParamBLL::ChamberVolume).toDouble();
            double area = reportData->data(TestReportParamBLL::AREA).toDouble();
            double temperature = reportData->data(TestReportParamBLL::TEMPERATURE).toDouble();
            int reportID = reportData->data(TestReportParamBLL::ID).toDouble();
            QMap<TestListBLL::TestListType,QString> valMap;
            QDateTime currentTime = QDateTime::currentDateTime();
            QString testName = currentTime.toString("yyyyMMddHHmmss")+QString('A'+cavity-cavityA)+QString::number(mode);
            valMap.insert(TestListBLL::TESTNAME,testName);
            valMap.insert(TestListBLL::TESTTIME,currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz"));
            valMap.insert(TestListBLL::TESTISCOMPLETE,QString::number(TestListBLL::Run));
            valMap.insert(TestListBLL::FLOWID,QString::number(flowID));
            valMap.insert(TestListBLL::CAVCOUNT,QString::number(cavity));
            valMap.insert(TestListBLL::ACCOUNT,g_power->userName());
            valMap.insert(TestListBLL::REPORTID,QString::number(reportID));
            m_dataTrackingBll->addRecord(g_power->userName(),"",testName,10000,11012,0);
            int testID = m_testListBll->addRecord(valMap);
            m_workerList.value(cavity)->init(this->mode,testID,testName,area,chamberVolume,thickness,temperature);
        }
        if(curFlow.flowType>=cavityTestA&&curFlow.flowType!=stopTest)
        {
            Cavity cavity = Cavity(curFlow.flowType-cavityTestA+cavityA);
            cmdSend(Instruct::cavityTrafficSetup,cavity,curFlow.flowValue);
        }
        else
        {
            int cavityNum  = g_dev->getCavityNum();
            for(int index = 0;index<cavityNum;index++)
            {
                Cavity cavity = Cavity(index+cavityA);
                cmdSend(Instruct::cavityTrafficSetup,cavity,curFlow.flowValue);
            }
        }

        QList<int> trapInts = intsListFromStrs(curFlow.trap);
        QByteArray postData = DevManage::convertIntToArr(curFlow.flowType,1)+DevManage::byteArrFormInts(trapInts,3);
        cmdSend(Instruct::flowControl,Cavity::none,postData);
        if(curFlow.flowType>=cavityTestA&&curFlow.flowType!=stopTest)
        {
            m_dataTrackingBll->addRecord(g_power->userName(),"",m_workerList.value(Cavity(curFlow.flowType-cavityTestA+cavityA))->getTestName(),10000,11037,0,int(curFlow.flowType));
        }
        else if(curFlow.flowType!=FlowType::unStart&&curFlow.flowType!=FlowType::stopTest)
        {
            m_dataTrackingBll->addRecord(g_power->userName(),"",curTestName,10000,11037,0,int(curFlow.flowType));
        }
    }
}


void Controller::onTestFinish(Cavity cavity, bool success)
{
    dealTraps();
    QString testName = m_workerList.value(cavity)->getTestName();
    m_dataTrackingBll->addRecord(g_power->userName(),"",testName,10000,11013,0);
    int testID = m_workerList.value(cavity)->getTestID();
    m_testListBll->finishTest(testID,success);
}


void Controller::onTimeOut()
{
    for(int cavityIndex = 0;cavityIndex<g_dev->getCavityNum();cavityIndex++)
    {
        Cavity cavity = Cavity(cavityA+cavityIndex);
        if(curFlow.flowType>=cavityTestA&&curFlow.flowType!=stopTest)
        {
            Cavity curCavity = Cavity(curFlow.flowType-cavityTestA+cavityA);
            if(cavity==curCavity)
            {
                emit dataUpdate(cavity,{{RealDataType::voltage,FilterManage::getInstance()->readData(Cavity::none,RealDataType::voltage)}
                                        ,{RealDataType::temperateType,FilterManage::getInstance()->readData(cavity,RealDataType::temperateType)}
                                        ,{RealDataType::humidType,FilterManage::getInstance()->readData(cavity,RealDataType::humidType)}
                                        ,{RealDataType::flowType,FilterManage::getInstance()->readData(cavity,RealDataType::flowType)}
                                },QDateTime::currentDateTime());
            }
            else
            {
                emit dataUpdate(cavity,{{RealDataType::voltage,FilterManage::getInstance()->readRealData(Cavity::none,RealDataType::voltage)}
                                        ,{RealDataType::temperateType,FilterManage::getInstance()->readRealData(cavity,RealDataType::temperateType)}
                                        ,{RealDataType::humidType,FilterManage::getInstance()->readRealData(cavity,RealDataType::humidType)}
                                        ,{RealDataType::flowType,FilterManage::getInstance()->readRealData(cavity,RealDataType::flowType)}
                                },QDateTime::currentDateTime());
            }
        }
        else if(curFlow.flowType==stopTest)
        {

            emit dataUpdate(cavity,{{RealDataType::temperateType,FilterManage::getInstance()->readRealData(cavity,RealDataType::temperateType)}
                                    ,{RealDataType::humidType,FilterManage::getInstance()->readRealData(cavity,RealDataType::humidType)}
                                    ,{RealDataType::flowType,FilterManage::getInstance()->readRealData(cavity,RealDataType::flowType)}
                            },QDateTime::currentDateTime());
        }
        else
        {
            emit dataUpdate(cavity,{{RealDataType::voltage,FilterManage::getInstance()->readRealData(Cavity::none,RealDataType::voltage)}
                                    ,{RealDataType::temperateType,FilterManage::getInstance()->readRealData(cavity,RealDataType::temperateType)}
                                    ,{RealDataType::humidType,FilterManage::getInstance()->readRealData(cavity,RealDataType::humidType)}
                                    ,{RealDataType::flowType,FilterManage::getInstance()->readRealData(cavity,RealDataType::flowType)}
                            },QDateTime::currentDateTime());
        }
    }
    emit dataUpdate(Environment,{{RealDataType::temperateType,FilterManage::getInstance()->readData(Environment,RealDataType::temperateType)}},QDateTime::currentDateTime());
    if(curFlow.flowType==FlowType::wash)
    {
        double curHumid = FilterManage::getInstance()->readData(cavityA,RealDataType::humidType);
        double curTemerature = FilterManage::getInstance()->readData(cavityA,RealDataType::temperateType);
        double curFlow = FilterManage::getInstance()->readData(cavityA,RealDataType::flowType);
        double curVoltage = FilterManage::getInstance()->readData(Cavity::none,RealDataType::voltage);
        double curSensorTemperature = FilterManage::getInstance()->readData(Cavity::Environment,RealDataType::temperateType);
        double gasPremeation = 0;
        emit updateCaculateData(cavityA,curHumid,gasPremeation,curTemerature,
                                curFlow,curVoltage,curSensorTemperature,QDateTime::currentDateTime());
    }
    if(curFlow.flowType != unStart&&curFlow.flowType!=stopTest)
    {
        if(curFlow.flowType==FlowType::temperateControl)
        {
            bool finishControl = true;
            for(auto it = reportDataMap.begin();it!=reportDataMap.end();it++)
            {
                QSharedPointer<Row> reportData = it.value();
                double temperature = reportData->data(TestReportParamBLL::TEMPERATURE).toDouble();
                if(qAbs(FilterManage::getInstance()->readData(it.key(),RealDataType::temperateType)-temperature)>0.5)
                {
                    finishControl = false;
                }
            }
            if(finishControl)
            {
                nextFlow(autoMode);
                return;
            }
        }
        if(curFlow.beginTime.secsTo(QDateTime::currentDateTime())>=curFlow.overTime)
        {
            nextFlow(autoMode);
        }
    }
}

void Controller::onReciveData(Instruct instruct, Cavity cavity, QVariant value, QVariant modifyValue)
{
    if(instruct==Instruct::infraredSensorVal)
    {
        FilterManage::getInstance()->filter(cavity,RealDataType::voltage,modifyValue.toDouble());
    }
    else if(instruct == Instruct::temperature)
    {
        FilterManage::getInstance()->filter(cavity,RealDataType::temperateType,modifyValue.toDouble());
    }
    else if(instruct == Instruct::humid)
    {
        FilterManage::getInstance()->filter(cavity,RealDataType::humidType,modifyValue.toDouble());
    }
    else if(instruct == Instruct::humidFlow)
    {
        FilterManage::getInstance()->filter(cavity,RealDataType::flowType,modifyValue.toDouble());
    }
}

QString Controller::getTestName(FlowType flowType)
{
    QString testName = curTestName;
    if(flowType>=cavityTestA&&flowType<stopTest)
        testName= m_workerList.value(Cavity(flowType-cavityTestA+cavityA))->getTestName();
    return testName;
}


