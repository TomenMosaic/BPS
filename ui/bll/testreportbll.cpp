#include "testreportbll.h"
#include "qdebug.h"

TestReportBLL::TestReportBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void TestReportBLL::init()
{
    dal.initTable(tableName,fieldList,true);
    qDebug()<<Q_FUNC_INFO<<__LINE__<<"lastID:"<<dal.getLastID();
}
