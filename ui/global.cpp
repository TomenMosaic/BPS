#include "global.h"

#include <QApplication>
#include <QFileInfo>

AppConfig *g_config;
DataBase *g_db;

// AppConfig.cpp or a corresponding implementation file
AppConfig::AppConfig()
{
    // Constructor implementation
}

AppConfig::~AppConfig()
{
    // Destructor implementation
}

void AppConfig::loadFromIni(QString &filePath)
{
    if (filePath.isEmpty()){
        filePath = QApplication::applicationDirPath() + DEFAULT_SETTING; // 配置文件路径
    }
    m_globalSettings = new QSettings(filePath, QSettings::IniFormat);

    // 检查配置文件是否存在
    QFileInfo checkFile(filePath);
    if (!checkFile.exists() || !checkFile.isFile())
    {
        save();
        return;
    }

    // 从文件中读取配置
    m_globalSettings->beginGroup("Work");
    m_workConfig.workMode = static_cast<WorkModeEnum>(m_globalSettings->value("workMode").toInt());
    m_workConfig.isUseRegex = m_globalSettings->value("isUseRegex").toBool();
    if (m_workConfig.isUseRegex && !m_workConfig.codeRegex.isEmpty()){
        m_workConfig.codeRegex = m_globalSettings->value("codeRegex").toString();
    }
    m_workConfig.isSocketServer = m_globalSettings->value("isSocketServer").toBool();
    if (m_workConfig.isSocketServer){
         m_workConfig.socketServerType = static_cast<SocketServerTypeEnum>(m_globalSettings->value("socketServerType").toInt());
         m_workConfig.socketPort = m_globalSettings->value("socketPort").toInt();
    }
    m_globalSettings->endGroup();

    // 打印机配置
    m_globalSettings->beginGroup("Printer");
    m_printerConfig.isEnable = m_globalSettings->value("isEnable").toBool();
    if (m_printerConfig.isEnable){
        m_printerConfig.printerType = static_cast<LabelPrinterTypeEnum>(m_globalSettings->value("printerType").toInt());
        m_printerConfig.printerIp = m_globalSettings->value("printerIp").toString();
        m_printerConfig.printerPort = m_globalSettings->value("printerPort").toInt();
    }
    m_globalSettings->endGroup();

    // 备份配置
    m_globalSettings->beginGroup("Backup");
    m_backupConfig.isEnable = m_globalSettings->value("isEnable").toBool();
    if (m_backupConfig.isEnable){
        m_backupConfig.backupDir = m_globalSettings->value("backupDir").toString();
        m_backupConfig.dbHost = m_globalSettings->value("dbHost").toString();
        m_backupConfig.dbUser = m_globalSettings->value("dbUser").toString();
        m_backupConfig.dbPassword = m_globalSettings->value("dbPassword").toString();
    }
    m_globalSettings->endGroup();

    // 设备配置
    m_globalSettings->beginGroup("Device");
    m_deviceConfig.deviceType = m_globalSettings->value("deviceType").toString();
    m_deviceConfig.deviceNo = m_globalSettings->value("deviceNo").toString();
    m_deviceConfig.deviceName = m_globalSettings->value("deviceName").toString();
    m_deviceConfig.deviceIp = m_globalSettings->value("deviceIp").toString();
    m_deviceConfig.devicePort = m_globalSettings->value("devicePort").toInt();
    m_deviceConfig.importDir = m_globalSettings->value("importDir").toString();
    Threshold threshold;
    threshold.name = "围边(非窄条)";
    threshold.condition = "{LayerCount}>1 && {PackageWidth}>100";
    threshold.length = 36;
    threshold.width = 36;
    m_deviceConfig.thresholds.append(threshold);

    Threshold threshold2;
    threshold2.name = "围边高度(非窄条)";
    threshold2.condition = "{LayerCount}>1 && {PackageHeight} >= 36 && {PackageWidth}>100";
    threshold2.heightExpression = "{PackageHeight} <= 80 ? (Math.ceil({PackageHeight} / 20) * 21) - {PackageHeight} : 0";
    m_deviceConfig.thresholds.append(threshold2);

    Threshold threshold3;
    threshold3.name = "窄条箱型";
    threshold3.condition = "{PackageWidth}<=100";
    threshold3.packTemplate = "4160010";
    m_deviceConfig.thresholds.append(threshold3);

    m_globalSettings->endGroup();

    // 清理配置
    m_globalSettings->beginGroup("Clean");
    m_cleanConfig.isEnable = m_globalSettings->value("isEnable").toBool();
    if (m_cleanConfig.isEnable){
        m_cleanConfig.startTime = m_globalSettings->value("startTime").toString();
        m_cleanConfig.keepDays = m_globalSettings->value("keepDays").toInt();
        m_cleanConfig.keepCount = m_globalSettings->value("keepCount").toInt();
    }
    m_globalSettings->endGroup();

    // 箱型配置
    m_globalSettings->beginGroup("PackTemplate");
    m_packTemplateConfig.isSort = m_globalSettings->value("isSort").toBool();
    m_packTemplateConfig.isUtilization = m_globalSettings->value("isUtilization").toBool();
    m_packTemplateConfig.defaultTemplate = m_globalSettings->value("defaultTemplate").toString();
    m_globalSettings->endGroup();
}

void AppConfig::save()
{
    // 工作模式
    m_globalSettings->beginGroup("Work");
    m_globalSettings->setValue("workMode", m_workConfig.workMode);
    m_globalSettings->setValue("isUseRegex", m_workConfig.isUseRegex);
    if (m_workConfig.isUseRegex && !m_workConfig.codeRegex.isEmpty()){
        m_globalSettings->setValue("codeRegex", m_workConfig.codeRegex);
    }else{
        m_globalSettings->remove("codeRegex");
    }
    m_globalSettings->setValue("isSocketServer", m_workConfig.isSocketServer);
    if (m_workConfig.isSocketServer){
        m_globalSettings->setValue("socketServerType", m_workConfig.socketServerType);
        m_globalSettings->setValue("socketPort", m_workConfig.socketPort);
    }else{
        m_globalSettings->remove("socketServerType");
        m_globalSettings->remove("socketPort");
    }
    m_globalSettings->endGroup();

    // 打印机配置
    m_globalSettings->beginGroup("Printer");
    m_globalSettings->setValue("isEnable", m_printerConfig.isEnable);
    if (m_printerConfig.isEnable){
        m_globalSettings->setValue("printerType", m_printerConfig.printerType);
        m_globalSettings->setValue("printerIp", m_printerConfig.printerIp);
        m_globalSettings->setValue("printerPort", m_printerConfig.printerPort);
    }else{
        m_globalSettings->remove("printerType");
        m_globalSettings->remove("printerIp");
        m_globalSettings->remove("printerPort");
    }
    m_globalSettings->endGroup();

    // 备份配置
    m_globalSettings->beginGroup("Backup");
    m_globalSettings->setValue("isEnable", m_backupConfig.isEnable);
    if (m_backupConfig.isEnable){
        m_globalSettings->setValue("backupDir", m_backupConfig.backupDir);
        m_globalSettings->setValue("dbHost", m_backupConfig.dbHost);
        m_globalSettings->setValue("dbUser", m_backupConfig.dbUser);
        m_globalSettings->setValue("dbPassword", m_backupConfig.dbPassword);
    }else{
        m_globalSettings->remove("backupDir");
        m_globalSettings->remove("dbHost");
        m_globalSettings->remove("dbUser");
        m_globalSettings->remove("dbPassword");
    }
    m_globalSettings->endGroup();

    // 设备配置
    m_globalSettings->beginGroup("Device");
    m_globalSettings->setValue("deviceType", m_deviceConfig.deviceType);
    m_globalSettings->setValue("deviceNo", m_deviceConfig.deviceNo);
    m_globalSettings->setValue("deviceName", m_deviceConfig.deviceName);
    m_globalSettings->setValue("deviceIp", m_deviceConfig.deviceIp);
    m_globalSettings->setValue("devicePort", m_deviceConfig.devicePort);
    m_globalSettings->setValue("importDir", m_deviceConfig.importDir);
    m_globalSettings->endGroup();

    // 清理配置
    m_globalSettings->beginGroup("Clean");
    m_globalSettings->setValue("isEnable", m_cleanConfig.isEnable);
    if (m_cleanConfig.isEnable){
        m_globalSettings->setValue("startTime", m_cleanConfig.startTime);
        m_globalSettings->setValue("keepDays", m_cleanConfig.keepDays);
        m_globalSettings->setValue("keepCount", m_cleanConfig.keepCount);
    }else{
        m_globalSettings->remove("startTime");
        m_globalSettings->remove("keepDays");
        m_globalSettings->remove("keepCount");
    }
    m_globalSettings->endGroup();

    // 箱型配置
    m_globalSettings->beginGroup("PackTemplate");
    m_globalSettings->setValue("isSort", m_packTemplateConfig.isSort);
    m_globalSettings->setValue("isUtilization", m_packTemplateConfig.isUtilization);
    m_globalSettings->setValue("defaultTemplate", m_packTemplateConfig.defaultTemplate);
    m_globalSettings->endGroup();

    // 确保设置被写入文件
    m_globalSettings->sync();

    if (m_globalSettings->status() != QSettings::NoError) {
        // 有错误发生
        if (m_globalSettings->status() == QSettings::AccessError) { // 处理访问错误
            CLOG_ERROR(QString("写入配置文件失败，访问错误").toUtf8());
        } else if (m_globalSettings->status() == QSettings::FormatError) { // 处理格式错误
            CLOG_ERROR(QString("写入配置文件失败，文件格式错误").toUtf8());
        }

    }
}

PrinterConfig  AppConfig::getPrinterConfig(){
    return m_printerConfig;
}
WorkConfig  AppConfig::getWorkConfig(){
    return m_workConfig;
}
BackupConfig  AppConfig::getBackupConfig(){
    return m_backupConfig;
}
DeviceConfig  AppConfig::getDeviceConfig(){
    return m_deviceConfig;
}
CleanConfig  AppConfig::getCleanConfig(){
    return m_cleanConfig;
}
PackTemplateConfig  AppConfig::getPackTemplateConfig(){
    return m_packTemplateConfig;
}

void  AppConfig::setWorkConfig(WorkConfig workConfig){
    bool hasChanged = false;
    if (m_workConfig.workMode != workConfig.workMode){
        m_workConfig.workMode = workConfig.workMode;
        hasChanged = true;
    }
    if (m_workConfig.isUseRegex != workConfig.isUseRegex){
        m_workConfig.isUseRegex = workConfig.isUseRegex;
        hasChanged = true;
    }
    if (m_workConfig.codeRegex != workConfig.codeRegex){
        m_workConfig.codeRegex = workConfig.codeRegex;
        hasChanged = true;
    }
    if (hasChanged){
        save();
    }
}
void  AppConfig::setBackupConfig(BackupConfig backupConfig){}
void  AppConfig::setDeviceConfig(DeviceConfig deviceConfig){}
void  AppConfig::setCleanConfig(CleanConfig cleanConfig){}
void  AppConfig::setPackTemplateConfig(PackTemplateConfig packTemplateConfig){}


