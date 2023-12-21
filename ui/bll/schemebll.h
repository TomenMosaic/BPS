#ifndef SCHEMEBLL_H
#define SCHEMEBLL_H

#include <QObject>
#include "tabledal.h"
class SchemeBLL : public QObject
{
    Q_OBJECT
public:
    enum SchemeType
    {
        id,
        schemeName,
        account,
        time,
        isdefault
    };
    Q_ENUM(SchemeType);
    QStringList fieldList;
    explicit SchemeBLL(QObject *parent = nullptr);

    void init();

    QSharedPointer<Row> getScheme(int schemeID);

    QList<QSharedPointer<Row>> getRowList();

    QStringList getNameList();

    int addRecord(QString newName,QString userName);

    void setDefault(int noID);

    bool removeRow(int noID);

    bool reName(QString newName,int noID);

    int getDefaultID();
signals:

private:
    QString tableName = "scheme";
    TableDAL dal;
};

#endif // SCHEMEBLL_H
