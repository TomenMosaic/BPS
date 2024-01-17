#ifndef PANELBLL_H
#define PANELBLL_H

#include <QObject>
#include "core/tabledal.h"
#include "panel.h"

class PanelBLL: public QObject
{
    Q_OBJECT

public:
    enum PanelStatusEnum{
        PanelStatusEnum_Init = 0,
        PanelStatusEnum_Scan = 1
    };

    enum PanelColEnum
    {
        ID,
        ExternalId,
        Length,
        Width,
        Height,
        Layer,
        Rotated,
        Position,
        Name,
        No,
        Remark,
        OrderNo,
        CustomerName,
        Location,

        Status,
        PackId,
        CreateTime,
        LastModifyTime,
        RemovedAt
    };
    QList<DataTableColumn> dbColumnList = {
        {"id", "INTEGER PRIMARY KEY AUTOINCREMENT"},
        {"external_id", "VARCHAR(40)"},
        {"length", "INTEGER"},
        {"width", "INTEGER"},
        {"height", "INTEGER"},
        {"layer", "INTEGER"},
        {"rotated", "INTEGER NOT NULL DEFAULT 0"},
        {"position", "VARCHAR(40)"},
        {"name", "TEXT"},
        {"no", "TEXT"},
        {"remark", "TEXT"},
        {"order_no", "VARCHAR(100)"},
        {"customer_name", "VARCHAR(100)"},
        {"location", "VARCHAR(100)"},

        {"status", "INTEGER"},
        {"pack_id", "INTEGER"},
        {"create_at", "DATETIME"},
        {"update_at", "DATETIME"},
        {"removed_at", "DATETIME"}
    };

    /**
     * @description:
     */
    explicit PanelBLL(QObject *parent = nullptr);

    // 检查并创建表
    void checkAndCreateTable();

    void init();

    /**
     * @description: 插入数据
     */
    int insert(uint length, uint width, uint height, uint packId, QString name, QString remark);

    QList<QSharedPointer<Row>> getRowList(bool isReload = false);

    // 根据包裹id找到关联的板件列表
    QList<Panel> getPanelsByPackId(int packId);

    // 通过UPI找到板件的记录
    QSharedPointer<Panel> getSingleByUPI(QString upi);

    static PanelBLL *getInstance(QObject *parent = nullptr);

    QString getTableName(){
        return tableName;
    }

private:
    static PanelBLL *m_panelBll;

    int newId = 0;

    TableDAL dal;

    QString tableName = "panel";

    QStringList dbColumnNames;
};

#endif // PANELBLL_H
