#ifndef PACK_INSTANCE_H
#define PACK_INSTANCE_H

#include <QString>
#include <QtSql/QSqlDatabase>

class pack_instance
{
public:
    uint id;
    float length;
    float width;
    float height;
    QString orderNo;
    QString productName;

public:
    pack_instance getSingle(long id);
    QList<pack_instance> getTop10();

    void create(const pack_instance& pack);
    void update(const pack_instance& pack);
    void remove(uint id);

    pack_instance();
    virtual       ~pack_instance();
};

#endif // PACK_INSTANCE_H
