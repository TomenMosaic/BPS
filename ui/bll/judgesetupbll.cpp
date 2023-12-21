#include "judgesetupbll.h"
#include <QMetaEnum>

JudgeSetupBLL::JudgeSetupBLL(QObject *parent) : QObject(parent)
{
    const QMetaObject &mo = JudgeSetupBLL::staticMetaObject;
    QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("JudgeSetupType"));
    for (int i = 0; i < metaEnum.keyCount(); ++i)
    {
        fieldList.append(metaEnum.key(i));
    }
    init();
}

void JudgeSetupBLL::init()
{
    m_tableDal.initTable(tableName,fieldList,true);
    int devNum = g_dev->getCavityNum();
    for(int devIndex = 0;devIndex<devNum;devIndex++)
    {
        Cavity cavity = Cavity(cavityA+devIndex);
        if(getCavityEdit(cavity).isNull())
            init(cavity);
    }
}

QSharedPointer<Row> JudgeSetupBLL::getCavityEdit(Cavity cavity)
{
    QSharedPointer<Row> maxRow;
    for(int rowIndex = 0;rowIndex<m_tableDal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row> curRow = m_tableDal.getRow(rowIndex);
        if(curRow->data(cavityType).toInt()==cavity)
        {
            if(maxRow.isNull())
                maxRow = curRow;
            else if(curRow->data(id).toInt()>maxRow->data(id).toInt())
            {
                maxRow = curRow;
            }
        }
    }
    return maxRow;
}

void JudgeSetupBLL::addRecord(QMap<JudgeSetupBLL::JudgeSetupType, QVariant> recordData)
{
    QMap<QString,QVariant> valMap;
    for(auto it = recordData.begin();it!=recordData.end();it++)
    {
        valMap.insert(fieldList.at(it.key()),it.value());
    }
    m_tableDal.appendRow(valMap);
    this->init();
}

void JudgeSetupBLL::init(Cavity cavity)
{
    QMap<QString,QVariant> mapList =
    {
        {fieldList.at(beginTime),0},
        {fieldList.at(gatherTimeInterval),60},
        {fieldList.at(gatherNum),3},
        {fieldList.at(standardDeviation),0},
        {fieldList.at(absoluteDeviation),0},
        {fieldList.at(relativeDeviation),0},
        {fieldList.at(autoJudgeCol),true},
        {fieldList.at(cavityType),cavity}
    };
    m_tableDal.appendRow(mapList);


}
