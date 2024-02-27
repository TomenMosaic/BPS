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

class PackageDto {
public:
    enum StatusEnum{
        Status_Init = 0,

        // 计算结束
        Status_Step1_Calculated = 109,
        // 等待齐套扫码
        Status_Step1_Waiting4ScanFull = 110,
        // 齐套
        Status_Step1_ScanFull = 119,


        /**
         * @brief 等待容差值的录入
         */
        Status_Step3_Waiting4ScanTolerance = 210,
        /**
         * @brief 容差值已获取
         */
        Status_Step3_GotScanTolerance = 219,


        /**
         * @brief 等待发送包裹标识到测量站，测量站得到标识信息后，对应的板剁才会进入线体
         */
        Status_Step2_Waiting4SendPackNo = 220,
        /**
         * @brief 完成发送包裹条码数据
         */
        Status_Step2_SentPackNo = 229,


        /**
         * @brief 等待测量高度
         */
        Status_Step2_Waiting4MeasuringHeight = 260,
        /**
         * @brief 已经获取到了高度的测量值
         */
        Status_Step2_GotMeasuringHeight = 269,


        /**
         * @brief 等待发送加工数据到裁纸机
         */
        Status_Step4_WaitingForSend = 900,
        /**
         * @brief 已发送
         */
        Status_Step4_Sent = 919,
        /**
         * @brief 结束
         */
        Status_Step4_Finish = 999
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

    PackageDto() = default;

    PackageDto(PackageAO pack, StatusEnum status = PackageDto::StatusEnum::Status_Init) {
        id = pack.id;
        length = pack.length;
        width = pack.width;
        height = pack.height;
        no = pack.no;
        orderNo = pack.orderNo;
        customerName = pack.customerName;
        panelTotal = pack.getPanelTotal();
        layerTotal = pack.layers.size();
        flowNo = pack.flowNo;
        this->status = status;
        panels = pack.getPanels();
    }

    uint id;
    uint length;
    uint width;
    uint height;
    QString templateName;
    StatusEnum status = PackageDto::StatusEnum::Status_Init;
    QString no;
    QString orderNo;
    QString customerName;
    PackTypeEnum type = PackageDto::PackTypeEnum::PackType_Socket;
    uint scanPanelCount;
    uint panelTotal;
    uint layerTotal;
    uint flowNo;
    QString originIp;
    QStringList logs;
    QDateTime sentTime;
    QDateTime createTime;
    QDateTime lastModifyTime;
    QDateTime removedAt;

    QList<Panel> panels;

    /**
     * @description: 状态枚举转换为中文
     */
    static QString statusEnumToString(StatusEnum status){
        switch (status) {
        case Status_Init:
            return QStringLiteral("初始化");
        case Status_Step1_Calculated:
            return QStringLiteral("计算结束");
        case Status_Step1_Waiting4ScanFull:
            return QStringLiteral("等待齐套");
        case Status_Step1_ScanFull:
            return QStringLiteral("已齐套");
        case Status_Step2_Waiting4SendPackNo:
            return QStringLiteral("进板信号待发");
        case Status_Step2_SentPackNo:
            return QStringLiteral("进板信号已发");
        case Status_Step2_Waiting4MeasuringHeight:
            return QStringLiteral("等待测高");
        case Status_Step2_GotMeasuringHeight:
            return QStringLiteral("测高完成");

        case Status_Step3_Waiting4ScanTolerance:
            return QStringLiteral("等待容差扫码");
        case Status_Step3_GotScanTolerance:
            return QStringLiteral("已获取容差");

        case Status_Step4_WaitingForSend:
            return QStringLiteral("等待发送");
        case Status_Step4_Sent:
            return QStringLiteral("已发送");
        case Status_Step4_Finish:
            return QStringLiteral("结束");

        default:
            return QStringLiteral("-");
        }
    }

    QString getScript(QString expression) const {
        QString script = expression;
        if (! this->customerName.isEmpty()){
            script = script.replace("{CustomerName}", this->customerName);
        }
        if (! this->orderNo.isEmpty()){
            script = script.replace("{OrderNo}", this->orderNo);
        }
        script = script.replace("{LayerCount}", QString::number(this->layerTotal)); // 层数
        script = script.replace("{PackageHeight}", QString::number(this->height)); // 包裹高度
        script = script.replace("{PackageWidth}", QString::number(this->width)); // 包裹宽度
        script = script.replace("{PackageLength}", QString::number(this->length)); // 包裹长度

        // 包含板件名称、说明、位置、特殊工艺列表
        QSet<QString> panelNameSet; // name
        QSet<QString> panelRemarkSet; // remark
        QSet<QString> panelLocationSet; // 位置
        QSet<QString> panelSculptSet; // 工艺
        for (const Panel& panel: panels){
            panelNameSet.insert(panel.name);
            panelRemarkSet.insert(panel.remark);
            panelLocationSet.insert(panel.location);
            panelSculptSet.insert(panel.sculpt);
        }

        script = script.replace("{PanelNames}", panelNameSet.toList().join(","));
        script = script.replace("{PanelRemarks}", panelRemarkSet.toList().join(","));
        script = script.replace("{PanelLocations}", panelLocationSet.toList().join(","));
        script = script.replace("{PanelSculpts}", panelSculptSet.toList().join(","));

        return script;
    }

    QString getKey() const {
        QSet<QString> panelLocationSet; // 位置
        for (const Panel& panel: qAsConst(panels)){
            panelLocationSet.insert(panel.location);
        }

        QString result ;
        if (customerName.isEmpty() && panelLocationSet.isEmpty()){
            result = no;
        }else{
            result = QString("%1_%2").arg(customerName, panelLocationSet.toList().join(","));
        }
        return result;
    }

    //
    bool isAllowSendDataToMachine() const{
        return this->status == StatusEnum::Status_Step1_Waiting4ScanFull ||
                this->status == StatusEnum::Status_Step2_Waiting4MeasuringHeight ||
                this->status == StatusEnum::Status_Step3_Waiting4ScanTolerance;
    }

    /**
     * @description: 状态枚举转换为中文
     */
    QString printStatusEnumToString(PrintStatusEnum status);
};

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
        // 层数
        LayerTotal,
        // 所在格（订单中包裹的序号）
        FlowNo,
        // 来源IP
        OriginIp,

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
        {"layer_total", "INTEGER"},
        {"flow_no", "INTEGER"},

        {"origin_ip", "VARCHAR(20)"},

        {"logs", "TEXT"},
        {"sent_at", "DATETIME"},
        {"create_at", "DATETIME"},
        {"update_at", "DATETIME"},
        {"removed_at", "DATETIME"}
    };

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
    QSharedPointer<PackageDto> getPackageByDbId(uint packId);

    bool save(QList<QVariantList> valueList);

    /**
     * @description: 获取分页数据
     */
    QList<QSharedPointer<Row>> getPage();

    /**
     * @description: 插入数据
     */
    int insert(QString no, uint length, uint width, uint height, PackageDto::PackTypeEnum type);

    int insertByPackStruct(const PackageDto& package);

    QVector<int> insertByPackStructs(const QList<PackageDto>& packages);

    /**
     * @description: 更新数据
     */
    bool update(uint packId);

    /**
     * @description: 删除数据
     */
    bool remove(uint packId);

    /**
     * @description: 获取详情
     */
    QSharedPointer<Row> detail(uint packId);

    QSharedPointer<Row> detail(QString no);

    QSharedPointer<PackageDto> detailStruct(QString no);

    bool Step1_Full(uint packId);
//
    bool Step1_Calculated(uint packId);
    // 等待包裹标识数据发送到 拼板工位
    bool Step2_Waiting4PackNo_PanelSockStation(uint packId);
    bool Step2_SentPackNo_PanelSockStation(uint packId);
    bool Step2_SentPackNo_WaitinMeasuringHeight(uint packId);
    bool Step2_GotMeasuringHeight(uint packId, uint height);

    bool Step3_Waiting4ScanToleranceValue(uint packId, QString message = "");
    bool Step3_SentScanToleranceValue(uint packId);

    bool Step4_Waiting4SendWorkData(uint packId);
    bool Step4_SentWorkData(uint packId, QString message = "");

        // bool Step2_Waiting4MeasuringHeight(uint packId);
    //bool Step4_Finish(uint packId);

    bool beginPrint(uint packId);

    bool sentToPrinter(uint packId);

    bool finishPrint(uint packId);

    // 完成了扫码
    bool panelScanned(QString upi);

    QSharedPointer<PackageDto> convertRow2Package(QSharedPointer<Row> packageRow);

    static PackBLL *getInstance(QObject *parent = nullptr);

signals:

private:
    void refreshLastID();
    bool updateStatus(uint packId,
                      PackageDto::StatusEnum status,
                      QString message = "",
                      QMap<QString, QVariant> updates = QMap<QString, QVariant>{});

private:
    static PackBLL *m_packBll;

    int newId = 0;

    TableDAL dal;

    QString tableName = "pack";

    QStringList dbColumnNames;

    int m_pageSize = 26;
};

#endif // PACKBLL_H
