#ifndef CALIBRATIONALGORITHM_H
#define CALIBRATIONALGORITHM_H

#include <QObject>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include <QDebug>
class DecalibrationAlgorithm : public QObject
{
    Q_OBJECT
public:
    explicit DecalibrationAlgorithm(QObject *parent = nullptr);

    ~DecalibrationAlgorithm();

    void setValueList(QVector<double> xValues, QVector<double> yValues);

    float getCalibrationResult(float result);

    bool isUserful();

private:
    gsl_interp_accel *acc = nullptr;
    gsl_spline *spline = nullptr;
    QVector<double>xValues;
    QVector<double>yValues;
    double minValue = 0;
    double maxValue = 0;
    double maxk = 1;
    double minK = 50;
signals:

};

#endif // CALIBRATIONALGORITHM_H
