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
    dal.initTable(tableName, this->dbColumnNames, QStringList(), orders, 12, 0, true);
}

QString PackBLL::statusEnumToString(StatusEnum status){
    switch (status) {
    case Status_Init:
        return QStringLiteral("初始化");
    case Status_Calculated:
        return QStringLiteral("计算结束");
    case Status_WaitingForScan:
        return QStringLiteral("等待扫码");
    case Status_WaitingForSend:
        return QStringLiteral("等待发送");
    case Status_Sent:
        return QStringLiteral("已发送");
    case Status_Finish:
        return QStringLiteral("结束");
    default:
        return QStringLiteral("-");
    }
}

void PackBLL::checkAndCreateTable(){
    this->dal.checkAndCreateTable(this->tableName, this->dbColumnList);
}

QList<QSharedPointer<Row>> PackBLL::getRowList(bool isReload)
{
    if (isReload){
        dal.reload();
    }
    return dal.getRowList();
}

int PackBLL::insert(QString no, uint length, uint width, uint height, PackTypeEnum type){
    QMap<QString,QString> mapList;
    mapList.insert(this->dbColumnNames.at(No),no);
    mapList.insert(this->dbColumnNames.at(Length),QString::number(length));
    mapList.insert(this->dbColumnNames.at(Width),QString::number(width));
    mapList.insert(this->dbColumnNames.at(Height),QString::number(height));
    mapList.insert(this->dbColumnNames.at(Type),QString::number(type));
    mapList.insert(this->dbColumnNames.at(Status),QString::number(Status_Init));
    QDateTime currentTime =  QDateTime::currentDateTime();
    QString formattedTime = currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
    mapList.insert(this->dbColumnNames.at(CreateTime),formattedTime);
    mapList.insert(this->dbColumnNames.at(LastModifyTime),formattedTime);
    dal.appendRow(mapList);
    newId = dal.getLastID();
    return newId;
}

int PackBLL::insertByPackStruct(const Package& package){

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    int newPackId ;

    try {
        QSqlQuery query(db);

        //1. 插入数据到pack表
        //1.1. 需要插入的数据
        QMap<QString,QVariant> mapList;
        mapList.insert(this->dbColumnNames.at(No),package.no);
        mapList.insert(this->dbColumnNames.at(CustomerName),package.customerName);
        mapList.insert(this->dbColumnNames.at(Length),package.length);
        mapList.insert(this->dbColumnNames.at(Width),package.width);
        mapList.insert(this->dbColumnNames.at(Height),package.height);
        mapList.insert(this->dbColumnNames.at(Type),PackBLL::PackType_Socket); //TODO, 此种方式不一定是Socket
        mapList.insert(this->dbColumnNames.at(Status),QString::number(Status_Init));
        QDateTime currentTime = QDateTime::currentDateTime();
        QString formattedTime = currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
        mapList.insert(this->dbColumnNames.at(CreateTime),formattedTime);
        mapList.insert(this->dbColumnNames.at(LastModifyTime),formattedTime);

        //1.2. 构造sql参数
        QStringList packColnames = mapList.keys();
        QStringList packBindVals;
        for (const auto &colName : qAsConst(packColnames)) {
            packBindVals << (":" + colName);
        }
        query.prepare(QString("INSERT INTO pack (%1) VALUES (%2);").
                      arg(packColnames.join(","), packBindVals.join(",")));
        for (const auto &colName : qAsConst(packColnames)) {
            query.addBindValue(mapList.value(colName));
        }
        if (!query.exec()) {
            QSqlError error = query.lastError();
            throw std::runtime_error(("无法插入 pack 表记录, "+error.text()).toStdString());
        }

        //1.3. 获取插入的packid
        newPackId = query.lastInsertId().toInt();
        query.clear();

        //2. 插入关联的panel记录
        foreach (Layer layer, package.layers) {
            foreach(const Panel& panel, layer.panels){
                QMap<QString, QVariant> panelMapList;
                panelMapList.insert("pack_id", newPackId);
                panelMapList.insert("name", panel.name);
                panelMapList.insert("remark", panel.remark);
                panelMapList.insert("external_id", panel.externalId);
                panelMapList.insert("layer", panel.layerNumber);
                panelMapList.insert("position", QString("%1,%2").
                                                    arg(panel.position.x()).
                                                    arg(panel.position.y()));
                panelMapList.insert("rotated",QString::number(panel.rotated));
                panelMapList.insert(this->dbColumnNames.at(Length),QString::number(panel.length));
                panelMapList.insert(this->dbColumnNames.at(Width),QString::number(panel.width));
                panelMapList.insert(this->dbColumnNames.at(Height),QString::number(panel.height));

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
                query.clear();

            }
        }

        //3. 提交
        db.commit();
        query.clear();

        // 刷新数据
        dal.reload();
    }  catch (...) {
        db.rollback();
    }

    return newPackId;
}

bool PackBLL::remove(uint id){
    return false;
}

QSharedPointer<Row> PackBLL::detail(uint id){
    return nullptr;
}

bool PackBLL::calculated(uint id){
    return updateStatus(id, PackBLL::Status_Calculated);
}

bool PackBLL::waitingForScan(uint id, QString message){
    return updateStatus(id, PackBLL::Status_WaitingForScan, message);
}

bool PackBLL::waitingForSend(uint id){
    return updateStatus(id, PackBLL::Status_WaitingForSend);
}

bool PackBLL::sent(uint id, QString message){
    return updateStatus(id, PackBLL::Status_Sent, message);
}

bool PackBLL::finish(uint id){
    return updateStatus(id, PackBLL::Status_Finish);
}

bool PackBLL::beginPrint(uint id){
    return true;
}

bool PackBLL::sentToPrinter(uint id){
    return true;
}

bool PackBLL::finishPrint(uint id){
    return true;
}

bool PackBLL::updateStatus(uint id, PackBLL::StatusEnum status, QString message){
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(curRow->data(PackBLL::ID).toInt()==id)
        {
            auto now =  QDateTime::currentDateTime();
            curRow->setData(PackBLL::Status, status);
            if (status == Status_Sent){
                curRow->setData(PackBLL::SentTime, now);
            }
            curRow->setData(PackBLL::LastModifyTime, now);
            QString log = curRow->data(PackBLL::Logs).toString();
            if (!message.isEmpty()){
                log += message;
            }
            log += QString("status -> %1, at %2 \n\n").arg(statusEnumToString(status), now.toString("MM-dd HH:mm:ss.mmm"));
            curRow->setData(PackBLL::Logs, log);
            dal.update(curRow);
            return true;
        }
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
