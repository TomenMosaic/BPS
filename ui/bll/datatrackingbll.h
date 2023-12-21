#ifndef DATATRACKINGBLL_H
#define DATATRACKINGBLL_H

#include <QObject>
#include "tabledal.h"

class DataTrackingBLL : public QObject
{
    Q_OBJECT
public:
    enum DataTrackType{
        ID,
        OPERATIONTIME,
        USERNAME,
        EQUIPMENT,
        TESTNAME,
        OPERATIONID,
        ACTIONID,
        RESULTID,
        ISDETAILS,
        REMARK,
        DETAIL
    };
    QStringList fieldList =
    {
        "id",
        "operation_time",
        "user_name",
        "equipment_number",
        "test_name",
        "operation_id",
        "action_id",
        "results_id",
        "isdetails",
        "remark",
        "detail"
    };
    explicit DataTrackingBLL(QObject *parent = nullptr);

    void init();

    int addRecord(QString userName, QString equipmentNumber,
                  QString testName, int operationID,
                  int actionID, int isdetails, int remarkID = -1, int resultID = -1,QString detail = "");

    int addRecord(QString userName,int operationID,
                  int actionID,int isdetails);


signals:

private:
    void refreshLastID();
private:
    int newId = 0;
    QString tableName = "data_tracking";
    TableDAL dal;
};

#endif // DATATRACKINGBLL_H
