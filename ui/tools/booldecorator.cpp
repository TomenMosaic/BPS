#include "booldecorator.h"
class BoolDecorator::Implementation
{
public:
    Implementation(BoolDecorator* boolDecorator, const bool& _value)
        :boolDecorator(boolDecorator)
        , value(_value)
    {
    }

    BoolDecorator* boolDecorator{nullptr};

    bool value;
};
BoolDecorator::BoolDecorator(Entity *parentEntity, const QString &key, const QString &label, const bool &value)
    :DataDecorator(parentEntity,key,label)
{
     implementation.reset(new Implementation(this, value));
}

BoolDecorator::~BoolDecorator()
{

}

BoolDecorator &BoolDecorator::setValue(const bool &value)
{
    if(value != implementation->value) {
        implementation->value = value;
        emit valueChanged();
    }
    return *this;
}

const bool &BoolDecorator::value() const
{
    return implementation->value;
}

QJsonValue BoolDecorator::jsonValue() const
{
     return QJsonValue::fromVariant(QVariant(implementation->value));
}

void BoolDecorator::update(const QJsonObject &jsonObject)
{
    if(jsonObject.contains(key()))
    {
        setValue(jsonObject.value(key()).toBool());
    }
}
