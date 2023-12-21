#ifndef FILTERMANAGE_H
#define FILTERMANAGE_H

#include <QObject>
#include "Global.h"
#include "kalmanfilter.h"
class FilterManage : public QObject
{
    Q_OBJECT
public:
    static FilterManage *getInstance(QObject *parent = nullptr);

    double filter(Cavity cavity,RealDataType realData,double value);

    double readData(Cavity cavity,RealDataType realData);

    void setRange(Cavity cavity,RealDataType realData,double range);

    void setRange(double range);

    double readRealData(Cavity cavity,RealDataType realData);

    void setRealData(Cavity cavity,RealDataType realData,double value);

    void setCoefficient(float coefficient);

    void setCoefficient(Cavity cavity,RealDataType realData,float coefficient);

    void clear();
private:
    KalmanFilter *getFilter(Cavity cavity,RealDataType realData);

private:
    float m_coefficient = 1;
    QMap<std::pair<Cavity,RealDataType>,KalmanFilter*> filterMap;
    explicit FilterManage(QObject *parent = nullptr);
    static FilterManage *pFilter;

signals:

};

#endif // FILTERMANAGE_H
