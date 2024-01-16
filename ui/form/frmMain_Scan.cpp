#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>
#include <QTextToSpeech>

void frmMain::handleScannedData_YZ(const QString &scannedData){
    // 是否等待队列为空，如果为空就退出
    if (this->m_waitingQueue.isEmpty()){
        return;
    }

    // 解析条码中的数据，格式为“[ysbc,{DataCLass},{Value}]”
    // 例如 ysbc,hyz,10  标识了这是一个识别码，高度预值为10
    QList<QString> array = scannedData.split(",");
    QString head = array[0];
    if (head != "ysbc"){
        qWarning() << "二维码格式错误！";
        return;
    }
    QString type = array[1]; // 判断类型，比如 hyz 高度预值（mchyz 每层高度预值），ls 层数
    QString threshold = array[2];

    // 根据信息更新订单对应的预值
    Package& pack = this->m_waitingQueue.head();
    QString key = pack.getKey();

    if (!this->m_orderThreshold.contains(key)){
        DimensionThresholds dt;
        this->m_orderThreshold[key] = dt;
    }

    // 获取表达式
    if (type == "hyz"){ // 高度预值
        this->m_orderThreshold[key].heightThreshold = threshold;
    }else if(type == "mchyz"){ // 每层高度的预值
        QString thresholdExpression =  QString("%1*{LayerCount}").arg(threshold);
        this->m_orderThreshold[key].heightThreshold = thresholdExpression;
    }else if(type == "wyz"){ // 宽度预值
        this->m_orderThreshold[key].widthThreshold = threshold;
    }else if(type == "lyz"){ // 长度预值
        this->m_orderThreshold[key].lengthThreshold = threshold;
    }

    // 处理等待队列中的数据
    if (!this->m_waitingQueue.isEmpty() && this->m_waitingQueue.head().needsScanConfirmation){
        this->m_waitingQueue.head().pendingScan = false; //
    }
}

FullScreenWindow *FullScreenWindow::currentInstance = nullptr;
void frmMain::handleScannedData_Barcode(const QString &scannedData){
    // 默认是板件的条码
    QString panelBarcode = scannedData;

    //1. 在板件表中查找匹配的记录
    auto panel = this->m_panelBll->getSingleByUPI(panelBarcode);
    if (panel.isNull()){
        //TODO  提示错误
        qWarning() << scannedData << " 没有查询到对应的板件数据！";
        return;
    }

    //2. 更新
    int packFlowIndex = 0;
    if (!panel->isScaned){   // 如果没有扫码
        //2.1. 更新板件的状态为已扫码
        this->m_packBll->panelScanned(panel->no);

        // 重新加载包裹列表
        auto packList = this->m_packBll->getList(panel->orderNo);
        this->initForm_PackDataBinding(false);

        // 设置包裹列表中选中包裹为 当前板件对应的包裹
        int selectedRow = 0;
        for (int i = 0; i < packList.size(); i++) {
            auto pack = packList[i];
            auto packId = pack->data(PackBLL::PackColEnum::ID).toUInt();
            if (packId == panel->dbPackId){
                selectedRow = i;
                break;
            }
        }
        QModelIndex index = this->m_packModel->index(selectedRow, 0);
        QItemSelectionModel *selectionModel = this->ui->tvPackList->selectionModel();
        selectionModel->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    //3. 弹窗提示 & 语音提示
    QString textToShow = QString("%1/n%2车%3格")
            .arg(panel->customerName)
            .arg(QString::number(packFlowIndex/10 + 1))
            .arg(QString::number(packFlowIndex%10));
    FullScreenWindow window(textToShow);
    window.showFullScreen();
    window.speak(textToShow);
}


void frmMain::handleScannedData(const QString &data) {
    qDebug() << "handleScannedData:" << data;
    QString scannedData = data.toLower();

    if (scannedData.startsWith("ysbc")){
        handleScannedData_YZ(scannedData);
    }else{
        handleScannedData_Barcode(scannedData);
    }
}
