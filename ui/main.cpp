#include <QApplication>

#include "frmmain.h"
#include "appinit.h"
#include "quihelper.h"
#include "log.h"
#include "global.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
#include <QScreen>


void writePidToFile() {
    QFile file("app.pid");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << QCoreApplication::applicationPid();
    }
    file.close();
}

// 检查是否有运行中的实例，始终只能保持一个实例运行
bool checkAndTerminatePreviousInstance(const QString &processName) {
    static QSystemSemaphore semaphore("MyAppSemaphore", 1);  // 互斥信号量
    semaphore.acquire();

#ifndef Q_OS_WIN32
    // 在非Windows操作系统下使用共享内存来确保只有一个实例
    QSharedMemory nix_fix_shared_memory("MyAppSharedMemory");
    if (nix_fix_shared_memory.attach()) {
        semaphore.release();
        return false;
    }
    nix_fix_shared_memory.create(1);
#endif

    static QSharedMemory sharedMemory("MyAppUniqueKey");
    if (sharedMemory.attach()) {
        semaphore.release();
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, QObject::tr("已有实例运行"),
                                      QObject::tr("检测到应用程序已经在运行。\n"
                                                  "您想要关闭已运行的实例吗？"),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QFile file("app.pid");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                qint64 pid;
                in >> pid;
                file.close();

                QProcess::execute("taskkill", QStringList() << "/pid" << QString::number(pid) << "/f");
            }

            writePidToFile();  // 写入PID
        } else {
            return false;
        }
    } else {
        sharedMemory.create(1);
        semaphore.release();
        writePidToFile();  // 写入PID
    }
}


// 自定义消息处理函数
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    auto utf8Bytes = msg.toUtf8();

    // 根据消息类型添加不同的前缀
    switch (type) {
    case QtDebugMsg:
        CLOG_DEBUG(utf8Bytes);
        break;
    case QtInfoMsg:
        CLOG_INFO(utf8Bytes);
        break;
    case QtWarningMsg:
        CLOG_WARNING(utf8Bytes);
        break;
    case QtCriticalMsg:
        CLOG_ERROR(utf8Bytes);
        break;
    case QtFatalMsg:
        CLOG_FATAL(utf8Bytes);
        break;
    }

    // 如果是致命错误，关闭应用程序
    if (type == QtFatalMsg) {
        abort();
    }
}

//
int main(int argc, char *argv[])
{
    // log init
    CLog::LogConfig logConfig;
    logConfig.isRecord2File = true;
    logConfig.level = 0;
    CLog::init(logConfig);

    try {
        // 安装消息处理程序
        qInstallMessageHandler(customMessageHandler);

        // 初始化
        QUIHelper::initMain();
        QApplication app(argc, argv);

        // 保证应用程序单实例运行
        QString processName = "bps.exe"; // 进程名
        if (!checkAndTerminatePreviousInstance(processName)) {
            // 如果用户选择不关闭现有的实例或无法关闭现有的实例
            return 0;
        }

        AppInit::Instance()->start();
        QUIHelper::setFont();
        QUIHelper::setCode();

        // 应用程序名称
        app.setApplicationName("制箱预处理工作站");

        // 加载主窗体
        frmMain main;
        main.showMaximized();

        // add log
        CLOG_INFO(QString("app start.").toUtf8());

        return app.exec();
    } catch (const std::exception &e) {
        // 处理或记录异常
        qFatal( "Exception caught: %s", e.what());
    }
}

