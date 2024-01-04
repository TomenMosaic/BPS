#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
#include <QTextCodec>
#include <QSqlRecord>
#include <QSharedPointer>
#include <QTime>
#include <QFile>
#include "database.h"
#include "log.h"
#include "qjsonarray.h"

DataBase *DataBase::PdataBase = nullptr;

DataBase *DataBase::getDataBase(QObject *parent, QString dataBaseName, QString password)
{
    if(PdataBase == nullptr)
    {
        DataBase::PdataBase = new DataBase(parent,dataBaseName,password);
    }
    return DataBase::PdataBase;
}

QJsonObject DataBase::execQuery(QString cmd)
{
    QJsonObject obj;
    QSqlDatabase db= QSqlDatabase::database();
    QSqlQuery query(db);
    bool ok = query.exec(cmd);

    if(!ok)
    {
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"cmd:"<<cmd;
        qDebug()<<Q_FUNC_INFO<<__LINE__<<db.lastError()<<query.lastError().text();
    }
    else
    {
        QSqlRecord record = query.record();
        int count = record.count();
        QStringList fieldNames;
        for(int index=0;index<count;index++)
        {
            fieldNames.append(record.fieldName(index));
        }
        QJsonArray fieldList=QJsonArray::fromStringList(fieldNames);
        QJsonArray valueList;
        while(query.next())
        {
            QJsonArray recorArr;
            for(int index=0;index<count;index++ )
            {
                recorArr.append(query.value(index).toJsonValue());
            }
            valueList.append(recorArr);
        }
        obj.insert("field",fieldList);
        obj.insert("value",valueList);
    }
    query.clear();
    return obj;
}

QSqlQuery DataBase::query(QString cmd)
{
    QJsonObject obj;
    QSqlDatabase db= QSqlDatabase::database();
    QSqlQuery query(db);
    bool ok = query.exec(cmd);

    // 检查是否执行成功
    if (!ok) {
        // 错误处理: 可以记录错误信息，或者抛出异常等
        qWarning() << "query Error: " << query.lastError().text() << cmd;
        CLOG_ERROR(QString("query error: "+ query.lastError().text()).toUtf8());
    }

    return query;
}



void DataBase::updateCmd(QString cmd)
{
    emit newQueryCmd(cmd);
}

void DataBase::insertCmd(QString tableName, QStringList fields, QStringList values)
{
    assert(fields.length()==values.length());

    QList<QString>valueList;
    for(int index = 0;index<values.length();index++)
    {
        QString value = values.at(index);
        valueList.append("'"+value+"'");
    }
    QString cmd = QString("insert into %1(%2) values(%3)").arg(tableName,
                                                              fields.join(","),
                                                              valueList.join(","));
    emit newQueryCmd(cmd);
}

void DataBase::SetData(QString path)
{
    QDir dir(path);
    QDir dir_file(path);
    dir_file.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir_file.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList file_list = dir_file.entryInfoList(QDir::Files);

    for (int i = 0; i < file_list.size(); ++i)   //这个用来循环文件夹里文件
    {
        QFileInfo fileInfo = file_list.at(i);
        QSqlQuery query;

        QString tableName = QString(fileInfo.fileName());
        qDebug()<<Q_FUNC_INFO<<__LINE__<<"exec sql file:"<<tableName;
        CLOG_INFO(QString("exec sql file:%1").arg(tableName).toUtf8());
        QString sql;
        QFile file(path+"/"+tableName);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            in.setCodec("UTF8");
            sql = in.readAll();
            file.close();
        }
        //        sql = sql.arg(tableName);
        QStringList sl =sql.split(";");
        for (int i=0; i<sl.count(); i++)
        {
            QString s = sl.at(i);
            query.exec(s);
        }
        query.clear();
        file.remove();
    }

    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i != folder_list.size(); i++)   //这个用来寻找文件夹
    {
        QString namePath = folder_list.at(i).absoluteFilePath();
        QFileInfo folderInfo = folder_list.at(i);
        QStringList FolderList;
        SetData(namePath);   //找到文件夹进行递归
    }
}

void DataBase::ExecSqlFile(QString fileName)
{
    QSqlQuery query;
    QString sql;
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        in.setCodec("UTF8");
        sql = in.readAll();
        file.close();
    }

    QStringList sl =sql.split(";");
    for (int i=0; i<sl.count(); i++)
    {
        QString s = sl.at(i);
        bool ok =   query.exec(s);

        // 检查是否执行成功
        if (!ok) {
            // 错误处理: 可以记录错误信息，或者抛出异常等
            qWarning() << "ExecSqlFile Error: " << query.lastError().text();
            CLOG_ERROR(QString("ExecSqlFile error: "+ query.lastError().text()).toUtf8());
        }
    }
    query.clear();
}

DataBase::~DataBase()
{
    if(DataBasepri)
    {
        DataBasepri->stop();
        DataBasepri->wait();
        delete DataBasepri;
        DataBasepri = nullptr;
    }

}

DataBase::DataBase(QObject *parent, QString dataBaseName, QString password):QObject(parent)
{
    // 数据库文件是否存在
    QFile dbFile(dataBaseName);
    bool isExists = dbFile.exists();

    DataBasepri = new DataBasePrivate;
    DataBasepri->setDataBaseName(dataBaseName);
    connect(this,SIGNAL(newQueryCmd(QString)),DataBasepri,SLOT(addCmd(QString)),Qt::QueuedConnection);
    DataBasepri->start();

    QSqlDatabase db = QSqlDatabase::addDatabase("SQLITECIPHER"); // QSQLITE
    CLOG_INFO(QString("dataBaseName:%1").arg(dataBaseName).toUtf8());
    db.setDatabaseName(dataBaseName);
    dbPassword = password;
    db.setPassword(dbPassword);
    /*
    QSQLITE_USE_CIPHER=sqlcipher：指定用于加密的密码学库是 SQLCipher。SQLCipher 是一个 SQLite 的扩展，提供了全数据库加密功能。
    SQLCIPHER_LEGACY=1：启用与 SQLCipher 较早版本的兼容模式。这在打开用旧版本 SQLCipher 加密的数据库时可能是必需的。
    SQLCIPHER_LEGACY_PAGE_SIZE=4096：指定旧版本 SQLCipher 使用的页大小（字节为单位）。这与 SQLCIPHER_LEGACY=1 配置项一起使用，以确保可以正确地读取和写入用旧版本 SQLCipher 加密的数据库。
    */
    if (!isExists){
        // 不存在时创建密码
        db.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; SQLCIPHER_LEGACY=1; SQLCIPHER_LEGACY_PAGE_SIZE=4096; QSQLITE_CREATE_KEY");
    }else{
        db.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; SQLCIPHER_LEGACY=1; SQLCIPHER_LEGACY_PAGE_SIZE=4096");
    }

    //qDebug()<<Q_FUNC_INFO<<__LINE__<<" file exist:"<<isExists;
    bool isOpen = db.open();
    qDebug() << "db open: " << isOpen;

    if (!db.isOpen())
    {
        qWarning() << "Connection failed: " << db.lastError().driverText();
        CLOG_ERROR(QString("Connection failed: "+ db.lastError().driverText()).toUtf8());
        exit(-1);
    }

    if(isOpen&&!isExists)
    {
        //qDebug()<<"Create DB";
        //SetData("TablesSql");
    }
    else
    {
        qDebug()<<"db name:"<<db.databaseName();
    }
}

QList<QStringList> DataBase::selectDataFromBase(QString strSql)
{
    QSqlQuery query(strSql);

    QList<QStringList> dataList;

    while (query.next())
    {
        QStringList rowData ;
        int i = 0;
        QVariant temp;
        while ((temp = query.value(i++)).isValid())
        {
            rowData <<temp.toString();
        }
        dataList.append(rowData);
    }
    query.clear();
    return dataList;
}


bool DataBase::ExecSql(const QString& sql, const QMap<QString, QVariant>& params)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery query(db);
    query.prepare(sql);
    for (auto it = params.cbegin(); it != params.cend(); ++it) {
        query.bindValue(it.key(), it.value());
    }
    bool ok = query.exec();

    // 检查是否执行成功
    if (ok) {
        db.commit();
    } else {
        db.rollback();

        // 错误处理: 可以记录错误信息，或者抛出异常等
        qWarning() << "query Error: " << query.lastError().text() << sql;
        CLOG_ERROR(QString("query error: "+ query.lastError().text()).toUtf8());
    }

    query.clear();
    return ok;
}

bool DataBase::ExecSql(QString sql, QList<QVariant> data)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery query(db);
    query.prepare(sql);
    foreach (QVariant val, data)
    {
        query.addBindValue(val);
    }
    bool ok = query.exec();
    if (ok) {
        db.commit();
    } else {
        db.rollback();

        // 错误处理: 可以记录错误信息，或者抛出异常等
        qWarning() << "ExecSql Error: " << query.lastError().text();
        CLOG_ERROR(QString("ExecSql error: "+ query.lastError().text()).toUtf8());

    }
    query.clear();
    return ok;
}

bool DataBase::ExecSql(QString sql)
{
    QList<QVariant> data;
    return ExecSql(sql, data);
}

void DataBase::ExecAddSqlBlock(QString sql, QList<QVariantList> tableData)
{
    if(!tableData.isEmpty())
    {
        QSqlDatabase::database().transaction();
        QListIterator<QVariantList> itr(tableData);
        while(itr.hasNext())
        {
            QVariantList itemList = itr.next();
            QSqlQuery query;
            query.prepare(sql);
            foreach (QVariant val, itemList)
            {
                query.addBindValue(val);
            }
            query.exec();
            query.clear();
        }
        QSqlDatabase::database().commit();
    }
}

bool DataBase::executeParameterizedSqlQuery(QString sql, QList<QVariant> data)
{
    auto db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery query;

    query.prepare(sql);
    for (const auto& val : data) {
        query.addBindValue(val);
    }

    bool ok = query.exec();
    if (!ok) {
        qWarning() << "ExecEditSql Error: " << query.lastError().text();
        CLOG_ERROR(QString("ExecEditSql error: " + query.lastError().text()).toUtf8());
        db.rollback(); // 回滚事务
    } else {
        db.commit(); // 只有在成功的情况下提交事务
    }

    query.clear();
    return ok;
}

DataBasePrivate::DataBasePrivate(QObject *parent):QThread(parent)
{
    condition = new QWaitCondition;
    timer = new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    timer->start(1500);
}

void DataBasePrivate::run()
{
    QString connectionName = "insertSQL";
    QSqlDatabase db;
    if (!QSqlDatabase::contains("insertSQL"))
        db = QSqlDatabase::addDatabase("QSQLITE","insertSQL");  //第一次打开数据库时，连接数据库。
    else
        db = QSqlDatabase::database("insertSQL");
    //qDebug()<<Q_FUNC_INFO<<__LINE__<<" threadID:"<<QThread::currentThreadId()<<"  dataBaseName:"<<dataBaseName;
    db.setDatabaseName(dataBaseName);
    db.open();
    while(1)
    {
        mutex.lock();
        //没活的时候刚好让你休息就直接休息吧
        if(cmdList.isEmpty())
        {
            if(!isRun)
            {
                mutex.unlock();
                return;
            }
            //没活又不让你休息就等活
            else
                condition->wait(&mutex);
        }
        //有活的时候继续干活
        QStringList newCmdList = cmdList;
        cmdList.clear();
        mutex.unlock();
        //        QTime startTime = QTime::currentTime();
        db.transaction();
        for(int index =0;index<newCmdList.length();index++)
        {
            QString cmd = newCmdList.at(index);

            QSqlQuery query(db);
            bool ok = query.exec(cmd);
            if(!ok)
            {
                qDebug()<<Q_FUNC_INFO<<__LINE__<<cmd<<"insertCMD error:"<<query.lastError().text();
                CLOG_ERROR("insertCMD error:"+query.lastError().text().toUtf8());
            }
            //else
            //    qDebug()<<Q_FUNC_INFO<<__LINE__<<cmd<<"exec CMD success:";

            query.clear();
        }
        bool ok = db.commit();
        if(!ok)
        {
            qDebug()<<Q_FUNC_INFO<<__LINE__<<"exec CMDList:"<<newCmdList<<" error message:"<<db.lastError().text();
            CLOG_ERROR(QString("exec CMDList error: "+ db.lastError().text()).toUtf8());
        }
        //        QTime stopTime = QTime::currentTime();
        //        int elapsed = startTime.msecsTo(stopTime);
        //        qDebug()<<"QTime.currentTime ="<<elapsed<<"ms"<<"  num:"<<newCmdList.length();
    }
    db.close();
    QSqlDatabase::removeDatabase("insertSQL");
}

DataBasePrivate::~DataBasePrivate()
{
    if(condition)
    {
        delete condition;
        condition = nullptr;
    }
}

void DataBasePrivate::stop()
{
    isRun = false;
    condition->notify_all();
}

void DataBasePrivate::startRunning()
{
    isRun = true;
    this->start();
}

void DataBasePrivate::setDataBaseName(QString databaseName)
{
    this->dataBaseName = databaseName;
}

void DataBasePrivate::addCmd(QString cmd)
{
    mutex.lock();
    cmdList.append(cmd);
    mutex.unlock();
    condition->notify_all();
}

void DataBasePrivate::onTimeOut()
{
    if(!cmdList.isEmpty())
        condition->notify_all();
}
