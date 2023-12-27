#include "conditionbll.h"

ConditionBLL *ConditionBLL::m_conditionBll = nullptr;

ConditionBLL *ConditionBLL::getInstance(QObject *parent)
{
    if(m_conditionBll == nullptr)
    {
        ConditionBLL::m_conditionBll = new ConditionBLL(parent);
    }
    return ConditionBLL::m_conditionBll;
}

ConditionBLL::ConditionBLL(QObject *parent) :
    QObject(parent)
{
    this->init();
}

void ConditionBLL::init()
{
    // 设置字段列表
    this->dbColumnNames = this->dal.getColumnNames(this->dbColumnList);

    // 检查数据库表是否存在
    this->dal.checkAndCreateTable(this->tableName, this->dbColumnList);

    // 初始化 databable
    //QStringList orders;
    //orders.append("id desc");
    //dal.initTable(tableName, this->dbColumnNames, QStringList(), orders, 12, 0, true);
}

QList<ConditionDto> ConditionBLL::getRowList(ConditionDto::TypeEnum* type){
    QStringList wheres;
    wheres.append(QString("%1 is null").arg(this->dbColumnNames[DbColoumnType::RemovedAt]));
    if (type != nullptr){
        QString where = QString("%1='%2'").arg(
                    this->dbColumnNames[DbColoumnType::RemovedAt],
                    QString::number(*type));
        wheres.append(where);
    }
    this->dal.initTable(tableName, this->dbColumnNames, wheres, QStringList(), 0, 0, true);
    auto rows = dal.getRowList();

    QList<ConditionDto> result;
    for(const QSharedPointer<Row>& row : qAsConst(rows)){
        ConditionDto dto ;
        dto.ID = row->data(DbColoumnType::ID).toInt();
        dto.Name = row->data(DbColoumnType::Name).toString();
        dto.Condition = row->data(DbColoumnType::Condition).toString();
        dto.Type = static_cast<ConditionDto::TypeEnum>(row->data(DbColoumnType::Type).toInt());
        dto.lengthExpression = row->data(DbColoumnType::lengthExpression).toString();
        dto.widthExpression = row->data(DbColoumnType::widthExpression).toString();
        dto.heightExpression = row->data(DbColoumnType::heightExpression).toString();

        dto.packTemplate = row->data(DbColoumnType::packTemplate).toString();
        dto.action = static_cast<ConditionDto::ActionEnum>(row->data(DbColoumnType::action).toInt());

        dto.CreateTime = row->data(DbColoumnType::CreateTime).toDateTime();
        dto.LastModifyTime = row->data(DbColoumnType::LastModifyTime).toDateTime();
        dto.RemovedAt = row->data(DbColoumnType::RemovedAt).toDateTime();

        result.append(dto);
    }
    return result;
}
