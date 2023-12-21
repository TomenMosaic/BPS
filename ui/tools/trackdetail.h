#ifndef TRACKDETAIL_H
#define TRACKDETAIL_H

#include <QObject>
#include "entity.h"
#include "intdecorator.h"
#include "stringdecorator.h"
#include "enumeratordecorator.h"
#include "booldecorator.h"
//保存单一的值
//密码：{"初始值"：{},"修改值":""}
class NodeParam;
class ModifyParam;
class BaseParam : public Entity
{
    Q_OBJECT
public:
    BaseParam *create(QObject *parent, const QJsonObject &json)override;
    explicit BaseParam(QObject *parent = nullptr,QString key = "BaseParam");
    BaseParam(QString name,bool trans,QObject *parent= nullptr, QString key = "BaseParam");
    BaseParam(QObject *parent, const QJsonObject &json);
    void setValue(QString nametmp,bool transtmp);
    QString getValue();
    bool isTrans();
    StringDecorator *name{nullptr};
    BoolDecorator *trans{nullptr};
};
//保存最后的键值对
class NodeParam :public BaseParam
{
    Q_OBJECT
public:
    NodeParam (QObject *parent = nullptr,QString key = "NodeParam");
    NodeParam(QString keyName,bool keyTrans,QString valueName,bool valueTrans,QObject *parent = nullptr,QString key = "NodeParam");
    NodeParam(QObject *parent, const QJsonObject &json);
    QString key();
    QString value();
    BaseParam *valueParam{nullptr};
};

class ModifyParam : public BaseParam
{
    Q_OBJECT
public:
    explicit ModifyParam(QObject *parent = nullptr);

    ModifyParam(QString originalVal,bool originalTrans,QString modifyVal,bool modifyTrans,QObject *parent);
    ModifyParam(QString key,bool trans,QString originalVal,bool originalTrans,QString modifyVal,bool modifyTrans,QObject *parent);
    ModifyParam(QObject *parent, const QJsonObject &json);
    QString key();
    QString modify();
    QString original();
    NodeParam *modifyParam{nullptr};
    NodeParam *originalParam{nullptr};
};
class TrackParam : public BaseParam
{
    Q_OBJECT
public:
    explicit TrackParam(QObject *parent = nullptr,QString key = "TrackParam");
    TrackParam(QString name,bool trans,QObject *parent= nullptr, QString key = "TrackParam");
    void update(const QJsonObject &json);
    TrackParam(QObject *parent, const QJsonObject &json);
    void addChild(BaseParam *child);
    EntityCollection<BaseParam>* params{nullptr};
};

//class TrackDetail :public Entity
//{
//    Q_OBJECT
//public:
//    explicit TrackDetail(QObject* parent = nullptr);
//    TrackDetail(QObject *parent , const QJsonObject &json);



//    EnumeratorDecorator *type{nullptr};
//    IntDecorator *operationID{nullptr};
//    EntityCollection<TrackParam>* params{nullptr};
//    EntityCollection<TrackParam>* originalParams{nullptr};

//};

#endif // TRACKDETAIL_H
