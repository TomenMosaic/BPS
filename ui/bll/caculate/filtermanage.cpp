#include "filtermanage.h"
FilterManage *FilterManage::pFilter = nullptr;

FilterManage *FilterManage::getInstance(QObject *parent)
{
    if(pFilter == nullptr)
    {
        FilterManage::pFilter = new FilterManage(parent);
    }

    return FilterManage::pFilter;
}

double FilterManage::filter(Cavity cavity, RealDataType realData, double value)
{
    return getFilter(cavity,realData)->filter(value);
}


double FilterManage::readData(Cavity cavity, RealDataType realData)
{
    return getFilter(cavity,realData)->readData();
}

void FilterManage::setRange(Cavity cavity, RealDataType realData, double range)
{
    return getFilter(cavity,realData)->setRange(range);
}

void FilterManage::setRange(double range)
{
    for(auto it = filterMap.begin();it!=filterMap.end();it++)
    {
        it.value()->setRange(range);
    }
}

double FilterManage::readRealData(Cavity cavity, RealDataType realData)
{
    return getFilter(cavity,realData)->getNewVal();
}

void FilterManage::setRealData(Cavity cavity, RealDataType realData, double value)
{
    return getFilter(cavity,realData)->setRealData(value);
}

void FilterManage::setCoefficient(float coefficient)
{
    m_coefficient = coefficient;
    for(auto it = filterMap.begin();it!=filterMap.end();it++)
    {
        it.value()->setCoefficient(coefficient);
    }
}

void FilterManage::setCoefficient(Cavity cavity, RealDataType realData, float coefficient)
{
    return getFilter(cavity,realData)->setCoefficient(coefficient);
}

void FilterManage::clear()
{
    for(auto it = filterMap.begin();it!=filterMap.end();it++)
    {
        it.value()->clear();;
    }
}

KalmanFilter *FilterManage::getFilter(Cavity cavity, RealDataType realData)
{
    if(filterMap.contains({cavity,realData}))
    {
        return filterMap.value({cavity,realData});
    }
    else
    {
        KalmanFilter *filter = new KalmanFilter(this);
        if(g_config->contains("filterValue"))
        {
            int filterValue = g_config->getRecord("filterValue").toInt();
            filter->setRange(filterValue);
        }
        filter->setCoefficient(m_coefficient);
        filterMap.insert({cavity,realData},filter);
        return filter;
    }
}

FilterManage::FilterManage(QObject *parent) : QObject(parent)
{
}
