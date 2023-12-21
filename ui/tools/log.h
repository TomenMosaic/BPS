#ifndef LOG_H
#define LOG_H

#include <QFile>

class  CLog : public QObject
{
    Q_OBJECT
public:

    /*!
     *  @brief 日志等级
     */
    enum CLOG_LEVEL
    {
        RDEBUG,               /*!<    调试  */
        RINFO,               /*!<    提示  */
        RWARNING,            /*!<    警告  */
        RERROR,               /*!<    错误  */
        RFATAL               /*!<    严重错误  */
    };
    Q_FLAG(CLOG_LEVEL)

    struct LogConfig
    {
        bool isRecord2File;     /*!< 是否记录到文件 */
        int level;              /*!< 记录日志等级，大于等于此level的日志将被记录 */
    };

    CLog();

    void setLogLevel(const CLog::CLOG_LEVEL & level);
    CLog::CLOG_LEVEL getLogLevel(){return logLevel;}

    static bool init(CLog::LogConfig & logConfig);

    static bool createDir(QString dirPath);

    static void log(CLOG_LEVEL nLevel,const char * fileDesc,const char * functionDesc, int lineNum,const char* data, ...);

private:
    static void rotateLogs(int daysToKeep, int minimumLogFiles);
    static QString getLeveDesc(CLOG_LEVEL level);

private:
    static bool isRecord2File;
    static bool isFileReady;
    static CLOG_LEVEL logLevel;
};

#ifdef Q_OS_WIN
#define FILE_SEPARATOR '\\'
#else
#define FILE_SEPARATOR '/'
#endif

#define FUNC_SEPARATOR '::'

#ifndef QT_NO_DEBUG
#define __FILENAME__ (strrchr(__FILE__, FILE_SEPARATOR) ? (strrchr(__FILE__, FILE_SEPARATOR) + 1):__FILE__)
#define __FUNNAME__ (strrchr(__FUNCTION__,FUNC_SEPARATOR)?(strrchr(__FUNCTION__, FUNC_SEPARATOR) + 1):__FUNCTION__)
#else
#define __FILENAME__ NULL
#define __FUNNAME__ NULL
#endif

#define CLOG_DEBUG(...)      CLog::log(CLog::RDEBUG,__FILENAME__,__FUNNAME__,__LINE__, __VA_ARGS__)
#define CLOG_INFO(...)       CLog::log(CLog::RINFO,__FILENAME__,__FUNNAME__,__LINE__, __VA_ARGS__)
#define CLOG_WARNING(...)    CLog::log(CLog::RWARNING, __FILENAME__,__FUNNAME__, __LINE__,__VA_ARGS__)
#define CLOG_ERROR(...)      CLog::log(CLog::RERROR,__FILENAME__,__FUNNAME__,__LINE__,__VA_ARGS__)
#define CLOG_FATAL(...)      CLog::log(CLog::RFATAL,__FILENAME__,__FUNNAME__,__LINE__, __VA_ARGS__)

#endif // LOG_H

