#include "medianfilter.h"
#include <QDebug>
MedianFilter::MedianFilter(QObject *parent) : Algorithm(parent)
{
    connect(&m_timer, &QTimer::timeout, [this](){
        if (m_buffer.size() > 0) {
            std::sort(m_buffer.begin(), m_buffer.end());
            int size = m_buffer.size();
            double median = 0.0;
            if (size % 2 == 0) {
                median = (m_buffer[size / 2 - 1] + m_buffer[size / 2]) / 2.0;
            } else {
                median = m_buffer[size / 2];
            }
            emit output(median);
            m_buffer.clear();
        }
    });
    m_timer.start(1000);
}

MedianFilter::~MedianFilter()
{
}

double MedianFilter::filter(double input)
{
    m_buffer.append(input);
    return input;
}

double MedianFilter::readData()
{
    if (m_buffer.size() > 0) {
        std::sort(m_buffer.begin(), m_buffer.end());
        int size = m_buffer.size();
        double median = 0.0;
        if (size % 2 == 0) {
            median = (m_buffer[size / 2 - 1] + m_buffer[size / 2]) / 2.0;
        } else {
            median = m_buffer[size / 2];
        }
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"m_buffer:"<<m_buffer;
        m_buffer.clear();
        return median;
    }
    else
        return 0.0;
}

void MedianFilter::clear()
{
    m_buffer.clear();
}
