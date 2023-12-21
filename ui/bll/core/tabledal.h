#ifndef TABLEDAL_H
#define TABLEDAL_H

#include <qsqlerror.h>
#include <qstandarditemmodel.h>
#include <qvariant.h>
#include <core/database.h>
#include <QObject>
#include <QSqlQueryModel>
#include <QDebug>

class TableDAL;

class Row
{
public:
    Row(TableDAL *table, QStringList fieldList, QVariantList variantList);
    bool isModify();
    QList<QStandardItem *> toItemList(QList<int> colList = {});
    QStringList toStringList(QList<int> colList = {});
    QVariantList toVariantList(QList<int> colList = {});
    int col(QString fieldName);
    QVariant data(int col);
    QVariant data(QString fieldName);
    void setData(int col, QVariant variant);
    void setData(QString fieldName, QVariant variant);
    // 获取字段列表
    QStringList getFieldList() const;
    // 获取值列表
    QVariantList getValueList() const;
    QString field(int col);

    QMap<int, QVariant> getUpdateList() const;
    void clearUpdateList();
    int colCount();
    // 调用setData后再调用update进行更新
    void update();

    QSharedPointer<Row> clone();

public:
    QStringList fieldList;
    QVariantList valueList;

private:
    QMap<int, QVariant> updateList;
    TableDAL *parentTable = nullptr;
};
QDebug operator<<(QDebug dbg, const Row &message);

struct PaginationInfo
{
    int offset;
    int count;
    int totalPages;
    int pageIndex;
    int pageSize;
};

struct DataTableColumn{
    QString name;
    QString type;
};

class TableDAL : public QObject
{
    Q_OBJECT
public:
    enum UpdateType
    {
        ASYNC, // 异步
        SYNC   // 同步
    };

    explicit TableDAL(QObject *parent = nullptr);

    void checkAndCreateTable(QString tableName, const QList<DataTableColumn> dbColumns);

    /**
     * @description: 重新加载表格
     * @return {*}
     */
    bool reload();

    /**
     * @description: 重新加载表格
     * @param {int} pageSize 每页显示记录行数
     * @param {int} pageIndex 当前页码（从0开始）
     * @return {*}
     */
    bool reload(int pageSize, int pageIndex);
    // bool reload(int beginNum, int endNum, const QString& orderBy = QString());

    /**
     * @description: 初始化表格
     * @param {QString} tableName 表名
     * @param {QStringList} fieldList 字段列表
     * @param {QStringList} conditions 条件列表
     * @param {QStringList} orders 排序
     * @param {int} pageSize 每页显示记录行数
     * @param {int} pageIndex 当前页码（从0开始）
     * @param {bool} load 是否加载
     * @return {*}
     */
    void initTable(QString tableName, QStringList fieldList,
                   QStringList conditions, QStringList orders,
                   int pageSize, int pageIndex,
                   bool load = true);

    /**
     * @description: 初始化表格
     * @param {QString} tableName 表名
     * @param {QStringList} fieldList 字段列表
     * @param {QStringList} conditions 条件列表
     * @param {QStringList} orders 排序
     * @param {bool} load 是否加载
     * @return {*}
     */
    void initTable(QString tableName, QStringList fieldList = QStringList(),
                   QStringList conditions = QStringList(), QStringList orders = QStringList(),
                   bool load = true);

    /**
     * @description: 初始化表格
     * @param {QString} tableName 表名
     * @param {QStringList} fieldList 字段列表
     * @param {bool} load 是否加载
     * @return {*}
     */
    void initTable(QString tableName, QStringList fieldList = QStringList(), bool load = true);

    /**
     * @description: 初始化表格
     * @param {QString} tableName 表名
     * @param {bool} load 是否加载
     * @return {*}
     */
    void initTable(QString tableName, bool load = true);

    /**
     * @description: 获取记录总数
     * @param {QString} tableName 表名
     * @param {QStringList} conditions 条件列表
     * @return {*}
     */
    int queryCount(QString tableName, QStringList conditions = QStringList());

    QList<QVariantList> getVarList(QList<int> colList = {});

    QList<QStringList> getStrList(QList<int> colList = {});

    QList<QSharedPointer<Row>> getRowList();

    QSharedPointer<Row> getRow(int row);

    QStringList getRowStrs(int row);

    QVariantList getRowVars(int row);

    QStandardItemModel *getModel();

    bool removeRow(QSharedPointer<Row> row, UpdateType type = SYNC);

    QVariantList getColCell(QString fieldName);

    QVariantList getColCell(int col);

    int getLastID();

    bool appendRow(QMap<QString, QVariant> valMap);
    bool appendRow(QMap<QString, QString> valMap);

    void update(QSharedPointer<Row> row, UpdateType type = SYNC);

    void remove(QStringList conditions = QStringList(), UpdateType type = SYNC);

    QStringList getFieldList() const;

    QString getColName(int col);

    int getColCount();

    int getRowCount();

    void clear();

    int getCol(QString fieldName);

    ~TableDAL();

    void refreshLastID();

    QString getTableName() const;

    static QString currentTime();

    static QStringList toStringList(QVariantList variantList);

    static QList<int> toIntList(QVariantList variantList);

    void calculatePagination(int pageSize, int pageIndex);

    QStringList getColumnNames(QList<DataTableColumn> dataTableColumns);

private:
    void addNewRow(QStringList nameList, QStringList valList);
    void addNewRow(QStringList nameList, QList<QVariant> valList);

signals:
    void error(QString errorMsg);

private:
    QStringList p_conditions;
    QStringList p_orders;

    PaginationInfo *p_pageinationInfo = nullptr;

    int p_recordTotal;
    int p_lastID = 0;
    QList<QSharedPointer<Row>> p_rowList;
    //    QList<QSharedPointer<Row>> rowList;
    QString p_tableName;
    QStringList p_fieldList;
};

#endif // TABLEDAL_H
