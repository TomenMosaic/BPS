#include "log.h"

#include <QApplication>
#include <QDir>
#include <QDate>
#include <QMetaEnum>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <stdarg.h>

#include <QDebug>
#include <sstream>

#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>

const char PATH_LogPath[] = "./logs";
const char Suffix[] = ".log";

#define MAX_LOG_LENGH 1024

bool CLog::isFileReady = false;
bool CLog::isRecord2File = true;
CLog::CLOG_LEVEL CLog::logLevel = CLog::RINFO;           //默认是info级

QFile localFile;
QMutex mutex;

CLog::CLog()
{
}

void CLog::setLogLevel(const CLog::CLOG_LEVEL &level)
{
    logLevel = level;
}

bool CLog::init(LogConfig &logConfig)
{
    isRecord2File = logConfig.isRecord2File;
    logLevel = (CLog::CLOG_LEVEL)logConfig.level;

    QString logDir = qApp->applicationDirPath() + QString(PATH_LogPath);
    if(createDir(logDir))
    {
        QString fileName = logDir + QDir::separator() + QDate::currentDate().toString("yyyy-MM-dd") + QString(Suffix);
        localFile.setFileName(fileName);
        if(localFile.open(QFile::WriteOnly|QFile::Append|QFile::Text))
        {
            isFileReady = true;
        }
    }else{
        rotateLogs(66, 66); // 滚动日志
    }
    return isFileReady;
}

// 滚动日志
void CLog::rotateLogs(int daysToKeep, int minimumLogFiles) {
    QDir logDir(qApp->applicationDirPath() + QString(PATH_LogPath));
    const auto logFiles = logDir.entryList(QStringList() << "*" + QString(Suffix), QDir::Files, QDir::Time);

    QDate dateThreshold = QDate::currentDate().addDays(-daysToKeep);

    int filesDeleted = 0;
    for (const QString &fileName : logFiles) {
        QFileInfo fileInfo(logDir.filePath(fileName));
        if (fileInfo.created().date() < dateThreshold && logFiles.size() - filesDeleted > minimumLogFiles) {
            logDir.remove(fileName);
            ++filesDeleted;
        }
    }
}

bool CLog::createDir(QString dirPath)
{
    QFileInfo fileInfo(dirPath);
    if(!fileInfo.exists())
    {
        QDir tmpDir;
        return tmpDir.mkpath(dirPath);
    }

    return true;
}

QString stacktrace_to_qstring() {
    const boost::stacktrace::stacktrace st;
    std::ostringstream oss;
    oss << st;
    std::string stacktrace_str = oss.str();
    return QString::fromStdString(stacktrace_str);
}

void CLog::log(CLOG_LEVEL nLevel, const char *fileDesc, const char *functionDesc, int lineNum, const char* data, ...)
{    
    QMutexLocker locker(&mutex);
    if(isFileReady && nLevel >= logLevel)
    {
        QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        if (!localFile.fileName().endsWith(currentDate + QString(Suffix))) {
            // 关闭当前文件并开启新的日志文件
            localFile.close();
            QString newFileName = qApp->applicationDirPath() + QString(PATH_LogPath) + QDir::separator() + currentDate + QString(Suffix);
            localFile.setFileName(newFileName);
            localFile.open(QFile::WriteOnly | QFile::Append | QFile::Text);
            rotateLogs(66, 66); // 滚动日志
        }

        QString recordInfo = QString("[%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));
        recordInfo.append(getLeveDesc(nLevel));

#ifndef QT_NO_DEBUG
        // recordInfo.append(QString("[%1:%2:%3]").arg(fileDesc).arg(functionDesc).arg(lineNum));
#endif
        va_list vlist;
        va_start(vlist,data);

        QByteArray byteArray;
        byteArray.resize(1);
#if defined(Q_OS_WIN)
        int recordLen = _vscprintf(data,vlist);
        byteArray.resize(recordLen);
#else
        byteArray.resize(1024);
#endif
        vsprintf(byteArray.data(),data,vlist);
        recordInfo.append(byteArray);
        va_end(vlist);

        if (nLevel == RERROR || nLevel == RFATAL){
            recordInfo.append("\n" + stacktrace_to_qstring());
        }

        recordInfo.append("\n");

        if(isRecord2File)
        {
            localFile.write(recordInfo.toLocal8Bit().data(),recordInfo.toLocal8Bit().length());
            localFile.flush();
            localFile.resize(localFile.size());
        }
        else
        {
            //qDebug()<<recordInfo;
        }
    }
}

QString CLog::getLeveDesc(CLog::CLOG_LEVEL level)
{
    static const QMap<CLOG_LEVEL, QString> levelDescriptions = {
        { RDEBUG,   "[DEBUG]" },
        { RINFO,    "[INFO]" },
        { RWARNING, "[WARN]" },
        { RERROR,   "[ERROR]" },
        { RFATAL,   "[FATAL]" }
    };
    return levelDescriptions.value(level);
}

