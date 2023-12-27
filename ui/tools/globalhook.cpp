// GlobalHook.cpp

#include "GlobalHook.h"
#include <QtDebug>
#include <iostream>

HHOOK GlobalHook::hHook = NULL;
std::wstring GlobalHook::inputBuffer;
DWORD GlobalHook::lastInputTime = 0;
GlobalHook *GlobalHook::instance = nullptr;

GlobalHook::GlobalHook() {
    instance = this;
}

GlobalHook::~GlobalHook() {
    releaseHook(); // 析构时释放钩子
}

void GlobalHook::setHook() {
    if (!hHook) {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0); // 安装钩子
    }
}

void GlobalHook::releaseHook() {
    if (hHook) {
        UnhookWindowsHookEx(hHook); // 移除钩子
        hHook = NULL;
    }
}

LRESULT CALLBACK GlobalHook::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

        TCHAR keyName[256];
        DWORD scanCode = kbdStruct->scanCode;
        // 包含扩展键标志
        scanCode |= (kbdStruct->flags & LLKHF_EXTENDED) ? 0xE000 : 0;
        GetKeyNameText(scanCode << 16, keyName, 256);
        auto currentChar = QString::fromWCharArray(keyName);

        if (currentChar == '[') { // 检测开始符号
            inputBuffer.clear();
            lastInputTime = GetTickCount();
        } else if (currentChar == ']') { // 检测结束符号
            if (instance) {
                qDebug() << "send Scanned Data:" << QString::fromStdWString(inputBuffer);
                // 发送信号
                QMetaObject::invokeMethod(instance, "scannedDataReceived", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdWString(inputBuffer)));
                inputBuffer.clear();
            }
        } else if (lastInputTime > 0 && GetTickCount() - lastInputTime > 166) {
            inputBuffer.clear();
            lastInputTime = 0;
        } else {
            lastInputTime = GetTickCount(); // 更新录入时间
            inputBuffer += currentChar.toStdWString(); // 添加按键到缓冲区
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
