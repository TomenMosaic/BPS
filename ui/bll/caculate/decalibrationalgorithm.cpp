#include "decalibrationalgorithm.h"
#include <QDebug>
#include <QtCore>
#include <numeric>
DecalibrationAlgorithm::DecalibrationAlgorithm(QObject *parent) : QObject(parent)
{

}

DecalibrationAlgorithm::~DecalibrationAlgorithm()
{
    if(spline!=nullptr)
    {
        gsl_spline_free (spline);
    }
    if(acc!=nullptr)
    {
        gsl_interp_accel_free (acc);
    }
}

void DecalibrationAlgorithm::setValueList(QVector<double> xValues, QVector<double> yValues)
{
    maxValue = 0;
    minValue = 0;
    if(spline!=nullptr)
    {
        gsl_spline_free (spline);
    }
    if(acc!=nullptr)
    {
        gsl_interp_accel_free (acc);
    }


    for(int i=0;i<xValues.size();i++){
        for(int j=i+1;j<xValues.size();j++){
            if(xValues[j]==xValues[i]){
                xValues.remove(i);
                yValues.remove(i);
                i--;
            }
        }
    }
    std::sort(xValues.begin(),xValues.end());
    std::sort(yValues.begin(),yValues.end());
    if(xValues.length()>0&&xValues.first()==0)
    {
        if(!xValues.contains(0.0005))
        {
            xValues.insert(1,0.0005);
            yValues.insert(1,0.00001);
        }
    }
    else
    {
        xValues.insert(0,0);
        xValues.insert(1,0.0005);
        yValues.insert(0,0);
        yValues.insert(1,0.00001);
    }
    if(xValues.length()<3||xValues.length()!=yValues.length())
    {
        return;
    }
    this->xValues = xValues;
    this->yValues = yValues;
    maxValue = *std::max_element(xValues.begin(),xValues.end());
    double maxYValue = *std::max_element(yValues.begin(),yValues.end());
    if(maxYValue!=0)
        maxk = maxValue/maxYValue;
    acc = gsl_interp_accel_alloc ();
    spline = gsl_spline_alloc (gsl_interp_cspline, xValues.length());
    gsl_spline_init (spline, &xValues[0], &yValues[0], xValues.length());
}


float DecalibrationAlgorithm::getCalibrationResult(float result)
{
    bool isPositive = true;
    if(result <0)
    {
        isPositive = false;
        result = -result;
    }
    double lastResult = 0;
    if(result>maxValue)
    {
        lastResult =  result/maxk;
    }
    else if(result>=minValue&&result<=maxValue)
    {
        lastResult = gsl_spline_eval (spline, result, acc);
    }
    else
    {
        lastResult = result/minK;
    }
    if(!isPositive)
    {
        lastResult = -lastResult;
    }
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"result:"<<result<<" lastResult:"<<lastResult;

    return lastResult;
}


bool DecalibrationAlgorithm::isUserful()
{
    if(spline==nullptr||maxValue==minValue||maxValue==0)
    {
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"un userful";
        return false;
    }
    else
        return true;
}


