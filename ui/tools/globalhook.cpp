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
        DWORD vkCode = kbdStruct->vkCode; // 虚拟键码

        // 判断是否为功能键
        if (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL ||
                vkCode == VK_LSHIFT || vkCode == VK_RSHIFT ||
                vkCode == VK_LMENU || vkCode == VK_RMENU) {
            return CallNextHookEx(hHook, nCode, wParam, lParam); // 直接返回，忽略功能键
        }

        TCHAR keyName[256];
        DWORD scanCode = kbdStruct->scanCode;
        scanCode |= (kbdStruct->flags & LLKHF_EXTENDED) ? 0xE000 : 0;
        GetKeyNameText(scanCode << 16, keyName, 256);
        auto currentChar = QString::fromWCharArray(keyName);

        // 设置正在扫描标志
        if (!instance->isActiveScan && inputBuffer.empty()){
            instance->isActiveScan = true;
            lastInputTime = GetTickCount();
        }

        // 检测开始符号
        if (currentChar == '[') { // [...] 扫码开始
            inputBuffer.clear();
            lastInputTime = GetTickCount();
        } else if ((currentChar == ']' || currentChar == "Enter" || currentChar == "Tab")
                   && instance->isActiveScan) { // [...] 扫码结束
            // 检测结束符号且处于活动扫描状态
            if (!inputBuffer.empty()) {
                qDebug() << "send Scanned Data:" << QString::fromStdWString(inputBuffer);
                QMetaObject::invokeMethod(instance, "scannedDataReceived", Qt::QueuedConnection,
                                          Q_ARG(QString, QString::fromStdWString(inputBuffer)));
            }
            inputBuffer.clear();
            instance->isActiveScan = false; // 重置扫描标志
        } else { //            
            if (GetTickCount() - lastInputTime > 166) { // 如果大于阈值，键盘录入的数据要舍弃
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
