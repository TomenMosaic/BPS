#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"

#include <QList>

const static QList<CommunicationField> panelDockingStation01Node = {
    CommunicationField("1号码垛台ID", 600, 2, "int32"),
    CommunicationField("1号码垛数据ID发送完成", 610, 2, "int32"),
};

const static QList<CommunicationField> panelDockingStation02Node = {
    CommunicationField("2号码垛台ID", 602, 2, "int32"),
    CommunicationField("2号码垛数据ID发送完成", 612, 2, "int32"),
};

const static QList<CommunicationField> packAndHeightNode = {
    CommunicationField("测高ID1", 620, 2, "int32"),
    CommunicationField("测高ID2", 622, 2, "int32"),
    CommunicationField("测高ID3", 624, 2, "int32"),
    CommunicationField("测高ID4", 626, 2, "int32"),
    CommunicationField("测高ID5", 628, 2, "int32"),
    CommunicationField("测高ID6", 630, 2, "int32"),
    CommunicationField("测高ID7", 632, 2, "int32"),
    CommunicationField("测高ID8", 634, 2, "int32"),
    CommunicationField("测高ID9", 636, 2, "int32"),
    CommunicationField("测高ID10", 638, 2, "int32"),

    CommunicationField("ID1对应高度", 650, 2, "int16"),
    CommunicationField("ID2对应高度", 652, 2, "int16"),
    CommunicationField("ID3对应高度", 654, 2, "int16"),
    CommunicationField("ID4对应高度", 656, 2, "int16"),
    CommunicationField("ID5对应高度", 658, 2, "int16"),
    CommunicationField("ID6对应高度", 660, 2, "int16"),
    CommunicationField("ID7对应高度", 662, 2, "int16"),
    CommunicationField("ID8对应高度", 664, 2, "int16"),
    CommunicationField("ID9对应高度", 666, 2, "int16"),
    CommunicationField("ID10对应高度", 668, 2, "int16"),
};

// 最后一个已处理的测量包裹id
int lastMeasuredPackId = 0;

//
QList<QPair<int, int>> convertValuesToStruct(const QList<CommunicationField> &list, const QList<QVariant> &values){
    QList<QPair<int, int>> result;

    QMap<QString, CommunicationField*> map;
    for (int i = 0; i < list.size(); ++i){
        // 直接关联 list 中的实例地址
        map[list[i].name] = const_cast<CommunicationField*>(&list[i]);
    }

    // 更新 list 中的值，注意这里我们不修改 list 的内容，因为它是 const 引用
    // 如果需要修改，list 参数不应该是 const
    for (int i = 0; i < values.size(); ++i){
        if(map.contains(list[i].name)) {
            auto field = map[list[i].name];
            field->value = values[i]; // 在原始列表中更新值
        }
    }

    for (int i = 1; i <= 10; ++i){
        QString idFieldName = QString("测高ID%1").arg(i);
        QString heightFieldName = QString("ID%1对应高度").arg(i);

        CommunicationField* idField = map.value(idFieldName, nullptr);
        CommunicationField* heightField = map.value(heightFieldName, nullptr);

        if (!idField || !heightField) {
            continue; // 如果找不到字段，跳过当前迭代
        }

        bool idOk, heightOk;
        int id = idField->value.toInt(&idOk);
        int height = heightField->value.toInt(&heightOk);

        if (!idOk || !heightOk || id <= 0){
            continue; // 如果值无效或转换失败，跳过当前迭代
        }

        result.append(qMakePair(id, height));
    }

    return result;
}

void frmMain::startMeasuringStationServer()
{
    // 配置中测量站必须开启
    if (!g_config->getMeasuringStationConfig().isOpen){
        return;
    }

    // connect to modbus tcp client
    this->m_modbusClient = new ModbusClient();
    QString tooltip = "unconnected";
    if (this->m_modbusClient->listen(g_config->getMeasuringStationConfig().modbusTcpClientIp,
                                     g_config->getMeasuringStationConfig().modbusTcpClientPort)) {
        tooltip = "connect";
    }
    QIcon nullIcon;
    this->m_customStatusBar->addIcon(nullIcon, this->StatusBar_IconName_Modbus, tooltip, nullptr);

    // 监听状态的改变，从而修改icon的tooltip
    connect(this->m_modbusClient, &ModbusClient::connectionClosed, this, [this](){
        this->m_customStatusBar->setTooltip(this->StatusBar_IconName_Modbus, "connection closed", true);
    });
    connect(this->m_modbusClient, &ModbusClient::connectionFailed, this, [this](QString error){
        this->m_customStatusBar->setTooltip(this->StatusBar_IconName_Modbus, error, true);
    });
    connect(this->m_modbusClient, &ModbusClient::connectionEstablished, this, [this](){
        this->m_customStatusBar->setTooltip(this->StatusBar_IconName_Modbus, "connect");
    });

    // 轮询 modbus tcp 中的包裹列表
    this->m_modbusClient_Timer = new QTimer(this);
    connect(this->m_modbusClient_Timer, &QTimer::timeout, this, [this](){
        // 如果定时器内部有任务正在运行，就退出
        if (this->m_modbusClient_isProcess){
            return;
        }
        this->m_modbusClient_isProcess = true; // 标记定时器内部有任务正在执行

        this->m_modbusClient->readHoldingRegisters(packAndHeightNode, [this](bool success, QList<QVariant> values) {
            try{
                if (!success) {
                    // 处理读取失败的情况
                    qWarning() << "Failed to read registers";
                }else{
                    // 和缓存中的数据进行对比，查看是否有新增的包裹高度数据
                    QList<QPair<int, int>> newMeasuredValues;
                    QList<QPair<int, int>> measuredValues = convertValuesToStruct(packAndHeightNode, values);
                    for (int i = measuredValues.size(); i > 0; i--){
                        auto& measuredValue = measuredValues[i-1];
                        if (measuredValue.first == lastMeasuredPackId){
                            break; // 退出循环，因为后续都是处理过的
                        }else{
                            newMeasuredValues.append(measuredValue);
                        }
                    }

                    // 更新到指定的包裹的高度，包裹的状态进行流转
                    for (QPair<int, int> pair : newMeasuredValues) {
                        int packId = pair.first;
                        // int height = pair.second;

                        // 查找包裹记录，判断当前包裹是否已经更新了高度
                        auto pack = this->m_packBll->getPackageByDbId(packId);
                        if (pack.isNull()){
                            qWarning() << "查找不到 pack id（" << packId << "）对应的数据！";
                            continue;
                        }
                    /*    if (pack->status >= PackageDto::StatusEnum::Status_Step2_GotMeasuringHeight){
                            qWarning() << "包裹（" << packId << "）测高数据已更新！";
                            continue;
                        } */

                        // 更新对应包裹的高度
                        // pack->height = height; // 赋值传递过去
                        auto targetStatus = PackageDto::StatusEnum::Status_Step2_GotMeasuringHeight;
                        this->runFlow(*pack, &targetStatus);

                    }

                    // 更新处理的一个包裹id
                    if (newMeasuredValues.size() > 0){
                        lastMeasuredPackId = measuredValues.last().first; // 列表的最后一个数据是最新的
                    }
                }

            }catch (const std::exception& e) {
                // 处理异常，记录错误等
                qWarning() << "Exception occurred:" << e.what();
            }

            this->m_modbusClient_isProcess = false; // 标记定时器没有任务在运行
        });
    });
    this->m_modbusClient_Timer->start(500); // 延迟500毫秒处理
}

// 发送包裹标识数据到 拼板工位
bool frmMain::send2PanelDockingStation(uint dbPackId, int panelDockingStationIndex){
    // 组织数据
    auto panelDockingStationNode = (panelDockingStationIndex == 1) ? panelDockingStation02Node : panelDockingStation01Node;
    CommunicationField* idField = nullptr;
    CommunicationField* markField = nullptr;
    for(auto& field : panelDockingStationNode){
        if (field.name.contains("码垛台ID")){
            field.value = dbPackId;
            idField = &field;
        }else if (field.name.contains("发送完成")){
            field.value = 1;
            markField = &field;
        }
    }

    // 数据校验
    if (!idField){
        qWarning() << "码垛台ID 通讯节点配置为空！";
        return false;
    }
    if (!markField){
        qWarning() << "码垛台对应写入标识 通讯节点配置为空！";
        return false;
    }

    // 同步发送数据
    try {
        bool isSuccess = this->m_modbusClient->writeRegisterSync(*idField);
        if (!isSuccess) {
            qWarning() << "发送数据到拼板口失败 1！";
            return isSuccess;
        }
        isSuccess = this->m_modbusClient->writeRegisterSync(*markField);
        if (!isSuccess) {
            qWarning() << "发送数据到拼板口失败 2！";
        }
        return isSuccess;
    } catch (const std::exception& e) {
        qWarning() << "发送数据到拼板口时发生异常：" << e.what();
        return false;
    }
}


bool frmMain::isAllowWrite2PanelDockingStation(int panelDockingStationIndex){
    // 组织数据
    auto panelDockingStationNode = panelDockingStation01Node;
    if (panelDockingStationIndex == 1){
        panelDockingStationNode = panelDockingStation02Node;
    }
    CommunicationField* markField;
    for(auto& field : panelDockingStationNode){
        if (field.name.contains("发送完成")){
            markField = &field;
        }
    }

    if (!markField){
        qWarning() << "没有在通讯配置中，找到 “发送完成标识” ";
        return false;
    }

    QVariant value ;
    bool isReadSuccess = this->m_modbusClient->readSingleRegisterSync(*markField, value);
    if (isReadSuccess){
        return value.toUInt() == 0;
    }

    return false;
}


