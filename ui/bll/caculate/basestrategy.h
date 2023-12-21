#ifndef BASESTRATEGY_H
#define BASESTRATEGY_H

#include <QObject>

class BaseStrategy : public QObject
{
    Q_OBJECT
public:
    explicit BaseStrategy(QObject *parent = nullptr);


signals:

};

#endif // BASESTRATEGY_H
