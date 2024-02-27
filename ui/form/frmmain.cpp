#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/tableview_controller.h"
#include "ExcelReader.h"

#include <QList>


namespace {
QString convertToRegex(const QString &pattern) {
    // 将 {Variable} 转换为 (?<variable>\\d+)
    QRegularExpression re("\\{(\\w+)\\}");
    QString regexPattern = pattern;

    QRegularExpressionMatchIterator i = re.globalMatch(pattern);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString variable = match.captured(1).toLower();
        QString replacement = "(?<" + variable + ">\\d+)";
        regexPattern.replace(match.captured(0), replacement);
    }

    return regexPattern;
}
}


frmMain::frmMain(QWidget *parent) : QDialog(parent),
    ui(new Ui::frmMain),
    m_tcpServer(new QTcpServer(this)), // 使用初始化列表来初始化tcpServer
    m_webSocketServer(new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this)) // 使用初始化列表来初始化 web socket server
{
    this->ui->setupUi(this);
    this->initForm(); // 初始化

    this->m_algorithm = new Algorithm(g_config->getDeviceConfig().maxLengthExceed,
                                      g_config->getDeviceConfig().maxWidthExceed,
                                      g_config->getDeviceConfig().maxWidth4Strip);

    this->ui->txtPackBarcode->installEventFilter(this);
    this->ui->txtPanelBarcode->installEventFilter(this);
    this->ui->stackedWidget->installEventFilter(this);

    // 键盘钩子
    this->m_globalHook = new GlobalHook();
    this->m_globalHook->setHook();
    QObject::connect(this->m_globalHook, &GlobalHook::scannedDataReceived, this, &frmMain::handleScannedData);

    // 处理待发送任务队列
    this->m_waitingQueue_timer = new QTimer(this);
    connect(this->m_waitingQueue_timer, &QTimer::timeout, this, [this](){
        if (this->m_waitingQueue_timer_isProcess){
            return;
        }
        this->m_waitingQueue_timer_isProcess = true;

        try {
            this->processTasks(); // 处理队列数据
        }  catch (const std::exception& e) {
            // 处理异常，记录错误等
            qWarning() << "Exception occurred:" << e.what();
        }

        this->m_waitingQueue_timer_isProcess = false;
    });
    this->m_waitingQueue_timer->start(500); // 延迟500毫秒处理
}

frmMain::~frmMain()
{    
    //
    if (this->m_tcpServer != nullptr){
        this->m_tcpServer->close();
    }
    if (this->m_webSocketServer != nullptr){
        this->m_webSocketServer->close();
    }
    qDeleteAll(m_tcpClients);
    qDeleteAll(m_webSocketClients);

    //
    delete ui;
}

// 重写 paintEvent， 设置水印
void frmMain::paintEvent(QPaintEvent *event) {
    QDialog::paintEvent(event); // 调用基类的paintEvent以正常绘制对话框

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(0.1);

    QFont font = painter.font();
    font.setPointSize(14);
    painter.setFont(font);

    QRect rect = this->rect();
    int stepX = 200; // 水印的水平间距
    int stepY = 100; // 水印的垂直间距

    // 水印旋转角度
    int rotationAngle = -30;

    // 循环绘制重复的水印文字
    for (int x = 0; x < rect.width(); x += stepX) {
        for (int y = 0; y < rect.height(); y += stepY) {
            painter.save(); // 保存当前画家状态
            painter.translate(x, y); // 移动坐标原点到当前绘制点
            painter.rotate(rotationAngle); // 旋转坐标系
            painter.drawText(0, 0, tr("伊索智能")); // 在旋转后的坐标系中绘制文本
            painter.restore(); // 恢复画家状态
        }
    }
}

bool frmMain::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->txtPackBarcode || watched == ui->txtPanelBarcode){
        //获取按键信号
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *k = static_cast<QKeyEvent *>(event);
            if(k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return)             //选定回车键(可自行定义其他按键)
            {
                if (watched == ui->txtPanelBarcode){
                    on_txtPanelBarcode_Enter();
                }else{
                    on_ButtonSend_clicked();               //链接槽信号
                }

                return true;
            }
        }
    }else{
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *k = static_cast<QKeyEvent *>(event);
            if(k->key() == Qt::Key_F5)
            {
                this->initForm_PackDataBinding(true);
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

// 初始化配置
void frmMain::initConfig() {
    QString filePath = QApplication::applicationDirPath()+DEFAULT_SETTING; // 配置文件路径
    g_config = new AppConfig(); // 初始化
    g_config->loadFromIni(filePath);

    // 清空，准备重新加载
    this->m_thresholdConditions.clear();
    this->m_waitingConditions.clear();
    this->m_packTemplateConditions.clear();

    // 从数据库中加载规则列表
    auto conditions = this->m_conditionBll->getRowList(nullptr);
    for (const ConditionDto& dto : qAsConst(conditions)){
        if (dto.Type == ConditionDto::TypeEnum::threshold){
            this->m_thresholdConditions.append(dto);
        } else if (dto.Type == ConditionDto::TypeEnum::waitingCondition){
            this->m_waitingConditions.append(dto);
        }else if (dto.Type == ConditionDto::TypeEnum::packTemplateCondition){
            this->m_packTemplateConditions.append(dto);
        }
    }
}

// 初始化数据库
void initializeDatabase(QObject *parent){
    // 数据库
    QString filePath = QApplication::applicationDirPath()+DEFAULT_DATABASE;
    g_db = DataBase::getDataBase(parent, filePath, DEFAULT_DATABASE_PASSWORD);
}

void frmMain::initForm()
{
    // 数据库初始化
    initializeDatabase(this);
    this->m_packBll=PackBLL::getInstance(this); // 包裹表
    this->m_panelBll=PanelBLL::getInstance(this); // 板件表
    this->m_conditionBll=ConditionBLL::getInstance(this); // 条件表

    // 加载配置（如果配置不存在就创建默认的配置文件，并保存到文件夹中；如果配置文件存在，就解析配置文件，转换为指定的class）
    initConfig();

    // UI初始化
    this->initForm_UiInit();

    // 数据绑定
    this->initForm_PackDataBinding();

    // 配置页面数据绑定
    this->initForm_SettingDataBinding();

    if (g_config->getWorkConfig().workMode == WorkModeEnum::socat){// socket server
        this->startSocketServer();
    }

    if (g_config->getMeasuringStationConfig().isOpen){ // 打开测量站
        this->startMeasuringStationServer();
    }

    // 初始化队列
    this->initQueue();
}

// 主窗体中处理菜单操作的槽函数
void frmMain::handlePackTableMenuAction(QAction *action, const QModelIndex &index) {
    if (!index.isValid()) {
        return; // 确保提供的索引是有效的
    }

    // 获取选中行的数据
    QModelIndex idIndex  = this->m_packModel->index(index.row(), PackBLL::PackColEnum::ID);
    int packId = this->m_packModel->data(idIndex).toInt();

    if (action->text() == "重新发送") {
        if (this->m_panelsPackage.id == packId){
            this->sendFileToHotFolder(this->m_panelsPackage);
        } else{
            qWarning() << QString("选中的包裹id（%1） != 缓存包裹id（%2）").
                          arg(QString::number(packId), QString::number(this->m_panelsPackage.id));
        }

    } else if (action->text() == "删除") {
        // 在队列中剔除
        this->m_waitingQueue.dequeueIf([packId](const PackageDto &x) {
            return x.id == packId;
        });
        for (int i = 0; i < this->m_entryQueues.size(); i ++){
            this->m_entryQueues[i].dequeueIf([packId](const PackageDto &x) {
                return x.id == packId;
            });
        }

        // 在数据中剔除
        this->m_packBll->remove(packId);
        this->initForm_PackDataBinding();
    } else if (action->text() == "详情") {
        // 处理操作2
    }
}

// 包裹列表
QList<QAction*> buildMyMenu(QTableView *view, const QModelIndex &index) {
    QList<QAction*> actions;
    actions.append(new QAction("重新发送", view));
    actions.append(new QAction("打印标签", view));
    actions.append(new QAction("删除", view));
    actions.append(new QAction("详情", view));
    return actions;
}

void frmMain::initForm_UiInit(){
    // 设置系统托盘图标
    this->trayIcon = new QSystemTrayIcon(this);
    this-> trayIcon->setIcon(QIcon(":/image/favicon.ico")); // 指定图标路径
    this-> trayIcon->setVisible(true);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &frmMain::onTrayIconActivated);// 连接托盘图标的激活信号以处理单击事件

    // 设置窗体
    QUIHelper::setFramelessForm(this);

    //ui->widgetMenu->setVisible(false);
    ui->widgetTitle->installEventFilter(this);
    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTop->setProperty("nav", "top");

    // 应用程序名称
    this->setWindowTitle(ui->labTitle->text());

    // 单独设置指示器大小
    int addWidth = 20;
    int addHeight = 10;
    int rbtnWidth = 15;
    int ckWidth = 13;
    int scrWidth = 12;
    int borderWidth = 3;

    QStringList qss;
    qss << QString("QComboBox::drop-down,QDateEdit::drop-down,QTimeEdit::drop-down,QDateTimeEdit::drop-down{width:%1px;}").arg(addWidth);
    qss << QString("QComboBox::down-arrow,QDateEdit[calendarPopup=\"true\"]::down-arrow,QTimeEdit[calendarPopup=\"true\"]::down-arrow,"
                   "QDateTimeEdit[calendarPopup=\"true\"]::down-arrow{width:%1px;height:%1px;right:2px;}").arg(addHeight);
    qss << QString("QRadioButton::indicator{width:%1px;height:%1px;}").arg(rbtnWidth);
    qss << QString("QCheckBox::indicator,QGroupBox::indicator,QTreeWidget::indicator,QListWidget::indicator{width:%1px;height:%1px;}").arg(ckWidth);
    qss << QString("QScrollBar:horizontal{min-height:%1px;border-radius:%2px;}QScrollBar::handle:horizontal{border-radius:%2px;}"
                   "QScrollBar:vertical{min-width:%1px;border-radius:%2px;}QScrollBar::handle:vertical{border-radius:%2px;}").arg(scrWidth).arg(scrWidth / 2);
    qss << QString("QWidget#widget_top>QToolButton:pressed,QWidget#widget_top>QToolButton:hover,"
                   "QWidget#widget_top>QToolButton:checked,QWidget#widget_top>QLabel:hover{"
                   "border-width:0px 0px %1px 0px;}").arg(borderWidth);
    qss << QString("QWidget#widgetleft>QPushButton:checked,QWidget#widgetleft>QToolButton:checked,"
                   "QWidget#widgetleft>QPushButton:pressed,QWidget#widgetleft>QToolButton:pressed{"
                   "border-width:0px 0px 0px %1px;}").arg(borderWidth);
    this->setStyleSheet(qss.join(""));

    // 设置顶部导航按钮
    QSize icoSize(32, 32); // icon 的尺寸
    int buttonMinWidth = 85; // 按钮的最小宽度
    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        btn->setIconSize(icoSize);// 设置按钮图标的尺寸
        btn->setMinimumWidth(buttonMinWidth);// 设置按钮的最小宽度
        btn->setCheckable(true);// 允许按钮处于选中和非选中状态

        //1. 判断文字数量是否为 2，如果是则增加文字间距
        QString buttonText = btn->text(); // 获取按钮上的文字
        if (buttonText.length() == 2) {
            QString spacedText = buttonText.mid(0, 1) + " " + buttonText.mid(1, 1);
            btn->setText(spacedText);
        }

        //2. 设置icon的颜色
        /* QIcon currentIcon = btn->icon(); // 获取按钮当前的图标
        btn->setIcon(setIconColor(currentIcon, (Qt::white)));

        //3. 设置样式表，设置按钮图标为白色，不同状态下的颜色
        btn->setStyleSheet("QToolButton { color: white; }"
                           "QToolButton:hover { color: #ccc; }"
                           "QToolButton:pressed { color: #ccc; }"
                           "QToolButton:checked { color: #ccc; }");*/

        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
    }

    // 默认选中第一个菜单
    ui->btnMenuPack->click();

    // 创建 MyController 实例
    QTableViewController *controller = new QTableViewController(this->ui->tvPackList, buildMyMenu);
    connect(controller, &QTableViewController::menuActionTriggered, this, &frmMain::handlePackTableMenuAction);

    // 为日志详情绑定点击事件
    this->ui->treePackLogs->setHeaderHidden(true);

    // 状态栏
    this->m_customStatusBar = new CustomStatusBar(this);
    this->m_customStatusBar->setInfoText("就绪");   // 设置初始消息
    this->ui->frmMain_VerticalLayout->addWidget(this->m_customStatusBar);
}


/*
*    QIcon icon    待修改的图标
*    QColor color  需修改的颜色
*    返回修改后的QIcon
*/
QIcon frmMain::setIconColor(QIcon icon, QColor color)
{
    QPixmap pixmap = icon.pixmap(QSize(64,64));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    QIcon colorIcon = QIcon(pixmap);
    return colorIcon;
}

void frmMain::buttonClick()
{
    QToolButton *b = (QToolButton *)sender();
    QString name = b->text();
    name.remove(QRegularExpression("\\s+")); // 移除中间的空格

    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        btn->setChecked(btn == b);
    }

    if (name == "包裹") {
        ui->stackedWidget->setCurrentIndex(0);
        // this->initForm_PackDataBinding(true);
    }
    else if (name == "设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "关于") {
        ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "预分包") {
    this->ui->stackedWidget->setCurrentIndex(2);
    }
}

std::tuple<int, int, int> frmMain::handler4Barccode(QString barcode){
    QString pattern = g_config->getWorkConfig().codeRegex;

    QString realPattern = convertToRegex(pattern);
    QRegularExpression re(realPattern);
    QRegularExpressionMatch match = re.match(barcode);

    if (match.hasMatch()) {
        int length = match.captured("length").toInt();
        int width = match.captured("width").toInt();
        int height = match.captured("height").toInt();

        qDebug() << "Length: " << length << ", Width: " << width << ", Height: " << height;

        return std::make_tuple(length, width, height);
    } else {
        qDebug()  << "No dimensions found in the text.";
    }
    return {};
}

// 处理包裹条码
void frmMain::handler4PackBarccode(){
    QString barcode = this->ui->txtPackBarcode->text();
    if (barcode.isEmpty()){
        return;
    }

    auto [length, width, height] = this->handler4Barccode(barcode);
    if (length > 0 && width > 0 && height > 0) {
        // 写入数据库
        this->m_packBll->insert("", length, width, height, PackageDto::PackTypeEnum::PackType_Socket);

        // 清空input
        this->ui->txtPackBarcode->clear();
    }
}


void frmMain::on_btnSearchPlate_clicked(){

}




