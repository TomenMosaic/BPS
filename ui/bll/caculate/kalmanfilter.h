#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include "algorithm.h"
//卡尔曼滤波算法
class KalmanFilter : public Algorithm
{
    Q_OBJECT
public:
    explicit KalmanFilter(QObject *parent = nullptr);
    ~KalmanFilter();

    double filter(double input) override;

    double readData()override;

    void clear()override;

    void setRange(int value);

    int getRange();

    double getNewVal() const;

    void setRealData(double x);


    float getCoefficient() const;

    void setCoefficient(float value);

private:
    double m_range = 10000;
    float m_coefficient = 1;
    double m_newVal;
    double m_x;
    double m_p;//协方差
    double m_q;//系统噪音方差
    double m_r;//观测噪音方差
};

#endif // KALMANFILTER_H
