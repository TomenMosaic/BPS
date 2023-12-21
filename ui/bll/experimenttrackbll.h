#ifndef EXPERIMENTTRACKBLL_H
#define EXPERIMENTTRACKBLL_H

#include <QObject>
#include "Global.h"
#include "tabledal.h"
class ExperimentTrackBLL : public QObject
{
    Q_OBJECT
public:
    enum ExperimentTracType
    {
        id              ,
        operation_time  ,
        user_name       ,
        equipment_number,
        test_name       ,
        operation_id    ,
        action_id       ,
        results_id      ,
        isdetails       ,
        remark          ,
        detail

    };
    Q_ENUM(ExperimentTracType)
    explicit ExperimentTrackBLL(QObject *parent = nullptr);
//    QList<QStringList> query(QDateTime beginTime, QDateTime endTime, QString userName, QString testName, int beginNum, int endNum);
    QList<QSharedPointer<Row>>query(QDateTime beginTime, QDateTime endTime, QString userName, QString testName, int beginNum, int endNum);
    int queryCount(QDateTime beginTime, QDateTime endTime, QString userName, QString testName);
signals:


private:
    QStringList fieldList;
};

#endif // EXPERIMENTTRACKBLL_H
