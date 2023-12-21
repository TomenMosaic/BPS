#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include "algorithm.h"
#include <QVector>
//中值滤波算法
class MedianFilter : public Algorithm
{
    Q_OBJECT
public:
    explicit MedianFilter(QObject *parent = nullptr);
    ~MedianFilter();

    double filter(double input) override;

    double readData() override;

    void clear() override;
private:
    QVector<double> m_buffer;
};

#endif // MEDIANFILTER_H
