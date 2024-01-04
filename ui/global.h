#ifndef GLOBAL_H
#define GLOBAL_H

#include "configmanage.h"
#include "log.h"
#include "core/database.h"

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

    // 阈值配置列表
    // group: Thresholds
    /*
    [Thresholds]
    count=2

    [Thresholds/1]
    condition=Condition1
    length=100
    width=200
    height=300

    [Thresholds/2]
    condition=Condition2
    length=400
    width=500
    height=600

    QList<Threshold> thresholds;

    // 等待的条件列表
    QList<WaitingCondition> waitingConditions;*/

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

};

// 全局变量 - 配置
extern AppConfig *g_config;
// 全局变量 - 数据库
extern DataBase *g_db;

#endif // GLOBAL_H
