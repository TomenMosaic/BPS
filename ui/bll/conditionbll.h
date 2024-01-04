#ifndef CONDITIONBLL_H
#define CONDITIONBLL_H

#include "core/tabledal.h"

#include <QObject>
#include <QMetaEnum>
#include <QDateTime>

class ConditionDto
{

public:
    enum TypeEnum{
        // 阈值
        threshold = 0,
        // 等待条件
        waitingCondition = 1,
        // 箱型选择
        packTemplateCondition = 2,
    };

    enum ActionEnum{
        scan = 0,
    };

   int ID;
   QString Name;
   TypeEnum Type;
   QString Condition;

   QString lengthExpression;
   QString widthExpression;
   QString heightExpression;

   QString packTemplate;
   ActionEnum action;

   QDateTime CreateTime;
   QDateTime LastModifyTime;
   QDateTime RemovedAt;
};

class ConditionBLL: public QObject
{
    Q_OBJECT

public:

    enum DbColoumnType
    {
        ID,
        Name,
        Type,
        Condition,

        lengthExpression,
        widthExpression,
        heightExpression,

        packTemplate,
        action,

        CreateTime,
        LastModifyTime,
        RemovedAt

    };
    const QList<DataTableColumn> dbColumnList = {
        {"id", "INTEGER PRIMARY KEY AUTOINCREMENT"},
        {"name", "VARCHAR(100)"},
        {"type", "INTEGER"},
        {"condition", "TEXT"},

        {"length_expression", "TEXT"},
        {"width_expression", "TEXT"},
        {"height_expression", "TEXT"},

        {"pack_template", "VARCHAR(30)"},
        {"action", "INTEGER"},

        {"created_at", "DATETIME"},
        {"updated_at", "DATETIME"},
        {"removed_at", "DATETIME"}
    };


    //
    QList<ConditionDto> getRowList(ConditionDto::TypeEnum* type);

    // 创建或者更新
    bool createOrUpdate(ConditionDto& conditionDto);

    /**
     * @description:
     */
    explicit ConditionBLL(QObject *parent = nullptr);

    static ConditionBLL *getInstance(QObject *parent = nullptr);

    void init();

private:
    static ConditionBLL *m_conditionBll;

    TableDAL dal;

    QString tableName = "conditions";

    QStringList dbColumnNames;
};

#endif // CONDITIONBLL_H
