#include "configmanage.h"
#include<QDebug>

//ConfigManage *ConfigManage::pConfig = nullptr;
//ConfigManage::AutoReleaseClass ConfigManage::autoRelease;

//ConfigManage *ConfigManage::getInstance(QString path)
//{
//    if(pConfig == nullptr)
//    {
//        ConfigManage::pConfig = new ConfigManage(path);
//    }
//    return ConfigManage::pConfig;
//}

ConfigManage::ConfigManage(QString settingPath, QObject *parent)
    : QObject{parent}
{
    if(setting==nullptr)
    {
        setting = new QSettings(settingPath,QSettings::Format::IniFormat);
    }
    else
    {
        delete setting;
        setting = new QSettings(settingPath,QSettings::Format::IniFormat);
    }
}

QSettings *ConfigManage::getSetting() const
{
    return setting;
}

bool ConfigManage::contains(QString key)
{
    return setting->contains(key);
}

void ConfigManage::removeKey(QString key)
{
    setting->remove(key);
}

void ConfigManage::addRecord(QString key, QVariant value)
{ 
    setting->setValue(key,value);//设置值
    setting->sync();//立即同步
}

QVariant ConfigManage::getRecord(QString key)
{
    QVariant variant = setting->value(key);
    return variant;
}

QStringList ConfigManage::keylist()
{
    return setting->allKeys();
}

QStringList ConfigManage::grouplist()
{
    return setting->childGroups();
}

void ConfigManage::setPath(QString path)
{
    if(setting==nullptr)
    {
        setting = new QSettings(path,QSettings::Format::IniFormat);
        currentPath= path;
    }
    else
    {
        if(currentPath!=path)
        {
            delete setting;
            setting = new QSettings(path,QSettings::Format::IniFormat);
            currentPath = path;
        }
    }
}

ConfigManage::~ConfigManage()
{
    if(setting)
    {
        delete setting;
        setting = nullptr;
    }
}

