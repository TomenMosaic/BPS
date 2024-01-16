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

        // 检测开始符号
        if (currentChar == '[') {
            inputBuffer.clear();
            lastInputTime = GetTickCount();
            instance->isActiveScan = true;  // 设置正在扫描标志
        } else if (currentChar == ']' && instance->isActiveScan) {
            // 检测结束符号且处于活动扫描状态
            if (!inputBuffer.empty()) {
                qDebug() << "send Scanned Data:" << QString::fromStdWString(inputBuffer);
                QMetaObject::invokeMethod(instance, "scannedDataReceived", Qt::QueuedConnection,
                                          Q_ARG(QString, QString::fromStdWString(inputBuffer)));
            }
            inputBuffer.clear();
            instance->isActiveScan = false; // 重置扫描标志
        } else if (instance->isActiveScan) {
            // 如果处于活动扫描状态，添加按键到缓冲区
            if (GetTickCount() - lastInputTime > 166) {
                // 如果输入延迟太长，清除缓冲区并重置扫描状态
                inputBuffer.clear();
                instance->isActiveScan = false;
            } else {
                inputBuffer += currentChar.toStdWString();
                lastInputTime = GetTickCount();
            }
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
