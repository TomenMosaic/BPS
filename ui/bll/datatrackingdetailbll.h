#ifndef DATATRACKINGDETAILBLL_H
#define DATATRACKINGDETAILBLL_H

#include "tabledal.h"
#include <QObject>

class DataTrackingDetailBLL : public QObject
{
    Q_OBJECT
public:
    enum DataTrackDetailType{
        ID,
        DATATRACKID,
        NAMEID,
        ORIGINALVALUE,
        MODIFYVALUE,
        NAMEVAR
    };
    QStringList fieldList =
    {
        "id",
        "data_tracking_id",
        "name_id",
        "original_value",
        "modify_value",
        "name_var"
    };
    explicit DataTrackingDetailBLL(QObject *parent = nullptr);

    int addDetail(int dataTrakingID,int nameID,QString originalVal,QString  modifyVal,QString nameVar);

    void init();
signals:

private:
    QString tableName = "data_tracking_details";
    TableDAL dal;
};

#endif // DATATRACKINGDETAILBLL_H
