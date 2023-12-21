#include "devmanage.h"
#include "comservices.h"
#include "qdebug.h"
#include "log.h"
#include <QBitArray>
#include <QMetaEnum>
#include <QTimer>
DevManage *DevManage::m_devManage = nullptr;

DevManage::DevManage(QObject *parent)
    : QObject{parent}
{
    m_device = new ComServices;
    m_device->setDataPacketFormat(m_header,m_tailer,m_length);
    comThread = new QThread;
    m_device->moveToThread(comThread);
    comThread->start();
    qRegisterMetaType<Mode>("Mode");
    qRegisterMetaType<Instruct>("Instruct");
    qRegisterMetaType<Cavity>("Cavity");

    m_calibrationAl = new CalibrationAlgorithms(m_cavityNum,this);
    m_calibrationAl->init();
    connect(m_device,SIGNAL(updateData(QByteArray,Mode)),this,SLOT(onReceiveData(QByteArray,Mode)));
    connect(m_device,SIGNAL(connectLost()),this,SIGNAL(connectLost()));
    connect(m_device,SIGNAL(reconnect()),this,SIGNAL(reconnect()));
    connect(m_device,SIGNAL(connectLost()),this,SLOT(changeDevState()));
    connect(m_device,SIGNAL(reconnect()),this,SLOT(changeDevState()));
    connect(this,SIGNAL(cmdSend(QByteArray)),m_device,SLOT(SendData(QByteArray)),Qt::BlockingQueuedConnection);
}

int DevManage::getCavityNum() const
{
    return m_cavityNum;
}

void DevManage::setCavityNum(int cavityNum)
{
    if(cavityNum<=0)
        cavityNum = 1;
    m_cavityNum = cavityNum;
    if(m_calibrationAl)
    {
        m_calibrationAl->init();
    }
}

DevState DevManage::getState() const
{
    return m_state;
}

void DevManage::setState(const DevState &state)
{
    if(m_state!=state)
    {
        m_state = state;
        emit stateChange(m_state);
    }
}

DevManage *DevManage::getInstance(QObject *parent)
{
    if(m_devManage == nullptr)
    {
        DevManage::m_devManage = new DevManage(parent);
    }
    return  DevManage::m_devManage;
}

bool DevManage::isOpen()
{
    return m_device->isOpen();
}

void DevManage::close()
{
    return m_device->close();
}

void DevManage::setSerialPort(QString newSerialPort, int newBaudrate)
{
    static_cast<ComServices*>(m_device)->setProperty(newSerialPort,newBaudrate);
}

bool DevManage::open()
{
    bool ok =  m_device->open();
    if(ok)
        setState(connected);
    else
        setState(unconnect);
    return ok;
}

bool DevManage::readData(Instruct instruct, Cavity cavity)
{
    QByteArray cmd;
    cmd.resize(m_length);
    QByteArray lengthArr = convertIntToArr((unsigned int)(m_length-m_header.length()-m_tailer.length()),1);
    QByteArray instructArr = convertIntToArr((unsigned int)(instruct),2);
    QByteArray address = convertIntToArr((unsigned int)(Mode::read),1) +convertIntToArr((unsigned int)(cavity),1);
    QByteArray valueArr("\x00\x00\x00\x00",4);
    QByteArray checkArr = ModbusCRC16(QByteArray(lengthArr+instructArr+address+valueArr));
    cmd = m_header +lengthArr+instructArr+address+valueArr+checkArr+m_tailer;
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"read cmd:"<<cmd.toHex();
    bool result =  cmdSend(cmd);
    return result;
}

bool DevManage::writeData(Instruct instruct, Cavity cavity, QVariant value, bool isModify)
{
    QByteArray cmd;
    cmd.resize(m_length);
    QByteArray lengthArr = convertIntToArr((unsigned int)(m_length-m_header.length()-m_tailer.length()),1);
    QByteArray instructArr = convertIntToArr((unsigned int)(instruct),2);
    QByteArray address = convertIntToArr((unsigned int)(Mode::write),1) +convertIntToArr((unsigned int)(cavity),1);

    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<" instructArr:"<<instructArr.toHex()<<  "ConvertIntToIEE754((unsigned int)(Mode::write),1):"
    //           <<ConvertIntToIEE754((unsigned int)(Mode::write),1).toHex()<<"  cavity:"<<ConvertIntToIEE754((unsigned int)(cavity),1).toHex();
    QByteArray valueArr;
    valueArr.resize(4);
    DataType type = getDataType(instruct);
    switch (type)
    {
    case floatType:
        //写入校准后的值
        if(isModify)
        {
            if(instruct ==  temperatureSetup)
            {
                float oldValue = value.toFloat();
                value = m_calibrationAl->getCalibrationSetupValue(cavity,temperature,value.toFloat());
                qDebug()<<Q_FUNC_INFO<<__LINE__<<"oldValue:"<<oldValue<<"  value:"<<value;
            }
            else if(instruct == humiditySetup)
            {
                value = m_calibrationAl->getCalibrationSetupValue(cavity,humid,value.toFloat());
            }
            else if(instruct == cavityTrafficSetup)
            {
                value = m_calibrationAl->getCalibrationSetupValue(cavity,humidFlow,value.toFloat());
            }
        }
        valueArr = IEEE754FloatToHexStr(value.toFloat());

        break;
    case unsignedIntType:
        valueArr = convertIntToArr(value.toInt(),4);
        break;

    case customType:
        valueArr = value.toByteArray();
        break;
    default:
        return false;
        break;
    };
    QByteArray checkArr = ModbusCRC16(QByteArray(lengthArr+instructArr+address+valueArr));
    cmd = m_header +lengthArr+instructArr+address+valueArr+checkArr+m_tailer;
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"cmd:"<<cmd;
    bool ok =  cmdSend(cmd);
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"********ok"""""""""":"<<ok;
    return ok;
}

void DevManage::onReceiveData(QByteArray receiveData, Mode mode)
{
    Instruct instruct = getCmdInstruct(receiveData);
    Cavity cavity = getCavity(receiveData);
    QVariant result = getResult(instruct,receiveData);
    QVariant modifyValue = result;
    bool isReal = true;

    if(instruct!= temperature&&instruct!=humidFlow&&instruct!=infraredSensorVal&&instruct!=humid)
    {
        isReal = false;
        if(instruct==temperatureSetup)
        {
            modifyValue = m_calibrationAl->getCalibrationValue(cavity,temperature,result.toDouble());
        }
        else if(instruct==humiditySetup)
        {
            modifyValue = m_calibrationAl->getCalibrationValue(cavity,humid,result.toDouble());
        }
        else if(instruct == cavityTrafficSetup)
        {
            modifyValue = m_calibrationAl->getCalibrationValue(cavity,humidFlow,result.toDouble());
        }
    }
    else
    {
        modifyValue = m_calibrationAl->getCalibrationValue(cavity,instruct,result.toDouble());
    }
    QString modeStr;
    switch (mode)
    {
    case Mode::read:
        modeStr = "read";
        break;
    case Mode::write:
        modeStr = "write";
        break;
    case Mode::writeError:
        modeStr = "writeError";
        break;
    default:
        break;
    }

    if(instruct==Instruct::flowControl)
    {
        QList<int>valueList = result.value<QList<int>>();
        valueList.removeFirst();
        flowList = valueList;
        emit tapChange(valueList);
    }
    else if(instruct==Instruct::gateDebug)
    {
        QList<int> valueList = result.value<QList<int>>();
        flowList = valueList;
        emit tapChange(valueList);
    }
    emit valueChange(instruct,cavity,result,modifyValue);
    if(mode==Mode::read)
    {
        CLOG_INFO(receiveData.toHex());
        if(!isWriteReport(receiveData))
        {
            if(instruct==Instruct::gateDebug)
            {
                emit tapChange(flowList);
            }
        }
        emit recordChange(modeStr,receiveData.toHex(),result,modifyValue,isReal);
        if(instruct==Instruct::temperatureDebug
                ||instruct == Instruct::flowControl)
        {
            emit operationChange(instruct,cavity,result.toInt());
        }
    }
    else
    {
        emit recordChange(modeStr,receiveData.toHex(),result,modifyValue,false);
    }

}

Instruct DevManage::getCmdInstruct(const QByteArray &data)
{
    QByteArray cmdInstruct = data.mid(3,2);
    Instruct instruct = Instruct(hexStrToInt(cmdInstruct));

    return instruct;
}

Cavity DevManage::getCavity(const QByteArray &data)
{
    QByteArray cavityInstruct = data.mid(6,1);
    Cavity cavity = Cavity(hexStrToInt(cavityInstruct));
    return cavity;
}

QByteArray DevManage::getData(const QByteArray &data)
{
    QByteArray result = data.mid(7,4);
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<" result:"<<result<<"  data:"<<data;
    return result;
}
DataType DevManage::getDataType(Instruct instruct)
{
    switch (Instruct(instruct))
    {
    case temperatureSetup:
    case kpSetup:
    case TiSetup:
    case TdSetup:
    case humidKpSetup:
    case humidTiSetup:
    case humidTdSetup:
    case temperature:
    case humid:
    case humidFlow:
    case humiditySetup:
    case infraredSensorVal:
    case cavityTrafficSetup:
    case humidDebug:
        //        qDebug()<<Q_FUNC_INFO<<__LINE__<<"float value:"<<value.toFloat();
        return DataType::floatType;
        break;
    case temperatureControl:
    case temperatureDebug:
    case dataAutoTranslate:
    case temperatureSelftuningControl:
    case remoteUpgrade:
    case version:
    case humidSelftuningControl:
    case humidControl:
        //        qDebug()<<Q_FUNC_INFO<<__LINE__<<"int value:"<<value.toInt();
        return DataType::unsignedIntType;
        break;
    case deviceID:
    case flowControl:
    case gateDebug:
        return DataType::customType;
        //        qDebug()<<Q_FUNC_INFO<<__LINE__<<"custom value:"<<value.toInt();
        break;
    default:
        return DataType::customType;
        break;
    };
}

QVariant DevManage::getResult(Instruct instruct, QByteArray result)
{
    DataType dataType = getDataType(instruct);
    QByteArray lastResult = getData(result);
    QVariant endResult = lastResult;

    if(dataType==floatType)
    {
        endResult =  IEEE754HexStrToFloat(lastResult);
    }
    else if(dataType==unsignedIntType)
    {
        endResult =  hexStrToInt(lastResult);
    }
    else
    {
        if(instruct==flowControl)
        {
            QByteArray flowIDArr =  lastResult.left(2);
            QByteArray tapIDsArr = lastResult.mid(2,3);
            QList<int>intsList = intsFromByteArr(tapIDsArr);
            intsList.push_front(flowIDArr.toInt());
            endResult = QVariant::fromValue<QList<int>>(intsList);
        }
        else if(instruct == gateDebug)
        {
            QList<int>intsList = intsFromByteArr(lastResult);
            endResult = QVariant::fromValue<QList<int>>(intsList);
        }
    }
    return endResult;
}

void DevManage::changeDevState()
{
    if(m_device)
    {
        if(m_device->isOpen())
        {
            setState(connected);
            return;
        }
    }
    setState(unconnect);
}

bool DevManage::isWriteReport(QByteArray &byteArray)
{
    if(byteArray.size()>6)
    {
        return byteArray.toHex().mid(5,1)==QByteArray("\x01");
    }
    return false;
}

QByteArray DevManage::bitsToBytes(const QBitArray &bits)
{
    QByteArray bytes;
    bytes.resize(bits.count() / 8 + ((bits.count() % 8)? 1: 0));
    bytes.fill(0x00);
    for (int b = 0; b < bits.count(); ++b)
        bytes[b / 8] = ( bytes.at(b / 8) | ((bits[b] ? 1: 0) << (7 - (b % 8))));
    return bytes;
}

QBitArray DevManage::bytesToBits(const QByteArray &bytes)
{
    QBitArray bits(bytes.count() * 8);
    for (int i = 0; i < bytes.count(); ++i)
        for (int b = 0; b < 8; ++b)
            bits.setBit(i * 8 + b, bytes.at(i) & (0x01 << (7 - b)));
    return bits;
}

QByteArray DevManage::byteArrFormInts(QList<int> intList, int byteLength)
{
    QBitArray bitArr;
    bitArr.resize(byteLength*8);
    bitArr.fill(false);
    for(int index = 0;index<intList.length();index++)
    {
        int value = intList.at(index);
        if(value<=byteLength*8)
        {
            bitArr.setBit(byteLength*8-value,true);
        }
    }
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"bitArr:"<<bitArr;
    return bitsToBytes(bitArr);
}

QList<int> DevManage::intsFromByteArr(QByteArray array)
{
    QList<int> intList;
    QBitArray bitArr = bytesToBits(array);
    for(int index = 0;index<bitArr.size();index++)
    {
        if(bitArr.at(index))
        {
            intList.append(bitArr.size()-index);
        }

    }
    std::sort(intList.begin(),intList.end());
    return intList;
}

void DevManage::reInitAlgorithms()
{
    m_calibrationAl->reCaculate();
}

QByteArray DevManage::IEEE754FloatToHexStr(float value)
{
    const quint32 *i = reinterpret_cast<const quint32 *>(&value);
    QByteArray ba;
    ba.append(char(*i >> 24));
    ba.append(char(*i >> 16));
    ba.append(char(*i >>  8));
    ba.append(char(*i >>  0));
    return ba;
}

float DevManage::IEEE754HexStrToFloat(QByteArray str)
{
    //    const QByteArray ba = QByteArray::fromHex(str);
    const QByteArray ba = str;
    if (ba.size() != 4)
    {
        qWarning()<<Q_FUNC_INFO<<__LINE__<<("str length errror:"+str);
        throw("str length errror:"+str);
        return 0;
    }

    quint32 word = quint32((quint8(ba.at(0)) << 24) |
                           (quint8(ba.at(1)) << 16) |
                           (quint8(ba.at(2)) <<  8) |
                           (quint8(ba.at(3)) <<  0));

    const float *f = reinterpret_cast<const float *>(&word);
    return *f;
}



QByteArray DevManage::convertIntToArr(unsigned int value, int byteLength)
{
    QString str16 = QString("%1").arg(value, byteLength*2, 16, QLatin1Char('0'));
    QByteArray result = QByteArray::fromHex(str16.toUtf8());
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<" value:"<<value<<" result:"<<result<<" hex:"<<result.toHex();
    return result;
}

unsigned int DevManage::hexStrToInt(QByteArray str)
{
    bool ok = false;
    str = str.toHex();
    unsigned int result = str.toUInt(&ok,16);
    if(ok)
    {
        return result;
    }
    else
    {
        qWarning()<<Q_FUNC_INFO<<__LINE__<<QString("IEEE754HexStrToInt Fail:")+str;
        throw(QString("IEEE754HexStrToInt Fail:%1")+str);
        return 0;
    }
}

QByteArray DevManage::ModbusCRC16(QByteArray senddata)
{
    int len=senddata.size();
    uint16_t wcrc=0XFFFF;//预置16位crc寄存器，初值全部为1
    uint8_t temp;//定义中间变量
    int i=0,j=0;//定义计数
    for(i=0;i<len;i++)//循环计算每个数据
    {
        temp=senddata.at(i);
        wcrc^=temp;
        for(j=0;j<8;j++){
            //判断右移出的是不是1，如果是1则与多项式进行异或。
            if(wcrc&0X0001){
                wcrc>>=1;//先将数据右移一位
                wcrc^=0XA001;//与上面的多项式进行异或
            }
            else//如果不是1，则直接移出
                wcrc>>=1;//直接移出
        }
    }
    temp=wcrc;//crc的值
    QByteArray result = QByteArray::fromHex(QByteArray::number(wcrc,16));
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"senddata:"<<senddata<<" result:"<<result;
    return result;
}

