//int-decorator.h
#ifndef INTDECORATOR_H
#define INTDECORATOR_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QScopedPointer>
#include <datadecorator.h>

class  IntDecorator : public DataDecorator
{
    Q_OBJECT
   // QML访问的接口
    Q_PROPERTY( int ui_value READ value WRITE setValue NOTIFY valueChanged )

public:
    IntDecorator(Entity* parentEntity = nullptr, const QString& key = "SomeItemKey", const QString& label = "", int value = 0);
    ~IntDecorator();

    //修改和获取对应的属性值
    IntDecorator& setValue(int value);
    int value() const;
public:
    QJsonValue jsonValue() const override;
    void update(const QJsonObject& jsonObject) override;

signals:
    void valueChanged();

private:
    class Implementation;
    QScopedPointer<Implementation> implementation;
};

#endif
