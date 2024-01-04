/*
 * @Date: 2023-11-16 19:21:12
 * @LastEditors: Your Name
 * @LastEditTime: 2023-11-21 16:00:11
 * @FilePath: \ui\bll\database.h
 */
#ifndef DATABASE_H
#define DATABASE_H

#include "qmutex.h"
#include "qsqlquery.h"
#include "qthread.h"
#include "qwaitcondition.h"
#include <QJsonObject>
#include <QList>
#include <QStringList>
#include <QVariantMap>
#include <QJsonArray>
#include <QTimer>
// #include "GlobalVar.h"

class DataBasePrivate : public QThread
{
    Q_OBJECT
signals:

public:
    explicit DataBasePrivate(QObject *parent = nullptr);
    void run();
    ~DataBasePrivate();

    void stop();
    void startRunning();
public slots:
    void setDataBaseName(QString databaseName);

    void addCmd(QString cmd);

    void onTimeOut();

private:
    QTimer *timer;
    // 更新删除指令
    QList<QString> cmdList;
    bool isRun = true;
    QWaitCondition *condition;
    QString dataBaseName;
    QMutex mutex;
};

class DataBase : public QObject
{
    Q_OBJECT
signals:
    void newQueryCmd(QString queryCmd);

public:
    /**
     * @description: 获取类对象指针的方法
     * @param {QObject} *parent
     * @param {QString} dataBaseName
     * @return {*}
     */
    static DataBase *getDataBase(QObject *parent = nullptr, QString dataBaseName = "./.config/packdata", QString password = "");

    // 类对象指针
    static DataBase *PdataBase;

    // 初始化数据库
    void initDB();

    /**
     * @description: 执行查询指令
     * @param {QString} cmd
     * @return {*}
     * field：为字段名称 ，value:[[]]为返回的值
     * {"field":[],"value":[[]]}
     */
    QJsonObject execQuery(QString cmd);

    /**
     * @description: 执行查询指令
     */
    QSqlQuery query(QString cmd);

    bool ExecSql(const QString& sql, const QMap<QString, QVariant>& params);

    /**
     * @description: 执行更新指令
     */
    void updateCmd(QString cmd);

    // 插入数据
    void insertCmd(QString tableName, QStringList fields, QStringList values);

    /**
     * @description: 根据表名查询数据
     */
    QList<QStringList> selectDataFromBase(QString tableName);

    /**
     * @description: 执行sql语句
     * @param {QString} sql 为sql语句
     * @param {QList<QVariant>} data 为sql语句中的参数
     * @return bool 执行成功返回true
     */
    bool ExecSql(QString sql, QList<QVariant> data);

    /**
     * @description: 执行sql语句
     * @param {QString} sql 为sql语句
     * @return bool 执行成功返回true
     */
    bool ExecSql(QString sql);

    /**
     * @description: 执行sql block语句
     * @param {QString} sql 为sql语句
     * @param {QList<QVariantList>} tableData 为sql语句中的参数
     */
    void ExecAddSqlBlock(QString sql, QList<QVariantList> tableData);

    /**
     * @description: 执行带参数的sql语句
     * @param {QString} sql 为sql语句
     * @param {QList<QVariant>} data 为sql语句中的参数
     * @return bool 执行成功返回true
     */
    bool executeParameterizedSqlQuery(QString sql, QList<QVariant> data);

    void SetData(QString path);
    void ExecSqlFile(QString fileName);
    ~DataBase();

private:
    QString dbPassword;
    DataBasePrivate *DataBasepri = nullptr;
    DataBase(QObject *parent = nullptr, QString dataBaseName = "./.config/packdata", QString password = "");
};

#endif // DATABASE_H
