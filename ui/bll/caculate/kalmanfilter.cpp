#include "kalmanfilter.h"
#include <QDebug>
#include <QThread>
#include <QtCore/qmath.h>
KalmanFilter::KalmanFilter(QObject *parent) : Algorithm(parent)
{
    m_x = 0;
    m_r = 10000;
    m_p = 100;
    m_q = 1;
    m_newVal= 0;
}
KalmanFilter::~KalmanFilter(){}

double KalmanFilter::filter(double input)
{
    m_newVal = input;
    if(m_x==0)
    {
        m_x = input;
        return m_x;
    }
    // 预测
    double x_pre = m_x;
    double p_pre = m_p + m_q;
    // 更新
    double k = p_pre / (p_pre + m_r);
    m_x = x_pre + k * (input - x_pre);
    m_p = (1 - k) * p_pre;
    return m_x;
}

double KalmanFilter::readData()
{
    return m_x;
}

void KalmanFilter::clear()
{
    m_x = m_newVal;
}

void KalmanFilter::setRange(int value)
{
    m_range = value;
    m_r = 1*m_range*m_coefficient;
}

int KalmanFilter::getRange()
{
    return m_r;
}

double KalmanFilter::getNewVal() const
{
    return m_newVal;
}

void KalmanFilter::setRealData(double x)
{
    m_x = x;
    m_newVal = x;
}

float KalmanFilter::getCoefficient() const
{
    return m_coefficient;
}

void KalmanFilter::setCoefficient(float value)
{
    if(value<0&&value>1)
        return;
    m_coefficient = value;
    m_r =   m_range*value;
}


