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

QList<Panel> PanelBLL::getPanelsByPackId(int packId) {
    QList<Panel> panels;

    // 设置查询参数
    QString strWhere = QString("%1=%2 and removed_at is null").
            arg(this->dbColumnList.at(PanelType::PackId).name).
            arg(packId);
    QList<QString> strWheres;
    strWheres.append(strWhere);

    // 排序
    QList<QString> strOrders;
    QString strOrder = QString("%1 asc").arg(this->dbColumnNames.at(PanelType::Layer));
    strOrders.append(strOrder);

    // 执行查询
    dal.initTable(tableName, dbColumnNames, strWheres, strOrders, 0, 0, true);
    QList<QSharedPointer<Row>> panelRows = dal.getRowList();
    foreach(QSharedPointer<Row> panelRow, panelRows){
        Panel panel ;
        panel.id = panelRow->data(PanelType::ID).toInt();
        panel.length = panelRow->data(PanelType::Length).toInt();
        panel.width = panelRow->data(PanelType::Width).toInt();
        panel.height = panelRow->data(PanelType::Height).toInt();
        panel.layerNumber = panelRow->data(PanelType::Layer).toInt();
        panel.createTime =  panelRow->data(PanelType::CreateTime).toDateTime();
        panel.name = panelRow->data(PanelType::Name).toString();
        panel.remark = panelRow->data(PanelType::Remark).toString();
        QList<QString> position = panelRow->data(PanelType::Position).toString().split(",");
        panel.position = QPoint(position[0].toInt(), position[1].toInt());
        panel.rotated = panelRow->data(PanelType::Rotated).toBool();

        panels.append(panel);
    }

    return panels;
}
