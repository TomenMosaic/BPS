#include "calibrationalgorithms.h"
#include <QMetaEnum>
#include <QDebug>
#include <iostream>
CalibrationAlgorithms::CalibrationAlgorithms(int cavityNum, QObject *parent) : QObject(parent)
{
    m_cavityNum=cavityNum;
    m_calibrationBll = new CalibrationBLL(parent);
}

CalibrationAlgorithms::~CalibrationAlgorithms()
{

}

void CalibrationAlgorithms::init()
{
    this->clear();
    for(int cavityIndex = 0;cavityIndex<m_cavityNum;cavityIndex++)
    {
        CaculateData *caculatetemperature = new CaculateData(Cavity(cavityA+cavityIndex),Instruct::temperature);
        CaculateData *caculateup = new CaculateData(Cavity(cavityA+cavityIndex),Instruct::humidFlow);
        CaculateData *caculatehumid = new CaculateData(Cavity(cavityA+cavityIndex),Instruct::humid);

        m_caculateDatas.append(caculatetemperature);
        m_caculateDatas.append(caculateup);
        m_caculateDatas.append(caculatehumid);

    }
    CaculateData *caculateDown = new CaculateData(Cavity::none,Instruct::infraredSensorVal);
    m_caculateDatas.append(caculateDown);
    reCaculate();
}

Instruct CalibrationAlgorithms::getInstruct(int value)
{
    switch (value)
    {
    case 0:
        return Instruct::temperature;
        break;
    case 1:
        return Instruct::humid;
        break;
    case 2:
        return Instruct::humidFlow;
        break;
    case 3:
        return Instruct::infraredSensorVal;
        break;
    default:
        return Instruct::temperature;
        break;
    }
}

CaculateData *CalibrationAlgorithms::getCaculate(Cavity cavity, Instruct instruct)
{
    for(int index = 0;index<m_caculateDatas.length();index++)
    {
        if(m_caculateDatas.at(index)->cavity==cavity&&m_caculateDatas.at(index)->instruct==instruct)
            return m_caculateDatas.at(index);
    }
    CaculateData *caculatehumid = new CaculateData(cavity,instruct);
    m_caculateDatas.append(caculatehumid);
    return caculatehumid;
}

double CalibrationAlgorithms::getCalibrationValue(Cavity cavity, Instruct instruct, double firstValue)
{
    CaculateData *caculateData = getCaculate(cavity,instruct);
    if(!caculateData)
    {
        return firstValue;
    }
    else
    {
        double lastValue =  caculateData->getCaculateValue(firstValue);
        return lastValue;
    }
}

double CalibrationAlgorithms::getCalibrationSetupValue(Cavity cavity, Instruct instruct, double lastValue)
{
    CaculateData *caculateData = getCaculate(cavity,instruct);
    if(!caculateData)
    {
        return lastValue;
    }
    else
    {
        double firstValue =  caculateData->getCaculateFirstValue(lastValue);
        //        qDebug()<<Q_FUNC_INFO<<__LINE__<<" firstValue:"<<firstValue<<" lastValue:"<<lastValue;
        return firstValue;
    }
}

void CalibrationAlgorithms::clear()
{
    for(int index = 0;index<m_caculateDatas.length();index++)
    {
        delete m_caculateDatas.at(index);
    }
    m_caculateDatas.clear();
}

void CalibrationAlgorithms::reCaculate()
{
    m_calibrationBll->init();
    foreach(CaculateData *caculateData,m_caculateDatas)
    {
        caculateData->clear();
    }
    QList<QSharedPointer<Row>> rowList = m_calibrationBll->getRowList();
    for(int rowIndex = 0;rowIndex<rowList.count();rowIndex++)
    {
        QSharedPointer<Row>curRow = rowList.at(rowIndex);
        Cavity cavity = Cavity(curRow->data(CalibrationBLL::cavityType).toInt()+cavityA);
        Instruct instruct = getInstruct(curRow->data(CalibrationBLL::calibrateParam).toInt());
        double outputValue = curRow->data(CalibrationBLL::outputValue).toDouble();
        double realValue = curRow->data(CalibrationBLL::realValue).toDouble();
        if(outputValue!=0&&realValue!=0)
            getCaculate(cavity,instruct)->addPoint(outputValue,realValue);
    }
    foreach(CaculateData *caculateData,m_caculateDatas)
    {
        caculateData->recaculate();
    }
}

CaculateData::CaculateData(Cavity newCavity, Instruct newInstruct)
{
    cavity = newCavity;
    instruct=newInstruct;
}

double CaculateData::getCaculateValue(double firstValue)
{
    return k*firstValue + d;
}

double CaculateData::getCaculateFirstValue(double lastValue)
{
    double firstValue = lastValue-d;
    if(k!=0)
        firstValue = firstValue/k;

    return firstValue;
}

void CaculateData::addPoint(double xValue, double yValue)
{
    xList.append(xValue);
    yList.append(yValue);
}

void CaculateData::clear()
{
    xList.clear();
    yList.clear();
    k = 1;
    d = 0;
}

bool CaculateData::recaculate()
{
    if(xList.isEmpty())
        return false;
    else
    {
        if(xList.length()==1)
        {
            k = 1;
            d = yList.first()-xList.first();
        }
        else
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
        //        qDebug()<<Q_FUNC_INFO<<__LINE__<<"begin:"<<"  xList:"<<xList<<"  yList:"<<yList<<"  k:"<<k<<"  d:"<<d;

        return true;


    }
}
