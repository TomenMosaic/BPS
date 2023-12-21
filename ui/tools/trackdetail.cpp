#include "trackdetail.h"
#include "Global.h"
#include <QDebug>
TrackParam::TrackParam(QObject *parent, QString key): BaseParam(parent, key)
{
    params = static_cast<EntityCollection<BaseParam>*>(addChildCollection(new EntityCollection<BaseParam>(this, "params")));
    params->clear();
}

TrackParam::TrackParam(QString name, bool trans, QObject *parent, QString key):TrackParam(parent,key)
{
    setValue(name,trans);
}

void TrackParam::update(const QJsonObject &json)
{
    BaseParam::update(json);
    if(!params)
    {
        params = static_cast<EntityCollection<BaseParam>*>(addChildCollection(new EntityCollection<BaseParam>(this, "params")));

    }

    params->clear();
    QJsonArray array = json.value("params").toArray();
    foreach(QJsonValue oneVal,array)
    {
        QJsonObject obj = oneVal.toObject();
        QStringList keys = obj.keys();
        BaseParam *oneParam = nullptr;
        if(keys.contains("params"))
        {
            oneParam = new TrackParam(this,obj);
        }
        else if(keys.contains("modify")&&keys.contains("original"))
        {
            oneParam = new ModifyParam(this,obj);
        }
        else if(keys.contains("value"))
        {
            oneParam = new NodeParam(this,obj);
        }
        else{
            oneParam = new BaseParam(this,obj);
        }

        params->addEntity(oneParam);
    }

}

TrackParam::TrackParam(QObject *parent, const QJsonObject &json):TrackParam(parent)
{
    update(json);
}




void TrackParam::addChild(BaseParam *child)
{
    params->addEntity(child);
}

ModifyParam::ModifyParam(QObject *parent):BaseParam(parent,"ModifyParam")
{
    modifyParam = static_cast<NodeParam*>(addChild(new NodeParam(this), "modify"));
    originalParam = static_cast<NodeParam*>(addChild(new NodeParam(this), "original"));
    originalParam->name->setValue("60014");
    originalParam->trans->setValue(true);
    modifyParam->name->setValue("60015");
    modifyParam->trans->setValue(true);
}

ModifyParam::ModifyParam(QString key, bool trans, QString originalVal, bool originalTrans, QString modifyVal, bool modifyTrans, QObject *parent):ModifyParam(parent)
{
    originalParam->name->setValue(originalVal);
    originalParam->trans->setValue(originalTrans);
    modifyParam->name->setValue(modifyVal);
    modifyParam->trans->setValue(modifyTrans);
    this->setValue(key,trans);
}

ModifyParam::ModifyParam(QObject *parent, const QJsonObject &json):ModifyParam(parent)
{
    update(json);
}

QString ModifyParam::key()
{
    return this->getValue();
}

QString ModifyParam::modify()
{
    return modifyParam->getValue();
}

QString ModifyParam::original()
{
    return originalParam->getValue();
}

BaseParam *BaseParam::create(QObject *parent, const QJsonObject &json)
{
    if(json.contains("params"))
    {
        return new TrackParam(parent,json);
    }
    else if(json.contains("original")&&json.contains("modify"))
    {
        return new ModifyParam(parent,json);
    }
    else if(json.contains("value"))
    {
        return new NodeParam(parent,json);
    }
    else
        return new BaseParam(parent,json);
}

BaseParam::BaseParam(QObject *parent, QString key):Entity(parent,key)
{
    name = static_cast<StringDecorator*>(addDataItem(new StringDecorator(this,"name","名称","")));
    trans = static_cast<BoolDecorator*>(addDataItem(new BoolDecorator(this,"trans","名称")));
}

BaseParam::BaseParam(QString name, bool trans, QObject *parent, QString key):BaseParam(parent,key)
{
    setValue(name,trans);
}

BaseParam::BaseParam(QObject *parent, const QJsonObject &json):BaseParam(parent)
{
    update(json);
}

void BaseParam::setValue(QString nametmp, bool transtmp)
{
    name->setValue(nametmp);
    trans->setValue(transtmp);
}

QString BaseParam::getValue()
{
    if(trans->value())
    {
        return g_trans->getTranslation(name->value().toInt());
    }
    else
    {
        return name->value();
    }
}

bool BaseParam::isTrans()
{
    return trans->value();
}

NodeParam::NodeParam(QObject *parent, QString key):BaseParam(parent,key)
{
    valueParam =  static_cast<BaseParam*>(addChild(new BaseParam(this), "value"));
}

NodeParam::NodeParam(QString keyName, bool keyTrans, QString valueName, bool valueTrans, QObject *parent, QString key):NodeParam(parent,key)
{
    valueParam->setValue(valueName,valueTrans);
    this->setValue(keyName,keyTrans);
}

NodeParam::NodeParam(QObject *parent, const QJsonObject &json):NodeParam(parent)
{
    //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"begin:"<<json;
    update(json);
}

QString NodeParam::key()
{
    return this->getValue();
}

QString NodeParam::value()
{
    return valueParam->getValue();
}
