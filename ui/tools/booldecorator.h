#ifndef BOOLDECORATOR_H
#define BOOLDECORATOR_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <datadecorator.h>
#include <QVariant>
class BoolDecorator : public DataDecorator
{
    Q_OBJECT

    Q_PROPERTY( bool ui_value READ value WRITE setValue NOTIFY valueChanged )

public:
    explicit BoolDecorator(Entity* parentEntity = nullptr, const QString& key = "SomeItemKey", const QString& label = "", const bool& value = false);
    ~BoolDecorator();

    BoolDecorator &setValue(const bool &value);
    const bool& value()const;

    QJsonValue jsonValue() const override;
    void update(const QJsonObject &jsonObject) override;
signals:
    void valueChanged();

private:
    class Implementation;
    QScopedPointer<Implementation> implementation;
};

#endif // BOOLDECORATOR_H
