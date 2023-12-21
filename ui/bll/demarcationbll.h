#ifndef DEMARCATIONBLL_H
#define DEMARCATIONBLL_H

#include <QObject>
#include "tabledal.h"
#include "Global.h"
class DemarcationBLL : public QObject
{
    Q_OBJECT
public:
    enum DemarcationType
    {
        id,
        operationTime,
        transmittance,
        pressureVariation,
        schemeID,
        shifting
    };
    Q_ENUM(DemarcationType);
    QStringList fieldList;

    explicit DemarcationBLL(QObject *parent = nullptr);

    void init();

    QList<QSharedPointer<Row>> getRowListByID(int schemeID);

    void removeScheme(int schemeID);

    void removeRow(int id);

    int addRecord(QString operationTime, double transmittance, double pressureVariation,double shiftValue, int schemeID);

    bool modifyRecord(double transmittance, double pressureVariation,double shiftValue,int id);
private:
    void freshLastID();

signals:

private:
    int newId = 0;
    QString tableName = "demarcation";
    TableDAL dal;
};

#endif // DEMARCATIONBLL_H
