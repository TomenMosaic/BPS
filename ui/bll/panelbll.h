#ifndef PANELBLL_H
#define PANELBLL_H

#include <QObject>
#include "core/tabledal.h"
#include "panel.h"

class PanelBLL: public QObject
{
    Q_OBJECT

public:
    enum PanelType
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

    QList<Panel> getPanelsByPackId(int packId);

    static PanelBLL *getInstance(QObject *parent = nullptr);

private:
    static PanelBLL *m_panelBll;

    int newId = 0;

    TableDAL dal;

    QString tableName = "panel";

    QStringList dbColumnNames;
};

#endif // PANELBLL_H
