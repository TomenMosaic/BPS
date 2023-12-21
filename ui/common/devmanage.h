#ifndef DEVICEMANAGE_H
#define DEVICEMANAGE_H

#include <QBitArray>
#include <QDateTime>
#include <QObject>
#include <QThread>
#include <QVariant>
#include "calibrationalgorithms.h"
#include "baseservice.h"
#include "GlobalVar.h"
class DevManage : public QObject
{
    Q_OBJECT
public:

    static DevManage *getInstance(QObject *parent = nullptr);

    bool isOpen();
    void close();
    DevState getState() const;
    void setState(const DevState &state);

    int getCavityNum() const;
    void setCavityNum(int cavityNum);

    static QByteArray byteArrFormInts(QList<int> intList, int byteLength);
    static QList<int> intsFromByteArr(QByteArray array);
    //将int型数据转化十进制QByteArray
    static QByteArray convertIntToArr(unsigned int value,int byteLength = 2);
    void reInitAlgorithms();

public slots:
    void setSerialPort(QString newSerialPort,int newBaudrate);
    bool open();
    bool readData(Instruct instruct, Cavity cavity);
    bool writeData(Instruct instruct, Cavity cavity, QVariant value,bool isModify = true);
    void onReceiveData(QByteArray receiveData, Mode mode);
    void changeDevState();

signals:
    bool cmdSend(QByteArray);
    //阀门控制
    void tapChange(QList<int>openTapList);
    //温控调试状态
    void temperateCavityChange(Cavity,int);
    //温控调试状态,放置样品操作,抽屉，夹具
    void operationChange(Instruct,Cavity,int);

    void recordChange(QString mode,QString cmd,QVariant value,QVariant modifyVal,bool isReal = true);
    void connectLost();
    void reconnect();
    //状态变化
    void stateChange(DevState);
    void valueChange(Instruct instruct,Cavity cavity,QVariant variant,QVariant modifyValue = QVariant());
protected:
    bool isWriteReport(QByteArray &byteArray);
    Instruct getCmdInstruct(const QByteArray &data);
    Cavity getCavity(const QByteArray &data);
    QByteArray getData(const QByteArray &data);
    DataType getDataType(Instruct instruct);
    QVariant getResult(Instruct instruct,QByteArray result);
    static QByteArray bitsToBytes(const QBitArray &bits);
    static QBitArray bytesToBits(const QByteArray &bytes);
    //将浮点数转化成十进制QByteArray
    QByteArray IEEE754FloatToHexStr(float value);
    //将十六进制的QByteArray的数据转化成float数
    float IEEE754HexStrToFloat(QByteArray str);

    //将十六进制的数据转化成INT型数据
    unsigned int hexStrToInt(QByteArray str);
    //senddata:要校验的数据 十进制ByteArray 结果：十进制的校验码（高位->低位）
    QByteArray ModbusCRC16(QByteArray senddata);
private:
    explicit DevManage(QObject *parent = nullptr);
private:
    QList<int> flowList;
    QMap<Cavity,QList<int>> trapMap;
    bool isRecord = false;
    CalibrationAlgorithms *m_calibrationAl = nullptr;
    int m_cavityNum = 3;
    static DevManage *m_devManage;
    BaseService *m_device = nullptr;
    QThread* comThread = nullptr;
    QByteArray m_header = "\x5A\xA5";
    QByteArray m_tailer = "\x0D\x0A";
    int m_length = 15;
    DevState m_state = unconnect;
signals:

};

#endif // DEVICEMANAGE_H
