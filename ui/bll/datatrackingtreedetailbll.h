#ifndef DATATRACKINGTREEDETAILBLL_H
#define DATATRACKINGTREEDETAILBLL_H

#include "tabledal.h"
#include <QObject>

class DataTrackingTreeDetailBLL : public QObject
{
    Q_OBJECT
public:
    enum DataTrackingTreeDetailType
    {
        ID,
        DATATRACKINGID,
        CODE,
        PARENDCODE,
        VALUE,
        VALUETRANSLATE,
        DATATYPE
    };
    QStringList fieldList =
    {
        "id",
        "data_tracking_id",
        "code",
        "parent_code",
        "value",
        "value_translate",
        "data_type"
    };
    explicit DataTrackingTreeDetailBLL(QObject *parent = nullptr);

    void addTreeRecord(int dataTrackID, QString value, int code, bool valueTranslate, int dataType, int parentCode);

    void init();
signals:
private:
    QString tableName = "data_tracking_treedetails";
    TableDAL dal;
};

#endif // DATATRACKINGTREEDETAILBLL_H
