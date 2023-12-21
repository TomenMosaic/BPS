#include "userbll.h"
#include "log.h"
#include "qdebug.h"
#include "GlobalVar.h"

UserBLL *UserBLL::m_userBll = nullptr;

UserBLL *UserBLL::getInstance(QObject *parent)
{
    if(m_userBll == nullptr)
    {
        UserBLL::m_userBll = new UserBLL(parent);
    }
    return UserBLL::m_userBll;
}

bool UserBLL::changePwd(QString userName, QString password, QString newPassword)
{
    bool remeberPwd = false;
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>row = rowList.at(rowIndex);
        bool nameFlag = (VarToString(row->data(USERNAME))==userName);
        bool pwdFlag = (VarToString(row->data(PASSWORD))==password);
        if(nameFlag&&pwdFlag)
        {
            row->setData(DEFAULTTIMES,0);
            row->setData(MODIFYTIME,TableDAL::currentTime());
            row->setData(RSREMEBERPWD,remeberPwd);
            row->setData(PASSWORD,newPassword);
            return true;
        }
    }
    return false;
}

UserBLL::UserBLL(QObject *parent)
    : QObject{parent}
{
    init();
}

bool UserBLL::login(QString userName, QString password, bool remeberPwd)
{
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>row = rowList.at(rowIndex);
        bool nameFlag = (VarToString(row->data(USERNAME))==userName);
        bool pwdFlag = (VarToString(row->data(PASSWORD))==password);
        bool remeberFlag = (row->data(RSREMEBERPWD).toBool()==remeberPwd);
        if(nameFlag&&pwdFlag)
        {
            setCurrentUser(row);
            row->setData(TRYLOGINTIME,TableDAL::currentTime());
            row->setData(DEFAULTTIMES,0);
            if(!remeberFlag)
            {
                row->setData(RSREMEBERPWD,remeberPwd);
            }
            dal.update(row);
            return true;
        }
        else if(nameFlag&&!pwdFlag)
        {
            row->setData(RSREMEBERPWD,false);
            row->setData(TRYLOGINTIME,TableDAL::currentTime());
            row->setData(DEFAULTTIMES,qMax(1,row->data(DEFAULTTIMES).toInt()+1));
            dal.update(row);

        }
    }
    return false;
}

bool UserBLL::checkPwd(QString userName, QString password)
{
    QList<QSharedPointer<Row>>rowList = dal.getRowList();
    for(int rowIndex = 0;rowIndex<rowList.length();rowIndex++)
    {
        QSharedPointer<Row>row = rowList.at(rowIndex);
        bool nameFlag = (VarToString(row->data(USERNAME))==userName);
        bool pwdFlag = (VarToString(row->data(PASSWORD))==password);
        if(nameFlag&&pwdFlag)
            return true;
    }
    return false;
}

int UserBLL::registerUser(QString userName, QString password,int userState)
{
    int userID = dal.getLastID()+1;
    QMap<QString,QString> mapList;
    mapList.insert(fieldList.at(USERNAME),userName);
    mapList.insert(fieldList.at(PASSWORD),password);
    mapList.insert(fieldList.at(ISUSE),QString::number(userState));
    mapList.insert(fieldList.at(REGISETERTIME),TableDAL::currentTime());
    mapList.insert(fieldList.at(MODIFYTIME),TableDAL::currentTime());
    mapList.insert(fieldList.at(USERID),QString::number(dal.getLastID()+1));
    bool ok = dal.appendRow(mapList,TableDAL::SYNC);
    if(!ok)
        return -1;
    return userID;
}

UserBLL::~UserBLL()
{

}

QList<int> UserBLL::getIDList()
{
    QVariantList variantList = dal.getColCell(USERID);
    QList<int> remeberPWDList;
    foreach(QVariant variant,variantList)
        remeberPWDList.append(variant.toInt());

    return remeberPWDList;
}

QStringList UserBLL::getUserNameList()
{
    QVariantList variantList = dal.getColCell(USERNAME);
    return TableDAL::toStringList(variantList);
}

QStringList UserBLL::getPasswordList()
{
    QVariantList variantList = dal.getColCell(PASSWORD);

    return TableDAL::toStringList(variantList);
}

QList<bool> UserBLL::getRemeberPWDList()
{
    QVariantList variantList = dal.getColCell(RSREMEBERPWD);
    QList<bool> remeberPWDList;
    foreach(QVariant variant,variantList)
        remeberPWDList.append(variant.toBool());

    return remeberPWDList;

}


void UserBLL::init()
{
    dal.initTable("user",fieldList,true);
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row> row = dal.getRow(index);
        if(row->data(UserBLL::MODIFYTIME).toString().isEmpty())
        {
            row->setData(UserBLL::MODIFYTIME,TableDAL::currentTime());
            dal.update(row);
        }
    }
}

void UserBLL::setCurrentUser(QSharedPointer<Row>currentUser)
{
    if(this->currentUser)
    {
        currentUser = nullptr;
    }
    this->currentUser = currentUser;
}

QVariant UserBLL::getCurrentVar(UserType type)
{
    QVariant variant;
    if(currentUser)
        variant = currentUser->data(type);
    else
        CLOG_ERROR("Current user is empty");
    return variant;
}

QList<QSharedPointer<Row>> UserBLL::getRowList()
{
    return dal.getRowList();
}

QSharedPointer<Row>UserBLL::getUser(int userID)
{
    for(int index = 0;index<dal.getRowCount();index++)
    {
        QSharedPointer<Row>row = dal.getRow(index);
        if(row->data(USERID).toInt()==userID)
        {
            return row;
        }
    }
    CLOG_ERROR((QString("add unexist userID：%1").arg(userID)).toUtf8());
    return nullptr;
}

void UserBLL::updateUser(QSharedPointer<Row>user)
{
    dal.update(user,TableDAL::SYNC);
}

void UserBLL::removeUser(QSharedPointer<Row>user)
{
    dal.removeRow(user,TableDAL::SYNC);
}

