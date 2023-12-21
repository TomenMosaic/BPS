#ifndef CALIBRATIONALGORITHMS_H
#define CALIBRATIONALGORITHMS_H

#include <QObject>
#include <Eigen/Dense>
#include "calibrationbll.h"
#include "GlobalVar.h"
using namespace  std;
using namespace  Eigen;
//保存一个腔体的某个数据的校准系数
struct CaculateData
{
    CaculateData (Cavity newCavity,Instruct newInstruct);
    Cavity cavity;
    Instruct instruct;

    double getCaculateValue(double firstValue);
    double getCaculateFirstValue(double lastValue);
    void addPoint(double xValue,double yValue);
    void clear();
    bool recaculate();
    QList<double>xList;
    QList<double>yList;
    double k = 1;
    double d = 0;
};

class CalibrationAlgorithms : public QObject
{
    Q_OBJECT

public:

    explicit CalibrationAlgorithms(int cavityNum ,QObject *parent = nullptr);

    ~CalibrationAlgorithms();

    void init();

    Instruct getInstruct(int value);

    CaculateData *getCaculate(Cavity cavity,Instruct instruct);

    double getCalibrationValue(Cavity cavity,Instruct instruct,double firstValue);

    double getCalibrationSetupValue(Cavity cavity,Instruct instruct,double lastValue);

    void clear();

public slots:
    void reCaculate();
signals:


private:
    int m_cavityNum =3;
    QList<CaculateData*>m_caculateDatas;
    CalibrationBLL *m_calibrationBll;

};

#endif // CALIBRATIONALGORITHMS_H
