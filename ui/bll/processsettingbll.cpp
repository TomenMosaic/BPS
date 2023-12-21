#include "processsettingbll.h"
#include <QMetaEnum>
#include "Global.h"
#include <QDebug>
ProcessSettingBLL *ProcessSettingBLL::m_processSettingBll = nullptr;
ProcessSettingBLL *ProcessSettingBLL::getInstance(QObject *parent)
{
    if(m_processSettingBll == nullptr)
    {
        ProcessSettingBLL::m_processSettingBll = new ProcessSettingBLL(parent);
    }
    return ProcessSettingBLL::m_processSettingBll;
}

void ProcessSettingBLL::initPrcessSetting(TestProcessMode mode)
{
    m_lastID++;
//    int cavityNum = g_dev->getCavityNum();
    int maxID = 9;
    int flowValue = 40;
    QMap<QString,QString>valMap;
    for(int index = 0;index<maxID;index++)
    {
        QString maxTime = "14:00:00";
        QStringList flowDetail = {"4","5","6","7"};
        if(index == 0)
        {
            maxTime="02:00:00";
            flowDetail.clear();
            flowValue = 20;
        }
        else if(index == 1)
        {
            maxTime = "00:30:00";
        }
        else if(index == 2)
        {
            maxTime = "01:00:00";
            flowDetail.push_front("1");
        }
//        else
//        {
//            flowDetail.push_front(QString::number(index-cavityNum+1));
//        }
        valMap.insert(fieldList.at(ProcessSettingBLL::flowValue),QString::number(flowValue));
        valMap.insert(fieldList.at(ProcessSettingBLL::modeType),QString::number(mode));
        valMap.insert(fieldList.at(ProcessSettingBLL::flow),QString::number(index+1));
        valMap.insert(fieldList.at(ProcessSettingBLL::maxTime),maxTime);
        valMap.insert(fieldList.at(enable),QString::number(true));
        valMap.insert(fieldList.at(modeType),QString::number(mode));
        valMap.insert(fieldList.at(processNo),QString::number(m_lastID));
        valMap.insert(fieldList.at(valueDetails),flowDetail.isEmpty()?"":flowDetail.join(","));
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"valMap:"<<valMap;
        dal.appendRow(valMap);
    }
    dal.initTable(tableName,fieldList,true);

//    QList<int> idList = {1,2,3,4,5,6,7,8,9,10,11};
//    QList<QString>time ={"02:00:00","00:00:15","00:00:10","00:30:00",
//                         "00:00:05","00:00:05","00:00:01","00:00:05"
//                         ,"00:00:28","00:00:30","24:00:00"};
//    QList<QList<QString>>flowSetupList = {
//        {"","1,3","1,3,5,14","1,3,5,8,14","1,3,5,14","1,3"
//         ,"","","4,8","",""},
//        {"","1,3","1,3,6,14","1,3,6,9,14","1,3,6,14","1,3"
//         ,"","","4,9","",""},
//        {"","1,3","1,3,7,14","1,3,7,10,14","1,3,7,14","1,3"
//         ,"","","4,10","",""}
//    };
//    valMap.insert(fieldList.at(modeType),QString::number(mode));
//    valMap.insert(fieldList.at(enable),QString::number(true));
//    m_lastID++;
//    valMap.insert(fieldList.at(processNo),QString::number(m_lastID));
//    QList<QString> flowSetup = flowSetupList.at(mode);
//    for(int idIndex= 0;idIndex<idList.length();idIndex++)
//    {
//        valMap.insert(fieldList.at(flow),QString::number(idList.at(idIndex)));
//        valMap.insert(fieldList.at(maxTime),time.at(idIndex));
//        valMap.insert(fieldList.at(valueDetails),flowSetup.at(idIndex));
//        dal.appendRow(valMap);
//    }
//    dal.initTable(tableName,fieldList,true);
}

ProcessSettingBLL::ProcessSettingBLL(QObject *parent) : QObject(parent)
{
    init();
}

void ProcessSettingBLL::init()
{
    dal.initTable(tableName,fieldList,true);
    updateLastID();
}

//int ProcessSettingBLL::getProcessNum(TestProcessMode mode)
//{
//    int maxNum = -1;
//    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
//    {
//        QSharedPointer<Row>row = dal.getRow(rowIndex);

//        if(row->data(modeType).toInt()==int(mode))
//        {
//            maxNum = qMax(maxNum,row->data(processNo).toInt());
//        }
//    }
//    if(maxNum == -1)
//    {
//        initPrcessSetting(mode);
//        for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
//        {
//            QSharedPointer<Row>row = dal.getRow(rowIndex);

//            if(row->data(modeType).toInt()==int(mode))
//            {
//                maxNum = qMax(maxNum,row->data(processNo).toInt());
//            }
//        }
//    }

//    return maxNum;
//}

QList<QSharedPointer<Row>> ProcessSettingBLL::getProcessDetail(TestProcessMode mode)
{
    int processNum = -1;
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);

        if(row->data(modeType).toInt()==int(mode))
        {
            processNum = qMax(processNum,row->data(processNo).toInt());
        }
    }

    if(processNum == -1)
    {
        initPrcessSetting(mode);
        for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>row = dal.getRow(rowIndex);

            if(row->data(modeType).toInt()==int(mode))
            {
                processNum = qMax(processNum,row->data(processNo).toInt());
            }
        }
    }
    QList<QSharedPointer<Row>> rowList;
    if(processNum==-1)
        return rowList;
    else
    {
        int maxFlow = g_dev->getCavityNum()+3;
        for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>row = dal.getRow(rowIndex);
            if(row->data(processNo).toInt()==processNum&&row->data(ProcessSettingType::flow).toInt()<=maxFlow)
                rowList.append(row);
        }

    }
    return rowList;
}

void ProcessSettingBLL::updateLastID()
{
    QVariantList valueList = dal.getColCell(processNo);
    QList<int> intList = TableDAL::toIntList(valueList);
    if(intList.length()>0)
    {
        qSort(intList);
        //    qDebug()<<Q_FUNC_INFO<<__LINE__<<"intList:"<<intList;
        m_lastID = intList.last();
    }
}

void ProcessSettingBLL::insertProcess(QList<QMap<QString, QString> > valueList)
{
    m_lastID++;
    for(int index = 0;index<valueList.length();index++)
    {
        QMap<QString,QString> valueMap = valueList.at(index);
        valueMap.insert(fieldName((processNo)),QString::number(m_lastID));
        dal.appendRow(valueMap);
    }
    dataModify();
}

QString ProcessSettingBLL::fieldName(ProcessSettingBLL::ProcessSettingType type)
{
    return fieldList.at(type);
}
