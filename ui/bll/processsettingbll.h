#ifndef PROCESSSETTINGBLL_H
#define PROCESSSETTINGBLL_H

#include <QObject>
#include "tabledal.h"
#include "GlobalVar.h"
class ProcessSettingBLL : public QObject
{
    Q_OBJECT
public:
    enum ProcessSettingType
    {
        id,
        processNo,
        modeType,
        enable,
        maxTime,
        flow,
        flowValue,
        valueDetails
    };
    Q_ENUM(ProcessSettingType);

    static ProcessSettingBLL *getInstance(QObject *parent = nullptr);

    QStringList fieldList
    ={
        "id",
        "processNo",
        "cavityNo",
        "enable",
        "maxTime",
        "flow",
        "flowValue",
        "valueDetails"};

    void initPrcessSetting( TestProcessMode mode);

    void init();
    //    //获取腔体对应的流程号
    //    int getProcessNum(TestProcessMode mode);
    //获取流程号对应的具体设置
    QList<QSharedPointer<Row>>getProcessDetail(TestProcessMode mode);

    void updateLastID();

    void insertProcess(QList<QMap<QString, QString> > valueList);

    QString fieldName(ProcessSettingType type);

private:
    explicit ProcessSettingBLL(QObject *parent = nullptr);

signals:
    void dataModify();

private:
    static ProcessSettingBLL *m_processSettingBll;
    int m_lastID = 0;
    QString tableName = "process_setting";
    TableDAL dal;
};

#endif // PROCESSSETTINGBLL_H
