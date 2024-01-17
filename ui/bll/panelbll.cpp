#include "panelbll.h"
#include "global.h"

PanelBLL *PanelBLL::m_panelBll = nullptr;

PanelBLL *PanelBLL::getInstance(QObject *parent)
{
    if(m_panelBll == nullptr)
    {
        PanelBLL::m_panelBll = new PanelBLL(parent);
    }
    return PanelBLL::m_panelBll;
}

PanelBLL::PanelBLL(QObject *parent) : QObject(parent) // Correct base class
{
    this->init();
}

void PanelBLL::init()
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

void PanelBLL::checkAndCreateTable(){
    this->dal.checkAndCreateTable(this->tableName, this->dbColumnList);
}


Panel convertRow2Panel(QSharedPointer<Row> panelRow){
    Panel panel ;
    panel.id = panelRow->data(PanelBLL::PanelColEnum::ID).toInt();
    panel.no = panelRow->data(PanelBLL::PanelColEnum::No).toString();
    panel.externalId = panelRow->data(PanelBLL::PanelColEnum::ExternalId).toString();

    panel.length = panelRow->data(PanelBLL::PanelColEnum::Length).toInt();
    panel.width = panelRow->data(PanelBLL::PanelColEnum::Width).toInt();
    panel.height = panelRow->data(PanelBLL::PanelColEnum::Height).toInt();
    panel.layerNumber = panelRow->data(PanelBLL::PanelColEnum::Layer).toInt();
    panel.createTime =  panelRow->data(PanelBLL::PanelColEnum::CreateTime).toDateTime();
    panel.name = panelRow->data(PanelBLL::PanelColEnum::Name).toString();
    panel.remark = panelRow->data(PanelBLL::PanelColEnum::Remark).toString();

    panel.orderNo = panelRow->data(PanelBLL::PanelColEnum::OrderNo).toString();
    panel.customerName = panelRow->data(PanelBLL::PanelColEnum::CustomerName).toString();
    panel.location = panelRow->data(PanelBLL::PanelColEnum::Location).toString();

    auto status = static_cast<PanelBLL::PanelStatusEnum>(panelRow->data(PanelBLL::PanelColEnum::Status).toInt());
    panel.isScaned = status == PanelBLL::PanelStatusEnum::PanelStatusEnum_Scan; // 是否扫码

    QList<QString> position = panelRow->data(PanelBLL::PanelColEnum::Position).toString().split(",");
    panel.position = QPoint(position[0].toInt(), position[1].toInt());
    panel.rotated = panelRow->data(PanelBLL::PanelColEnum::Rotated).toBool();
    panel.dbPackId =  panelRow->data(PanelBLL::PanelColEnum::PackId).toUInt();

    return panel;
}

QList<Panel> PanelBLL::getPanelsByPackId(int packId) {
    QList<Panel> panels;

    // 设置查询参数
    QString strWhere = QString("%1=%2 and removed_at is null").
            arg(this->dbColumnList.at(PanelColEnum::PackId).name).
            arg(packId);
    QList<QString> strWheres;
    strWheres.append(strWhere);

    // 排序
    QList<QString> strOrders;
    QString strOrder = QString("%1 asc").arg(this->dbColumnNames.at(PanelColEnum::Layer));
    strOrders.append(strOrder);

    // 执行查询
    dal.initTable(tableName, dbColumnNames, strWheres, strOrders, 0, 0, true);
    QList<QSharedPointer<Row>> panelRows = dal.getRowList();
    foreach(QSharedPointer<Row> panelRow, panelRows){
        auto panel = convertRow2Panel(panelRow);
        panels.append(panel);
    }

    return panels;
}

QSharedPointer<Panel> PanelBLL::getSingleByUPI(QString upi) {
    // 设置查询参数
    QString strWhere = QString("%1=%2 and removed_at is null").
            arg(this->dbColumnList.at(PanelColEnum::No).name).
            arg(upi);
    QList<QString> strWheres;
    strWheres.append(strWhere);

    // 执行查询
    dal.initTable(tableName, dbColumnNames, strWheres, QList<QString>{}, 0, 0, true);
    QList<QSharedPointer<Row>> panelRows = dal.getRowList();
    if (panelRows.empty()) {
        return nullptr;
    } else if (panelRows.size() > 1) {
        qWarning() << upi << " 关联了多条板件记录";
        return nullptr;
    }

    // 使用智能指针创建 Panel 对象
    auto panel = QSharedPointer<Panel>::create(convertRow2Panel(panelRows[0]));
    return panel;
}


