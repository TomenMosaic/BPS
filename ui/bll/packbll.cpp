#include "packbll.h"
#include "panelbll.h"
#include "global.h"
#include "algorithm.h"
#include <QMetaEnum>
#include <QDateTime>

PackBLL *PackBLL::m_packBll = nullptr;

PackBLL *PackBLL::getInstance(QObject *parent)
{
    if(m_packBll == nullptr)
    {
        PackBLL::m_packBll = new PackBLL(parent);
    }
    return PackBLL::m_packBll;
}

PackBLL::PackBLL(QObject *parent) :
    QObject(parent)
{
    this->init();
}

void PackBLL::init()
{
    // 设置字段列表
    this->dbColumnNames = this->dal.getColumnNames(this->dbColumnList);

    // 检查数据库表是否存在
    checkAndCreateTable();

    // 初始化 databable
    QStringList orders;
    orders.append("id desc");
    dal.initTable(tableName, this->dbColumnNames, QStringList(), orders, this->m_pageSize, 0, true);
}

void PackBLL::checkAndCreateTable(){
    this->dal.checkAndCreateTable(this->tableName, this->dbColumnList);
}

QList<QSharedPointer<Row>> PackBLL::getList(QString orderNo)
{
    QStringList wheres;
    int pageSize = this->m_pageSize;
    if (!orderNo.isEmpty()){
        wheres.append(QString("%1='%2'")
                      .arg(this->dbColumnNames[PackColEnum::OrderNo])
                .arg(orderNo));
        pageSize = 0; // 如果指定了orderno，就要显示对应的所有数据
    }

    dal.reload(pageSize, 0, wheres, QStringList()); // 最多显示26条最新的数据
    return dal.getRowList();
}

QList<QSharedPointer<Row>> PackBLL::getCacheList()
{
    return dal.getRowList();
}

int PackBLL::insert(QString no, uint length, uint width, uint height, PackageDto::PackTypeEnum type){
    QMap<QString,QString> mapList;
    mapList.insert(this->dbColumnNames.at(No),no);
    mapList.insert(this->dbColumnNames.at(Length),QString::number(length));
    mapList.insert(this->dbColumnNames.at(Width),QString::number(width));
    mapList.insert(this->dbColumnNames.at(Height),QString::number(height));
    mapList.insert(this->dbColumnNames.at(Type),QString::number(type));
    mapList.insert(this->dbColumnNames.at(Status),QString::number(PackageDto::StatusEnum::Status_Init));
    QDateTime currentTime =  QDateTime::currentDateTime();
    QString formattedTime = currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
    mapList.insert(this->dbColumnNames.at(CreateTime),formattedTime);
    mapList.insert(this->dbColumnNames.at(LastModifyTime),formattedTime);
    dal.appendRow(mapList);
    newId = dal.getLastID();
    return newId;
}

QVector<int> PackBLL::insertByPackStructs(const QList<PackageDto>& packages){
    QVector<int> result;
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    int newPackId ;

    try {
        QSqlQuery query(db);

        for (const auto& packDto : packages){
            query.clear();
            //1. 插入数据到pack表
            //1.1. 需要插入的数据
            QMap<QString,QVariant> mapList;
            mapList.insert(this->dbColumnNames.at(PackColEnum::No),packDto.no);
            mapList.insert(this->dbColumnNames.at(PackColEnum::CustomerName),packDto.customerName);
            mapList.insert(this->dbColumnNames.at(PackColEnum::OrderNo),packDto.orderNo);
            mapList.insert(this->dbColumnNames.at(PackColEnum::Length),packDto.length);
            mapList.insert(this->dbColumnNames.at(PackColEnum::Width),packDto.width);
            mapList.insert(this->dbColumnNames.at(PackColEnum::Height),packDto.height);
            mapList.insert(this->dbColumnNames.at(PackColEnum::Type), packDto.type);
            if (packDto.type == PackageDto::PackTypeEnum::PackType_PrePackaging){
                mapList.insert(this->dbColumnNames.at(PackColEnum::FlowNo), packDto.flowNo);
            }
            mapList.insert(this->dbColumnNames.at(PackColEnum::PanelTotal), packDto.panelTotal);
            mapList.insert(this->dbColumnNames.at(PackColEnum::Status),
                           QString::number(PackageDto::StatusEnum::Status_Init));
            if (!packDto.originIp.isEmpty()){
                mapList.insert(this->dbColumnNames.at(PackColEnum::OriginIp), packDto.originIp);
            }
            QDateTime currentTime = QDateTime::currentDateTime();
            QString formattedTime = currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
            mapList.insert(this->dbColumnNames.at(PackColEnum::CreateTime), formattedTime);
            mapList.insert(this->dbColumnNames.at(PackColEnum::LastModifyTime), formattedTime);

            //1.2. 构造sql参数
            QStringList packColnames = mapList.keys();
            QStringList packBindVals;
            for (const auto &colName : qAsConst(packColnames)) {
                packBindVals << (":" + colName);
            }
            QString sql = QString("INSERT INTO pack (%1) VALUES (%2);")
                    .arg(packColnames.join(","), packBindVals.join(","));
            query.prepare(sql);
            for (const auto &colName : qAsConst(packColnames)) {
                query.addBindValue(mapList.value(colName));
            }
            if (!query.exec()) {
                QSqlError error = query.lastError();
                qDebug() << "SQL error:" << error.text();
                qDebug() << "Executed SQL:" << sql; // 输出实际执行的 SQL 语句
                throw std::runtime_error(("无法插入 pack 表记录, "+error.text()).toStdString());
            }

            //1.3. 获取插入的packid
            newPackId = query.lastInsertId().toInt();
            result.append(newPackId);

            //2. 插入关联的panel记录
            foreach(const Panel& panel, packDto.panels){
                query.clear();
                QMap<QString, QVariant> panelMapList;
                panelMapList.insert("pack_id", newPackId);
                panelMapList.insert("name", panel.name);
                panelMapList.insert("no", panel.no);
                panelMapList.insert("remark", panel.remark);
                panelMapList.insert("external_id", panel.externalId);
                panelMapList.insert("layer", panel.layerNumber);
                panelMapList.insert("position", QString("%1,%2").
                                    arg(panel.position.x()).
                                    arg(panel.position.y()));
                panelMapList.insert("rotated",QString::number(panel.rotated));

                panelMapList.insert(this->dbColumnNames.at(Length), QString::number(panel.length));
                panelMapList.insert(this->dbColumnNames.at(Width), QString::number(panel.width));
                panelMapList.insert(this->dbColumnNames.at(Height), QString::number(panel.height));

                panelMapList.insert("customer_name", panel.customerName);
                panelMapList.insert("order_no", panel.orderNo);
                panelMapList.insert("location", panel.location);

                panelMapList.insert(this->dbColumnNames.at(CreateTime),formattedTime);
                panelMapList.insert(this->dbColumnNames.at(LastModifyTime),formattedTime);

                QStringList panelColNames = panelMapList.keys();
                QStringList panelBindVals;
                for (const auto &colName : qAsConst(panelColNames)) {
                    panelBindVals << (":" + colName);
                }
                query.prepare(QString("INSERT INTO panel (%1) VALUES (%2);").
                              arg(panelColNames.join(","), panelBindVals.join(",")));
                for (const auto &key : qAsConst(panelColNames)) {
                    query.addBindValue(panelMapList.value(key));
                }
                if (!query.exec()) {
                    QSqlError error = query.lastError();
                    throw std::runtime_error(("无法插入 panel 表记录, "+error.text()).toStdString());
                }
            }
        }

        //3. 提交
        if (!db.commit()) {
            throw std::runtime_error(("Transaction commit failed:" + db.lastError().text()).toStdString());
        }
        query.clear();

        // 刷新数据
        dal.reload();
    }  catch (...) {
        db.rollback();
    }

    return result;
}

int PackBLL::insertByPackStruct(const PackageDto& package){
    QList<PackageDto> packages;
    packages.append(package);
    QVector<int> ids = insertByPackStructs(packages);
    if (ids.size() == 1)
        return ids[0];

    return -1;
}

bool PackBLL::remove(uint packId){
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(PackBLL::ID).toInt()!=packId)
            continue;

        // 更新数据库字段
        curRow->setData(PackBLL::PackColEnum::RemovedAt, QDateTime::currentDateTime()); // 更新removeat字段
        QMap<QString, QVariant> wheres;
        wheres[this->dbColumnNames.at(PackBLL::PackColEnum::ID)] = packId; // 使用主键
        dal.update(curRow, wheres);

        // 删除model中对应记录
        dal.removeRow(curRow);

        return true;
    }

    return false;
}

QSharedPointer<PackageDto>  PackBLL::convertRow2Package(QSharedPointer<Row> packageRow){
    if (packageRow.isNull()){
        return nullptr;
    }

    QSharedPointer<PackageDto> pack = QSharedPointer<PackageDto>::create();

    pack->id = packageRow->data(PackBLL::PackColEnum::ID).toInt();
    pack->no = packageRow->data(PackBLL::PackColEnum::No).toString();

    pack->length = packageRow->data(PackBLL::PackColEnum::Length).toInt();
    pack->width = packageRow->data(PackBLL::PackColEnum::Width).toInt();
    pack->height = packageRow->data(PackBLL::PackColEnum::Height).toInt();

    pack->orderNo = packageRow->data(PackBLL::PackColEnum::OrderNo).toString();
    pack->customerName = packageRow->data(PackBLL::PackColEnum::CustomerName).toString();

    pack->originIp = packageRow->data(PackBLL::PackColEnum::OriginIp).toString();

    auto status = static_cast<PackageDto::StatusEnum>(packageRow->data(PackBLL::PackColEnum::Status).toInt());
    pack->status = status;
    pack->flowNo = packageRow->data(PackBLL::PackColEnum::FlowNo).toInt();

    return pack;
}

QSharedPointer<PackageDto> PackBLL::getPackageByDbId(uint packId){
    QSharedPointer<Row> row = detail(packId);
    return convertRow2Package(row);
}

QSharedPointer<Row> PackBLL::detail(uint packId){
    // 先在缓存中查找，再到数据库里面查找
    QMap<QString, QVariant> conditions;
    conditions.insert(this->dbColumnNames.at(PackColEnum::ID), packId);

    return dal.getRow(conditions);
}

bool PackBLL::Step1_Calculated(uint packId){
    return updateStatus(packId, PackageDto::StatusEnum::Status_Step1_Calculated);
}

bool PackBLL::Step1_Full(uint packId){
    return updateStatus(packId, PackageDto::StatusEnum::Status_Step1_ScanFull);
}

bool PackBLL::Step2_Waiting4PackNo_PanelSockStation(uint packId){
    return updateStatus(packId, PackageDto::StatusEnum::Status_Step2_Waiting4SendPackNo);
}

bool PackBLL::Step2_SentPackNo_PanelSockStation(uint packId){
    return  updateStatus(packId, PackageDto::StatusEnum::Status_Step2_SentPackNo);
}

bool PackBLL::Step2_SentPackNo_WaitinMeasuringHeight(uint packId){
    return   updateStatus(packId, PackageDto::StatusEnum::Status_Step2_Waiting4MeasuringHeight);
}

bool PackBLL::Step2_GotMeasuringHeight(uint packId, uint height){
    if (height <= 0 || height >= 200){ //TODO　200 需要作为参数录入
        qWarning() << packId << " 对应高度值非法";
    }
    if (packId <= 0){
        qWarning() << "包裹ID不正确";
    }
    QStringList messages;

    auto pack = getPackageByDbId(packId);

    QMap<QString, QVariant> updates;
    if (!pack.isNull()){
        // 是否状态已经改变了
        if (pack->status >= PackageDto::StatusEnum::Status_Step2_GotMeasuringHeight){
            qWarning() << QString("当前包裹（%1）状态为（%2）不能再更新测量高度！")
                          .arg(QString::number(packId))
                          .arg(PackageDto::statusEnumToString(pack->status));
            return false;
        }

        // 是否高度值有变化
        if (pack->height != height){
            updates[this->dbColumnNames.at(PackColEnum::Height)] = height;
            auto msg = QString("height: %1 -> measuring height: %2")
                    .arg(QString::number(pack->height))
                    .arg(QString::number(height));
            messages.append(msg);
        }
    }else{
        updates[this->dbColumnNames.at(PackColEnum::Height)] = height;
    }

    // update
    return updateStatus(packId,
                        PackageDto::StatusEnum::Status_Step2_GotMeasuringHeight,
                        messages.join(";"),
                        updates);
}

// 等待扫码
bool PackBLL::Step3_Waiting4ScanToleranceValue(uint packId, QString message){
    return updateStatus(packId, PackageDto::StatusEnum::Status_Step3_Waiting4ScanTolerance, message);
}

// 完成容差值的获取
bool PackBLL::Step3_SentScanToleranceValue(uint packId){
    return updateStatus(packId, PackageDto::StatusEnum::Status_Step3_GotScanTolerance);
}

// 等待发送
bool PackBLL::Step4_Waiting4SendWorkData(uint packId){
    return updateStatus(packId, PackageDto::StatusEnum::Status_Step4_WaitingForSend);
}

// 发送成功
bool PackBLL::Step4_SentWorkData(uint packId, QString message){
    return updateStatus(packId, PackageDto::StatusEnum::Status_Step4_Sent, message);
}

/*
bool PackBLL::beginPrint(uint packId){
    return true;
}

bool PackBLL::sentToPrinter(uint packId){
    return true;
}

bool PackBLL::finishPrint(uint packId){
    return true;
}*/

bool PackBLL::panelScanned(QString upi){
    auto panelBll = PanelBLL::getInstance(nullptr);

    // 查找upi关联的板件数据
    auto panel = panelBll->getSingleByUPI(upi);
    if (panel == nullptr){
        qWarning()  << upi << " 对应板件数据为空";
        return false;
    }
    if (panel->isScaned){
        qWarning() << upi << " 已经完成扫码";
        return false;
    }
    int packId = panel->dbPackId;

    // 更新panel表对应记录的status状态
    QString panelUpdateSql = QString("update %1 set %2=%3, update_at = datetime('now', 'localtime') where %4='%5'")
            .arg(panelBll->getTableName())
            .arg(panelBll->dbColumnList[PanelBLL::Status].name)
            .arg(QString::number(PanelBLL::PanelStatusEnum_Scan))
            .arg(panelBll->dbColumnList[PanelBLL::ID].name)
            .arg(QString::number(panel->id));

    // 更新pack表，从panel表中读取已经扫描的总数更新到pack表
    QString packUpdateSql = QString("update %1 set "
                                    "%2=(select count(1) from %3 where %4=%1.id and %5 = %6),"
                                    " update_at = datetime('now', 'localtime') "
                                    " where id=%7")
            .arg(this->tableName)
            .arg(this->dbColumnNames[PackBLL::PackColEnum::ScanPanelCount])
            .arg(panelBll->getTableName())
            .arg(panelBll->dbColumnList[PanelBLL::PackId].name)
            .arg(panelBll->dbColumnList[PanelBLL::Status].name)
            .arg(QString::number(PanelBLL::PanelStatusEnum::PanelStatusEnum_Scan))
            .arg(QString::number(packId));

    // 当扫码总数 == 板件总数时，更新pack表对应记录的status字段
    QString packUpdateStatusSql = QString("update %1 set %2=%3, "
                                          "update_at = datetime('now', 'localtime') "
                                          " where %4=%5 and id=%6")
            .arg(this->tableName)
            .arg(this->dbColumnNames[PackBLL::PackColEnum::Status])
            .arg(QString::number(PackageDto::StatusEnum::Status_Step1_ScanFull))
            .arg(this->dbColumnNames[PackBLL::PackColEnum::ScanPanelCount])
            .arg(this->dbColumnNames[PackBLL::PackColEnum::PanelTotal])
            .arg(QString::number(packId));

    // 执行
    QList<QString> sqls =  QList<QString>{panelUpdateSql, packUpdateSql, packUpdateStatusSql};
    auto result = g_db->ExecSql(sqls);

    return result;
}

bool PackBLL::updateStatus(uint packId,
                           PackageDto::StatusEnum status,
                           QString message,
                           QMap<QString, QVariant> updates){

    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(PackBLL::ID).toInt()!=packId)
            continue;

        auto now =  QDateTime::currentDateTime();
        curRow->setData(PackBLL::Status, status);
        if (status == PackageDto::StatusEnum::Status_Step4_Sent){
            curRow->setData(PackBLL::SentTime, now);
        }
        curRow->setData(PackBLL::LastModifyTime, now);

        // 额外需要更新的字段
        for (const auto key: updates.keys()){
            if (key == this->dbColumnNames.at(PackBLL::PackColEnum::Status)){
                qWarning() << "状态字段重复赋值！";
                continue;
            }

            curRow->setData(key, updates[key]);
            message += QString("col:%1 value:%2->%3; ")
                    .arg(key)
                    .arg(curRow->data(PackBLL::Height).toString())
                    .arg(updates[key].String);
        }

        QString log = curRow->data(PackBLL::Logs).toString();
        if (!message.isEmpty()){
            log = message + log;
        }
        log = QString("status -> %1, at %2 \n\n")
                .arg(PackageDto::statusEnumToString(status), now.toString("MM-dd HH:mm:ss.mmm")) + log;
        curRow->setData(PackBLL::Logs, log);

        QMap<QString, QVariant> wheres;
        wheres[this->dbColumnNames.at(PackBLL::PackColEnum::ID)] = packId; // 使用主键
        dal.update(curRow, wheres);
        return true;
    }

    return false;
}

void PackBLL::refreshLastID()
{
    newId = 0;
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(ID).toInt();
        if(curID>newId)
        {
            newId = curID;
        }
    }
}

QSharedPointer<Row> PackBLL::detail(QString no){
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        QString currentNo = row->data(PackColEnum::No).toString();
        if(no == currentNo)
        {
            return row;
        }
    }
    return nullptr;
}

QSharedPointer<PackageDto> PackBLL::detailStruct(QString no){
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        QString currentNo = row->data(PackColEnum::No).toString();
        if(no.toLower() == currentNo.toLower()) // 不区分大小写
        {
            auto package = convertRow2Package(row);
            return package;
        }
    }
    return nullptr;
}


