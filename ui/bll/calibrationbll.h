#ifndef CALIBRATIONBLL_H
#define CALIBRATIONBLL_H

#include <QObject>
#include "tabledal.h"
class CalibrationBLL : public QObject
{
    Q_OBJECT
public:

    enum CalibrationType
    {
        id,
        cavityType,
        calibrateParam,
        outputValue,
        realValue
    };
    Q_ENUM(CalibrationType);

    QStringList fieldList;

    explicit CalibrationBLL(QObject *parent = nullptr);

    void init();

    QList<QSharedPointer<Row>> getRowList();

    bool save(QList<QVariantList> valueList);

private:
    TableDAL dal;
    QString tableName = "calibration";
};

#endif // CALIBRATIONBLL_H
