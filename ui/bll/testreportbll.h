#ifndef TESTREPORTBLL_H
#define TESTREPORTBLL_H

#include "tabledal.h"
#include <QObject>

class TestReportBLL : public QObject
{
    Q_OBJECT
public:
    enum TestReportType
    {
        ID,
        TESTID,
        TESTUNIT,
        TESTNAME,
        TESTCOMPLETIME,
        REPORTNAME,
        PRINTDATE,
        ACCOUNT,
        REMARKS,
        STANDARD,
        MIXTURENAME
    };
    QStringList fieldList =
    {
        "id",
        "test_id",
        "test_unit",
        "test_name",
        "test_complete_time",
        "report_name",
        "print_date",
        "account",
        "remarks",
        "standard",
        "MixtureName"
    };
    explicit TestReportBLL(QObject *parent = nullptr);

    void init();
signals:


private:
    QSharedPointer<Row>currentRow = nullptr;
    QString tableName;
    TableDAL dal;
};

#endif // TESTREPORTBLL_H
