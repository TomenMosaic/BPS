#include "permissionbll.h"
#include "log.h"
PermissionBLL *PermissionBLL::m_permissionBll = nullptr;

PermissionBLL::PermissionBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

PermissionBLL *PermissionBLL::getInstance(QObject *parent)
{
    if(m_permissionBll == nullptr)
    {
        PermissionBLL::m_permissionBll = new PermissionBLL(parent);
    }
    return PermissionBLL::m_permissionBll;
}

void PermissionBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}

QStringList PermissionBLL::getCodeFromIDs(QList<int> idList)
{
    QStringList codeList;
    for(int index = 0;index<idList.length();index++)
    {
        int id = idList.at(index);
        QString code;
        for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>row = dal.getRow(rowIndex);
            if(row->data(ID).toInt()==id)
            {
                code = row->data(CODE).toString();
                break;
            }
        }
        if(code.isEmpty())
        {
            CLOG_ERROR(QString("PermissionBLL exist Perrmission :%1").arg(id).toUtf8());
        }
        codeList.append(code);
    }
    return codeList;
}

QStringList PermissionBLL::getNameFromIDs(QList<int> idList)
{
    QStringList nameList;
    for(int index = 0;index<idList.length();index++)
    {
        int id = idList.at(index);
        QString name;
        for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>row = dal.getRow(rowIndex);
            if(row->data(ID).toInt()==id)
            {
                name = row->data(NAME).toString();
                break;
            }
        }
        if(name.isEmpty())
            CLOG_ERROR(QString("PermissionBLL exist Perrmission :%1").arg(id).toUtf8());
        nameList.append(name);
    }
    return nameList;
}

QList<QSharedPointer<Row>> PermissionBLL::getAllPermissions()
{
    return dal.getRowList();
}

QList<int> PermissionBLL::getAllPermissionIDs()
{
    QList<int> permissionIDs;
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>curRow = dal.getRow(rowIndex);
        permissionIDs.append(curRow->data(CODE).toInt());
    }
    return permissionIDs;
}

void PermissionBLL::addParentIDs(QList<int> &idList)
{
    QMap<int,int>dataMap;
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> curRow = dal.getRow(index);
        if(idList.contains(curRow->data(CODE).toInt()))
        {
           int parentID = curRow->data(PARENTID).toInt();
           if(!idList.contains(parentID))
               idList.append(parentID);
        }
    }
}
