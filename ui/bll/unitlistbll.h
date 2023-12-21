#ifndef UNITLISTBLL_H
#define UNITLISTBLL_H

#include <QObject>
#include "tabledal.h"

class UnitListBLL : public QObject
{
    Q_OBJECT
public:
    enum UnitType
    {
        id,
        unitname,
        unitconverter,
        unitID
    };
    Q_ENUM(UnitType);
    QStringList fieldList;

    explicit UnitListBLL(QObject *parent = nullptr);

    void init();

    QList<QSharedPointer<Row>> getRowList(int unitID);

    QList<int> getUnitList();

    double getUnitCovertValue(int id);

    QSharedPointer<Row> getRow(int id);
signals:

private:
    QString tableName  = "unit_list";
    TableDAL dal;

};

#endif // UNITLISTBLL_H
