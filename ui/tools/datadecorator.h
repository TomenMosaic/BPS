//data-decorator.h
#ifndef DATADECORATOR_H
#define DATADECORATOR_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QScopedPointer>

class Entity;
class DataDecorator : public QObject
{
    Q_OBJECT
    //导出QML中需要访问属性
    Q_PROPERTY( QString ui_describe READ label CONSTANT )
public:
    /**@brief 构造描述
   * @1 该属性字段属于哪个数据实体
   * @2 该属性字段对应的key值
   * @3 该属性字段对应的描述信息
   */
    DataDecorator(Entity* parent = nullptr, const QString& key = "SomeItemKey", const QString& label = "");
    virtual ~DataDecorator();

    const QString& key() const;   //key值用来查找该属性
    const QString& label() const; //label用来描述该属性字段
    Entity* parentEntity();       //属性字段属于哪个数据实体(比如用户等)

    //Json和数据类之间的相互转化
    virtual QJsonValue jsonValue() const = 0;
    virtual void update(const QJsonObject& jsonObject) = 0;

private:
    //私有的数据类
    class Implementation;
    QScopedPointer<Implementation> implementation;
};
#endif
