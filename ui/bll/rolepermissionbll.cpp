#include "rolepermissionbll.h"
RolePermissionBLL *RolePermissionBLL::m_rolePermissionBll = nullptr;

RolePermissionBLL::RolePermissionBLL(QObject *parent)
    : QObject{parent}
{
    this->init();
}

RolePermissionBLL *RolePermissionBLL::getInstance(QObject *parent)
{
    if(m_rolePermissionBll == nullptr)
    {
        RolePermissionBLL::m_rolePermissionBll = new RolePermissionBLL(parent);
    }
    return RolePermissionBLL::m_rolePermissionBll;
}

void RolePermissionBLL::init()
{
    dal.initTable(tableName,fieldList,true);
}

void RolePermissionBLL::addPermission(int roleID, QList<int> permissionIDs)
{
    QMap<QString,QString> mapList;
    mapList.insert(fieldList.at(ROLEID),QString::number(roleID));
    for(int index = 0;index<permissionIDs.length();index++)
    {
        int lastID = dal.getLastID()+1;
        mapList.insert(fieldList.at(ID),QString::number(lastID));
        mapList.insert(fieldList.at(PERMISSIONID),QString::number(permissionIDs.at(index)));
        dal.appendRow(mapList,TableDAL::SYNC);
    }
}

void RolePermissionBLL::removePermission(int roleID, QList<int> permissionIDs)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>row = rowList.at(rowIndex);
        if(row->data(ROLEID).toInt()==roleID&&permissionIDs.contains(row->data(PERMISSIONID).toInt()))
        {
            dal.removeRow(row,TableDAL::SYNC);
        }
    }
}

QList<int> RolePermissionBLL::getRolePermissions(int roleID)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    QList<int> permissions;
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>row = rowList.at(rowIndex);
        if(row->data(ROLEID).toInt()==roleID&&!permissions.contains(row->data(PERMISSIONID).toInt()))
            permissions.append(row->data(PERMISSIONID).toInt());
    }
    return permissions;
}

void RolePermissionBLL::resetPermission(int roleID, QList<int> permissionIDs)
{
    //记录要删除的
    QList<int> curPermissions = getRolePermissions(roleID);
    for(int index = permissionIDs.length()-1;index>=0;index--)
    {
        int permission = permissionIDs.at(index);
        if(curPermissions.contains(permission))
        {
            curPermissions.removeOne(permission);
            permissionIDs.removeOne(permission);
        }
    }
    if(!curPermissions.isEmpty())
        this->removePermission(roleID,curPermissions);

    if(!permissionIDs.isEmpty())
        this->addPermission(roleID,permissionIDs);
}

void RolePermissionBLL::removePermission(int roleID)
{
    QList<QSharedPointer<Row>> rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>row = rowList.at(rowIndex);
        if(row->data(ROLEID).toInt()==roleID)
        {
            dal.removeRow(row);
        }
    }
    init();
}


