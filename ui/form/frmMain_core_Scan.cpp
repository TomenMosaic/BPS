#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>
#include <QTextToSpeech>

bool frmMain::workFlow_WaitingForScan_ToleranceValues(PackageDto& pack){
    bool isWaitingForAction = false;
    for(const auto& waitingCondition: qAsConst(this->m_waitingConditions)){
        QString script = pack.getScript(waitingCondition.Condition);

        // 执行字符串脚本
        QScriptEngine engine;
        bool result = engine.evaluate(script).toBool();
        if (!result)
            continue;

        // 日志
        QList<QString> condtionMsgs;
        condtionMsgs.append("threshold name:"+waitingCondition.Name);
        condtionMsgs.append("action:"+waitingCondition.action);
        qDebug() << QString("pack (%1, %2, %3x%4x%5) ").
                    arg(QString::number(pack.id),
                        pack.no,
                        QString::number(pack.length),
                        QString::number(pack.width),
                        QString::number(pack.height))
                 << condtionMsgs.join(",");

        // logs
        QString  message = QString("waiting condition name: %1, %2 ; \n")
                .arg(waitingCondition.Name)
                .arg(condtionMsgs.join(","));
        qDebug() << message;

        // 如果是 扫码就更新包裹的 状态
        if (waitingCondition.action == ConditionDto::ActionEnum::scan){
            this->m_packBll->Step3_Waiting4ScanToleranceValue(pack.id, condtionMsgs.join(","));

            isWaitingForAction = true;
            break;
        }

    }

    return isWaitingForAction;
}

void frmMain::handleScannedData_RC(const QString &scannedData){
    // 是否等待队列为空，如果为空就退出
    /* if (this->m_waitingQueue.isEmpty()){
        return;
    } */

    // 解析条码中的数据，格式为“[ysbc,{几号口},{容差类型},{容差值}]”
    // 例如 ysbc,1,hrc,10 ，1号口，高度的容差值为 10
    QList<QString> array = scannedData.split(",");
    QString head = array[0].toLower();
    if (head != "ysbc"){
        qWarning() << "二维码格式错误！";
        return;
    }
    int entryIndex = array[1].toInt();
    QString type = array[2]; // 判断类型，比如 hyz 高度容差（mchyz 每层高度容差），ls 层数
    QString threshold = array[3];

    // 根据信息更新订单对应的容差

    QSharedPointer<PackageDto> pack = this->m_entryQueues[entryIndex-1].peek();
    if (!pack || pack->status != PackageDto::StatusEnum::Status_Step3_Waiting4ScanTolerance) {
        qWarning() << "没有测量容差的包裹！";
        return;
    }
    QString key = pack->getKey();

    // 在缓存的阻塞包裹(key)中更新当前包裹的尺寸信息
    if (!this->m_orderThreshold.contains(key)){
        DimensionThresholds dt;
        this->m_orderThreshold[key] = dt;
    }

    // 获取表达式 //TODO 又赋值了一次，没有必要
    if (type == "hrc"){ // 总高度容差
        this->m_orderThreshold[key].heightThreshold = threshold;
    }else if(type == "mchrc"){ // 每层高度的容差
        QString thresholdExpression =  QString("%1*{LayerCount}").arg(threshold);
        this->m_orderThreshold[key].heightThreshold = thresholdExpression;
    }else if(type == "wrc"){ // 宽度容差
        this->m_orderThreshold[key].widthThreshold = threshold;
    }else if(type == "lrc"){ // 长度容差
        this->m_orderThreshold[key].lengthThreshold = threshold;
    }

    // 更新包裹状态
    auto targetStatus = PackageDto::StatusEnum::Status_Step3_GotScanTolerance;
    this->runFlow(*pack, &targetStatus);
}

void frmMain::handleScannedData_Barcode(const QString &scannedData){
    if (scannedData.size() < 8){
        qWarning() << scannedData << " 长度过短！";
        return;
    }

    //1. 在板件表中查找匹配的记录
    auto panel = this->m_panelBll->getSingleByUPI(scannedData); // 默认是板件的条码
    if (panel.isNull()){
        //1.1. 是否是一个包裹条码，如果是就按照包裹记录发送
        auto pack = this->m_packBll->detailStruct(scannedData);
        if (pack != nullptr){
            this->sendFileToHotFolder(*pack);
            return;
        }

        //TODO 提示错误
        qWarning() << scannedData << " 没有查询到对应的板件 / 包裹数据！";
        return;
    }

    //2. 查找包裹记录
    QString currentOrderNo = panel->orderNo;
    int packFlowIndex = 0; // 包裹在订单中的序号
    int selectedRow = 0; // 在model中的序列号
    QList<QSharedPointer<Row>> packList = this->m_packBll->getCacheList();
    if (this->m_currentOrderNo != currentOrderNo){ // 如果当前包裹不对应
        packList = this->m_packBll->getList(panel->orderNo);
        this->m_currentOrderNo = currentOrderNo;
    }
    for (int i = 0; i < packList.size(); i++) {
        auto pack = packList[i];
        auto packId = pack->data(PackBLL::PackColEnum::ID).toUInt();
        if (packId == panel->dbPackId){
            selectedRow = i;
            packFlowIndex = pack->data(PackBLL::PackColEnum::FlowNo).toUInt();
            break;
        }
    }

    //3. 更新
    if (!panel->isScaned){   // 如果没有扫码
        // 更新板件的状态为已扫码
        this->m_packBll->panelScanned(panel->no);

        // 重新加载包裹列表
        this->initForm_PackDataBinding(false);

        // 设置当前选中的包裹
        QModelIndex index = this->m_packModel->index(selectedRow, 0);
        QItemSelectionModel *selectionModel = this->ui->tvPackList->selectionModel();
        selectionModel->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    //4. 弹窗提示 & 语音提示
    int carIndex = packFlowIndex/10 + 1;
    int cellIndex = (packFlowIndex)%10;
    QString textToShow = QString("%1, %2车%3格")
                .arg(panel->location)
                .arg(QString::number(carIndex))
                .arg(QString::number(cellIndex));
    QString textToSpeed = QString("%1, 请放到 第%2车的第%3格")
                .arg(panel->location)
                .arg(QString::number(carIndex))
                .arg(QString::number(cellIndex));
    FullScreenMask::getInstance()->showMessage(textToShow, 10*1000, true, textToSpeed);
}


void frmMain::handleScannedData(const QString &data) {
    qDebug() << "handleScannedData:" << data;
    QString scannedData = data.toLower();

    if (scannedData.startsWith("ysbc")){
        handleScannedData_RC(scannedData);
    }else{
        handleScannedData_Barcode(scannedData);
    }
}
