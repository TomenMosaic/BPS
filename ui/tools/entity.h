//entity.h
#ifndef ENTITY_H
#define ENTITY_H

#include <map>
#include <QObject>
#include <QScopedPointer>
#include <datadecorator.h>
#include <entitycollecionobject.h>

class Entity : public QObject
{
    Q_OBJECT

public:
    Entity(QObject* parent = nullptr, const QString& key = "SomeEntityKey");
    virtual Entity *create(QObject *parent, const QJsonObject &json);

    Entity(QObject* parent, const QString& key, const QJsonObject& jsonObject);
    virtual ~Entity();

public:
    //数据信息体对应的key值
    const QString& key() const;

    //数据信息类和Json结构之间的相互转换
    void update(const QJsonObject& jsonObject);
    QJsonObject toJson() const;

signals:
    //信息体集合发生变化
    void childCollectionsChanged(const QString& collectionKey);
    //包含的信息体发生变化
    void childEntitiesChanged();
    //包含的属性发生变化
    void dataDecoratorsChanged();

protected:
    //添加包含的信信息体
    Entity* addChild(Entity* entity, const QString& key);
    //添加包含的信息实体列表
    EntityCollectionBase* addChildCollection(EntityCollectionBase* entityCollection);
    //添加子属性
    DataDecorator* addDataItem(DataDecorator* dataDecorator);
protected:
    class Implementation;
    QScopedPointer<Implementation> implementation;
};

#endif
