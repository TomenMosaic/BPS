#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QObject>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QDebug>
#include <QtEndian>
#include <QTimer>
#include <QEventLoop>

class CommunicationField {
public:
    QString name;
    uint startAddress;
    uint length;
    QString datatype;
    QVariant value; // 使用 QVariant 来存储不同类型的值

public:
    CommunicationField(QString pName, uint pStartAddress, uint pLength, QString pDataType){
        this->name = pName;
        this->startAddress = pStartAddress;
        this->length = pLength;
        this->datatype = pDataType;
        //this->value = pValue;
    }
};

class ModbusClient : public QObject {
    Q_OBJECT

private:
    QMap<int, QVariant> m_cacheValues;

public:
    ModbusClient(QObject *parent = nullptr): QObject(parent), m_modbusClient(new QModbusTcpClient(this)) {
        connect(m_modbusClient, &QModbusClient::stateChanged, this, &ModbusClient::onStateChanged);
        connect(m_modbusClient, &QModbusClient::errorOccurred, this, &ModbusClient::onErrorOccurred);
    }

    ~ModbusClient(){
        if (m_modbusClient->state() == QModbusDevice::ConnectedState) {
            m_modbusClient->disconnectDevice();
        }
    }

    bool listen(const QString &serverAddress, int port){
        if(!m_modbusClient){
            return false;
        }

        if (m_modbusClient->state() != QModbusDevice::ConnectedState) {       //判断当前连接状态是否为断开状态
            m_modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, serverAddress);
            m_modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);

            if (!m_modbusClient->connectDevice()) {
                qDebug()<< "连接modbus设备失败";
                return false;
            }
            else {
                qDebug()<< "成功连接到modbs设备";
                return true;
            }
        }else{
            qDebug()<< "modbus client connected!";
            return false;
        }
    }

    bool checkConnect(){
        const int maxRetryCount = 6; // 最大重试次数
        int retryCount = 0; // 当前重试次数

        QString serverAddress = m_modbusClient->connectionParameter(QModbusDevice::NetworkAddressParameter).toString();
        int port = m_modbusClient->connectionParameter(QModbusDevice::NetworkPortParameter).toInt();

        while (retryCount < maxRetryCount) {
            if (!m_modbusClient || m_modbusClient->state() != QModbusDevice::ConnectedState) {
                qDebug() << "Modbus client is not connected. Trying to reconnect... Retry count:" << retryCount + 1;
                if (listen(serverAddress, port)) {
                    qDebug() << "Successfully connected to Modbus device.";
                    return true; // 连接成功，返回 true
                } else {
                    ++retryCount; // 连接失败，增加重试计数
                    qDebug() << "Failed to reconnect. Waiting for next retry...";
                }
            } else {
                if (retryCount > 0){
                    qDebug() << "Modbus client is already connected.";
                }
                return true; // 已连接，返回 true
            }
        }

        qDebug() << "Exceeded maximum retry attempts. Unable to connect to Modbus device.";
        return false; // 达到最大重试次数，返回 false
    }

    void printReadValues(const QList<CommunicationField> &fields, const QList<QVariant> &outValues) {
        QMap<int, QVariant> changedValues; // 使用 startAddress 作为键
        for (int i = 0; i < fields.size(); i++) {
            const auto& field = fields[i];
            const auto& value = outValues[i];
            bool hasChanged = false;

            if (this->m_cacheValues.contains(field.startAddress)) {
                hasChanged = this->m_cacheValues[field.startAddress].toString() != value.toString();
            } else {
                hasChanged = true;
            }

            if (hasChanged) {
                this->m_cacheValues[field.startAddress] = value;
                changedValues[i] = value; // 使用 index 作为键
            }
        }

        if (!changedValues.isEmpty()) {
            QStringList messages;
            for (auto it = changedValues.constBegin(); it != changedValues.constEnd(); ++it) {
                const auto& field = fields[it.key()];
                QString msg = QString("name: %1, address: %2, type: %3, value: %4")
                        .arg(field.name)
                        .arg(it.key())
                        .arg(field.datatype)
                        .arg(it.value().toString());
                messages.append(msg);
            }
            qDebug() << "read >> " << messages.join(";");
        }
    }

    // 读取单个寄存器
    void readSingleRegister(const CommunicationField& field,
                            std::function<void(bool, QVariant)> callback,
                            int serverAddress = 1) {
        QList<CommunicationField> fields;
        fields.append(field);

        // 调用 readHoldingRegisters，传递封装好的字段列表
        readHoldingRegisters(fields, [callback, fields](bool success, QList<QVariant> values) {
            if (success) {
                // 假设 results 中的第一个 QVariant 包含所需的值
                callback(true, values.first());
            } else {
                callback(false, 0); // 读取失败或结果为空
            }
        }, serverAddress);
    }

    void readHoldingRegisters(const QList<CommunicationField> &fields,
                              std::function<void(bool, QList<QVariant>)> callback,
                              int serverAddress = 1) {
        // 首先检查 Modbus 客户端的连接状态
        if (!checkConnect()) {
            callback(false, QList<QVariant>{}); // 使用回调报告错误
            return;
        }

        int startAddress, endAddress;
        std::tie(startAddress, endAddress) = determineReadRange(fields);

        QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddress, endAddress - startAddress);
        auto *reply = m_modbusClient->sendReadRequest(readUnit, serverAddress);
        if (!reply) {
            callback(false, QList<QVariant>{});
            return;
        }

        if (reply->isFinished()) {
            delete reply;
            callback(false, QList<QVariant>{});
            return;
        }

        connect(reply, &QModbusReply::finished, this, [this, startAddress, reply, fields, callback]() mutable {
            //handleReply(reply, fields, startAddress, callback);
            if (reply->error() != QModbusDevice::NoError) {
                qWarning() << "Modbus Error:" << reply->error() << reply->errorString();
                callback(false, QList<QVariant>{});
                return;
            }

            const QModbusDataUnit unit = reply->result();
             QList<QVariant> values;
            for (auto &field : fields) {
              auto value = parseFieldData(unit, field, startAddress);
              values.append(value);
            }
            callback(true, values);

            // print
            printReadValues(fields, values);

            reply->deleteLater();
        });
    }


    bool readHoldingRegistersSync(const QList<CommunicationField> &fields, QList<QVariant> &outValues, int serverAddress = 1) {
        // 检查Modbus客户端的连接状态
        if (!checkConnect()) {
            return false;
        }

        int startAddress, endAddress;
        std::tie(startAddress, endAddress) = determineReadRange(fields);

        QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddress, endAddress - startAddress);
        QModbusReply *reply = m_modbusClient->sendReadRequest(readUnit, serverAddress);
        if (!reply) {
            return false;
        }

        // 使用事件循环等待操作完成
        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
        timeoutTimer.start(6666); // 设置超时时间，例如300毫秒
        loop.exec();

        if (reply->isFinished()) {
            if (reply->error() == QModbusDevice::NoError) {
                const QModbusDataUnit unit = reply->result();
                for (const auto &field : fields) {
                    outValues.append(parseFieldData(unit, field, startAddress));
                }

                // print
                printReadValues(fields, outValues);

                reply->deleteLater();
                return true;
            } else {
                qWarning() << "Modbus Error:" << reply->error() << reply->errorString();
            }
        }

        reply->deleteLater();
        return false;
    }

    bool readSingleRegisterSync(CommunicationField field, QVariant &value, int serverAddress = 1) {
        QList<CommunicationField> fields;
        fields.append(field);

        QList<QVariant> values;
        readHoldingRegistersSync(fields, values);
        if (!values.isEmpty()) {
            value = values.first();
            return true;
        }

        return false;
    }


    // 写入单个寄存器
    void writeSingleRegister(CommunicationField field,
                             std::function<void(bool)> callback,
                             int serverAddress = 1) {
        // 创建一个包含单个字段的列表
        QList<CommunicationField> fields;
        fields.append(field);

        // 调用 writeRegisters 方法，传递字段列表
        writeRegisters(fields, callback, serverAddress);
    }

    // 批量写入寄存器的方法
    void writeRegisters(QList<CommunicationField> &fields,
                        std::function<void(bool)> callback,
                        int serverAddress = 1) {
        if (fields.isEmpty()) {
            callback(false); // 如果字段列表为空，则立即调用回调并返回
            return;
        }

        // 首先检查 Modbus 客户端的连接状态
        if (!checkConnect()) {
            callback(false); // 使用回调报告错误
            return;
        }

        // 寄存器类型
        QModbusDataUnit::RegisterType type = QModbusDataUnit::HoldingRegisters;

        // 确定写入范围
        int startAddress = fields.first().startAddress;
        int endAddress = fields.last().startAddress;
        QModbusDataUnit writeUnit(type, startAddress, endAddress - startAddress + 1);

        // 设置要写入的值
        for (const auto &field : fields) {
            int index = field.startAddress - startAddress;

            if (field.datatype.toLower() == "int32" || field.datatype.toLower() == "float") {
                // 假设 field.value 是一个32位整数或浮点数
                quint32 value = field.value.toUInt();
                quint16 highWord = value >> 16; // 提取高16位
                quint16 lowWord = value & 0xFFFF; // 提取低16位

                // 交换高低字以匹配 CDAB 格式
                writeUnit.setValue(index, lowWord); // 先写低字
                writeUnit.setValue(index + 1, highWord); // 再写高字
            } else {
                // 对于其他数据类型（例如 int16），直接写入
                writeUnit.setValue(index, field.value.toUInt());
            }
        }

        // 发送写请求
        if (auto *reply = m_modbusClient->sendWriteRequest(writeUnit, serverAddress)) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, [this, reply, callback]() {
                    callback(reply->error() == QModbusDevice::NoError); // 回调执行结果
                    reply->deleteLater();
                });
            } else {
                delete reply;
                callback(false); // 立即失败回调
            }
        } else {
            callback(false); // 无法发送请求回调
        }
    }

    bool writeRegisterSync(const CommunicationField &field, int serverAddress = 1) {

        // 创建一个包含单个字段的列表
        QList<CommunicationField> fields;
        fields.append(field);

        return writeRegistersSync(fields, serverAddress);
    }


    bool writeRegistersSync(const QList<CommunicationField> &fields, int serverAddress = 1) {
        if (fields.isEmpty()) {
            return false;
        }

        if (!checkConnect()) {
            return false;
        }

        bool allWritesSuccessful = true;
        QList<CommunicationField> contiguousFields;
        int previousEndAddress = -1;

        // 分割为连续的寄存器块并写入
        for (const auto &field : fields) {
            int fieldEndAddress = field.startAddress + fieldLength(field) - 1;  // 计算字段结束地址
            if (previousEndAddress != -1 && field.startAddress > previousEndAddress) {
                if (!writeAndVerifySync(contiguousFields, serverAddress)) {
                    allWritesSuccessful = false;
                }
                contiguousFields.clear();
            }

            contiguousFields.append(field);
            previousEndAddress = fieldEndAddress;  // 更新上一个字段的结束地址
        }

        // 处理最后一批连续字段
        if (!contiguousFields.isEmpty()) {
            if (!writeAndVerifySync(contiguousFields, serverAddress)) {
                allWritesSuccessful = false;
            }
        }

        return allWritesSuccessful;
    }

    bool writeAndVerifySync(const QList<CommunicationField> &fields, int serverAddress) {
        if (fields.isEmpty()) {
            return false; // 如果没有字段
        }

        // 确定写入范围
        int startAddress = fields.first().startAddress;
        int endAddress = fields.last().startAddress + fields.last().length - 1;
        QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, startAddress, endAddress - startAddress + 1);

        // 设置要写入的值
        for (const auto &field : fields) {
            int index = field.startAddress - startAddress;

            if (field.datatype.toLower() == "int32" || field.datatype.toLower() == "float") {
                quint32 value = field.value.toUInt();
                quint16 highWord = value >> 16; // 提取高16位
                quint16 lowWord = value & 0xFFFF; // 提取低16位

                // 交换高低字以匹配 CDAB 格式
                writeUnit.setValue(index, lowWord); // 先写低字
                writeUnit.setValue(index + 1, highWord); // 再写高字
            } else {
                // 对于其他数据类型（例如 int16），直接写入
                writeUnit.setValue(index, field.value.toUInt());
            }
        }

        // 发送写请求
        auto *reply = m_modbusClient->sendWriteRequest(writeUnit, serverAddress);
        if (!reply) {
            qWarning() << "Failed to send write request";
            return false;
        }

        // 等待回复完成或超时
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
        timer.start(666); // 等待0.666秒
        loop.exec();

        bool writeSuccess = (reply->error() == QModbusDevice::NoError);
        reply->deleteLater();

      /*  if (!writeSuccess) {
            qWarning() << "Write request failed with error:" << reply->errorString();
            return false;
        } */

        // 读取并校验写入结果
        QList<QVariant> readValues;
        bool isReadSuccess = readHoldingRegistersSync(fields, readValues, serverAddress);
        if (!isReadSuccess || readValues.size() != fields.size()) {
            qWarning() << "Failed to read or mismatch in read values";
            return false;
        }

        for (int i = 0; i < fields.size(); ++i) {
            if (fields[i].value != readValues[i]) {
                qWarning() << "Verification failed for field at address " << fields[i].startAddress;
                return false;
            }
        }

        // 打印成功日志
        QStringList msgSuccess;
        for (const auto &field : fields) {
            QString msg = QString("name: %1, address: %2, type: %3, value: %4")
                            .arg(field.name)
                            .arg(field.startAddress)
                            .arg(field.datatype)
                            .arg(field.value.toString());
            msgSuccess.append(msg);
        }
        qDebug() << "Write success: " << msgSuccess.join("; ");

        return true;
    }


signals:
    void connectionEstablished();
    void connectionFailed(QString error);
    void connectionClosed();

private slots:

    // 当 Modbus 客户端状态改变时调用
    void onStateChanged(QModbusDevice::State state) {
            // 处理连接状态的改变
        if (state == QModbusDevice::ConnectedState) {
            emit connectionEstablished();
        } else if (state == QModbusDevice::UnconnectedState) {
            qWarning() << "modbus tcp client unconnected";
            emit connectionClosed();
        }
    }

    // 当 Modbus 客户端发生错误时调用
    void onErrorOccurred(QModbusDevice::Error error) {
         // 处理连接过程中发生的错误
        if (error != QModbusDevice::NoError) {
            qWarning() << "Modbus error:" << m_modbusClient->errorString();
            emit connectionFailed(m_modbusClient->errorString());
        }
    }

private:
    QModbusTcpClient *m_modbusClient;

private:
    int fieldLength(const CommunicationField &field) {
        // 根据数据类型返回字段长度
        if (field.datatype.toLower() == "int32" || field.datatype.toLower() == "float") {
            return 2;  // 32位数据类型占用两个寄存器
        } else {
            return 1;  // 默认为16位数据类型，占用一个寄存器
        }
    }

    std::pair<int, int> determineReadRange(const QList<CommunicationField> &fields) {
        int startAddress = std::numeric_limits<int>::max();
        int endAddress = 0;
        for (const auto &field : fields) {
            startAddress = std::min(startAddress, static_cast<int>(field.startAddress));
            endAddress = std::max(endAddress, static_cast<int>(field.startAddress + field.length));
        }
        return {startAddress, endAddress};
    }

    void handleReply(QModbusReply *reply,
                     const QList<CommunicationField> &fields,
                     int startAddress,
                     std::function<void(bool, QList<QVariant>)> callback) {
        if (reply->error() != QModbusDevice::NoError) {
            callback(false, QList<QVariant>{});
            return;
        }

        const QModbusDataUnit unit = reply->result();
         QList<QVariant> values;
        for (auto &field : fields) {
          auto value = parseFieldData(unit, field, startAddress);
          values.append(value);
        }
        callback(true, values);
    }

    QVariant parseFieldData(const QModbusDataUnit &unit, const CommunicationField &field, int startAddress) {
        // 计算字段在 Modbus 回复中的起始位置
        int fieldIndex = field.startAddress - startAddress;

        // 返回值
        QVariant reValue;

        // 根据数据类型解析数据
        if (field.datatype.toLower() == "int32") {
            quint32 wordC = static_cast<quint32>(unit.value(fieldIndex));
            quint32 wordD = static_cast<quint32>(unit.value(fieldIndex + 1));
            quint32 combinedValue = (wordD << 16) | wordC;  // CDAB to ABCD
            reValue = QVariant::fromValue(static_cast<qint32>(combinedValue));
        }else if (field.datatype.toLower() == "float") {
            quint32 wordC = static_cast<quint32>(unit.value(fieldIndex));
            quint32 wordD = static_cast<quint32>(unit.value(fieldIndex + 1));
            quint32 combinedValue = (wordD << 16) | wordC;  // CDAB to ABCD
            float value;
            memcpy(&value, &combinedValue, sizeof(float));
            reValue = QVariant::fromValue(value);
        } else if (field.datatype.toLower() == "int16") {
                int16_t value = static_cast<int16_t>(unit.value(fieldIndex));
                reValue = QVariant::fromValue(value);
        }

        return reValue;
    }
};

#endif // MODBUSCLIENT_H

