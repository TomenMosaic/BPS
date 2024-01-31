#ifndef GLOBAL_H
#define GLOBAL_H

#include "configmanage.h"
#include "log.h"
#include "core/database.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

constexpr auto DEFAULT_SETTING = "/.config/setting.ini";
constexpr auto DEFAULT_DATABASE = "/.config/packdata";
constexpr auto DEFAULT_DATABASE_PASSWORD = "Pps@666!";

// 工作模式
enum WorkModeEnum
{
    // 扫描包裹
    pack_scan = 0, 
    // 扫描板件
    board_scan = 1,
    // 通过socket获取数据
    socat = 2
};

// 标签打印机类型
enum LabelPrinterTypeEnum{
  // 斑马
    Zebra = 0,
    // 爱普生
    Epson = 1
    // TSC
    // TSC = 2
};

// socket 服务的类型
enum SocketServerTypeEnum
{
    // tcp
    tcp = 0,
    // web socket
    web = 1 ,
};

// group: Printer
struct PrinterConfig {
    // 是否启用打印机
    bool isEnable = false;
    // 打印机类型
    LabelPrinterTypeEnum printerType = LabelPrinterTypeEnum::Zebra;
    // 打印机ip
    QString printerIp;
    // 打印机端口
    int printerPort;
};

// group: Backup
struct BackupConfig {
    // 是否开启备份
    bool isEnable = false;
    // 备份数据库host
    QString dbHost;
    // 备份数据库用户名
    QString dbUser;
    // 备份数据库密码
    QString dbPassword;
    // 备份目录
    QString backupDir;
};

// group: PackTemplate
struct PackTemplateConfig {
    // 是否按照顺序选择箱型
    bool isSort = false;
    // 按照利用率优先选择箱型
    bool isUtilization = false;
    // 默认箱型
    QString defaultTemplate = "4160000";
};

/*
// 获取包裹数据后的等待条件
struct WaitingCondition{
    QString name;
    // 条件
    QString condition;
    // scan 等待扫码
    // measure 测量
    QString action;
};

// 阈值
struct Threshold{

    QString name;

    QString condition;

    int length = 0;
    QString lengthExpression;

    int width = 0;
    QString widthExpression;

    int height = 0;
    QString heightExpression;

    QString packTemplate;
};
*/

// group: Device
struct DeviceConfig {
    // 设备类型
    QString deviceType = "350A";
    // 设备编号
    QString deviceNo;
    // 设备名称
    QString deviceName;
    // 设备ip
    QString deviceIp;
    // 设备端口
    int devicePort = 6969;
    // 目标设备的热文件夹
    QString importDir = "E:\\Dropfolder";

    // 单层可超出的长度
    int maxLengthExceed = 50;
    // 单层可超出的宽度
    int maxWidthExceed = 50;
    // 窄条的最大的宽度
    int maxWidth4Strip = 100;
};

// group: Work
struct WorkConfig {
    // 工作模式
    WorkModeEnum workMode = WorkModeEnum::socat;
    // 是否等待扫码，再一些特殊的条件下，需要等待扫码才可以继续处理
    bool isWaiting4Scan = false;
    // 是否使用正则表达式
    bool isUseRegex = true;
    // 解析条码的正则表达式
    QString codeRegex = "{Length}x{Width}x{Height}";

    // 是否是 socket 服务器
    bool isSocketServer = true;
    // socket 服务器的类型
    SocketServerTypeEnum socketServerType = SocketServerTypeEnum::web;
    // socket 端口
    int socketPort = 6061;
};

// group: MeasuringStation
struct MeasuringStationConfig{
    bool isOpen = false;
    QString modbusTcpClientIp;
    uint modbusTcpClientPort = 502;
    // 扫码入口列表
    QList<QString> scanEntries;
};

/**
 * @brief ColumnMapping 结构体用于描述Excel文件中的列映射。
 */
struct ColumnMapping {
    int columnNumber;       ///< Excel中的列号
    QString propertyName;   ///< 对应的属性名称
    QString dataType;       ///< 数据类型（如"Int", "String"等）
};

/**
 * @brief 预分包的的配置
 */
class YFBConfig {
public:
    /**
     * @brief 构造函数，初始化默认导入映射。
     */
    YFBConfig() {
        // 设置默认的importMappingsCache
        m_importMappingsCache = {
            {1, "no", "String"},
            {2, "name", "String"},
            {3, "remark", "String"},
            {4, "orderNo", "String"},
            {5, "customerName", "String"},
            {6, "location", "String"},
            {7, "length", "Int"},
            {8, "width", "Int"},
            {9, "height", "Int"},
            {11, "sculpt", "String"},
        };
    }

    bool isOpen = true;           ///< 是否开启预分包功能
    QString importTemplate;       ///< 导入数据的JSON模板字符串
    QString exportTemplate;       ///< 导出数据的JSON模板字符串

    /**
     * @brief 设置导入或导出模板字符串，并更新缓存。
     * @param templateStr 模板的JSON字符串。
     * @param isImport 如果为true，则设置导入模板；否则，设置导出模板。
     */
    void setTemplate(const QString &templateStr, bool isImport) {
        if (isImport) {
            importTemplate = templateStr;
            m_importMappingsCache = parseTemplate(templateStr);
        } else {
            exportTemplate = templateStr;
            m_exportMappingsCache = parseTemplate(templateStr);
        }
    }

    /**
     * @brief 获取导入映射列表。
     * @return 返回导入映射的列表。
     */
    QList<ColumnMapping> getImportMappings() const {
        return m_importMappingsCache;
    }

    /**
     * @brief 获取导出映射列表。
     * @return 返回导出映射的列表。
     */
    QList<ColumnMapping> getExportMappings() const {
        return m_exportMappingsCache;
    }

private:
    /**
     * @brief 解析JSON模板字符串为映射列表。
     * @param templateStr 模板的JSON字符串。
     * @return 返回解析后的映射列表。
     */
    QList<ColumnMapping> parseTemplate(const QString &templateStr) {
        QList<ColumnMapping> mappings;
        QJsonDocument doc = QJsonDocument::fromJson(templateStr.toUtf8());
        QJsonArray jsonArray = doc.array();

        for (const QJsonValue &value : jsonArray) {
            QJsonObject jsonObj = value.toObject();
            ColumnMapping mapping;
            mapping.columnNumber = jsonObj["columnNumber"].toInt();
            mapping.propertyName = jsonObj["propertyName"].toString();
            mapping.dataType = jsonObj["dataType"].toString();
            mappings.append(mapping);
        }
        return mappings;
    }

    /**
     * @brief 将映射列表序列化为JSON模板字符串。
     * @param mappings 映射列表。
     * @return 返回序列化后的JSON模板字符串。
     */
    QString serializeTemplate(const QList<ColumnMapping> &mappings) {
        QJsonArray jsonArray;
        for (const ColumnMapping &mapping : mappings) {
            QJsonObject jsonObj;
            jsonObj["columnNumber"] = mapping.columnNumber;
            jsonObj["propertyName"] = mapping.propertyName;
            jsonObj["dataType"] = mapping.dataType;
            jsonArray.append(jsonObj);
        }
        QJsonDocument doc(jsonArray);
        return QString(doc.toJson(QJsonDocument::Compact));
    }

    // 缓存
    QList<ColumnMapping> m_importMappingsCache;
    QList<ColumnMapping> m_exportMappingsCache;
};

// group：Clean
struct CleanConfig {
    // 是否启用清洗
    bool isEnable = false;
    // 清理开始时间
    QString startTime;
    // 数据保留时长
    int keepDays;
    // 至少保留数据量
    int keepCount;
};

class AppConfig {
private:
    PrinterConfig m_printerConfig;
    WorkConfig m_workConfig;
    PackTemplateConfig m_packTemplateConfig;
    BackupConfig m_backupConfig;
    DeviceConfig m_deviceConfig;
    CleanConfig m_cleanConfig;
    YFBConfig m_prePackConfig;
    MeasuringStationConfig m_measuringStationConfig;

    //
    QSettings *m_globalSettings;
private:
    // 默认配置
    void setDefaultConfig();
public:
    AppConfig();
    ~AppConfig();

    // 从ini文件中加载配置
    void loadFromIni(QString &filePath);

    // 保存
    void save();

    // 获取打印机配置
    PrinterConfig getPrinterConfig();
    // 获取工作配置
    WorkConfig getWorkConfig();
    // 获取备份配置
    BackupConfig getBackupConfig();
    // 获取设备配置
    DeviceConfig getDeviceConfig();
    // 获取清洗配置
    CleanConfig getCleanConfig();
    // 获取箱型配置
    PackTemplateConfig getPackTemplateConfig();
    // 获取预分包的配置
    YFBConfig getPrePackConfig();

    MeasuringStationConfig getMeasuringStationConfig();

    // 设置打印机配置
    void setPrinterConfig(PrinterConfig printerConfig);
    // 设置工作配置
    void setWorkConfig(WorkConfig workConfig);
    // 设置备份配置
    void setBackupConfig(BackupConfig backupConfig);
    // 设置设备配置
    void setDeviceConfig(DeviceConfig deviceConfig);
    // 设置清洗配置
    void setCleanConfig(CleanConfig cleanConfig);
    // 设置箱型配置
    void setPackTemplateConfig(PackTemplateConfig stockBinConfig);
    // 设置预分包的配置
    void setPrePackConfig(YFBConfig yfbConfig);

    void setMeasuringStationConfig(MeasuringStationConfig msConfig);

};

// 全局变量 - 配置
extern AppConfig *g_config;
// 全局变量 - 数据库
extern DataBase *g_db;

#endif // GLOBAL_H
