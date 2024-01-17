/*
 * @Date: 2023-11-21 16:07:37
 * @LastEditors: Your Name
 * @LastEditTime: 2023-11-22 12:42:37
 * @FilePath: \pack\ui\bll\packbll.h
 */
#ifndef PACKBLL_H
#define PACKBLL_H

#include <QObject>
#include "core/tabledal.h"
#include "package.h"

class PackBLL : public QObject
{
    Q_OBJECT

public:
    enum PackColEnum
    {
        ID,
        Length,
        Width,
        Height,
        // 模板名称
        TemplateName,
        Status,
        // 箱子的编码，预分包时会由当前系统生成
        No,
        // 关联的订单号
        OrderNo,
        // 客户名称
        CustomerName,
        // 类型
        Type,
        // 已扫码的板件数据
        ScanPanelCount,
        // 包裹中板件的总数量
        PanelTotal,
        // 所在格（订单中包裹的序号）
        FlowNo,

        Logs,
        SentTime,
        CreateTime,
        LastModifyTime,
        // 软删除的时间
        RemovedAt
    };
    const QList<DataTableColumn> dbColumnList = {
        {"id", "INTEGER PRIMARY KEY AUTOINCREMENT"},
        {"length", "INTEGER"},
        {"width", "INTEGER"},
        {"height", "INTEGER"},
        {"template_name", "TEXT"},
        {"status", "INTEGER"},
        {"no", "TEXT"},
        {"order_no", "TEXT"},
        {"customer_name", "TEXT"},
        {"type", "INTEGER"},

        {"scan_panel_count", "INTEGER"},
        {"panel_total", "INTEGER"},
        {"flow_no", "INTEGER"},

        {"logs", "TEXT"},
        {"sent_at", "DATETIME"},
        {"create_at", "DATETIME"},
        {"update_at", "DATETIME"},
        {"removed_at", "DATETIME"}
    };

    enum StatusEnum{
        Status_Init = 0,
        // 齐套
        Status_Full = 9,
        // 计算结束
        Status_Calculated = 19,
        // 等待扫码
        Status_WaitingForScan = 20,
        // 等待发送
        Status_WaitingForSend = 26,
        // 已经发送
        Status_Sent = 39,
        Status_Finish = 99
    };
    enum PrintStatusEnum{
        PrintStatus_Init = 0,
        PrintStatus_Sent = 1,
        PrintStatus_Printed = 9
    };
    enum PackTypeEnum{
        // 预分包（数据已经存在于当前数据库中）
        PackType_PrePackaging = 0,
        // socket server 获取到的数据
        PackType_Socket = 1,
    };

    /**
     * @description: 状态枚举转换为中文
     */
    QString statusEnumToString(StatusEnum status);

    /**
     * @description: 状态枚举转换为中文
     */
    QString printStatusEnumToString(PrintStatusEnum status);

    /**
     * @description:
     */
    explicit PackBLL(QObject *parent = nullptr);

    // 检查并创建表
    void checkAndCreateTable();

    void init();

    //QList<QSharedPointer<Row>> getRowList(bool isReload = false);

    QList<QSharedPointer<Row>> getList(QString orderNo = "");

    QList<QSharedPointer<Row>> getCacheList();

    //
    Package getPackageByDbId(uint packId);

    bool save(QList<QVariantList> valueList);

    /**
     * @description: 获取分页数据
     */
    QList<QSharedPointer<Row>> getPage();

    /**
     * @description: 插入数据
     */
    int insert(QString no, uint length, uint width, uint height, PackTypeEnum type);

    int insertByPackStruct(const Package& package, PackTypeEnum packType = PackTypeEnum::PackType_Socket);

    QVector<int> insertByPackStructs(const QList<Package> packages, PackTypeEnum packType = PackTypeEnum::PackType_Socket);

    /**
     * @description: 更新数据
     */
    bool update(uint id);

    /**
     * @description: 删除数据
     */
    bool remove(uint id);

    /**
     * @description: 获取详情
     */
    QSharedPointer<Row> detail(uint id);

    bool calculated(uint id);

    bool waitingForScan(uint id, QString message = "");

    bool waitingForSend(uint id);

    bool sent(uint id, QString message = "");

    bool finish(uint id);

    bool beginPrint(uint id);

    bool sentToPrinter(uint id);

    bool finishPrint(uint id);

    // 完成了扫码
    bool panelScanned(QString upi);

    static PackBLL *getInstance(QObject *parent = nullptr);

signals:

private:
    void refreshLastID();
    bool updateStatus(uint id, PackBLL::StatusEnum status, QString message = "");

private:
    static PackBLL *m_packBll;

    int newId = 0;

    TableDAL dal;

    QString tableName = "pack";

    QStringList dbColumnNames;
};

#endif // PACKBLL_H
