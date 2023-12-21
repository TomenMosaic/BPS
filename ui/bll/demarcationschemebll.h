#ifndef DEMARCATIONSCHEMEBLL_H
#define DEMARCATIONSCHEMEBLL_H

#include <QObject>
#include "tabledal.h"
class DemarcaSchemeBLL : public QObject
{
    Q_OBJECT
public:
    enum SchemeType
    {
        id,
        schemeName,
        account,
        time,
        cavityType
    };
    Q_ENUM(SchemeType);
    QStringList fieldList;
    explicit DemarcaSchemeBLL(QObject *parent = nullptr);

    void init();

    QList<QSharedPointer<Row>> getRowList();

    QStringList getNameList();

    int addRecord(QString newName,QString userName,QString time,int cavityNum);

    void setCavityDefault(int noID, int cavityNum);

    bool removeRow(int noID);

    bool reName(QString newName,int noID);

    int getDefaultID(int cavityNum);

signals:

private:
    void freshLastID();
private:
    int newId = 0;

    QString tableName = "demarcascheme";
    TableDAL dal;
};

#endif // DEMARCATIONSCHEMEBLL_H
