#include "rolebll.h"
#include <QDebug>
#include "GlobalVar.h"
RoleBLL *RoleBLL::m_roleBll = nullptr;

RoleBLL::RoleBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

RoleBLL *RoleBLL::getInstance(QObject *parent)
{
    if(m_roleBll == nullptr)
    {
        RoleBLL::m_roleBll = new RoleBLL(parent);
    }
    return RoleBLL::m_roleBll;
}

void RoleBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}

int RoleBLL::addRole(QMap<RoleType, QVariant> roleVal)
{
    int lastID = dal.getLastID()+1;
    roleVal.insert(ID,lastID);
    roleVal.insert(CODE,lastID);
    QMap<QString,QString>mapList;
    for(auto it = roleVal.begin();it!=roleVal.end();it++)
    {
        mapList.insert(fieldList.at(it.key()),VarToString(it.value()));
    }

    dal.appendRow(mapList,TableDAL::SYNC);
//    init();
    return lastID;
}

bool RoleBLL::removeRole(int id)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    for(int index = 0;index<rowList.length();index++)
    {
        QSharedPointer<Row>row = rowList.at(index);
        if(row->data(ID).toInt()==id)
        {
            dal.removeRow(row);
            return true;
        }
    }
    return false;
}

QList<QStringList> RoleBLL::getAllRole(QList<RoleType> tyleList)
{
    QList<QStringList> valueList;
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QVariantList rowData;
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        for(int index = 0;index<tyleList.length();index++)
        {
            rowData.append(row->data(tyleList.at(index)));
        }
        valueList.append(TableDAL::toStringList(rowData));
    }
    return valueList;
}

QList<QSharedPointer<Row>> RoleBLL::getAllRole()
{
    return dal.getRowList();
}

QList<QSharedPointer<Row>> RoleBLL::getRoles(QList<int> roleIDs)
{
    QList<QSharedPointer<Row>> roles;
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    for(int index = 0;index<rowList.length();index++)
    {
        QSharedPointer<Row>row = rowList.at(index);
        if(roleIDs.contains(row->data(RoleType::CODE).toInt()))
        {
            roles.append(row);
        }
    }
    return roles;
}

bool RoleBLL::reName(int roleID, QString newName, QString editor)
{
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        if(row->data(ID).toInt()==roleID)
        {
            row->setData(NAME,newName);
            row->setData(EDITOR,editor);
            row->setData(EDITED,TableDAL::currentTime());
        }
        dal.update(row);
        return true;
    }
    return false;
}

bool RoleBLL::updateRole(int roleID, QMap<RoleBLL::RoleType, QString> roleData)
{
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        if(row->data(ID).toInt()==roleID)
        {
            for(auto it = roleData.begin();it!=roleData.end();it++)
            {
                row->setData(int(it.key()),it.value());
            }
            row->setData(EDITED,TableDAL::currentTime());
            dal.update(row);
            return true;
        }
    }
    return false;
}

QStringList RoleBLL::getCodeListFromIDs(QList<int> idList)
{
    QStringList strList;
    for(int index = 0;index<idList.length();index++)
    {
        for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
        {
            QSharedPointer<Row>curRow = dal.getRow(rowIndex);
            if(curRow->data(ID).toInt()==idList.at(index))
            {
                strList.append(curRow->data(NAME).toString());
                break;
            }
        }
    }
    return strList;
}
