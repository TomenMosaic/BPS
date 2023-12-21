#include "userrolebll.h"
UserRoleBLL *UserRoleBLL::m_userRole = nullptr;
UserRoleBLL *UserRoleBLL::getInstance(QObject *parent)
{
    if(m_userRole == nullptr)
    {
        UserRoleBLL::m_userRole = new UserRoleBLL(parent);
    }
    return UserRoleBLL::m_userRole;
}
UserRoleBLL::UserRoleBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

void UserRoleBLL::addRole(int userID, int roleID)
{
    QMap<QString,QString> mapList;
    mapList.insert(fieldList.at(USERID),QString::number(userID));
    mapList.insert(fieldList.at(ROLEID),QString::number(roleID));
    mapList.insert(fieldList.at(ID),QString::number(dal.getLastID()+1));
    mapList.insert(fieldList.at(CREATED),TableDAL::currentTime());
    dal.appendRow(mapList,TableDAL::SYNC);
}

void UserRoleBLL::removeRole(int userID, int roleID)
{
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        if(row->data(USERID).toInt()==userID&&row->data(ROLEID).toInt()==roleID)
        {
            dal.removeRow(row,TableDAL::SYNC);
            return;
        }
    }
}

QList<int> UserRoleBLL::getRoleList(int userID)
{
    QList<int> roleIDs;
    for(int rowIndex  =0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row  = dal.getRow(rowIndex);
        if(row->data(USERID).toInt()==userID)
        {
            roleIDs.append(row->data(ROLEID).toInt());
        }
    }
    return roleIDs;
}

void UserRoleBLL::removeUser(int userID)
{
    for(int rowIndex = dal.getRowCount()-1;rowIndex>=0;rowIndex--)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        if(row->data(USERID).toInt()==userID)
        {
            dal.removeRow(row);
        }
    }
}

QList<int> UserRoleBLL::getUserListForRole(int roleID)
{
    QList<int>userList;
    for(int rowIndex = 0;rowIndex<dal.getRowCount();rowIndex++)
    {
        QSharedPointer<Row>row = dal.getRow(rowIndex);
        if(row->data(ROLEID).toInt()==roleID)
        {
            userList.append(row->data(USERID).toInt());
        }
    }
    return userList;
}

void UserRoleBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}
