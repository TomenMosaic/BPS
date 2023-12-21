#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"
#include "packbll.h"

#include <QList>
#include <QScriptEngine>

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
    ui->setupUi(this);
    this->initForm(); // 初始化

    this->m_algorithm = new Algorithm(g_config->getDeviceConfig().maxLengthExceed, g_config->getDeviceConfig().maxWidthExceed);

    ui->txtPackBarcode->installEventFilter(this);
    ui->txtPanelBarcode->installEventFilter(this);
    ui->stackedWidget->installEventFilter(this);
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
            if(k->key() == Qt::Key_F5)             //选定回车键(可自行定义其他按键)
            {
                this->initForm_PackDataBinding(true);
                return true;
            }
        }

        /* else if (event->type() == QEvent::MouseButtonDblClick) {
            if (watched == ui->widgetTitle) {
                on_btnMenu_Max_clicked();
                return true;
            }
        }*/
    }

    return QWidget::eventFilter(watched, event);
}


// 初始化配置
void initializeConfig(QObject *parent) {
    QString filePath = QApplication::applicationDirPath()+DEFAULT_SETTING; // 配置文件路径
    g_config = new AppConfig(); // 初始化
    g_config->loadFromIni(filePath);
}

// 初始化数据库
void initializeDatabase(QObject *parent){
    // 数据库
    QString filePath = QApplication::applicationDirPath()+DEFAULT_DATABASE;
    g_db = DataBase::getDataBase(parent, filePath, DEFAULT_DATABASE_PASSWORD);
}

void frmMain::initForm()
{
    // 加载配置（如果配置不存在就创建默认的配置文件，并保存到文件夹中；如果配置文件存在，就解析配置文件，转换为指定的class）
    initializeConfig(this);

    // 数据库初始化
    initializeDatabase(this);

    // UI初始化
    this->initForm_UiInit();

    // 数据绑定
    this->m_packBll=PackBLL::getInstance(this); // 包裹表
    this->initForm_PackDataBinding();
    this->m_panelBll=PanelBLL::getInstance(this); // 板件表

    // socket server
    this->startSocketServer();
}

void frmMain::initForm_PackDataBinding(bool isReload){
    ui->tvPackList->verticalHeader()->setVisible(false); // 显示表头

    m_packModel = new QStandardItemModel(this);
    m_packModel->setHorizontalHeaderLabels({ "包号","尺寸", "操作"}); // 列头
    ui->tvPackList->setModel(m_packModel);

    m_packModel->setRowCount(0); // ??
    QList<QSharedPointer<Row>> packList = m_packBll->getRowList(isReload);
    for(int index = 0;index<packList.length();index++)
    {
        QSharedPointer<Row>row = packList.at(index); // row
        QList<QStandardItem*>itemList; // 成员
        int colIndex = 0;

        // ID
       /* QStandardItem *idItem = new QStandardItem(row->data(PackBLL::ID).toString());
        idItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, idItem);
        colIndex++;*/

        // no
        QStandardItem *noItem = new QStandardItem(row->data(PackBLL::No).toString());
        noItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, noItem);
        colIndex++;

        // size
        QString formattedDimensions = QString("%1 x %2 x %3")
                .arg(row->data(PackBLL::Length).toString(),
                     row->data(PackBLL::Width).toString(),
                     row->data(PackBLL::Height).toString());
        QStandardItem *dimensionItem = new QStandardItem(formattedDimensions);
        dimensionItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, dimensionItem);
        colIndex++;

        // status
        PackBLL::StatusEnum statusValue = static_cast<PackBLL::StatusEnum>(row->data(PackBLL::Status).toInt()); // 转换为枚举值
       /* QString statusText = m_packBll->statusEnumToString(statusValue); // 转换为枚举对应的中文
        QStandardItem *statusItem = new QStandardItem(statusText);
        statusItem->setTextAlignment(Qt::AlignCenter); // 居中
        itemList.insert(colIndex, statusItem);
        colIndex++;*/

        // create time
     /*   QString strCreateTime = row->data(PackBLL::CreateTime).toDateTime().toString("MM-dd HH:mm:ss");
        QStandardItem *createTimeItem = new QStandardItem(strCreateTime);
        createTimeItem->setTextAlignment(Qt::AlignCenter); // 居中
        itemList.insert(colIndex, createTimeItem);
        colIndex++; */

        m_packModel->appendRow(itemList);

        // 操作按钮：重新发送 / 打印
        QWidget *container = new QWidget();// 创建一个容器小部件
        QHBoxLayout *layout = new QHBoxLayout(container);// 创建一个水平布局
        layout->setContentsMargins(3, 3, 3, 3); // 设置最小边距

        // 重新发送
        if (statusValue == PackBLL::StatusEnum::Status_Sent || statusValue == PackBLL::StatusEnum::Status_Finish){
            QPushButton *btnReSend = new QPushButton("重新发送");
            btnReSend->setObjectName(QString::number(row->data(PackBLL::ID).toInt()));
            QObject::connect(btnReSend, &QPushButton::clicked, this, &frmMain::onBtnReSendClicked);
            layout->addWidget(btnReSend);
        }

        // 打印
        if (g_config->getPrinterConfig().isEnable){
            QPushButton *btnReprint = new QPushButton("打印");
            btnReprint->setObjectName(QString::number(row->data(PackBLL::ID).toInt()));
            QObject::connect(btnReprint, &QPushButton::clicked, this, &frmMain::onBtnReprintClicked);
            layout->addWidget(btnReprint);
        }

        layout->setAlignment(Qt::AlignCenter);
        layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        ui->tvPackList->setIndexWidget(m_packModel->index(m_packModel->rowCount()-1, colIndex), container);

    }

    ui->tvPackList->resizeColumnsToContents(); // 根据内容调整列
    ui->tvPackList->resizeRowsToContents(); // 这会调整行高以适应内容
    ui->tvPackList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch
    ui->tvPackList->horizontalHeader()->setSectionResizeMode(m_packModel->columnCount() - 1, QHeaderView::ResizeToContents); // 按钮列使用 ResizeToContents


    // pack table
    this->ui->tvPackList->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this->ui->tvPackList->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &previous) {
        int rowIndex = current.row();
        if (rowIndex < 0 || current.row() == previous.row()){
            return ;
        }

        // 当前选中行
        QSharedPointer<Row> newRow = this->m_packBll->getRowList().at(rowIndex);
        int packId = newRow->data(PackBLL::ID).toInt();
        Package currentPackage;
        currentPackage.no = newRow->data(PackBLL::No).toString();
        currentPackage.customerName = newRow->data(PackBLL::CustomerName).toString();
        currentPackage.length = newRow->data(PackBLL::Length).toInt();
        currentPackage.width = newRow->data(PackBLL::Width).toInt();
        currentPackage.height = newRow->data(PackBLL::Height).toInt();
        QString createTime = newRow->data(PackBLL::CreateTime).toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString msg = QString("id: %1， 包裹号: %2， 长： %3， 宽： %4， 高： %5， 创建时间：%6 \n %7").
                            arg(packId).
                            arg(currentPackage.no).
                            arg(currentPackage.length).
                            arg(currentPackage.width).
                            arg(currentPackage.height).
                            arg(createTime).
                            arg(newRow->data(PackBLL::Logs).toString());
        this->ui->lblPackInfo->setText(msg);

        // 查找关联的板件列表
        this->m_panels = this->m_panelBll->getPanelsByPackId(packId);
        for (int i = 0; i < this->m_panels.size(); ++i) {
            Panel& panel = this->m_panels[i];
            Layer* layer = currentPackage.single(panel.layerNumber);
            if (layer == nullptr) {
                Layer newLayer;
                newLayer.addPanel(panel);
                currentPackage.addLayer(newLayer);
            } else {
                layer->addPanel(panel);
            }
        }

        // 赋值给当前包裹
        this->m_panelsPackage = currentPackage;
        // 重新加载table&preview
        this->initForm_PanelDataBinding();
        this->initForm_PanelDataPreview();
    });
}

void frmMain::initForm_PanelDataBinding(bool isReload){
    // table ------
    this->ui->tblPanels->verticalHeader()->setVisible(false); // 显示表头

    QStandardItemModel *panelModel = new QStandardItemModel(this);
    panelModel->setHorizontalHeaderLabels({"ID", "尺寸（单位：mm）", "创建时间", "层", "是否旋转", "操作"}); // 列头
    this->ui->tblPanels->setModel(panelModel);

    panelModel->setRowCount(0); // ??
    for(int index = 0;index<this->m_panels.length();index++)
    {
        Panel panel = this->m_panels[index]; // 板件
        QList<QStandardItem*>itemList; // 成员
        int columnIndex = 0;

        // ID
        QStandardItem *idItem = new QStandardItem(QString::number(panel.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, idItem);
        columnIndex++;

        // size
        QString formattedDimensions = QString("%1 x %2 x %3")
                .arg(QString::number(panel.length),
                     QString::number(panel.width),
                     QString::number(panel.height));
        QStandardItem *dimensionItem = new QStandardItem(formattedDimensions);
        dimensionItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, dimensionItem);
        columnIndex++;

        // create time
        QStandardItem *createTimeItem = new QStandardItem(panel.createTime.toString("yy-MM-dd HH:mm:ss"));
        createTimeItem->setTextAlignment(Qt::AlignCenter); // 居中
        itemList.insert(columnIndex, createTimeItem);
        columnIndex++;

        // layer
        QStandardItem *layerItem = new QStandardItem(QString::number(panel.layerNumber));
        layerItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, layerItem);
        columnIndex++;

        // 是否旋转
        QStandardItem *rotationItem = new QStandardItem(panel.rotated ? "是" : "否");
        rotationItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, rotationItem);
        columnIndex++;

        panelModel->appendRow(itemList);

        // 操作按钮
        QWidget *container = new QWidget();// 创建一个容器小部件
        QHBoxLayout *layout = new QHBoxLayout(container);// 创建一个水平布局
        layout->setContentsMargins(3, 3, 3, 3); // 设置最小边距

        QPushButton *btnRemove = new QPushButton("删除");
        btnRemove->setObjectName(QString::number(panel.id));
        QObject::connect(btnRemove, &QPushButton::clicked, this, &frmMain::onBtnRemovePanelClicked);
        layout->addWidget(btnRemove);

        layout->setAlignment(Qt::AlignCenter);
        layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        this->ui->tblPanels->setIndexWidget(panelModel->index(panelModel->rowCount()-1, columnIndex), container);

    }

    this->ui->tblPanels->resizeColumnsToContents(); // 根据内容调整列
    this->ui->tblPanels->resizeRowsToContents(); // 这会调整行高以适应内容
    this->ui->tblPanels->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch
    this->ui->tblPanels->horizontalHeader()->setSectionResizeMode(panelModel->columnCount()-1, QHeaderView::ResizeToContents); // 按钮列使用 ResizeToContents

}

void frmMain::initForm_PanelDataPreview() {
    // 清空 QTabWidget 中的所有 tabs
    this->ui->tabLayerPreview->clear();

    // 获取 tab 的容器的大小
    QSize tabSize = this->ui->tabLayerPreview->size();

    // 计算缩放因子
    qreal scaleX = tabSize.width() / qreal(this->m_panelsPackage.length);
    qreal scaleY = tabSize.height() / qreal(this->m_panelsPackage.width);
    qreal scaleFactor = qMin(scaleX, scaleY) * 0.94;

    // 遍历层，在tab中创建新的tab，tab的内容是预览图
    for (int i = 0; i < m_panelsPackage.layers.size(); ++i) {
        const Layer &layer = m_panelsPackage.layers[i];

        // 为每层创建一个 QGraphicsScene 和 QGraphicsView
        QGraphicsScene *scene = new QGraphicsScene();
        QGraphicsView *view = new QGraphicsView(scene);
        view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 设置为扩展策略

        // 当有多张板件的时候，创建底图矩形，示意当前层的大小
        if (layer.getUsedArea() < this->m_panelsPackage.length * this->m_panelsPackage.width) {
            QGraphicsRectItem *baseRectItem = new QGraphicsRectItem(0, 0,
                                                                    this->m_panelsPackage.length,
                                                                    this->m_panelsPackage.width);
            baseRectItem->setPen(QPen(Qt::red, 2)); // 1像素红色边框
            baseRectItem->setBrush(QBrush(Qt::lightGray)); // 浅灰色的背景
            scene->addItem(baseRectItem);
        }

        // 创建每个板件的图形矩形
        for (const Panel* panel : layer.panels) {
            int x = panel->position.x();
            int y = panel->position.y();

            QString text = QString("%1; %2x%3x%4%5")
                    .arg(panel->id)
                    .arg(panel->length)
                    .arg(panel->width)
                    .arg(panel->height)
                    .arg(panel->rotated ? "; r" : "");

            // 创建矩形，位置和大小匹配板件的坐标和尺寸，设置边框和背景色
            QGraphicsRectItem *rectItem ;
            if (panel->rotated) {// 如果板件旋转，长宽对调
                rectItem = new QGraphicsRectItem(x, y, panel->width, panel->length);
            }else{
                rectItem = new QGraphicsRectItem(x, y, panel->length, panel->width);
            }
            rectItem->setPen(QPen(Qt::black, 1)); // 1像素黑色边框
            rectItem->setBrush(QBrush(Qt::green)); //
            rectItem->setToolTip(text+QString("; %1,%2").arg(x).arg(y)); // tooltip
            scene->addItem(rectItem);

            // 在矩形上显示板件信息
            QGraphicsTextItem *textItem = scene->addText(text);
            QFont font;
            font.setPixelSize(14);  // 设置字体大小为 14 像素
            textItem->setFont(font);  // 应用字体
            textItem->setDefaultTextColor(Qt::black);  // 设置文本颜色
            //QRectF rect = textItem->boundingRect();
            textItem->setPos(x, y);  // 设置文本项的位置
        }

        // 设置 QGraphicsView 缩放
        view->scale(scaleFactor, scaleFactor);
        // view->setSceneRect(0, 0, this->m_panelsPackage.length, this->m_panelsPackage.width);
        // view->setBackgroundBrush(QBrush(Qt::lightGray));

        // 创建每个 tab 的页面 widget
        QWidget *tabPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(tabPage);
        layout->setContentsMargins(0, 0, 0, 0); // 设置边距为 0
        layout->addWidget(view); // 将 view 添加到 tabPage 的布局中
        tabPage->setLayout(layout);

        // 设置 QGraphicsView 缩放
        view->setSceneRect(scene->itemsBoundingRect()); // 调整视图的场景矩形以适应所有项的边界
        view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio); // 缩放视图以适应所有内容

        // 添加新的 tab，页签名字为层号
        QString tabLabel = QString("第 %1 层").arg(i + 1);
        this->ui->tabLayerPreview->addTab(tabPage, tabLabel);
    }
}

void frmMain::onBtnReSendClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        int id = button->objectName().toInt();
        QList<QSharedPointer<Row>> packList = m_packBll->getRowList();
        for(int index = 0;index<packList.length();index++)
        {
            QSharedPointer<Row>row = packList.at(index); // row

            // 是否为对应的记录
            int tmpId = row->data(PackBLL::ID).toInt();
            if (tmpId != id){
                continue;
            }

            // 包裹基本数据
            Package pack ;
            pack.id = id;
            pack.no =  row->data(PackBLL::No).toString();
            pack.length = row->data(PackBLL::Length).toInt();
            pack.width = row->data(PackBLL::Width).toInt();
            pack.height = row->data(PackBLL::Height).toInt();
            pack.customerName = row->data(PackBLL::CustomerName).toString();

            // 发送包裹数据
            this->sendFileToHotFolder(pack);

            // 重置数据
            this->initForm_PackDataBinding();

            break;
        }

    }
}

void frmMain::onBtnRemovePanelClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        int id = button->objectName().toInt();

        // 剔除某个板件，重新计算
        for (QList<Panel>::iterator it = this->m_panels.begin(); it != this->m_panels.end(); ) {
            if (it->id == id) {
                it = this->m_panels.erase(it);  // 移除元素并更新迭代器位置
            } else {
                ++it;
            }
        }

        //
        this->m_panelsPackage =  m_algorithm->createLayers(this->m_panels); // 重新计算，并赋值给私有变量
        this->initForm_PanelDataBinding();
        this->initForm_PanelDataPreview();
    }
}

void frmMain::onBtnReprintClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        //TODO 发送打印数据到标签打印机

        int id = button->objectName().toInt();
        this->m_packBll->finishPrint(id);

        this->initForm_PackDataBinding();
    }
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
    /*else if (name == "板件") {
        ui->stackedWidget->setCurrentIndex(1);
        this->initForm_PanelDataBinding();
        this->initForm_PanelDataPreview();
    } else if (name == "历史") {
        ui->stackedWidget->setCurrentIndex(2);
    } */
    else if (name == "设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "关于") {
        ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "退出") {
        exit(0);
    }
}

/*
void frmMain::on_btnMenu_Min_clicked()
{
    showMinimized();
}

void frmMain::on_btnMenu_Max_clicked()
{
    static bool max = false;
    static QRect location = this->geometry();

    if (max) {
        this->setGeometry(location);
    } else {
        location = this->geometry();
        this->setGeometry(QUIHelper::getScreenRect());
    }

    this->setProperty("canMove", max);
    max = !max;
}

void frmMain::on_btnMenu_Close_clicked()
{
    close();
}*/

void frmMain::on_btnSearch_clicked()
{
    this->handler4PackBarccode();
    initForm_PackDataBinding();
}

//信号与槽  - 链接显示和输入文本框信息
void frmMain::on_ButtonSend_clicked()
{
    this->handler4PackBarccode();
    this->initForm_PackDataBinding();
}

// 板件条码中识别尺寸
void frmMain::on_txtPanelBarcode_Enter(){
    this->handler4PanelBarccode();
    this->m_panelsPackage = this->m_algorithm->createLayers(this->m_panels); // 重新计算，并赋值给私有变量

    this->initForm_PanelDataBinding();
    this->initForm_PanelDataPreview();
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
    QString barcode = ui->txtPackBarcode->text();
    if (barcode.isEmpty()){
        return;
    }

    auto [length, width, height] = this->handler4Barccode(barcode);
            if (length > 0 && width > 0 && height > 0) {
        // 写入数据库
        m_packBll->insert("", length, width, height, PackBLL::PackType_Input);

        // 清空input
        ui->txtPackBarcode->clear();
    }
}

// 处理板件条码
void frmMain::handler4PanelBarccode()
{
    QString barcode = ui->txtPanelBarcode->text();
    if (barcode.isEmpty()){
        return;
    }

    auto [length, width, height] = this->handler4Barccode(barcode);
            if (length > 0 && width > 0 && height > 0) {
        int id = 1;
        if (!this->m_panels.isEmpty()){
            auto maxIt = std::max_element(this->m_panels.begin(), this->m_panels.end(),
                                          [](const Panel &a, const Panel &b) { return a.id < b.id; });
            id = maxIt->id + 1;
        }
        Panel panel(id, length, width, height, "");
        this->m_panels.append(panel);

        // 清空input
        ui->txtPanelBarcode->clear();
    }
}

void frmMain::on_btnSearchPlate_clicked(){

}

void frmMain::startSocketServer()
{
    bool isConnect = false;
    if (g_config->getWorkConfig().isSocketServer){
        if (g_config->getWorkConfig().socketServerType == SocketServerTypeEnum::web){
            // 设置和启动WebSocket服务器
            if (!this->m_webSocketServer->listen(QHostAddress::Any, g_config->getWorkConfig().socketPort)) {
                qDebug() << "web socket server could not start: " << this->m_webSocketServer->errorString();
            }else {
                qDebug() << "web socket server started on " << g_config->getWorkConfig().socketPort;
                connect(this->m_webSocketServer, &QWebSocketServer::newConnection, this, &frmMain::handlerWebSocketNewConnection);
            }
        }else{
            if (!this->m_tcpServer->listen(QHostAddress::Any, g_config->getWorkConfig().socketPort)) {
                // 处理错误，例如显示一个消息框或写入日志
                qDebug() << "tcp server could not start: " << m_tcpServer->errorString();
            } else {
                qDebug() << "tcp server started on " << g_config->getWorkConfig().socketPort;
                connect(m_tcpServer, &QTcpServer::newConnection, this, &frmMain::handleSocketNewConnection);
            }
        }
    }

}

void frmMain::handleSocketNewConnection()
{
    QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
    this->m_tcpClients.append(clientSocket);
    connect(clientSocket, &QTcpSocket::readyRead, this, &frmMain::onTcpReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
}

void frmMain::onTcpReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        QByteArray data = clientSocket->readAll();
        qDebug() << "Received data:" << data; // 打印数据

        //
        this->parseSocketClientData(data);

        // 根据需要发送响应给客户端
        clientSocket->write("ok");
    }
}

void frmMain::handlerWebSocketNewConnection()
{
    if (!m_webSocketServer->hasPendingConnections()) {
        qWarning() << "No pending connections to handle.";
        return;
    }

    QWebSocket *clientSocket = m_webSocketServer->nextPendingConnection();
    if (clientSocket) {
        QString clientIp = clientSocket->peerAddress().toString();
        qDebug() << "New WebSocket client connected:" << clientIp;
        m_webSocketClients[clientIp] = clientSocket;  // 使用映射存储客户端，以IP为键

        connect(clientSocket, &QWebSocket::textMessageReceived, this, [this, clientSocket](const QString &message) {
            QString clientIp = clientSocket->peerAddress().toString();
            qDebug() << "Client:" << clientIp << ", Received data:" << message; // 打印数据

            // 现在您可以知道是哪个客户端发送的消息
            bool isSuccess = this->parseSocketClientData(message);
            if (!isSuccess) {
                qWarning() << "json parse error from client:" << clientIp;
            }
        });

        connect(clientSocket, &QWebSocket::binaryMessageReceived, this, [this, clientSocket](const QByteArray &binaryMessage) {
            QString clientIp = clientSocket->peerAddress().toString();
            qDebug() << "Client:" << clientIp << ", Received data:" << binaryMessage.size(); // 打印数据

            // 现在您可以知道是哪个客户端发送的消息
            bool isSuccess = this->parseSocketClientData(binaryMessage);
            if (!isSuccess) {
                qWarning() << "json parse error from client:" << clientIp;
            }
        });

        connect(clientSocket, &QWebSocket::disconnected, this, [this, clientIp, clientSocket]() {
            qDebug() << "Client disconnected:" << clientIp;
            clientSocket->deleteLater();
            m_webSocketClients.remove(clientIp);  // 断开连接时移除客户端
        });
    } else {
        qWarning() << "Failed to retrieve client socket from the server.";
    }
}

void frmMain::sendFileToHotFolder(const Package &originPackage) {
    QDateTime now = QDateTime::currentDateTime();
    Package package = originPackage;
    QString packTemaplte = g_config->getPackTemplateConfig().defaultTemplate;

    // 查找阈值
    QString message;
    foreach(const auto threshold, g_config->getDeviceConfig().thresholds){
        QString script = originPackage.getScript(threshold.condition);

        // 执行字符串脚本
        QScriptEngine engine;
        bool result = engine.evaluate(script).toBool();
        if (result){
            // 基本的阈值
            int tLength = threshold.length;
            int tWidth = threshold.width;
            int tHeight = threshold.height;

            // 表达式
            if (!threshold.lengthExpression.isEmpty()){
                QString tmpScript = originPackage.getScript(threshold.lengthExpression);
                qint32 tmpLength = engine.evaluate(tmpScript).toInt32();
                tLength += tmpLength;
            }
            if (!threshold.widthExpression.isEmpty()){
                QString tmpScript = originPackage.getScript(threshold.widthExpression);
                qint32 tmpWidth = engine.evaluate(tmpScript).toInt32();
                tWidth += tmpWidth;
            }
            if (!threshold.heightExpression.isEmpty()){
                QString tmpScript = originPackage.getScript(threshold.heightExpression);
                qint32 tmpHeight = engine.evaluate(tmpScript).toInt32();
                tHeight += tmpHeight;
            }

            package.length += tLength;
            package.width += tWidth;
            package.height += tHeight;

            if (!threshold.packTemplate.isEmpty()) {
                packTemaplte = threshold.packTemplate;
            }

            // 日志
            QList<QString> condtionMsgs;
            condtionMsgs.append("threshold name:"+threshold.name);
            condtionMsgs.append("condition:"+threshold.condition);
            if (!threshold.lengthExpression.isEmpty()){
                condtionMsgs.append("length Expression:"+threshold.lengthExpression);
            }
            if (!threshold.widthExpression.isEmpty()){
                condtionMsgs.append("width Expression:"+threshold.widthExpression);
            }
            if (!threshold.heightExpression.isEmpty()){
                condtionMsgs.append("height Expression:"+threshold.heightExpression);
            }
            QList<QString> resultMessages;
            if (tLength > 0){
                 resultMessages.append("length + "+QString::number(tLength));
            }
            if (tWidth > 0){
                 resultMessages.append("width + "+QString::number(tWidth));
            }
            if (tHeight > 0){
                 resultMessages.append("height + "+QString::number(tHeight));
            }
            if (!threshold.packTemplate.isEmpty()){
                resultMessages.append("pack template:"+packTemaplte);
            }
            qDebug() << QString("pack (%1, %2, %3x%4x%5) use ").
                        arg(QString::number(package.id),
                            package.no,
                            QString::number(originPackage.length),
                            QString::number(originPackage.width),
                            QString::number(originPackage.height))
                     << condtionMsgs.join(",") << ";" << resultMessages.join(",");

            // logs
            message += QString("threshold name: %1, %2 ; \n").
                    arg(threshold.name, resultMessages.join(","));
        }
    }

    // 创建CSV文件夹路径
    QString csvFolderPath = QApplication::applicationDirPath() + "/csv";
    QString monthSubfolder = now.toString("yyyy/MM");
    QDir csvDir(csvFolderPath);
    if (!csvDir.exists(monthSubfolder) && !csvDir.mkpath(monthSubfolder)) {
        qWarning() << "Could not create month subfolder in csv directory:" << csvDir.filePath(monthSubfolder);
        return;
    }

    // 文件名和路径
    QString fileName = package.no;
    QStringList invalidChars = {"<", ">", ":", "\"", "/", "\\", "|", "?", "*"};
    for (const QString &invalidChar : invalidChars) {
        fileName.replace(invalidChar, "_");
    }
    fileName += "."+ now.toString("HHmmss")+".csv";
    QString filePath = csvDir.filePath(monthSubfolder + "/" + fileName);

    // 写入本地CSV文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        // 写入CSV头和内容
        out << "Type;OrderId;Quantity;ProductionGroupName;Length;Width;Height;CorrugateQuality;DesignId\n";
        out << QString("socket;%1;1;;%2;%3;%4;;%5\n").arg(package.no,
                                                          QString::number(package.length),
                                                          QString::number(package.width),
                                                          QString::number(package.height),
                                                          packTemaplte);
        file.close(); // 成功写入后关闭文件
    } else {
        qWarning() << "Could not write to local csv file:" << filePath;
        return;
    }

    // 确定热文件夹路径
    QString hotFolderPath = g_config->getDeviceConfig().importDir;
    QDir hotDir(hotFolderPath);
    if (!hotDir.exists() && !hotDir.mkpath(".")) {
        qWarning() << "Hot folder path does not exist and could not be created:" << hotFolderPath;
        return;
    }
    QString hotFilePath = hotDir.filePath(fileName);// 文件路径在热文件夹中
    if (!file.copy(hotFilePath)) { // 复制文件到热文件夹
        qWarning() << "Could not copy file to hot folder" << hotFilePath;
    }

    // 更新状态为已发送
    this->m_packBll->sent(package.id, message);
}

bool frmMain::parseSocketClientData(const QByteArray &binaryMessage)
{
    QTextCodec *codec;
    QString decodedString;
    QStringList encodings = {"UTF-8", "GBK", "GB18030", "ISO-8859-1", "Big5"};
    foreach (const QString &encoding, encodings) {
        codec = QTextCodec::codecForName(encoding.toUtf8());
        decodedString = codec->toUnicode(binaryMessage);

        // 解析JSON数据
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(decodedString.toUtf8(), &parseError);

        // 检查JSON数据是否正确解析
        if (parseError.error != QJsonParseError::NoError) {
            // 解析错误，处理错误情况
            qDebug() << "JSON parse error:" << parseError.errorString();
            continue; // 使用下一种编码进行解析
        }

        if (!document.isObject()) {
            qWarning() << "JSON is not an object.";
            break;
        }

        bool isSuccess = this->parseSocketClientData(decodedString);
        if (!isSuccess){
            qWarning() << "json parse error!";
        }
        return  isSuccess;
    }
    return false;
}

bool frmMain::parseSocketClientData(QString message)
{
    // 打印数据
    qDebug() << "Received data: " << message;

    // 解析JSON数据
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    // 检查JSON数据是否正确解析
    if (parseError.error != QJsonParseError::NoError) {
        // 解析错误，处理错误情况
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!document.isObject()) {
        qWarning() << "JSON is not an object.";
        return false;
    }

    // 如果JSON数据是一个对象
    QJsonObject jsonObj = document.object();
    QString packNo = jsonObj.value("ID").toString();
    QString customerName = jsonObj.value("CustomerName").toString();

    // 处理Panel列表字段
    QList<Panel> panels;
    QJsonArray panelsArray = jsonObj.value("Boards").toArray();
    for (const QJsonValue &panelVal : qAsConst(panelsArray)) {
        if (panelVal.isObject()) {
            QJsonObject panelObj = panelVal.toObject();
            Panel panel;
            panel.externalId = panelObj.value("ID").toString();
            panel.length = panelObj.value("Length").toInt();
            panel.width = panelObj.value("Width").toInt();
            panel.height = panelObj.value("Height").toInt();
            panel.name = panelObj.value("Name").toString();
            panel.remark = panelObj.value("Remark").toString();
            panels.append(panel); // 添加到列表
        }
    }

    // 计算包裹
    Package pack = this->m_algorithm->createLayers(panels);
    pack.no = packNo;
    pack.customerName = customerName;

    // 创建包裹数据
    int newPackId = this->m_packBll->insertByPackStruct(pack);
    this->initForm_PackDataBinding();

    // 发送包裹数据
    pack.id = newPackId;
    this->sendFileToHotFolder(pack);

    // 更新包裹状态
    this->initForm_PackDataBinding();

    // 成功处理
    return true;
}


