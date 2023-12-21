#include "testlistbll.h"
#include "log.h"
#include "qdatetime.h"
#include "qdebug.h"
#include "Global.h"
TestListBLL *TestListBLL::m_testListBll = nullptr;

TestListBLL *TestListBLL::getInstance(QObject *parent)
{
    if(m_testListBll == nullptr)
    {
        TestListBLL::m_testListBll = new TestListBLL(parent);
    }
    return TestListBLL::m_testListBll;
}

TestListBLL::TestListBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void TestListBLL::init()
{
    //初始化
    dal.initTable(tableName,fieldList,true);
    //更新ID，并设置最新的报告记录
    freshLastID();
}

int TestListBLL::addRecord(QMap<TestListBLL::TestListType, QString> valMap)
{
    newId ++;
    valMap.insert(ID,QString::number(newId));
    QMap<QString,QString> strMap;
    for(auto it = valMap.begin();it!=valMap.end();it++)
    {
        strMap.insert(fieldList.at(it.key()),it.value());
    }
    bool ok = dal.appendRow(strMap);
    if(!ok)
    {
        return -1;
    }
    else
    {
        return newId;
    }
}

QSharedPointer<Row>TestListBLL::getRecord(int testID)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = dal.getRow(index);
        if(curRow->data(ID).toInt()==testID)
            return curRow;
    }
    return nullptr;
}

QSharedPointer<Row>TestListBLL::getRecord(QString testName)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = dal.getRow(index);
        if(curRow->data(TESTNAME).toString()==testName)
        {
            return curRow;
        }
    }
    return nullptr;
}

bool TestListBLL::finishTest(int testID, bool success)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = dal.getRow(index);
        if(curRow->data(ID).toInt()==testID)
        {
            if(success)
                curRow->setData(TESTISCOMPLETE,TestFinished);
            else
                curRow->setData(TESTISCOMPLETE,DefaultError);
            curRow->setData(TESTCOMPLETETIME,TableDAL::currentTime());
            dal.update(curRow,TableDAL::ASYNC);
            return true;
        }
    }
    return false;
}

bool TestListBLL::finishReport(int testID)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = dal.getRow(index);
        if(curRow->data(ID).toInt()==testID)
        {
            curRow->setData(TESTISCOMPLETE,BuildReport);
            dal.update(curRow);
            return true;
        }
    }
    return false;
}



bool TestListBLL::isFinish()
{
    if(currentTest)
    {
        ReportState state = (ReportState)currentTest->data(TESTISCOMPLETE).toInt();
        CLOG_INFO(QString("Test Finish=%1").arg(state).toUtf8());
        if(state==ReportState::Ready||state==ReportState::Run)
            return false;
        else
            return true;
    }
    else
    {
        CLOG_INFO("current Test is empty");
        return true;
    }
    return true;
}

QSharedPointer<Row>TestListBLL::getCurrentTset()
{
    return currentTest;
}

//bool TestListBLL::finishTest(int totalCount)
//{
//    if(currentTest)
//    {
//        currentTest->setData(TOTALCOUNT,QString::number(totalCount));
//        currentTest->setData(TESTCOMPLETETIME,TableDAL::currentTime());
//        currentTest->setData(TESTISCOMPLETE,QString::number(BuildReport));
//        dal.update(currentTest,TableDAL::SYNC);
//        CLOG_INFO(QString("finish currentTest:%1").arg(currentTest->data(ID).toInt()).toUtf8());
//        //        qDebug()<<Q_FUNC_INFO<<__LINE__<<"当前实验结束:"<<currentTest->getValueList();
//        return true;
//    }
//    else
//    {
//        CLOG_INFO(QString("current Test is Empty").toUtf8());
//        return false;
//    }
//}

//int TestListBLL::nextTest(QString userName, int reportID, int settingID, int runAction)
//{
//    QDateTime dateTime = QDateTime::currentDateTime();
//    QString testTime = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
//    QString testName = QString("%1").arg(dateTime.toString("yyyyMMddhhmmss"));
//    //    int id = newId+1;
//    QMap<QString,QString> mapList;
//    //    mapList.insert(fieldList.at(ID),QString::number(id));
//    mapList.insert(fieldList.at(ACCOUNT),userName);
//    mapList.insert(fieldList.at(TESTISCOMPLETE),QString::number(false));
//    mapList.insert(fieldList.at(RUNACTION),QString::number(runAction));
//    mapList.insert(fieldList.at(TESTTIME),testTime);
//    mapList.insert(fieldList.at(TESTNAME),testName);
//    mapList.insert(fieldList.at(REPORTID),QString::number(reportID));
//    mapList.insert(fieldList.at(PARAMID),QString::number(settingID));
//    mapList.insert(fieldList.at(ISUSE),QString::number(1));
//    dal.appendRow(mapList,TableDAL::SYNC);
//    CLOG_INFO(QString("new Test:%1").arg(testName).toUtf8());
//    //重新初始化,并更新最新ID
//    this->init();
//    return newId;
//}


int TestListBLL::lastID()
{
    return newId;
    //    return dal.getLastID();
}

bool TestListBLL::setReportID(int reportID)
{
    if(currentTest)
    {
        currentTest->setData(REPORTID,QString::number(reportID));
        dal.update(currentTest,TableDAL::SYNC);
        CLOG_INFO(QString("change Test%1 ReportID TO %2:").arg(currentTest->data(ID).toInt()).arg(reportID).toUtf8());
        return true;
    }
    else
    {
        CLOG_ERROR(QString("change Test ReportID fail(currentTest is empty)").toUtf8());
        return false;
    }
}

bool TestListBLL::removeTest(int testID)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = dal.getRow(index);
        if(curRow->data(ID).toInt()==testID)
        {
            curRow->setData(TESTISCOMPLETE,DefaultError);
            curRow->setData(TESTCOMPLETETIME,TableDAL::currentTime());
            dal.removeRow(curRow,TableDAL::ASYNC);
            return true;
        }
    }
    return false;
}

bool TestListBLL::blockTest(int testID)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>curRow = dal.getRow(index);
        if(curRow->data(ID).toInt()==testID)
        {
            curRow->setData(TESTISCOMPLETE,DefaultError);
            curRow->setData(TESTCOMPLETETIME,TableDAL::currentTime());
            dal.update(curRow,TableDAL::ASYNC);
            return true;
        }
    }
    return false;
}

QJsonObject TestListBLL::getReportData(int testID)
{
    QJsonObject reportObj;
    QList<TestListBLL::TestListType>reportList =
    {
        TESTNAME,
        TESTTIME,
        ACCOUNT,
        TESTUNIT
    };
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(ID).toInt();
        if(curID==testID)
        {
            foreach(TestListBLL::TestListType type,reportList)
            {
                reportObj[fieldList.at(type)] = row->data(type).toJsonValue();
            }
            return reportObj;
        }
    }
    CLOG_ERROR(QString("unexist report where testID =%1").arg(testID).toUtf8());
    return reportObj;
}

int TestListBLL::getReportID(int testID)
{
    if(testID==-1)
    {
        if(currentTest)
        {
            return currentTest->data(PARAMID).toInt();
        }
        else
        {
            CLOG_ERROR("no running test for paramID");
            return -1;
        }
    }
    else
    {
        for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>row = dal.getRow(rowIndex);
            int curID = row->data(ID).toInt();
            if(curID==testID)
            {
                return row->data(REPORTID).toInt();
            }
        }
        return -1;
    }
}

QString TestListBLL::testName(int testID)
{
    if(testID==-1)
    {
        if(currentTest)
        {
            return currentTest->data(TESTNAME).toString();
        }
        else
        {
            CLOG_ERROR("no running test for testID");
            return "";
        }
    }
    else
    {
        for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>row = dal.getRow(rowIndex);
            int curID = row->data(ID).toInt();
            if(curID==testID)
            {
                return row->data(TESTNAME).toString();
            }
        }
        CLOG_ERROR(QString("unexist testName where testID=%1").arg(testID).toUtf8());
        return "";
    }
}

bool TestListBLL::getParam(QString testName, int &flowID, int &reportID)
{
    init();
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        //        qDebug()<<Q_FUNC_INFO<<__LINE__<<"curTestName:"<<row->data(TESTNAME).toString()<<"  testName:"<<testName;
        if(row->data(TESTNAME).toString()==testName)
        {
            flowID = row->data(FLOWID).toInt();
            reportID = row->data(REPORTID).toInt();
            return true;
        }
    }
    return false;
}

QString TestListBLL::testTime(int testID)
{
    if(testID==-1)
    {
        if(currentTest)
        {
            return currentTest->data(TESTTIME).toString();
        }
        else
        {
            CLOG_ERROR("no running test for testID");
            return "";
        }
    }
    else
    {
        for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>row = dal.getRow(rowIndex);
            int curID = row->data(ID).toInt();
            if(curID==testID)
            {
                return row->data(TESTTIME).toString();
            }
        }
        CLOG_ERROR(QString("unexist testName where testID=%1").arg(testID).toUtf8());
        return "";
    }
}

QList<QSharedPointer<Row>> TestListBLL::query(QDateTime beginTime, QDateTime endTime)
{
    //    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    //    beginTime.setTime(QTime(0,0,0));
    //    endTime.setTime(QTime(23,59,59));
    //    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    //    {
    //        QSharedPointer<Row>curRow = rowList.at(rowIndex);
    //        QDateTime curTime = QDateTime::fromString(curRow->data(TESTTIME).toString());
    //        if(curTime>beginTime&&endTime<endTime)
    //        {
    //            rowList.append(curRow);
    //        }
    //    }
    //    return rowList;
    TableDAL queryDal;
    QStringList conditions;
    QString condition = QString("test_time between '%1 00:00:01'and '%2 23:59:59' order by test_time").arg(beginTime.toString("yyyy-MM-dd")).arg(endTime.toString("yyyy-MM-dd"));
    conditions.append(condition);
    queryDal.initTable(tableName,fieldList,conditions,true);
    return queryDal.getRowList();

}

QList<QStringList> TestListBLL::query(QDateTime beginTime, QDateTime endTime, QString userName, QString testName, QList<int> idList)
{
    QStringList conditions;
    //    QStringList fieldList;
    //    fieldList = QStringList{"id","account","test_name","test_time","test_complete_time","test_is_complete"};
    QString condition = QString("test_time between '%1 00:00:00'and '%2 23:59:59'").arg(beginTime.toString("yyyy-MM-dd")).arg(endTime.toString("yyyy-MM-dd"));
    conditions.append(condition);
    if(!testName.isEmpty())
    {
        conditions.append(QString("test_name like '%%1%'").arg(testName));
    }
    if(!userName.isEmpty())
    {
        conditions.append(QString("account like '%%1%'").arg(userName));
    }
    dal.initTable(tableName,fieldList,conditions,true);
    //    return dal.getStrList(QList<int>()<<ID<<ACCOUNT<<TESTNAME<<TESTTIME<<TESTCOMPLETETIME<<TESTISCOMPLETE);
    return dal.getStrList(idList);
}

//bool TestListBLL::finishTest(int testID, int testCount)
//{
//    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
//    {
//        QSharedPointer<Row>row = dal.getRow(rowIndex);
//        if(row->data(ID).toInt()==testID)
//        {
//            row->setData(TOTALCOUNT,testCount);
//            row->setData(TESTCOMPLETETIME,TableDAL::currentTime());
//            row->setData(TESTISCOMPLETE,QString::number(TestFinished));
//            dal.update(row,TableDAL::SYNC);
//            return true;
//        }
//    }
//    return false;
//}

void TestListBLL::freshLastID()
{
    newId = 0;
    for(int rowIndex = 0;rowIndex < dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        int curID = row->data(ID).toInt();
        newId = qMax(curID,newId);
    }
}
