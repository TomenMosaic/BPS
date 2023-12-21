#ifndef SYSTEMTRACKBLL_H
#define SYSTEMTRACKBLL_H

#include <QObject>

class SystemtrackBLL : public QObject
{
    Q_OBJECT
public:
    explicit SystemtrackBLL(QObject *parent = nullptr);

    QList<QStringList>query(QDateTime beginTime,QDateTime endTime,QString userName,int beginNum = 0,int endNum = 20);

    int queryCount(QDateTime beginTime,QDateTime endTime,QString userName);

    QList<QStringList> getDetail(int roleID, int detailType=1);
signals:

};

#endif // SYSTEMTRACKBLL_H
