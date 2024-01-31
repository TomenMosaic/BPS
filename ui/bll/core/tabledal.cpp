#include "database.h"
#include "tabledal.h"
#include <qdebug.h>
#include <qsqlrecord.h>
#include "global.h"
#include "convert.h"

#include <QDateTime>
// #include "GlobalVar.h"

DataBase *_dataBase = nullptr;

TableDAL::TableDAL(QObject *parent)
    : QObject{parent}
{
    _dataBase = nullptr;
}


bool TableDAL::reload()
{
    if (this->p_pageinationInfo!=nullptr){
        return reload(this->p_pageinationInfo->pageSize, this->p_pageinationInfo->pageIndex);
    }
    return reload(0, 0);
}

bool TableDAL::reload(int pageSize, int pageIndex, QStringList conditions, QStringList orders){
    this->p_conditions = conditions;
    if (orders.size() > 0){
        this->p_orders = orders;
    }    

    return reload(pageSize, pageIndex);
}

void TableDAL::checkAndCreateTable(QString tableName, const QList<DataTableColumn> dbColumnList){
    QStringList strs;
    for (int i = 0; i < dbColumnList.length(); i ++){
        DataTableColumn dbColumn = dbColumnList[i];
        strs.append(dbColumn.name+" "+dbColumn.type);
    }

    //1. 表不存在，创建表
    QString createTableQuery = QString("CREATE TABLE IF NOT EXISTS %1 (%2)").
            arg(tableName, strs.join(", "));
    g_db->ExecSql(createTableQuery);

    //2. 检测字段是否存在
    //2.1. 获取表的元数据
    QString pragmaQuery = QString("PRAGMA table_info(%1)").arg(tableName);
    auto tmpQuery =  g_db->query(pragmaQuery);

    //2.2. 存储表中存在的字段名称
    QSet<QString> existingColumns;
    while (tmpQuery.next()) {
        QString columnName = tmpQuery.value(1).toString(); // 第二列包含字段名称
        existingColumns.insert(columnName);
    }

    //2.3. 检查每个期望的字段是否存在
    for (const DataTableColumn& dbColumn : dbColumnList) {
        if (!existingColumns.contains(dbColumn.name)) {
            // 字段不存在，添加字段
            QString alterTableQuery = QString("ALTER TABLE %1 ADD COLUMN %2 %3").
                    arg(tableName,dbColumn.name,dbColumn.type);
            if (!g_db->ExecSql(alterTableQuery)) {
                qDebug() << "Failed to add column:" << dbColumn.name;
            }else{
                qInfo() << "add new column: " << alterTableQuery;
            }
        }
    }
}

bool TableDAL:: reload(int pageSize, int pageIndex) //TODO beginNum endNum 可以为空
{
    // 1. 参数验证
    // 1.1 验证开始编号和结束编号的有效性
    TableDAL::calculatePagination(pageSize, pageIndex);

    // 2. 数据清理
    // 2.1 清空当前数据列表
    this->clear();

    // 3. 构建SQL查询命令
    // 3.1 构建字段列表字符串，若this->p_fieldList为空则选择所有字段(*)
    QString fields = this->p_fieldList.isEmpty() ? "*" : ("`" + this->p_fieldList.join("`, `") + "`");

    // 3.2 初始化查询命令
    QString cmd = QString("SELECT %1 FROM `%2`").arg(fields, this->p_tableName);

    // 4. 处理查询条件
    // 4.1 如果存在条件，则将其添加到查询命令中
    if (!this->p_conditions.isEmpty()) {
        QString condition = this->p_conditions.join(" AND ");
        cmd += " WHERE " + condition;
    }

    // 5. 排序和分页
    // 5.1 如果提供了orderBy参数，将其添加到查询命令中
    if (!this->p_orders.isEmpty()) {
        QString orderby = this->p_orders.join(" , ");
        cmd += " ORDER BY " + orderby;
    }
    // 5.2 添加LIMIT子句用于分页
    if (this->p_pageinationInfo!=nullptr){ // 均为0，代表不需要limit
        int offset = this->p_pageinationInfo->offset;
        int pSize = this->p_pageinationInfo->pageSize;
        cmd += QString(" LIMIT %1, %2").arg(offset).arg(pSize); // 注意这里确保 LIMIT 的范围正确
    }

    // 6. 执行查询
    // 6.1 运行SQL查询
    QSqlQuery query = g_db->query(cmd);

    // 6.2 检查查询是否成功
    if (query.lastError().isValid()) {
        // 6.2.1 查询失败，记录错误信息
        QString errorMsg = QString("Run SQL CMD=%1---error:%2").arg(cmd, query.lastError().text()).toUtf8();
        emit error(query.lastError().text());
        CLOG_ERROR(errorMsg.toUtf8());
        qWarning() << Q_FUNC_INFO << __LINE__ << query.lastError().text() << " CMD:" << cmd;
        return false;
    }

    // 7. 处理查询结果
    // 7.1. 遍历查询结果集，并填充数据
    while (query.next()) {
        QVariantList rowVals;
        for (const auto& fieldName : qAsConst(this->p_fieldList)) {
            rowVals.append(query.value(fieldName));
        }
        QSharedPointer<Row> row(new Row(this, this->p_fieldList, rowVals));
        this->p_rowList.append(row);
    }

    // 8. 后处理
    // 8.1 更新最后一条记录的ID
    refreshLastID();

    // 8.2 返回成功
    return true;
}

void TableDAL::initTable(QString tableName, QStringList fieldList,
                         QStringList conditions, QStringList orders,
                         int pageSize, int pageIndex,
                         bool load)
{
    this->p_tableName = tableName;
    this->p_fieldList = fieldList;
    this->p_conditions = conditions;
    this->p_orders=orders;
    if(load)
        this->reload(pageSize, pageIndex);
}

void TableDAL::initTable(QString tableName, QStringList fieldList,
                         QStringList conditions, QStringList orders,
                         bool load)
{
    this->p_tableName = tableName;
    this->p_fieldList = fieldList;
    this->p_conditions = conditions;
    this->p_orders=orders;
    if(load)
    {
        this->reload();
    }
}

void TableDAL::initTable(QString tableName, QStringList fieldList, bool load)
{
    this->initTable(this->p_tableName,fieldList,QStringList(),QStringList(),load);
}

void TableDAL::initTable(QString tableName, bool load)
{
    this->initTable(tableName,QStringList(),QStringList(),QStringList(),load);
}

int TableDAL::queryCount(QString tableName, QStringList conditions)
{
    this->clear();
    QString cmd = QString("SELECT COUNT(*) FROM %1").arg(tableName);

    // 直接构建 WHERE 子句
    QString where = conditions.join(" AND ");
    if (!where.isEmpty()) {
        cmd += " WHERE " + where;
    }

    QSqlQuery query = g_db->query(cmd);

    if (!query.exec()) {
        // 错误处理
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

QList<QStringList> TableDAL::getStrList(QList<int> colList)
{
    QList<QStringList> strList;
    for(int index =0;index<this->p_rowList.length();index++)
    {
        strList.append(getRow(index)->toStringList(colList));
    }
    return strList;
}

QList<QVariantList> TableDAL::getVarList(QList<int> colList)
{
    QList<QVariantList> varList;
    for(int index =0;index<this->p_rowList.length();index++)
    {
        varList.append(getRow(index)->toVariantList(colList));
    }
    return varList;
}

QList<QSharedPointer<Row>> TableDAL::getRowList()
{
    return this->p_rowList;
}

QSharedPointer<Row> TableDAL::getRow(int row)
{
    if (row < 0 || row >= this->p_rowList.length()) {
        // 处理无效的 row，例如记录错误、返回错误码或抛出异常
        return this->p_rowList.at(row); // 或执行其他适当的错误处理
    }
    assert(row>=0&&row<this->p_rowList.length());
    return this->p_rowList.at(row);
}

QSharedPointer<Row> TableDAL::getRow(QMap<QString, QVariant> conditions){
    //1. 在现有缓存数据中查询
    for (int i = 0; i< this->p_rowList.size(); i++){
        auto row = this->p_rowList[i];
        bool isContain = false;
         for(const QString key : conditions.keys()){
             if (row->data(key) !=  conditions[key]){
                 isContain = false;
                 continue;
             }
         }
         if (isContain){
             return row;
         }
    }

    //
    // 3.1 构建字段列表字符串，若this->p_fieldList为空则选择所有字段(*)
    QString fields = this->p_fieldList.isEmpty() ? "*" : ("`" + this->p_fieldList.join("`, `") + "`");

    // 3.2 初始化查询命令
    QString cmd = QString("SELECT %1 FROM `%2`").arg(fields, this->p_tableName);

    // 4. 处理查询条件
    if (!conditions.isEmpty()) {
        QStringList conditionParts;
        for (auto it = conditions.constBegin(); it != conditions.constEnd(); ++it) {
            auto tmp = QString("`%1` = '%2'")
                    .arg(it.key(), it.value().toString());
            conditionParts.append(tmp);
        }
        QString condition = conditionParts.join(" AND ");
        cmd += " WHERE " + condition;
    }

    // 5. 排序和分页
    // 5.1 如果提供了orderBy参数，将其添加到查询命令中
    if (!this->p_orders.isEmpty()) {
        QString orderby = this->p_orders.join(" , ");
        cmd += " ORDER BY " + orderby;
    }

    // 6. 执行查询
    QSqlQuery query;
    if (!query.exec(cmd)) {
        // 处理查询失败的情况
        QString errorMsg = QString("Run SQL CMD=%1---error:%2").arg(cmd, query.lastError().text()).toUtf8();
        emit error(query.lastError().text());
        CLOG_ERROR(errorMsg.toUtf8());
        qWarning() << Q_FUNC_INFO << __LINE__ << query.lastError().text() << " CMD:" << cmd;
        return nullptr;
    }

    // 尝试获取第一行
    if (query.next()) {
        QVariantList rowVals;
        for (const auto& fieldName : qAsConst(this->p_fieldList)) {
            rowVals.append(query.value(fieldName));
        }
        QSharedPointer<Row> row(new Row(this, this->p_fieldList, rowVals));

        // 确保没有其他行
        if (query.next()) {
            // 如果有额外的行，处理错误或逻辑
            qWarning() << "More than one row returned for the query.";
            return nullptr;
        }
        return row;
    } else {
        // 如果没有行返回
        qWarning() << "No rows returned for the query.";
        return nullptr;
    }

    return nullptr;
}

QStandardItemModel *TableDAL::getModel()
{
    QStandardItemModel *model = new QStandardItemModel;
    for(int rowIndex = 0;rowIndex<this->p_rowList.length();rowIndex++)
    {
        QSharedPointer<Row> row = this->p_rowList.at(rowIndex);
        QList<QStandardItem*> itemList = row->toItemList();
        model->appendRow(itemList);
    }
    return model;
}

bool TableDAL::removeRow(QSharedPointer<Row>row,UpdateType type)
{
    int rowIndex = this->p_rowList.indexOf(row);
    if(rowIndex!=-1)
    {
        QStringList fieldList = row->getFieldList();
        QVariantList valueList = row->getValueList();
        QStringList removeDataList;
        for(int index = 0;index<row->colCount();index++)
        {
            QString field = fieldList.at(index);
            QString value = VarToString(valueList.at(index));
            if(value.isEmpty())
                continue;
            QString changeVar = QString("%1='%2'").arg(field, value);
            removeDataList.append(changeVar);
        }
        QString cmd = QString("delete from %1 where %2").arg(this->p_tableName, removeDataList.join(" and "));
        if(type==SYNC)
        {
            QSqlQuery query = g_db->query(cmd);
            query.clear();
        }
        else
        {
            g_db->updateCmd(cmd);
        }
        this->p_rowList.removeAt(rowIndex);
        return true;
    }
    else
    {
        if(row)
        {
            CLOG_ERROR(QString("remove un exist row :%1").arg(TableDAL::toStringList(row->getValueList()).join(",")).toUtf8());
            qWarning()<<Q_FUNC_INFO<<__LINE__<<QString("remove un exist row :%1").
                        arg(TableDAL::toStringList(row->getValueList()).join(","));
        }

        return false;
    }
}

QVariantList TableDAL::getColCell(QString fieldName)
{
    int col = getCol(fieldName);
    return this->getColCell(col);
}

QVariantList TableDAL::getColCell(int col)
{
    assert(col>=0&&col<this->p_fieldList.length());
    QVariantList varList;
    for(int index = 0;index<this->p_rowList.length();index++)
    {
        QSharedPointer<Row>row = getRow(index);
        varList.append(row->data(col));
    }
    return varList;
}

QStringList TableDAL::getColumnNames(QList<DataTableColumn> dataTableColumns){
    QStringList columns;
    for (int i = 0; i < dataTableColumns.length(); i++){
        columns.append(dataTableColumns[i].name);
    }
    return columns;
}

int TableDAL::getLastID()
{
    refreshLastID();
    return this->p_lastID;
}

bool TableDAL::appendRow(QMap<QString, QString> valMap){
    QMap<QString, QVariant> variantMap;
    for (auto it = valMap.constBegin(); it != valMap.constEnd(); ++it) {
        variantMap.insert(it.key(), QVariant(it.value()));
    }
    return TableDAL::appendRow(variantMap);
}

bool TableDAL::appendRow(QMap<QString, QVariant> valMap)
{
    if(valMap.isEmpty())
        return false;

    QStringList nameList = valMap.keys();
    QStringList valList;
    foreach(QString name,nameList)
    {
        valList.append(QString(":%1").arg(name));
    }

    // insert
    QString cmd = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(this->p_tableName, nameList.join(","), valList.join(","));
    QList<QVariant>values = valMap.values();
    bool ok = g_db->executeParameterizedSqlQuery(cmd, values);
    if (ok){
        // 添加新行
        //this->addNewRow(nameList,values);
        this->reload();

        // 刷新最新的id
        refreshLastID();
        this->p_recordTotal++; // 总行数增加

        return true;
    }

    return false;
}

void TableDAL::update(QSharedPointer<Row>row, const QMap<QString, QVariant>& primaryKeys, UpdateType type)
{
    QMap<int,QVariant> updateList = row->getUpdateList();
    if(updateList.isEmpty())
        return;

    QMap<QString, QVariant> params;

    // 构建WHERE条件
    QStringList whereConditions;
    if (!primaryKeys.isEmpty()) {
        for (auto it = primaryKeys.cbegin(); it != primaryKeys.cend(); ++it) {
            QString placeholder = QString(":where_%1").arg(it.key());
            whereConditions.append(QString("%1=%2").arg(it.key(), placeholder));
            params.insert(placeholder, it.value());  // 在参数映射中添加值
        }
    } else{
        for (int colIndex = 0; colIndex < row->colCount(); colIndex++) {
            QString fieldName = row->field(colIndex);
            if (updateList.contains(colIndex)) { // 只使用需要更新的列
                continue;
            }
            QString value = VarToString(row->data(colIndex));
            if (!value.isEmpty()) {
                QString placeholder = QString(":where_%1").arg(fieldName);
                whereConditions.append(QString("%1=%2").arg(fieldName, placeholder));
                params.insert(placeholder, value);  // 在参数映射中添加值
            }
        }
    }

    // 构建更新列表
    QStringList changeList;
    QMapIterator<int, QVariant> i(updateList);
    while (i.hasNext()) {
        i.next();
        QString fieldName = row->field(i.key());
        QString placeholder = QString(":update_%1").arg(i.key());
        changeList.append(QString("%1=%2").arg(fieldName, placeholder));  // 将字段和占位符添加到更新列表
        params.insert(placeholder, updateList.value(i.key()));  // 在参数映射中添加值
    }

    // sql 语句
    QString cmd = QString("update %1 set %2 where %3").
            arg(this->p_tableName, changeList.join(","), whereConditions.join(" and "));
    if(type==SYNC)
        g_db->ExecSql(cmd, params);  // 同步执行SQL
    else
        g_db->updateCmd(cmd);

    //
    row->update();
}

void TableDAL::remove(QStringList conditions, UpdateType type)
{
    // 检查条件是否为空，避免删除整张表
    if (conditions.isEmpty()) {
        qWarning() << "Delete operation aborted: No conditions provided.";
        return;
    }

    QString cmd =  QString("delete from %1").arg(this->p_tableName);
    if(!this->p_conditions.isEmpty())
        cmd = QString("delete from %1 where %2").arg(this->p_tableName, this->p_conditions.join(" and "));
    if(type==SYNC)
    {
        QSqlQuery query = g_db->query(cmd);
        if(query.lastError().isValid())
        {
            qWarning()<<Q_FUNC_INFO<<__LINE__<<query.lastError()<<"  cmd:"<<cmd;
            CLOG_ERROR(QString("exec cmd :%1----%2").arg(cmd, query.lastError().text()).toUtf8());
        }else{
            // 使用影响的行数更新 p_recordTotal
            int rowsAffected = query.numRowsAffected();
            this->p_recordTotal -= rowsAffected;
        }
        query.clear();
    }
    else
        g_db->updateCmd(cmd);
}


QStringList TableDAL::getFieldList() const
{
    return this->p_fieldList;
}

QString TableDAL::getColName(int col)
{
    assert(col>=0&&col<this->p_fieldList.length());
    return this->p_fieldList.at(col);
}

int TableDAL::getColCount()
{
    return this->p_fieldList.length();
}

int TableDAL::getTotal()
{
    // 准备 SQL 查询
    QString cmd = QString("SELECT COUNT(*) FROM %1").arg(this->p_tableName);
    if(!this->p_conditions.isEmpty())
        cmd += " WHERE "+ this->p_conditions.join(" and ");

    // 执行查询
    QSqlQuery query = g_db->query(cmd);

    if (query.exec())
    {
        // 确保查询返回了结果
        if (query.next())
        {
            // 返回计数结果
            return query.value(0).toInt();
        }
    }
    else
    {
        // 查询失败，可能需要记录错误或处理错误
        qWarning() << "Failed to execute query:" << query.lastError();
        CLOG_ERROR(QString("Failed to execute query:%1").arg(cmd).toUtf8());
    }

    // 如果查询失败或未返回结果，返回 0
    return 0;
}

int TableDAL::getRowCount()
{
    return this->p_rowList.size();
}

void TableDAL::calculatePagination(int pageSize, int pageIndex)
{
    if (pageSize <= 0 && pageIndex <= 0) {
        this->p_pageinationInfo = nullptr;
        return;
    }

    // 初始化
    this->p_pageinationInfo = new PaginationInfo();

    // 校正 pageSize
    this->p_pageinationInfo->pageSize = (pageSize <= 0) ? 20 : pageSize;

    // 计算总页数
    int totalPages = (this->p_recordTotal + this->p_pageinationInfo->pageSize - 1) / this->p_pageinationInfo->pageSize; // 向上取整
    totalPages = std::max(totalPages, 1); // 确保至少有1页

    // 校正页码
    this->p_pageinationInfo->pageIndex = std::max(pageIndex, 1); // 确保页码最小为1
    this->p_pageinationInfo->pageIndex = std::min(pageIndex, totalPages); // 确保页码不超过最大页数

    // 偏移值
    this->p_pageinationInfo->offset = this->p_pageinationInfo->pageSize * ( this->p_pageinationInfo->pageIndex - 1);

    // 总页数
    this->p_pageinationInfo->totalPages = totalPages;
}

void TableDAL::clear()
{
    this->p_rowList.clear();
}

int TableDAL::getCol(QString fieldName)
{
    return this->p_fieldList.indexOf(fieldName);
}

TableDAL::~TableDAL()
{
    clear();
}

void TableDAL::refreshLastID()
{
    this->p_lastID = 0;
    QString cmd = QString("select seq from sqlite_sequence where name = '%1'").arg(this->p_tableName);
    QJsonObject obj = g_db->execQuery(cmd);
    QJsonArray array = obj.value("value").toArray();
    if(array.size()>0)
    {
        QJsonArray roleArray = array[0].toArray();
        if(roleArray.size()>0)
        {
            QJsonValue value = roleArray.at(0);
            this->p_lastID = value.toInt();
        }
    }

}

QString TableDAL::getTableName() const
{
    return this->p_tableName;
}

QString TableDAL::currentTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

QStringList TableDAL::toStringList(QVariantList variantList)
{
    QStringList strList;
    for(int index = 0;index<variantList.length();index++)
    {
        strList.append(VarToString(variantList.at(index)));
    }

    return strList;
}

QList<int> TableDAL::toIntList(QVariantList variantList)
{
    QList<int> intList;
    foreach(QVariant value,variantList)
    {
        intList.append(VarToInt(value));
    }
    return intList;
}

Row::Row(TableDAL *table, QStringList fieldList, QVariantList variantList)
{
    this->parentTable = table;
    this->fieldList = fieldList;
    this->valueList = variantList;
}

bool Row::isModify()
{
    if(updateList.isEmpty())
        return false;
    else
        return true;
}

QList<QStandardItem *> Row::toItemList(QList<int> colList)
{
    QList<QStandardItem*> rowItems;
    QVariantList valueList = toVariantList(colList);
    for(int index = 0;index<valueList.length();index++)
    {
        QStandardItem *item = new QStandardItem;
        item->setData(valueList.at(index),Qt::DisplayRole);
        rowItems.append(item);
    }
    return rowItems;
}

QStringList Row::toStringList(QList<int>colList)
{
    QVariantList valueList = toVariantList(colList);
    QStringList rowStrs;
    for(int index = 0;index<valueList.length();index++)
    {
        rowStrs.append(VarToString(valueList.at(index)));
    }
    return rowStrs;
}

QVariantList Row::toVariantList(QList<int>colList)
{
    if(colList.isEmpty())
        return valueList;
    else
    {
        QVariantList rValueList;
        for(int index = 0;index<colList.length();index++)
        {
            rValueList.append(data(colList.at(index)));
        }
        return rValueList;
    }
}

int Row::col(QString fieldName)
{    
    return this->fieldList.indexOf(fieldName);
}

QVariant Row::data(int col)
{
    assert(col>=0&&col<valueList.length());
    return valueList.at(col);
}

QVariant Row::data(QString fieldName)
{
    return data(col(fieldName));
}

void Row::setData(int col, QVariant variant)
{
    if(this->data(col)!=variant)
    {
        updateList.insert(col,variant);
    }
}

void Row::setData(QString fieldName, QVariant variant)
{
    int curcol = col(fieldName);
    if(curcol)
    {
        setData(curcol,variant);
    }
}

QStringList Row::getFieldList() const
{
    return this->fieldList;
}

QVariantList Row::getValueList() const
{
    return valueList;
}

QString Row::field(int col)
{
    assert(col>=0&&col<this->fieldList.length());
    return this->fieldList.at(col);
}

void Row::update()
{
    for(auto it = this->updateList.begin();it!=this->updateList.end();it++)
    {
        int col = it.key();
        QVariant value = it.value();
        valueList.replace(col,value);
    }
    this->updateList.clear();
}

QSharedPointer<Row>Row::clone()
{
    QSharedPointer<Row>newRow = QSharedPointer<Row>(new Row(this->parentTable,this->fieldList,this->valueList));
    return newRow;
}

QMap<int, QVariant> Row::getUpdateList() const
{
    return this->updateList;
}

void Row::clearUpdateList()
{
    this->updateList.clear();
}

int Row::colCount()
{
    return this->fieldList.count();
}

QDebug operator<<(QDebug dbg, const Row &message)
{
    for(int index = 0;index<message.fieldList.length();index++)
    {
        dbg<<message.fieldList.at(index)<<":"<<message.valueList.at(index)<<"  ";
    }
    return dbg;
}
