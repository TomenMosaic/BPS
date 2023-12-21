#include "translationmanage.h"
#include "realtimelabel.h"
#include "Global.h"
#include "log.h"

#include <QLocale>
#include <QDebug>
#include <QJsonArray>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QStandardItemModel>
#include <QToolButton>

TranslationManage *TranslationManage::pTrans = nullptr;

QString TranslationManage::getTranslation(RealDataType dataType)
{
    switch(dataType)
    {
    case RealDataType::temperateType:
        return this->getTranslation(12001);
        break;
    case RealDataType::transmittance:
        return this->getTranslation(12000);
        break;
    case RealDataType::humidType:
        return this->getTranslation(12002);
        break;
    case RealDataType::flowType:
        return this->getTranslation(12003);
        break;
    case RealDataType::voltage:
        return this->getTranslation(12004);
        break;
    case RealDataType::sensorTemperature:
        return this->getTranslation(12005);
        break;

    }
    return "";
}

QString TranslationManage::getUnit(RealDataType dataType)
{
    //    switch(dataType)
    //    {
    //    case RealDataType::temperate:
    //        return this->getTranslation(10184);
    //        break;
    //    case RealDataType::transmittance:
    //        return this->getTranslation(12052);
    //        break;
    //    case RealDataType::upPressSure:
    //        return this->getTranslation(12050);
    //        break;
    //    case RealDataType::downPressSure:
    //        return this->getTranslation(12051);
    //        break;
    //    case RealDataType::pressureVariation:
    //        return this->getTranslation(12051);
    //        break;
    //    case RealDataType::environmentTemprate:
    //        return this->getTranslation(10184);
    //        break;
    //    case RealDataType::coolantState:
    //        return "";
    //        break;
    //    }
    return "";
}

QString TranslationManage::getTranslation(FlowType flowType)
{
    if (flowType==FlowType::unStart)
    {
        return this->getTranslation(70034);
    }
    else if(flowType == FlowType::stopTest)
    {
        return this->getTranslation(70025);
    }
    else
    {
        return this->getTranslation(70013+flowType-FlowType::temperateControl);
    }
}

QString TranslationManage::getTranslation(Cavity cavity)
{

    switch (cavity) {
    case cavityA:
        return g_trans->getTranslation(12006);
        break;
    case cavityB:
        return g_trans->getTranslation(12007);
        break;
    case cavityC:
        return g_trans->getTranslation(12008);
        break;
    case Environment:
        return g_trans->getTranslation(12009);
    default:
        return "";
    }

    return "";
}

QString TranslationManage::getTranslation(SampleTypes sampleType)
{
    switch (sampleType)
    {
    case thinFilm:
        return g_trans->getTranslation(12068);
        break;
    case packagingMaterial:
        return g_trans->getTranslation(12069);
        break;
    default:
        return "";
    }
}

bool TranslationManage::comtain(LanguageType type, QLocale locale)
{
    if(type==Chinese&&locale.language()==QLocale::Chinese)
    {
        return true;
    }
    else if(type==English&&locale.language()==QLocale::English)
    {
        return true;
    }
    return false;
}

TranslationManage *TranslationManage::getInstance(QObject *parent)
{
    if(pTrans == nullptr)
    {
        TranslationManage::pTrans = new TranslationManage(parent);
    }
    return TranslationManage::pTrans;
}

TranslationManage::TranslationManage(QObject *parent)
    : QObject{parent}
{
    init();
}

TranslationManage::LanguageType TranslationManage::getLanguage() const
{
    return language;
}

QLocale TranslationManage::getLanguageStr(LanguageType type)
{
    if(type==Chinese)
    {
        QLocale locale(QLocale::Chinese);
        return locale;
    }
    else
    {
        QLocale locale(QLocale::English);
        return locale;
    }
}

QString TranslationManage::languagefromlocale(QLocale locale)
{
    if(locale.language()==QLocale::Chinese)
        return "Chinese";
    else
        return "English";
}

void TranslationManage::setLanguage(QLocale newLanguage)
{
    switch(newLanguage.language())
    {
    case QLocale::Chinese:
        language = Chinese;
        break;
    case QLocale::English:
        language = English;
        break;
    default:
        language = Chinese;
        break;
    }
}

void TranslationManage::setLanguageType(LanguageType newLanguage)
{
    language = newLanguage;
}

QObjectList TranslationManage::getChildList(QObject *obj)
{
    QObjectList objList = obj->children();
    for(int index = 0;index<objList.length();index++)
    {
        QObject *child = objList.at(index);
        QObjectList childList = getChildList(child);
        for(int childIndex = 0;childIndex<childList.length();childIndex++)
        {
            if(!objList.contains(childList.at(childIndex)))
                objList.append(childList.at(childIndex));
        }
    }
    return objList;
}

void TranslationManage::init()
{
    QJsonObject obj = g_db->execQuery("select * from ResStr");
    QJsonArray values = obj.value("value").toArray();
    QJsonArray fields = obj.value("field").toArray();
    int idIndex;
    QList<int>valList;
    for(int index = 0;index<fields.count();index++)
    {
        QString fieldName = fields.at(index).toString();
        if(fieldName=="id")
        {
            idIndex = index;
        }
        else
        {
            valList.append(index);
        }
    }
#ifdef Q_OS_MACOS
    PNGPATHLOCALE  = QApplication::applicationDirPath()+"/../config";
#endif
    for(int index = 0; index<values.count(); index++)
    {
        QJsonArray oneArr = values.at(index).toArray();
        int id = oneArr.at(idIndex).toInt();
        QStringList valueList;
        for(int valIndex = 0; valIndex<valList.length(); valIndex++)
        {
            int valId = valList.at(valIndex);
            QString value = oneArr.at(valId).toString();
#ifdef Q_OS_MACOS
            value = value.replace("./config",PNGPATHLOCALE);
#endif
            valueList.append(value);
        }
        mapList.insert(id,valueList);
    }

    if(mapList.isEmpty())
    {
        CLOG_ERROR("翻译模块为空，请检查数据库是否完善！");
    }
}

QStringList TranslationManage::getTranslations(QList<int> idList)
{
    QStringList val;
    for(int index = 0;index<idList.length();index++)
    {
        QStringList valueList = mapList.value(idList.at(index));
        if(valueList.isEmpty()||valueList.length()<language)
        {
            val.append("");
        }
        else
        {
            val.append(valueList.at(language).trimmed());
        }
    }
    return val;
}

QString TranslationManage::getTranslation(int id)
{
    if(!mapList.contains(id))
    {
        CLOG_WARNING((QString("翻译%1失败：当前ID不存在，返回空翻译").arg(id)).toUtf8());
        return "";
    }
    QStringList valueList = mapList.value(id);

    if(valueList.isEmpty()&&valueList.length()<=language)
    {
        if(valueList.isEmpty())
            CLOG_WARNING((QString("翻译%1失败：当前ID不存在，返回空翻译").arg(id)).toUtf8());
        else
            CLOG_WARNING((QString("翻译%1失败：未设置当前语言，返回空翻译").arg(id)).toUtf8());
        return "";
    }
    else
    {
        return valueList.at(language).trimmed();
    }
}

QIcon TranslationManage::getTranslationPic(int id)
{
    QString iconPath = getTranslation(id);
    QIcon icon(iconPath);
    if(icon.isNull())
    {
        CLOG_WARNING(QString("获取%1图片失败：当前图片路径不可用").toUtf8());
    }
    return icon;
}

void TranslationManage::autoTransLate(QObject *parent)
{
    QObjectList childList = getChildList(parent);
    childList.append(parent);
    for(int index = 0;index<childList.length();index++)
    {
        QObject *child = childList.at(index);
        QString childName = child->objectName();

        if(childName.contains("Tran"))
        {
            QString className = child->metaObject()->className();
            QStringList nameList =  childName.split("-");
            int  objID = nameList.last().toInt();
            if(className == QStringLiteral("QLabel"))
            {
                QLabel *edit = static_cast<QLabel*>(child);
                edit->setText(getTranslation(objID));
            }
            else if(className == QStringLiteral("QPushButton"))
            {
                QPushButton *edit = static_cast<QPushButton*>(child);
                edit->setText(getTranslation(objID));
            }
            else if(className ==QStringLiteral("QComboBox"))
            {
                QComboBox *comboBox = static_cast<QComboBox*>(child);
                int curIndex = comboBox->currentIndex();
                QString val = getTranslation(objID);
                QStringList valList = val.split("&");
                comboBox->clear();
                comboBox->addItems(valList);
                comboBox->setCurrentIndex(valList.length()>curIndex?curIndex:0);
            }
            else if(className == QStringLiteral("RealTimeLabel"))
            {
                RealTimeLabel *real = static_cast<RealTimeLabel*>(child);
                real->setName(getTranslation(objID));
            }
            else if(className == QStringLiteral("QStandardItemModel"))
            {
                QStandardItemModel *model = static_cast<QStandardItemModel*>(child);
                int colCount = model->columnCount();
                for(int index =0;index<colCount;index++)
                {
                    QStandardItem *headerItem = model->horizontalHeaderItem(index);
                    if(headerItem)
                    {
                        QStringList headerList = headerItem->whatsThis().split("-");
                        if(headerList.contains("Tran"))
                        {
                            int headerID = headerList.last().toInt();
                            headerItem->setText(pTrans->getTranslation(headerID));
                        }
                    }
                }
            }
            else if(className == QStringLiteral("QTabWidget"))
            {
                QTabWidget* tabWidget = static_cast<QTabWidget*>(child);
                QString val = getTranslation(objID);
                QStringList valList = val.split("&");
                int count = tabWidget->count();
                for(int index = 0;index<valList.length()&&index<count;index++)
                {
                    tabWidget->setTabText(index,valList.at(index));
                }
            }
            else if(className == QStringLiteral("QTabBar"))
            {
                QTabBar *tabBar = static_cast<QTabBar*>(child);
                for(int index = 0;index<tabBar->count();index++)
                {
                    QStringList headerList = tabBar->tabWhatsThis(index).split("-");
                    if(headerList.contains("Tran"))
                    {
                        int headerID = headerList.last().toInt();
                        tabBar->setTabText(index,getTranslation(headerID));
                    }
                    else if(headerList.contains("Icon"))
                    {
                        int offID = 0;
                        int onID = 0;
                        if(headerList.length()==3)
                        {
                            offID = headerList[2].toInt();
                            onID = headerList[1].toInt();
                        }
                        else if(headerList.length()==2)
                        {
                            offID = headerList.last().toInt();
                            onID = headerList.last().toInt();
                        }
                        QIcon icon;
                        QString iconPath = getTranslation(offID);
                        icon.addFile(iconPath,QSize(), QIcon::Normal, QIcon::On);
                        iconPath = getTranslation(onID);
                        icon.addFile(iconPath,QSize(), QIcon::Normal, QIcon::Off);
                        tabBar->setTabIcon(index,icon);
                    }
                }

            }
        }
    }
}
