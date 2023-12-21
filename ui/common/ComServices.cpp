#include "comservices.h"
#include <QDebug>
#include <QEventLoop>
#include <QThread>
#include <QTimer>
#include "log.h"
ComServices::ComServices(QObject *parent)
    : BaseService{parent}
{
    m_serialPort = new QSerialPort;
    qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError&");
}

bool ComServices::isOpen()
{
    return m_serialPort->isOpen();
}

void ComServices::setProperty(const QString &port, const int &baudrate, QSerialPort::DataBits dataBit, QSerialPort::StopBits stopBit, QSerialPort::Parity parity)
{
    m_port = port;
    m_baudrate = baudrate;
    m_serialPort->setPortName(m_port);
    m_serialPort->setBaudRate(m_baudrate);
    m_serialPort->setDataBits(dataBit);
    m_serialPort->setStopBits(stopBit);
    m_serialPort->setParity(parity);
}

bool ComServices::open()
{
    if(m_serialPort==nullptr)
        return false;
    qDebug()<<Q_FUNC_INFO<<__LINE__<<" port:"<<m_serialPort->portName()<<" baudrate:"<<m_serialPort->baudRate();
    disconnect(m_serialPort,SIGNAL(aboutToClose()),this,SLOT(close()));
    disconnect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receviceData()));//有数据就读
    disconnect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onError(QSerialPort::SerialPortError)));
    if(m_serialPort->isOpen())
        m_serialPort->close();

    bool ok = m_serialPort->open(QIODevice::ReadWrite);

    if(ok)
    {
        if(m_serialPort->isOpen())
        {
            connect(m_serialPort,SIGNAL(aboutToClose()),this,SLOT(close()), Qt::QueuedConnection);
            connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receviceData()), Qt::QueuedConnection);//有数据就读
            connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onError(QSerialPort::SerialPortError)));
            return true;
        }
        else
            return false;
    }
    else
    {
        return false;
    }

}

void ComServices::setAutoReconnect(bool connect, int time)
{
    this->m_autoReconnect = connect;
    this->m_reconnectTime = time;
}

bool ComServices::isAutoReconnect()
{
    return this->m_autoReconnect;
}


void ComServices::close()
{
    disconnect(m_serialPort,SIGNAL(aboutToClose()),this,SLOT(close()));
    disconnect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receviceData()));//有数据就读
    disconnect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onError(QSerialPort::SerialPortError)));
    m_serialPort->close();
}

bool ComServices::SendData(QByteArray writeData)
{
    if (nullptr == m_serialPort)
    {
        return false;
    }

    if (!m_serialPort->isOpen())
    {
        return false;
    }

    if(writeData.isEmpty())
    {
        return false;
    }
    //QMutexLocker locker(&serialWriteMutex);
    //            qDebug()<<Q_FUNC_INFO<<__LINE__<<"writeData:"<<writeData.toHex();
    if(!m_commandList.contains(writeData))
        m_commandList.append(writeData);

    QMutexLocker locker(&mutex);
    //    m_writeCmdList.append(QPair<QByteArray,int>{writeData,1});
    CLOG_INFO("send Data:"+writeData);
    qint64 length = m_serialPort->write(writeData.data(), writeData.size());
    emit updateData(writeData,Mode::write);
    if(length != writeData.size())
    {
        if(!m_cmdTimer||!m_cmdTimer->isActive())
        {
            if(!m_cmdTimer)
            {
                m_cmdTimer = new QTimer;
                connect(m_cmdTimer,SIGNAL(timeout()),this,SLOT(onCmdTimeOut()));
            }
            m_cmdTimer->start(1000);
        }

        return false;
    }
    else
    {
        m_commandList.removeAll(writeData);
        return true;
    }

}



void ComServices::onTimeOut()
{
    if(m_serialPort)
    {
        //        if(!m_serialPort->isOpen())
        //            m_serialPort->open(QIODevice::ReadWrite);
        //        bool ok = m_serialPort->isOpen();

        if(this->open())
        {
            qDebug()<<Q_FUNC_INFO<<__LINE__<<"重连成功"<<" serial_name:"<<m_serialPort<<"  serial_baudrate:"<<m_baudrate;
            emit reconnect();
            timer->stop();
            delete timer;
            timer = nullptr;
        }
        else
        {
            qDebug()<<Q_FUNC_INFO<<__LINE__<<"重连失败";
        }
    }
    else
    {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
}

void ComServices::onCmdTimeOut()
{
    if(m_serialPort==nullptr||!m_serialPort->isOpen()||!m_serialPort->isWritable())
    {
        for(int index  = 0;index<m_commandList.length();index++)
        {
            emit cmdExecOverTime(m_commandList.at(index).command);
        }
        m_commandList.clear();
        m_cmdTimer->stop();
    }
    int cmdLength = m_commandList.length();
    for(int index = 0;index<cmdLength;index++)
    {
        Command command = m_commandList.at(index);
        if(command.isOverTime())
        {
            bool ok = command.resend();
            if(!ok)
            {
                m_commandList.removeAt(index);
                index--;
                cmdLength = m_commandList.length();
                emit cmdExecOverTime(command.command);
                emit updateData(command.command,Mode::writeError);
                qDebug()<<Q_FUNC_INFO<<__LINE__<<" resend command over time:"<<command;

            }
            else
            {
                qDebug()<<Q_FUNC_INFO<<__LINE__<<" resend command:"<<command;
                emit updateData(command.command,Mode::write);
                m_serialPort->write(command.command.data(),command.command.size());
                m_commandList.replace(index,command);
            }
        }
    }
}


void ComServices::setDataPacketFormat(QByteArray header, QByteArray tailer, int length)
{
    m_header = header;
    m_tailer = tailer;
    m_length = length;
    if((m_header.length()+m_tailer.length()>=m_length))
    {
        m_length = 0;
    }
}

QString ComServices::port() const
{
    return m_port;
}

void ComServices::setPort(const QString &newPort)
{
    m_port = newPort;
}

int ComServices::baudrate() const
{
    return m_baudrate;
}

void ComServices::setBaudrate(int newBaudrate)
{
    m_baudrate = newBaudrate;
}
QByteArray ComServices::ModbusCRC16(QByteArray senddata)
{
    int len=senddata.size();
    uint16_t wcrc=0XFFFF;//预置16位crc寄存器，初值全部为1
    uint8_t temp;//定义中间变量
    int i=0,j=0;//定义计数
    for(i=0;i<len;i++)//循环计算每个数据
    {
        temp=senddata.at(i);
        wcrc^=temp;
        for(j=0;j<8;j++)
        {
            //判断右移出的是不是1，如果是1则与多项式进行异或。
            if(wcrc&0X0001)
            {
                wcrc>>=1;//先将数据右移一位
                wcrc^=0XA001;//与上面的多项式进行异或
            }
            else//如果不是1，则直接移出
            {
                wcrc>>=1;//直接移出
            }
        }
    }
    temp=wcrc;//crc的值
    QByteArray result = QByteArray::fromHex(QByteArray::number(wcrc,16));
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"senddata:"<<senddata<<" result:"<<result;
    return result;
}
void ComServices::postDevData(QByteArray data)
{
    QByteArray regonizeData = data.mid(m_header.length(),data.length()-m_header.length()-m_tailer.length()-2);
    QByteArray checkCode = data.mid(data.length()-2-m_tailer.length(),2);
    QByteArray reCheckCode = ModbusCRC16(regonizeData);
    if(checkCode!=reCheckCode)
        return;
    if(m_commandList.contains(data))
    {
        m_commandList.removeAll(data);
        CLOG_INFO("receive Data:"+data);
        if(m_cmdTimer&&m_commandList.isEmpty())
        {
            m_cmdTimer->stop();
        }
    }
    emit cmdFinish(data);
    emit updateData(data,Mode::read);
}

bool ComServices::containWriteData(QByteArray data)
{
    for(int index = 0;index<m_writeCmdList.length();index++)
    {
        QPair<QByteArray,int> oneWriteData = m_writeCmdList.at(index);
        if(oneWriteData.first == data)
            return true;
    }
    return false;
}



bool ComServices::containReadData(QByteArray data)
{
    for(int index = 0;index<m_readCmdList.length();index++)
    {
        QPair<QByteArray,int> oneWriteData = m_readCmdList.at(index);
        if(oneWriteData.first.left(7) == data.left(7))
            return true;
    }
    return false;
}

void ComServices::removeWriteData(QByteArray data)
{
    for(int index = m_writeCmdList.length()-1;index>=0;index--)
    {
        QPair<QByteArray,int> oneWriteData = m_writeCmdList.at(index);
        if(oneWriteData.first == data)
            m_writeCmdList.removeAt(index);
    }

}

void ComServices::removeReadData(QByteArray data)
{
    for(int index = m_readCmdList.length()-1;index>=0;index--)
    {
        QPair<QByteArray,int> oneWriteData = m_readCmdList.at(index);
        if(oneWriteData.first.left(7) == data.left(7))
            m_writeCmdList.removeAt(index);
    }
}

void ComServices::receviceData()
{
    if(m_serialPort)
    {
        QByteArray readDataStr = nullptr;
        readDataStr = m_serialPort->readAll();
        if(readDataStr.isEmpty())
            return;
        //如果没有限制条件则直接发送
        if(m_header.isEmpty()&&m_tailer.isEmpty()&&m_length<=0)
        {
            postDevData(readDataStr);
        }
        else
        {
            m_receviceData.append(readDataStr);
            if(m_receviceData.length()<m_length||m_receviceData.length()<m_tailer.length()+m_header.length())
                return;
            else
            {
                QByteArray curHeader = m_header.isEmpty()?m_tailer:m_header;
                QByteArray curTailer = m_tailer.isEmpty()?m_header:m_tailer;
                bool lengthFlag = false;
                if(m_length>m_header.length()+m_tailer.length())
                    lengthFlag = true;
                int first = m_receviceData.indexOf(curHeader);
                int second = m_receviceData.indexOf(curTailer,lengthFlag?first+m_length-curTailer.length():first+curHeader.length());
                do
                {
                    if(first==-1)
                    {
                        m_receviceData = m_receviceData.right(m_header.size()-1);
                        return;
                    }
                    if(second==-1)
                    {
                        m_receviceData.remove(0,first);
                        return;
                    }
                    QByteArray content = m_header+ m_receviceData.mid(first+curHeader.length(),second-first-curHeader.length())+m_tailer;
                    if(lengthFlag)
                    {
                        if(second-first+m_tailer.length()>m_length)
                        {
                            m_receviceData.remove(0,first);
                        }
                        else if(second-first+m_tailer.length()==m_length)
                        {
                            postDevData(content);
                            m_receviceData.remove(0,second);
                        }
                        else
                        {
                            throw("un exist condition:"+m_receviceData+" header:"+m_header+" tailer:"+m_tailer+" content:"+content);
                        }
                    }
                    else
                    {
                        postDevData(content);
                        m_receviceData.remove(0,second);
                    }
                    first = m_receviceData.indexOf(curHeader);
                    second = m_receviceData.indexOf(curTailer,lengthFlag?first+m_length-curTailer.length():first+curHeader.length());
                }
                while(first!=-1&&second!=-1);
            }
        }
    }
}

void ComServices::onError(QSerialPort::SerialPortError errorSIG)
{
    qDebug()<<Q_FUNC_INFO<<__LINE__<<" errorSIG:"<<errorSIG;
    CLOG_INFO(QString("connect Error=%1").arg(errorSIG).toUtf8());
    disconnect(m_serialPort,SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onError(QSerialPort::SerialPortError)));
    emit error(errorSIG);
    if(errorSIG == QSerialPort::ResourceError||errorSIG==QSerialPort::PermissionError
            ||QSerialPort::DeviceNotFoundError)
    {
        this->close();
        emit connectLost();
        if(timer)
        {
            timer->start(1000);
        }
        else
        {
            timer = new QTimer;
            timer->start(1000);
        }
        connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    }
}



bool ComServices::Command::operator ==(const ComServices::Command &otherCommand)
{
    if(this->command==otherCommand.command||(this->command.left(7)==otherCommand.command.left(7)&&this->command.mid(5,1)==QByteArray("\x02",1)))
        return true;
    else
        return false;

}

bool ComServices::Command::operator ==(QByteArray otherCommand)
{
    qDebug()<<Q_FUNC_INFO<<__LINE__<<this->command.toHex().mid(5,1);

    if(this->command==otherCommand||(this->command.left(7)==otherCommand.left(7)&&this->command.toHex().mid(5,1)==QByteArray("\x02")))
        return true;
    else
        return false;
}

bool ComServices::Command::isOverTime()
{
    if(lastSendTime.secsTo(QDateTime::currentDateTime())>2)
        return true;
    else
        return false;
}

ComServices::Command::Command(QByteArray cmd):command(cmd)
{
    resendTime = 1;
    lastSendTime = QDateTime::currentDateTime();
}

bool ComServices::Command::resend()
{
    if(resendTime>=3)
        return false;
    else
    {
        resendTime ++;
        lastSendTime = QDateTime::currentDateTime();
        return true;
    }
}

