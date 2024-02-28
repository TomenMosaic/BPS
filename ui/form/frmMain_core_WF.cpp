#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/tableview_controller.h"
#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>

int frmMain::getScanEntryIndex(QString originIp){
    int scanEntryIndex = -1;
    for (int i = 0; i < g_config->getMeasuringStationConfig().scanEntries.size(); i++){
        auto ip = g_config->getMeasuringStationConfig().scanEntries[i];
        if (ip == originIp){
            scanEntryIndex = i;
            break;
        }
    }
    return scanEntryIndex;
}

PackageDto::StatusEnum frmMain::runFlow_send2PanelDockingStation(PackageDto& packDto){
    PackageDto::StatusEnum nextStatus;
    auto scanEntryIndex = getScanEntryIndex(packDto.originIp);

    bool isSuccess = this->send2PanelDockingStation(packDto.id, scanEntryIndex); // 发送数据到 “拼板口”（寄存器地址）
    if (isSuccess){
        this->m_packBll->Step2_SentPackNo_WaitinMeasuringHeight(packDto.id);
        nextStatus = PackageDto::StatusEnum::Status_Step2_Waiting4MeasuringHeight;
    }else{
        throw std::runtime_error("写入拼板口错误！");
    }

    return nextStatus;
}

PackageDto::StatusEnum frmMain::runFlow_WaitingForSend(PackageDto& packDto){
    PackageDto::StatusEnum nextStatus = PackageDto::StatusEnum::Status_Step4_WaitingForSend;
    this->m_packBll->Step4_Waiting4SendWorkData(packDto.id);

    return nextStatus;
}

void frmMain::runFlow(PackageDto& packDto, PackageDto::StatusEnum* targetStatus){
    PackageDto::StatusEnum nextStatus ;
    if (targetStatus != nullptr){
       nextStatus = *targetStatus;
    }
    auto scanEntryIndex = getScanEntryIndex(packDto.originIp);

    try{
        //1. 当为板件扫码结束齐套后 && 在socket模式下接收数据并计算结束后
        if (packDto.status == PackageDto::StatusEnum::Status_Step1_ScanFull ||
                (packDto.status == PackageDto::StatusEnum::Status_Step1_Calculated
                 && g_config->getWorkConfig().workMode == WorkModeEnum::socat)){
            bool isWaitingForAction = this->workFlow_WaitingForScan_ToleranceValues(packDto);
            //1.1. 需要录入容差值
            if (isWaitingForAction){
                this->m_packBll->Step3_Waiting4ScanToleranceValue(packDto.id);
                nextStatus = PackageDto::StatusEnum::Status_Step3_Waiting4ScanTolerance;
            }
            //1.2. 等待发送到拼板口
            else if (g_config->getMeasuringStationConfig().isOpen){
                this->m_packBll->Step2_Waiting4PackNo_PanelSockStation(packDto.id);
                nextStatus = PackageDto::StatusEnum::Status_Step2_Waiting4SendPackNo;               
            }
            //1.3. 如果不需要测高，也不需要增加容差值，就是等待发送到裁纸机
            else {
                nextStatus = this->runFlow_WaitingForSend(packDto);
            }
        } else if (targetStatus != nullptr){
            // 当前包裹的状态不能大于目标状态
            if (packDto.status > *targetStatus){
                qWarning() << QString("包裹（%1）.（%2) > （%3）")
                              .arg(QString::number(packDto.id))
                              .arg(PackageDto::statusEnumToString(packDto.status))
                              .arg(PackageDto::statusEnumToString(*targetStatus));
                return;
            }

            // 发送包裹数据到拼板口，等待测量高度
            if (*targetStatus == PackageDto::StatusEnum::Status_Step2_SentPackNo){
                nextStatus = runFlow_send2PanelDockingStation(packDto);
            }
            // 接收到测量站的高度数据，就等待发送到裁纸机
            else if(*targetStatus == PackageDto::StatusEnum::Status_Step2_GotMeasuringHeight){
                // 是否需要测量站
                if (g_config->getMeasuringStationConfig().isOpen){
                    // 发送数据到裁纸机
                   this->m_packBll->Step2_GotMeasuringHeight(packDto.id, packDto.height);

                    // 更新为待发送状态
                    nextStatus = this->runFlow_WaitingForSend(packDto);
                }
                // 如果不需要测量站，这个流转就是错误的
                else{
                    QString message = QString("在没有开启测高的情况下，状态（）无法流转到")
                            .arg(PackageDto::statusEnumToString(*targetStatus));
                    throw std::runtime_error(message.toUtf8());
                }
            }
             // 接收到扫码的容差值
            else if(*targetStatus == PackageDto::StatusEnum::Status_Step3_GotScanTolerance){
                // 有测量站的时候，发送包裹数据到拼板口
                if (g_config->getMeasuringStationConfig().isOpen){
                    nextStatus = runFlow_send2PanelDockingStation(packDto);
                }else{ // 没有测量站的时候，就等待发送到裁纸机
                    nextStatus = this->runFlow_WaitingForSend(packDto);
                }
            }
            // 发送到裁纸机
            else if (*targetStatus == PackageDto::StatusEnum::Status_Step4_Sent){
                this->sendFileToHotFolder(packDto);
                nextStatus = PackageDto::StatusEnum::Status_Step4_Sent;
            }
        }else{
            qWarning() << "非初始状态 且 目标状态为空！";
            return;
        }
    }catch (const std::exception& e) {
        // 处理异常，记录错误等
        qWarning() << "Exception occurred:" << e.what();
        return;
    }

    // 更新状态值
    packDto.status = nextStatus;
    // 写入队列
    if (scanEntryIndex >= 0 && scanEntryIndex < this->m_entryQueues.size()){
        if (packDto.status == PackageDto::StatusEnum::Status_Step2_Waiting4SendPackNo
                || packDto.status == PackageDto::StatusEnum::Status_Step3_Waiting4ScanTolerance){
            this->m_entryQueues[scanEntryIndex].enqueue(packDto);
        }else{
            // 更新入口的队列中对应包裹的信息
            if (g_config->getMeasuringStationConfig().isOpen){
                this->m_entryQueues[scanEntryIndex].modifyIf(
                            [packDto](const PackageDto &x) { return x.id == packDto.id; },
                [packDto](PackageDto &x) {
                    x.status = packDto.status; // 状态
                    if (x.height != packDto.height){
                        x.height = packDto.height;
                    }
                });
            }
        }
    }
    // 更新等待队列中对应包裹的信息
    if (g_config->getMeasuringStationConfig().isOpen){
        this->m_waitingQueue.modifyIf(
                    [packDto](const PackageDto &x) { return x.id == packDto.id; },
        [packDto](PackageDto &x) {
            x.status = packDto.status; // 状态
            if (x.height != packDto.height){
                x.height = packDto.height;
            }
        });
    }

    // 更新UI
    this->initForm_PackDataBinding();
}

// 发送文件到共享文件夹
void frmMain::sendFileToHotFolder(const PackageDto &originPackage) {
    QDateTime now = QDateTime::currentDateTime();
    PackageDto package = originPackage;
    QString packTemaplte = g_config->getPackTemplateConfig().defaultTemplate;
    QScriptEngine engine;// 执行字符串脚本

    // 查找容差
    QString message;
    for(const auto& threshold : qAsConst(this->m_thresholdConditions)){
        QString script = originPackage.getScript(threshold.Condition);

        bool result = engine.evaluate(script).toBool();
        if (result){
            // 基本的容差
            int tLength = 0, tWidth = 0, tHeight = 0;

            // 表达式
            if (!threshold.lengthExpression.isEmpty()){
                QString tmpScript = originPackage.getScript(threshold.lengthExpression);
                qint32 tmpLength = engine.evaluate(tmpScript).toInt32();
                tLength += tmpLength;
            }
            if (!threshold.widthExpression.isEmpty()){
                QString tmpScript = originPackage.getScript(threshold.widthExpression);
                qint32 tmpWidth = engine.evaluate(tmpScript).toInt32();
                tWidth += tmpWidth;
            }
            if (!threshold.heightExpression.isEmpty()){
                QString tmpScript = originPackage.getScript(threshold.heightExpression);
                qint32 tmpHeight = engine.evaluate(tmpScript).toInt32();
                tHeight += tmpHeight;
            }

            // 加上最终的容差
            package.length += tLength;
            package.width += tWidth;
            package.height += tHeight;

            // 日志
            QList<QString> condtionMsgs;
            condtionMsgs.append("threshold name:"+threshold.Name);
            condtionMsgs.append("condition:"+threshold.Condition);
            if (!threshold.lengthExpression.isEmpty()){
                condtionMsgs.append("length Expression:"+threshold.lengthExpression);
            }
            if (!threshold.widthExpression.isEmpty()){
                condtionMsgs.append("width Expression:"+threshold.widthExpression);
            }
            if (!threshold.heightExpression.isEmpty()){
                condtionMsgs.append("height Expression:"+threshold.heightExpression);
            }
            QList<QString> resultMessages;
            if (tLength > 0){
                resultMessages.append("length + "+QString::number(tLength));
            }
            if (tWidth > 0){
                resultMessages.append("width + "+QString::number(tWidth));
            }
            if (tHeight > 0){
                resultMessages.append("height + "+QString::number(tHeight));
            }

            qDebug() << QString("pack (%1, %2, %3x%4x%5) use ").
                        arg(QString::number(package.id),
                            package.no,
                            QString::number(originPackage.length),
                            QString::number(originPackage.width),
                            QString::number(originPackage.height))
                     << condtionMsgs.join(",") << ";" << resultMessages.join(",");

            // logs
            message += QString("threshold name: %1, %2 ; \n").
                    arg(threshold.Name, resultMessages.join(","));
        }
    }

    // 是否为等待扫码增加容差
    if (g_config->getWorkConfig().isWaiting4Scan &&
            this->m_orderThreshold.keys().contains(originPackage.getKey())) {
        QString key = originPackage.getKey();
        auto threshold = this->m_orderThreshold[key];
        if (threshold.hasValues()){
            QList<QString> condtionMsgs;
            if (threshold.lengthThreshold != 0){
                QString tmpScript = originPackage.getScript(threshold.lengthThreshold);
                qint32 tmpLength = engine.evaluate(tmpScript).toInt32();
                condtionMsgs.append("length +"+QString::number(tmpLength));
                package.length += tmpLength;
            }
            if (threshold.widthThreshold != 0){
                QString tmpScript = originPackage.getScript(threshold.widthThreshold);
                qint32 tmpWidth = engine.evaluate(tmpScript).toInt32();
                condtionMsgs.append("width +"+QString::number(tmpWidth));
                package.width += tmpWidth;
            }
            if (threshold.heightThreshold != 0){
                QString tmpScript = originPackage.getScript(threshold.heightThreshold);
                qint32 tmpHeight = engine.evaluate(tmpScript).toInt32();
                condtionMsgs.append("height +"+QString::number(tmpHeight));
                package.height += tmpHeight;
            }
            qDebug() << QString("pack (%1, %2, %3x%4x%5) use ").
                        arg(QString::number(package.id),
                            package.no,
                            QString::number(originPackage.length),
                            QString::number(originPackage.width),
                            QString::number(originPackage.height))
                     << condtionMsgs.join(",");

            // logs
            message += QString(" scan threshold: %1 ; \n").
                    arg(condtionMsgs.join(","));

        }else{
            qWarning() << key + "对应 扫码容差为空";
        }
    }

    // 箱型规则
    for(const auto& condition : qAsConst(this->m_packTemplateConditions)){
        // 要使用最新的尺寸来进行计算，增加了容差后长宽高都可能会改变
        QString script = package.getScript(condition.Condition);

        bool result = engine.evaluate(script).toBool();
        if (result){
            // 箱型
            if (!condition.packTemplate.isEmpty()) {
                packTemaplte = condition.packTemplate;

                qDebug() << QString("pack (%1, %2, %3x%4x%5) use ").
                            arg(QString::number(package.id),
                                package.no,
                                QString::number(originPackage.length),
                                QString::number(originPackage.width),
                                QString::number(originPackage.height))
                         << "pack template: " + packTemaplte;

                // logs
                message += QString("threshold name: %1, %2 ; \n").
                        arg(condition.Name,  "pack template: " + packTemaplte);

                break; // 采用逐一比对的方式，满足就使用
                //TODO 不是很灵活，无法两个条件同时满足的情况，比如窄条必须使用特殊箱型，但是有可能这个特殊箱型做不了，要使用另外的箱型
            }

        }
    }

    // 创建CSV文件夹路径
    QString csvFolderPath = QApplication::applicationDirPath() + "/csv";
    QString monthSubfolder = now.toString("yyyy/MM");
    QDir csvDir(csvFolderPath);
    if (!csvDir.exists(monthSubfolder) && !csvDir.mkpath(monthSubfolder)) {
        qWarning() << "Could not create month subfolder in csv directory:" << csvDir.filePath(monthSubfolder);
        return;
    }

    // 文件名和路径
    QString fileName = package.no;
    QStringList invalidChars = {"<", ">", ":", "\"", "/", "\\", "|", "?", "*"};
    for (const QString &invalidChar : invalidChars) {
        fileName.replace(invalidChar, "_");
    }
    fileName += "."+ now.toString("HHmmss")+".csv";
    QString filePath = csvDir.filePath(monthSubfolder + "/" + fileName);

    // 写入本地CSV文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        // 写入CSV头和内容
        out << "Type;OrderId;Quantity;ProductionGroupName;Length;Width;Height;CorrugateQuality;DesignId\n";
        out << QString("socket;%1;1;;%2;%3;%4;;%5\n").arg(package.no,
                                                          QString::number(package.length),
                                                          QString::number(package.width),
                                                          QString::number(package.height),
                                                          packTemaplte);
        file.close(); // 成功写入后关闭文件
    } else {
        qWarning() << "Could not write to local csv file:" << filePath;
        return;
    }

    // 确定热文件夹路径
    QString hotFolderPath = g_config->getDeviceConfig().importDir;
    QDir hotDir(hotFolderPath);
    if (!hotDir.exists() && !hotDir.mkpath(".")) {
        qWarning() << "Hot folder path does not exist and could not be created:" << hotFolderPath;
        return;
    }
    QString hotFilePath = hotDir.filePath(fileName);// 文件路径在热文件夹中
    if (!file.copy(hotFilePath)) { // 复制文件到热文件夹
        qWarning() << "Could not copy file to hot folder" << hotFilePath;
        return;
    }

    // 更新状态为已发送
    this->m_packBll->Step4_SentWorkData(package.id, message);
}
