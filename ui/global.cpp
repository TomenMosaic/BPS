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

    // 工作模式
    m_globalSettings->beginGroup("Work");
    m_workConfig.workMode = static_cast<WorkModeEnum>(m_globalSettings->value("workMode").toInt());
    m_workConfig.isWaiting4Scan = m_globalSettings->value("isWaiting4Scan").toBool();
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
   /* Threshold threshold;
    threshold.name = "围边(非窄条)";
    threshold.condition = "{LayerCount}>1 && {PackageWidth}>100 && '{PanelNames}'.indexOf('门板') !== -1";
    threshold.length = 40;
    threshold.width = 40;
    m_deviceConfig.thresholds.append(threshold);

    Threshold threshold2;
    threshold2.name = "柜体长高+5mm. 宽度+2mm";
    threshold2.condition = "{PackageWidth}>100 && '{PanelNames}'.indexOf('柜体') !== -1";
    threshold2.length = 5;
    threshold2.height = 5;
    threshold2.width = 2;
    m_deviceConfig.thresholds.append(threshold2);

    Threshold threshold3;
    threshold3.name = "窄条箱型 & 长+5mm";
    threshold3.condition = "{PackageWidth}<=100";
    threshold3.width = 5;
    threshold3.packTemplate = "4160010";
    m_deviceConfig.thresholds.append(threshold3);

    Threshold threshold4;
    threshold4.name = "17->18";
    threshold4.condition = "0=={PackageHeight}-{LayerCount}*17";
    threshold4.heightExpression = "{LayerCount}";
    m_deviceConfig.thresholds.append(threshold4);

    WaitingCondition waitingCondition;
    waitingCondition.name = "扫码确认阈值";
    waitingCondition.condition = "'{PanelRemarks}'.indexOf('看图') !== -1";
    waitingCondition.action = "scan";
    m_deviceConfig.waitingConditions.append(waitingCondition); */

    /*Threshold threshold2;
    threshold2.name = "围边高度(非窄条)";
    threshold2.condition = "{LayerCount}>1 && {PackageWidth}>100 && '{PanelNames}'.indexOf('门板') !== -1 && {PackageHeight} >= 36";
    threshold2.heightExpression = "{PackageHeight} <= 80 ? (Math.ceil({PackageHeight} / 20) * 21) - {PackageHeight} : 0";
    m_deviceConfig.thresholds.append(threshold2);*/

    m_deviceConfig.maxWidth4Strip = m_globalSettings->value("maxWidth4Strip").toInt();
    m_deviceConfig.maxLengthExceed = m_globalSettings->value("maxLengthExceed").toInt();
    m_deviceConfig.maxWidthExceed = m_globalSettings->value("maxWidthExceed").toInt();
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

    // 预分包
    m_globalSettings->beginGroup("YFB");
    m_prePackConfig.isOpen = m_globalSettings->value("isOpen").toBool();
    if ( m_prePackConfig.isOpen){
        m_prePackConfig.setTemplate(m_globalSettings->value("importTemplate").toString(), true);
        m_prePackConfig.setTemplate(m_globalSettings->value("exportTemplate").toString(), false);
    }
    m_globalSettings->endGroup();

    // 测量站
    m_globalSettings->beginGroup("MeasuringStation");
    this->m_measuringStationConfig.isOpen =  m_globalSettings->value("isOpen").toBool();
    if (this->m_measuringStationConfig.isOpen){
        m_measuringStationConfig.modbusTcpClientIp = m_globalSettings->value("modbusTcpClientIp").toString();
        m_measuringStationConfig.modbusTcpClientPort = m_globalSettings->value("modbusTcpClientPort").toUInt();
        m_measuringStationConfig.scanEntries = m_globalSettings->value("scanEntries").toString().split(",");
    }
    m_globalSettings->endGroup();
}

void AppConfig::save()
{
    // 工作模式
    m_globalSettings->beginGroup("Work");
    m_globalSettings->setValue("workMode", m_workConfig.workMode);
    m_globalSettings->setValue("isWaiting4Scan", m_workConfig.isWaiting4Scan);
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
    m_globalSettings->setValue("maxWidth4Strip", m_deviceConfig.maxWidth4Strip);
    m_globalSettings->setValue("maxLengthExceed", m_deviceConfig.maxLengthExceed);
    m_globalSettings->setValue("maxWidthExceed", m_deviceConfig.maxWidthExceed);
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

    // 预分包
    m_globalSettings->beginGroup("YFB");
    m_globalSettings->setValue("isSort", m_prePackConfig.isOpen);
    m_globalSettings->setValue("importTemplate", m_prePackConfig.importTemplate);
    m_globalSettings->setValue("exportTemplate", m_prePackConfig.exportTemplate);
    m_globalSettings->endGroup();

    // 测量站
    m_globalSettings->beginGroup("MeasuringStation");
    m_globalSettings->setValue("isOpen", this->m_measuringStationConfig.isOpen);
    if (this->m_measuringStationConfig.isOpen){
        m_globalSettings->setValue("modbusTcpClientIp", m_measuringStationConfig.modbusTcpClientIp);
        m_globalSettings->setValue("modbusTcpClientPort", m_measuringStationConfig.modbusTcpClientPort);
        m_globalSettings->setValue("scanEntries", m_measuringStationConfig.scanEntries.join(","));
    }
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

YFBConfig AppConfig::getPrePackConfig(){
    return m_prePackConfig;
}

MeasuringStationConfig AppConfig::getMeasuringStationConfig(){
    return m_measuringStationConfig;
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
    if (m_workConfig.isWaiting4Scan != workConfig.isWaiting4Scan){
        m_workConfig.isWaiting4Scan = workConfig.isWaiting4Scan;
        hasChanged = true;
    }
    if (hasChanged){
        save();
    }
}
void  AppConfig::setBackupConfig(BackupConfig backupConfig){}
void  AppConfig::setDeviceConfig(DeviceConfig deviceConfig){
    bool hasChanged = false;
    if (!deviceConfig.importDir.isEmpty() && m_deviceConfig.importDir != deviceConfig.importDir){
        m_deviceConfig.importDir = deviceConfig.importDir;
        hasChanged = true;
    }

    if (hasChanged){
        save();
    }
}
void  AppConfig::setCleanConfig(CleanConfig cleanConfig){}
void  AppConfig::setPackTemplateConfig(PackTemplateConfig packTemplateConfig){}

void AppConfig::setPrePackConfig(YFBConfig yfbConfig){
    bool hasChanged = false;

    // 检查并更新 isOpen
    if (this->m_prePackConfig.isOpen != yfbConfig.isOpen) {
        this->m_prePackConfig.isOpen = yfbConfig.isOpen;
        hasChanged = true;
    }

    // 如果配置是开启状态，检查模板是否更改
    if (yfbConfig.isOpen) {
        if (this->m_prePackConfig.importTemplate != yfbConfig.importTemplate) {
            this->m_prePackConfig.importTemplate = yfbConfig.importTemplate;
            hasChanged = true;
        }
        if (this->m_prePackConfig.exportTemplate != yfbConfig.exportTemplate) {
            this->m_prePackConfig.exportTemplate = yfbConfig.exportTemplate;
            hasChanged = true;
        }
    }

    // 如果有更改，则保存配置
    if (hasChanged) {
        save();
    }
}

void AppConfig::setMeasuringStationConfig(MeasuringStationConfig msConfig){
    bool hasChanged = false;
    if (msConfig.isOpen != this->m_measuringStationConfig.isOpen){
        this->m_measuringStationConfig.isOpen = msConfig.isOpen;
        hasChanged = true;
    }

    if (msConfig.isOpen){
        if (msConfig.modbusTcpClientIp != this->m_measuringStationConfig.modbusTcpClientIp){
            this->m_measuringStationConfig.modbusTcpClientIp = msConfig.modbusTcpClientIp;
            hasChanged = true;
        }
        if (msConfig.modbusTcpClientPort != this->m_measuringStationConfig.modbusTcpClientPort){
            this->m_measuringStationConfig.modbusTcpClientPort = msConfig.modbusTcpClientPort;
            hasChanged = true;
        }
        if (msConfig.scanEntries.size() != this->m_measuringStationConfig.scanEntries.size()){
            this->m_measuringStationConfig.scanEntries = msConfig.scanEntries;
            hasChanged = true;
        }else{
            for (int i = 0 ; i < msConfig.scanEntries.size(); i++){
                if (this->m_measuringStationConfig.scanEntries[i] != msConfig.scanEntries[i]){
                    this->m_measuringStationConfig.scanEntries[i] = msConfig.scanEntries[i];
                    hasChanged = true;
                }
            }
        }
    }

    if (hasChanged){
        save();
    }
}








