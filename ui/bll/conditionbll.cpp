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
    if (type != nullptr){ //TODO 不能直接写入where条件中，应该在结果中进行筛选
        QString where = QString("%1 is null").arg(
                    this->dbColumnNames[DbColoumnType::RemovedAt]);
        wheres.append(where);
    }
    this->dal.initTable(tableName, this->dbColumnNames, wheres, QStringList(), 0, 0, true);
    dal.reload();
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

bool ConditionBLL::createOrUpdate(ConditionDto& conditionDto){
    // 新增
    if (conditionDto.ID <= 0){
        QMap<QString,QString> mapList;
        mapList.insert(this->dbColumnNames.at(DbColoumnType::Name), conditionDto.Name);
        mapList.insert(this->dbColumnNames.at(DbColoumnType::Condition), conditionDto.Condition);
        mapList.insert(this->dbColumnNames.at(DbColoumnType::Type), QString::number(conditionDto.Type));

        if (conditionDto.Type == ConditionDto::TypeEnum::waitingCondition){
            mapList.insert(this->dbColumnNames.at(DbColoumnType::action), QString::number(conditionDto.action));
        }else if (conditionDto.Type == ConditionDto::TypeEnum::packTemplateCondition){
            mapList.insert(this->dbColumnNames.at(DbColoumnType::packTemplate), conditionDto.packTemplate);
        }else if (conditionDto.Type == ConditionDto::TypeEnum::threshold){
            mapList.insert(this->dbColumnNames.at(DbColoumnType::lengthExpression), conditionDto.lengthExpression);
            mapList.insert(this->dbColumnNames.at(DbColoumnType::widthExpression), conditionDto.widthExpression);
            mapList.insert(this->dbColumnNames.at(DbColoumnType::heightExpression), conditionDto.heightExpression);
        }

        QDateTime currentTime =  QDateTime::currentDateTime();
        QString formattedTime = currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
        mapList.insert(this->dbColumnNames.at(DbColoumnType::CreateTime),formattedTime);
        mapList.insert(this->dbColumnNames.at(DbColoumnType::LastModifyTime),formattedTime);
        dal.appendRow(mapList);

        int newId = dal.getLastID();
        conditionDto.ID = newId;
    }else{ // 修改
        int isExist = false;
        for(int index = 0;index<dal.getRowCount();index++)
        {
            QSharedPointer<Row> curRow = dal.getRow(index);
            if(curRow->data(ConditionBLL::ID).toInt()==conditionDto.ID)
            {
                isExist = true;

                // 已经被标记为删除
                if (curRow->data(ConditionBLL::RemovedAt).toString()!= ""){
                    qDebug() << QString("condition (%1) removed").arg(conditionDto.ID);
                    break; // 退出
                }

                // 更新字段
                bool isModify = false;
                if (conditionDto.RemovedAt.isNull() == false){ // 如果是剔除，更新其他的项目已经没有意义
                    curRow->setData(ConditionBLL::RemovedAt, conditionDto.RemovedAt);
                    isModify = true;
                }else{
                    if (curRow->data(ConditionBLL::Name).toString() != conditionDto.Name){
                        curRow->setData(ConditionBLL::Name, conditionDto.Name);
                        isModify = true;
                    }
                    if (curRow->data(ConditionBLL::Condition).toString() != conditionDto.Condition){
                        curRow->setData(ConditionBLL::Condition, conditionDto.Condition);
                        isModify = true;
                    }

                    if (conditionDto.Type == ConditionDto::TypeEnum::packTemplateCondition){
                        if (curRow->data(ConditionBLL::packTemplate).toString() != conditionDto.packTemplate){
                            curRow->setData(ConditionBLL::packTemplate, conditionDto.packTemplate);
                            isModify = true;
                        }
                    }else if (conditionDto.Type == ConditionDto::TypeEnum::threshold){
                        if (curRow->data(ConditionBLL::lengthExpression).toString() != conditionDto.lengthExpression){
                            curRow->setData(ConditionBLL::lengthExpression, conditionDto.lengthExpression);
                            isModify = true;
                        }
                        if (curRow->data(ConditionBLL::widthExpression).toString() != conditionDto.widthExpression){
                            curRow->setData(ConditionBLL::widthExpression, conditionDto.widthExpression);
                            isModify = true;
                        }
                        if (curRow->data(ConditionBLL::heightExpression).toString() != conditionDto.heightExpression){
                            curRow->setData(ConditionBLL::heightExpression, conditionDto.heightExpression);
                            isModify = true;
                        }
                    }
                }

                //PS. 类型不能改变

                // 是否有修改
                if (isModify){
                    auto now =  QDateTime::currentDateTime();
                    curRow->setData(ConditionBLL::LastModifyTime, now);
                    QMap<QString, QVariant> primaryKeys; // 使用主键进行查询
                    primaryKeys.insert(dbColumnNames.at(DbColoumnType::ID), conditionDto.ID);
                    dal.update(curRow, primaryKeys);
                }

                break; // 退出
            }
        }

        if (!isExist){
            qWarning() << "找不到对应的condition数据，id：" << conditionDto.ID;
        }
    }

    return true;
}
