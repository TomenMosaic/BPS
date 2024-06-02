#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"

#include <QList>


const static QList<CommunicationField> measuringSizeStationNode = {
    CommunicationField("包裹长", 500, 1, "int16"),
    CommunicationField("包裹宽", 501, 1, "int16"),
    CommunicationField("包裹高", 502, 1, "int16"),
    CommunicationField("包裹状态", 503, 1, "int16"),
    CommunicationField("包裹ID", 504, 2, "int32"),
};

const static QList<CommunicationField> measuringSizeFeedbackNode = {
    CommunicationField("包裹状态", 503, 1, "int16"),
    CommunicationField("包裹ID", 504, 2, "int32"),
};

void frmMain::startMeasuringStationServer2()
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

    // 包裹列表的轮询处理
    this->m_modbusClient_Timer = new QTimer(this);
    connect(this->m_modbusClient_Timer, &QTimer::timeout, this, [this]() {
        // 如果定时器内部有任务正在运行，则直接返回
        if (this->m_modbusClient_isProcess) {
            return;
        }
        this->m_modbusClient_isProcess = true; // 标记定时器内部有任务正在执行

        // 同步读取保持寄存器
        QList<QVariant> values;
        bool isSuccess = this->m_modbusClient->readHoldingRegistersSync(measuringSizeStationNode, values);
        if (!isSuccess) {
            qWarning() << "Failed to read registers";
            this->m_modbusClient_isProcess = false;
            return;
        }

        // 检查状态值是否有效
        bool statusOk;
        int status = values[3].toInt(&statusOk);
        if (!statusOk) {
            qWarning() << "Status conversion failed: " << values[3];
            this->m_modbusClient_isProcess = false;
            return;
        } else if (status != 1) { // 如果状态不为1，则任务未完成
            this->m_modbusClient_isProcess = false;
            return;
        }

        // 获取长度、宽度和高度信息
        uint length = values[0].toUInt();
        uint width = values[1].toUInt();
        uint height = values[2].toUInt();

        // 创建传输对象
        PackageDto packDto;
        packDto.length = length;
        packDto.width = width;
        packDto.height = height;

        // 创建包裹数据
        int newPackId = this->m_packBll->insertByPackStruct(packDto);
        packDto.id = newPackId;

        // 刷新界面，必须运行一下，否则后续的更新操作会报错
        this->initForm_PackDataBinding();

        // 如果尺寸信息有误，则生成错误状态的包裹
        auto targetStatus = PackageDto::StatusEnum::Status_Step1_GotMeasuringSize;
        if (length * width * height <= 0) {
            targetStatus = PackageDto::StatusEnum::Status_Step1_GotMeasuringSize_Error;
        }

        // 回馈PLC包裹ID
        CommunicationField idField = measuringSizeFeedbackNode[1];
        idField.value = newPackId;
        isSuccess = this->m_modbusClient->writeRegisterSync(idField);
        if (!isSuccess) {
            qWarning() << "Failed to send package ID to PLC!";
            this->m_modbusClient_isProcess = false; // 记得重置任务标记
            return;
        }

        // 回馈PLC状态为2，表示数据已经读取
        CommunicationField statusField = measuringSizeFeedbackNode[0];
        statusField.value = 2;
        isSuccess = this->m_modbusClient->writeRegisterSync(statusField);
        if (!isSuccess) {
            qWarning() << "Failed to send status (2) to PLC!";
            this->m_modbusClient_isProcess = false; // 记得重置任务标记
            return;
        }

        // 更新包裹状态并执行后续流程
        this->runFlow(packDto, &targetStatus);

        // 重置任务标记
        this->m_modbusClient_isProcess = false;
    });

    this->m_modbusClient_Timer->start(500); // 每500毫秒执行一次

}


// 发送包裹标识数据到 拼板工位
bool frmMain::send2FeedbackMeasuringSize(uint dbPackId){
    CommunicationField statusField = measuringSizeFeedbackNode[0];
    statusField.value = 2;
    CommunicationField idField = measuringSizeFeedbackNode[1];
    idField.value = dbPackId;

    QList<QVariant> outValues;
    auto isSuccess = this->m_modbusClient->readHoldingRegistersSync({
                                                                        CommunicationField("包裹ID", 504, 2, "int32"),
                                                                    }, outValues);

    // 同步发送数据
    try {
        bool isSuccess = this->m_modbusClient->writeRegisterSync(idField);
        if (!isSuccess) {
            qWarning() << "回馈PLC包裹id失败！";
            return isSuccess;
        }
        isSuccess = this->m_modbusClient->writeRegisterSync(statusField);
        if (!isSuccess) {
            qWarning() << "回馈PLC状态（2）失败";
        }
        return isSuccess;
    } catch (const std::exception& e) {
        qWarning() << "发送数据到拼板口时发生异常：" << e.what();
        return false;
    }
}
