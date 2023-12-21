#ifndef COMSERVICES_H
#define COMSERVICES_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#include "baseservice.h"
#include <QDateTime>
class ComServices : public BaseService
{
    Q_OBJECT
public:
    struct Command
    {
        bool operator ==(const Command &otherCommand);
        bool operator ==(QByteArray otherCommand);
        bool isOverTime();

        Command(QByteArray cmd);
        bool resend();
        friend QDebug& operator<<(QDebug &out, const Command& info)
        {
            out <<"command:"<<info.command.toHex()<<"  resendTime:"<<info.resendTime
               <<" lastSendTime:"<<info.lastSendTime;
            return out;
        }

        QByteArray command;
        int resendTime = 0;
        QDateTime lastSendTime;
    };
    explicit ComServices(QObject *parent = nullptr);
    //判断是否连接
    bool isOpen();

    void setProperty(const QString&port, const int& baudrate,QSerialPort::DataBits dataBit =QSerialPort::Data8,
                     QSerialPort::StopBits stopBit = QSerialPort::OneStop,
                     QSerialPort::Parity parity = QSerialPort::NoParity);
    //打开连接
    bool open();

    //设置自动连接和自动连接时间
    void setAutoReconnect(bool connect,int time);
    //获取自动连接状态
    bool isAutoReconnect();
    //设置头部跟尾部后会自动去除头部跟尾部，如果设置长度则会对长度进行校验
    void setDataPacketFormat(QByteArray header,QByteArray tailer,int length = -1);
    QString port() const;
    void setPort(const QString &newPort);

    int baudrate() const;

    void setBaudrate(int newBaudrate);

    QByteArray ModbusCRC16(QByteArray senddata);

public slots:
    void receviceData();


    void onError(QSerialPort::SerialPortError errorSIG);

    //关闭连接
    void close();

    //写数据
    bool SendData(QByteArray);

    void onTimeOut();

    void onCmdTimeOut();
signals:
    void error(QSerialPort::SerialPortError);

    void cmdFinish(QByteArray);


private:
    void postDevData(QByteArray data);
    bool containWriteData(QByteArray data);
    bool containReadData(QByteArray data);
    void removeWriteData(QByteArray data);
    void removeReadData(QByteArray data);
private:
    QTimer *m_cmdTimer = nullptr;
    QList<Command> m_commandList;
    QMutex mutex;
    QTimer *timer = nullptr;
    QList<QPair<QByteArray,int>> m_writeCmdList;
    QList<QPair<QByteArray,int>> m_readCmdList;
    //    QByteArrayList m_writeCmdList;
    //    QByteArrayList m_readCmdList;
    QSerialPort* m_serialPort = nullptr;
    QString m_port = "COM1";
    int m_baudrate = 115200;
};

#endif // COMSERVICES_H
