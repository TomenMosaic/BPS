//int-decorator.cpp
#include "intdecorator.h"
#include <QVariant>
#include "Global.h"
class IntDecorator::Implementation
{
public:
    Implementation(IntDecorator* intDecorator, int value)
        :intDecorator(intDecorator),
          value(value)
    {
    }
    int value;
    IntDecorator* intDecorator{nullptr};
};

IntDecorator::IntDecorator(Entity* parentEntity, const QString& key, const QString& label, int value)
    : DataDecorator(parentEntity, key, label)
{
    implementation.reset(new Implementation(this, value));
}

IntDecorator::~IntDecorator()
{
}

int IntDecorator::value() const
{
    return implementation->value;
}


IntDecorator& IntDecorator::setValue(int value)
{
    if(value != implementation->value) {
        // ...Validation here if required...
        implementation->value = value;
        emit valueChanged();
    }

    return *this;
}

QJsonValue IntDecorator::jsonValue() const
{
    return QJsonValue::fromVariant(QVariant(implementation->value));
}

void IntDecorator::update(const QJsonObject& jsonObject)
{
    if (jsonObject.contains(key())) {
        auto l_value = jsonObject.value(key()).toInt();
        setValue(l_value);
    } else {
        setValue(0);
    }
}
