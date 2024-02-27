#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QDialog>
#include <QStandardItemModel>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSystemTrayIcon>

#include <QWebSocketServer>
#include <QWebSocket>

#include <QGraphicsView>

#include <QQueue>

#include "packbll.h"
#include "panelbll.h"
#include "conditionbll.h"

#include "panel.h"
#include "algorithm.h"
#include "global.h"

#include "globalhook.h"
#include "common/CustomerStatusBar.h"
#include "common/FullScreenMask.h"
#include "common/MyQueue.h"

#include "ModbusClient.h"

#include <QMainWindow>

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

    // 板件列表中的按钮
    void onBtnRemovePanelClicked();

private:
    void initForm();
    void initForm_UiInit();
    void initConfig();
    void initForm_PackDataBinding(bool isReload = false);

    QGraphicsView* getLayerView(Layer layer, int packageLength, int packageWidth, qreal scaleFactor);

    //
    void initForm_PanelDataBinding(bool isReload = false);
    void initForm_PanelDataPreview();
    void initForm_SettingDataBinding();

    std::tuple<int, int, int> handler4Barccode(QString barcode);
    void handler4PackBarccode();
    void handler4PanelBarccode();

    bool workFlow_WaitingForScan_ToleranceValues(PackageDto& pack);
    void sendFileToHotFolder(const PackageDto &package);

    // 自动流转
    void runFlow(PackageDto& packDto,  PackageDto::StatusEnum* targetStatus = nullptr);
    PackageDto::StatusEnum runFlow_send2PanelDockingStation(PackageDto& packDto);
    PackageDto::StatusEnum runFlow_WaitingForSend(PackageDto& packDto);


    // 处理socket client发送的数据
    bool parseSocketClientData(const QString socketClientData, QString clientIp);
    bool parseSocketClientData(const QByteArray &binaryMessage, QString clientIp);

    // 处理键盘钩子获取到的扫码数据
    void handleScannedData(const QString &data);
    void handleScannedData_RC(const QString &data); // 容差处理
    void handleScannedData_Barcode(const QString &data); // 条码信息处理

    // 包裹列表中的右键菜单
    void handlePackTableMenuAction(QAction *action, const QModelIndex &index);

    // 预分包页签中的导入板件列表
    void page_yfb_panelDataBinding();
    void page_yfb_tvAlgorithmPackages_DataBinding();
    void page_yfb_tvAlgorithmPanles_DataBinding(QList<Panel> panels);



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
    // 开始工作站服务
    void startMeasuringStationServer();
    bool send2PanelDockingStation(uint dbPackId, int panelDockingStationIndex);
    bool isAllowWrite2PanelDockingStation(int panelDockingStationIndex);
    int getScanEntryIndex(QString originIp);

    // 队列
    void initQueue();

private:
    QStandardItemModel *m_packModel;
    QString m_currentOrderNo;

    PackBLL *m_packBll;
    PanelBLL *m_panelBll;
    ConditionBLL *m_conditionBll;

    QStandardItemModel *m_tbAddValueConditionsModel;
    QStandardItemModel *m_tbPackTemplateModel;

    QStandardItemModel *m_tbImportPanelsModel; // 绑定到table中的数据
    QList<Panel> m_importPanels; // 导入板件列表

    QStandardItemModel *m_algorithmPackagesModel; //
    QList<PackageAO> m_algorithmPackages; //
    PackageAO m_panelsPackage;
    QList<Panel> m_panels;

    // tcp server
    QTcpServer *m_tcpServer;
    QList<QTcpSocket *> m_tcpClients;

    // web socket server
    QWebSocketServer *m_webSocketServer;
    QMap<QString, QWebSocket*> m_webSocketClients;

    // 键盘钩子
    GlobalHook *m_globalHook;

    // 状态栏
    CustomStatusBar* m_customStatusBar;

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

    void on_btnImport_clicked();

    void on_btnAlgorithm_clicked();

    void on_pushButton_clicked();

    void on_btnExport_clicked();

    void on_isOpenMeasuringStation_stateChanged(int arg1);

private:
    QSystemTrayIcon *trayIcon;
    Algorithm* m_algorithm;

    // 入口队列
    QList<MyQueue<PackageDto>> m_entryQueues; //TODO
    // 等待队列
    MyQueue<PackageDto> m_waitingQueue;
    // 轮询等待队列的定时器
    QTimer* m_waitingQueue_timer;
    // 轮询等待队列的定时器 是否作业中
    bool m_waitingQueue_timer_isProcess = false;

    // 和容差的对应关系
    QMap<QString, DimensionThresholds> m_orderThreshold;
    // 容差条件列表
    QList<ConditionDto> m_thresholdConditions;
    // 箱型条件列表
    QList<ConditionDto> m_packTemplateConditions;
    // 等待条件列表
    QList<ConditionDto> m_waitingConditions;

    const QString StatusBar_IconName_Socket = "socket";
    const QString StatusBar_IconName_Modbus = "plc";
    const QString StatusBar_IconName_Queue = "queue";

    //
    ModbusClient *m_modbusClient;
    QTimer* m_modbusClient_Timer;
    bool m_modbusClient_isProcess = false;


};

#endif // FRMMAIN_H
