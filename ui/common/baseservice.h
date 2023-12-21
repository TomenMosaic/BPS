#ifndef BASESERVICE_H
#define BASESERVICE_H

#include <QObject>
#include "GlobalVar.h"
class BaseService : public QObject
{
    Q_OBJECT
public:
    explicit BaseService(QObject *parent = nullptr);

public slots:
    virtual bool open()=0;
    virtual void close()=0;
    virtual bool isOpen()=0;
    virtual void setAutoReconnect(bool connect,int time);
    virtual bool isAutoReconnect();
    virtual void setDataPacketFormat(QByteArray header,QByteArray tailer,int length);
    virtual bool SendData(QByteArray) = 0;
private:

signals:
    void cmdExecOverTime(QByteArray cmd);
    void reconnect();
    void connectLost();
    void updateData(const QByteArray,Mode mode);
protected:
    QByteArray m_receviceData = "";
    int m_length = 15;
    QByteArray m_header = "";
    QByteArray m_tailer = "";
    bool m_autoReconnect = false;
    int m_reconnectTime = 100;
};

#endif // BASESERVICE_H
