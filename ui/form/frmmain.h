#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QDialog>
#include <QStandardItemModel>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSystemTrayIcon>

#include <QWebSocketServer>
#include <QWebSocket>

#include <QQueue>

#include "packbll.h"
#include "panelbll.h"
#include "conditionbll.h"

#include "panel.h"
#include "algorithm.h"
#include "global.h"

#include "globalhook.h"

namespace Ui {
class frmMain;
}

struct DimensionThresholds {
    QString lengthThreshold;
    QString widthThreshold;
    QString heightThreshold;

    DimensionThresholds()
            : lengthThreshold(""), widthThreshold(""), heightThreshold("") {}

    bool hasValues(){
        return !lengthThreshold.isEmpty() || !widthThreshold.isEmpty() || !heightThreshold.isEmpty();
    }

};

class frmMain : public QDialog
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

public:
    QIcon setIconColor(QIcon icon, QColor color);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::frmMain *ui;

private slots:
    void buttonClick();

private slots:
    //void on_btnMenu_Min_clicked();
    //void on_btnMenu_Max_clicked();
    //void on_btnMenu_Close_clicked();

    void on_ButtonSend_clicked();
    void on_btnSearch_clicked();

    void on_txtPanelBarcode_Enter();
    void on_btnSearchPlate_clicked();

    // 包裹列表中的按钮
    void onBtnReSendClicked();
    void onBtnReprintClicked();

    // 板件列表中的按钮
    void onBtnRemovePanelClicked();

private:
    void initForm();
    void initForm_UiInit();
    void initConfig();
    void initForm_PackDataBinding(bool isReload = false);

    //
    void initForm_PanelDataBinding(bool isReload = false);
    void initForm_PanelDataPreview();
    void initForm_SettingDataBinding();

    std::tuple<int, int, int> handler4Barccode(QString barcode);
    void handler4PackBarccode();
    void handler4PanelBarccode();

    void sendFileToHotFolder(const Package &package);

    // 处理socket client发送的数据
    bool parseSocketClientData(const QString socketClientData);
    bool parseSocketClientData(const QByteArray &binaryMessage);

    // 处理键盘钩子获取到的扫码数据
    void handleScannedData(const QString &data);

private slots:
    // 开启 socket server
    void startSocketServer();

    //tcp server
    void handleSocketNewConnection();
    void onTcpReadyRead();

    //web socket server
    void handlerWebSocketNewConnection();
    // void onWebSocketTextMessageReceived(QWebSocket *clientSocket, const QString &message);

private:
    QStandardItemModel *m_packModel;
    PackBLL *m_packBll;
    PanelBLL *m_panelBll;
    ConditionBLL *m_conditionBll;

    QStandardItemModel *m_tbAddValueConditionsModel;
    QStandardItemModel *m_tbPackTemplateModel;

    Package m_panelsPackage;
    QList<Panel> m_panels;

    // tcp server
    QTcpServer *m_tcpServer;
    QList<QTcpSocket *> m_tcpClients;

    // web socket server
    QWebSocketServer *m_webSocketServer;
    QMap<QString, QWebSocket*> m_webSocketClients;

    // 键盘钩子
    GlobalHook *m_globalHook;

    // 添加任务到队列
    void enqueueTask(Package& pack);

    // 处理队列中的任务
    void processTasks();

protected:
    // 重写关闭事件
    void closeEvent(QCloseEvent *event) override {
        // 取消关闭并最小化到系统托盘
        if (trayIcon->isVisible()) {
            hide(); // 隐藏窗口
            event->ignore(); // 忽略关闭事件
        } else {
            event->accept(); // 允许关闭
        }
    }

    void paintEvent(QPaintEvent *event) override; // 声明重写paintEvent

private slots:
    // 处理系统托盘图标的激活事件
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            // 单击托盘图标时，显示主窗口
            showNormal();
            activateWindow(); // 将窗口带到前台
        }
    }

    void on_btnSendConfiguration_clicked();

    void on_isWaiting4Scan_stateChanged(int arg1);

    void on_btnInsert_AddValueCondition_clicked(bool checked);

    void on_btnInsert_PackTemplateCondition_clicked(bool checked);

    void on_btnRemove_AddValueCondition_clicked(bool checked);

    void on_btnRemove_PackTemplateCondition_clicked(bool checked);

private:
    QSystemTrayIcon *trayIcon;
    Algorithm* m_algorithm;

    // 等待队列
    QQueue<Package> m_waitingQueue;
    // 和预值的对应关系
    QMap<QString, DimensionThresholds> m_orderThreshold;
    // 预值条件列表
    QList<ConditionDto> m_thresholdConditions;
    // 箱型条件列表
    QList<ConditionDto> m_packTemplateConditions;
    // 等待条件列表
    QList<ConditionDto> m_waitingConditions;

};

#endif // FRMMAIN_H
