// GlobalHook.h

#ifndef GLOBALHOOK_H
#define GLOBALHOOK_H

#include <windows.h>
#include <string>
#include <QObject>

// 全局钩子类，用于捕获键盘事件
class GlobalHook : public QObject {
    Q_OBJECT

public:
    GlobalHook();
    ~GlobalHook();
    void setHook(); // 设置钩子
    void releaseHook(); // 释放钩子

signals:
    void scannedDataReceived(const QString &data); // 扫描数据接收信号

private:
    static HHOOK hHook; // 钩子句柄
    static std::wstring inputBuffer; // 输入缓冲区
    static DWORD lastInputTime; // 上次输入时间
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam); // 键盘事件处理回调函数
    static GlobalHook *instance; // 该类的实例指针
    // static bool isInputBegin; // 添加一个标志变量

    bool isActiveScan = false; // 添加一个成员变量来跟踪扫描状态
};

#endif // GLOBALHOOK_H
