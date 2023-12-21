#ifndef CONFIGMANAGE_H
#define CONFIGMANAGE_H

#include <QObject>
#include <QVariant>
#include <QSettings>

class ConfigManage : public QObject
{
    Q_OBJECT
public:
//    static ConfigManage *getInstance(QString path);
//    static ConfigManage *pConfig;//类对象指针
    bool contains(QString key);

    void removeKey(QString key);

    void addRecord(QString key,QVariant value);

    QVariant getRecord(QString key);

    QStringList keylist();

    QStringList grouplist();

    void setPath(QString path);

    ~ConfigManage();

    QSettings *getSetting() const;

    explicit ConfigManage(QString settingPath,QObject *parent = nullptr);

public:
//    static AutoReleaseClass autoRelease;

signals:

private:

private:
    QString currentPath = "";
    QSettings *setting = nullptr;
};

#endif // CONFIGMANAGE_H
