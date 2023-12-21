#ifndef JUDGE_SETUP_H
#define JUDGE_SETUP_H

#include <QObject>
#include "tabledal.h"
#include "Global.h"

class JudgeSetupBLL : public QObject
{
    Q_OBJECT
public:
    enum JudgeSetupType
    {
        id,
        cavityType,
        beginTime,
        gatherTimeInterval,
        gatherNum,
        standardDeviation,
        absoluteDeviation,
        relativeDeviation,
        autoJudgeCol
      };
    QStringList fieldList;
    Q_ENUM(JudgeSetupType)
    explicit JudgeSetupBLL(QObject *parent = nullptr);

    void init();

    QSharedPointer<Row> getCavityEdit(Cavity cavity);

    void addRecord(QMap<JudgeSetupType,QVariant>recordData);
signals:

private:
    void init(Cavity cavity);
private:
    QString tableName = "judge_setup";
    TableDAL m_tableDal;
};

#endif // JUDGE_SETUP_H
