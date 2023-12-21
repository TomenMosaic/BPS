#include "baseservice.h"

BaseService::BaseService(QObject *parent)
    : QObject{parent}
{

}

void BaseService::setAutoReconnect(bool connect, int time)
{
    m_autoReconnect = connect;
    m_reconnectTime = time;
}

bool BaseService::isAutoReconnect()
{
    return m_autoReconnect;
}

void BaseService::setDataPacketFormat(QByteArray header, QByteArray tailer, int length)
{
    m_header = header;
    m_tailer = tailer;
    m_length = length;
    if((m_header.length()+m_tailer.length()>=m_length))
    {
        m_length = 0;
    }
}
