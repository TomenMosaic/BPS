
#include <QString>
#include <QVariant>

#ifndef CONVERT_H
#define CONVERT_H


#endif // CONVERT_H

static QString VarToString(QVariant variant)
{
    switch (variant.type())
    {
    case QVariant::Int:
        return QString::number(variant.toInt());
        break;
    case QVariant::Double:
        return QString::number(variant.toDouble());
        break;
    case QVariant::String:
    default:
        return variant.toString();
        break;
    }
}
static int VarToInt(QVariant variant)
{
    switch(variant.type())
    {
    case QVariant::Int:
    case QVariant::Double:
        return variant.toInt();
        break;
    case QVariant::String:
    default:
        return variant.toString().toInt();
    }
}
