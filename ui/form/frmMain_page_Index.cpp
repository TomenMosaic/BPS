#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/tableview_controller.h"
#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>


void onTreeItemClicked(QTreeWidgetItem *item, int /* column */) {
    if (item->text(0) == "详情" || item->text(0) == "隐藏") {
        if (item->childCount() > 0){
            bool isHidden = item->child(0)->isHidden();
            for (int i = 0; i < item->childCount(); ++i) {
                item->child(i)->setHidden(!isHidden);
            }
            item->setText(0, isHidden ? "隐藏" : "详情");
        }

    }
}

void frmMain::initForm_PackDataBinding(bool isReload){
    ui->tvPackList->verticalHeader()->setVisible(false); // 显示表头

    m_packModel = new QStandardItemModel(this);
    m_packModel->setHorizontalHeaderLabels({ "ID", "包号", "尺寸", "已扫/总数", "状态"}); // 列头
    ui->tvPackList->setModel(m_packModel);

    m_packModel->setRowCount(0); // ??
    QList<QSharedPointer<Row>> packList = isReload ? m_packBll->getList() : m_packBll->getCacheList();

    for(int index = 0;index<packList.length();index++)
    {
        QSharedPointer<Row>row = packList.at(index); // row
        QList<QStandardItem*>itemList; // 成员
        int colIndex = 0;

        // ID
        QStandardItem *idItem = new QStandardItem(row->data(PackBLL::ID).toString());
        idItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, idItem);
        colIndex++;

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

        // 已扫码 / 总数
        QString formattedCount = QString("%1 / %2")
                .arg(row->data(PackBLL::ScanPanelCount).toString(),
                     row->data(PackBLL::PanelTotal).toString());
        QStandardItem *countItem = new QStandardItem(formattedCount);
        countItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, countItem);
        colIndex++;

        // status
        PackBLL::StatusEnum statusValue = static_cast<PackBLL::StatusEnum>(row->data(PackBLL::Status).toInt()); // 转换为枚举值
        QString statusText = m_packBll->statusEnumToString(statusValue); // 转换为枚举对应的中文
        QStandardItem *statusItem = new QStandardItem(statusText);
        statusItem->setTextAlignment(Qt::AlignCenter); // 居中
        itemList.insert(colIndex, statusItem);
        colIndex++;

        // 设置行的背景颜色
        QColor color = QColor(Qt::white);
        if (statusValue >= PackBLL::StatusEnum::Status_Full){ // 齐套后旧变绿
            color=QColor("#90ee90");
        }else if (row->data(PackBLL::ScanPanelCount).toUInt() > 0){ // 当有扫码的时候，整行变蓝
            color=QColor("#add8e6");
        }
        for (QStandardItem *item : itemList) {
            item->setBackground(color);
        }

        m_packModel->appendRow(itemList);
    }


    this->ui->tvPackList->setColumnHidden(0, true); // ID列隐藏
    this->ui->tvPackList->resizeColumnsToContents(); // 根据内容调整列
    this->ui->tvPackList->resizeRowsToContents(); // 这会调整行高以适应内容
    this->ui->tvPackList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch
    this->ui->tvPackList->horizontalHeader()->setSectionResizeMode(m_packModel->columnCount() - 1, QHeaderView::ResizeToContents); // 按钮列使用 ResizeToContents

    // pack table
    this->ui->tvPackList->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this->ui->tvPackList->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &previous) {
        int rowIndex = current.row();
        if (rowIndex < 0 || current.row() == previous.row()){
            return ;
        }

        // 当前选中行
        QSharedPointer<Row> newRow = this->m_packBll->getCacheList().at(rowIndex);
        int packId = newRow->data(PackBLL::ID).toInt();
        Package currentPackage;
        currentPackage.id = newRow->data(PackBLL::ID).toInt();
        currentPackage.no = newRow->data(PackBLL::No).toString();
        currentPackage.customerName = newRow->data(PackBLL::CustomerName).toString();
        currentPackage.orderNo = newRow->data(PackBLL::OrderNo).toString();
        currentPackage.length = newRow->data(PackBLL::Length).toInt();
        currentPackage.width = newRow->data(PackBLL::Width).toInt();
        currentPackage.height = newRow->data(PackBLL::Height).toInt();
        QString createTime = newRow->data(PackBLL::CreateTime).toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString msg = QString("ID: %1， 包裹号: %2， 长x宽x高： %3x%4x%5， 创建时间：%6 \n 客户：%7, 订单：%8 ").
                arg(packId).
                arg(currentPackage.no).
                arg(currentPackage.length).
                arg(currentPackage.width).
                arg(currentPackage.height).
                arg(createTime).
                arg(currentPackage.customerName).
                arg(currentPackage.orderNo);
        this->ui->lblPackInfo->setText(msg);

        // 日志列表
        QTreeWidgetItem *topItem = this->ui->treePackLogs->topLevelItem(0);
        qDeleteAll(topItem->takeChildren()); // 清除现有的子节点
        auto logs = newRow->data(PackBLL::Logs).toString().split("\n");
        for (auto& log:logs) {
            QTreeWidgetItem *childItem = new QTreeWidgetItem(topItem);
            childItem->setText(0, log); // 设置子节点的文本
        }
        bool isHide = topItem->text(0).contains("隐藏");
        topItem->setHidden(isHide); // 显示/隐藏

        // 查找关联的板件列表
        this->m_panels = this->m_panelBll->getPanelsByPackId(packId);
        for (int i = 0; i < this->m_panels.size(); ++i) {
            Panel& panel = this->m_panels[i];
            Layer* layer = currentPackage.single(panel.layerNumber);
            if (layer == nullptr) {
                Layer newLayer;
                newLayer.addPanel(panel);
                currentPackage.addLayer(newLayer);
                panel.layerNumber = currentPackage.layers[currentPackage.layers.size() - 1].layerNumber;
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
    panelModel->setHorizontalHeaderLabels({"ID", "名称", "尺寸（单位：mm）", "说明", "柜体", "层", "是否旋转", "操作"}); // 列头
    this->ui->tblPanels->setModel(panelModel);

    panelModel->setRowCount(0); // ??
    QMap<int, bool> shouldHideColumn;
    for (int i = 0; i < panelModel->columnCount(); i++){
        shouldHideColumn.insert(i, true);
    }
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

        // name
        if (!panel.name.isEmpty()){
            shouldHideColumn[columnIndex] = false;
        }
        QStandardItem *nameItem = new QStandardItem(panel.name);
        nameItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, nameItem);
        columnIndex++;

        // size
        shouldHideColumn[columnIndex] = false;
        QString formattedDimensions = QString("%1 x %2 x %3")
                .arg(QString::number(panel.length),
                     QString::number(panel.width),
                     QString::number(panel.height));
        QStandardItem *dimensionItem = new QStandardItem(formattedDimensions);
        dimensionItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, dimensionItem);
        columnIndex++;

        // remark
        if (!panel.remark.isEmpty()){
            shouldHideColumn[columnIndex] = false;
        }
        QStandardItem *remarkItem = new QStandardItem(panel.remark);
        remarkItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, remarkItem);
        columnIndex++;

        // location
        if (!panel.location.isEmpty()){
            shouldHideColumn[columnIndex] = false;
        }
        QStandardItem *locationItem = new QStandardItem(panel.location);
        locationItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, locationItem);
        columnIndex++;

        // layer
        shouldHideColumn[columnIndex] = false;
        QStandardItem *layerItem = new QStandardItem(QString::number(panel.layerNumber));
        layerItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, layerItem);
        columnIndex++;

        // 是否旋转
        shouldHideColumn[columnIndex] = false;
        QStandardItem *rotationItem = new QStandardItem(panel.rotated ? "是" : "否");
        rotationItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, rotationItem);
        columnIndex++;

        panelModel->appendRow(itemList);

        // 操作按钮
        shouldHideColumn[columnIndex] = false;
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

    for (auto i = shouldHideColumn.begin(); i != shouldHideColumn.end(); ++i) {
        int column = i.key();
        bool hide = i.value();
        if (hide){
            this->ui->tblPanels->setColumnHidden(column, true); // 列隐藏
        }
    }
    this->ui->tblPanels->setColumnHidden(0, true); // ID列隐藏
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
                                                                    this->m_panelsPackage.length  * scaleFactor,
                                                                    this->m_panelsPackage.width  * scaleFactor);
            baseRectItem->setPen(QPen(Qt::red, 2)); // 1像素红色边框
            baseRectItem->setBrush(QBrush(Qt::lightGray)); // 浅灰色的背景
            scene->addItem(baseRectItem);
        }

        // 创建每个板件的图形矩形
        for (const Panel& panel : layer.panels) {
            int x = panel.position.x() * scaleFactor;
            int y = panel.position.y() * scaleFactor;

            QString text = QString("%1; %2x%3x%4%5")
                    .arg(panel.id)
                    .arg(panel.length)
                    .arg(panel.width)
                    .arg(panel.height)
                    .arg(panel.rotated ? "; r" : "");

            // 创建矩形，位置和大小匹配板件的坐标和尺寸，设置边框和背景色
            QGraphicsRectItem *rectItem ;
            if (panel.rotated) {// 如果板件旋转，长宽对调
                rectItem = new QGraphicsRectItem(x, y, panel.width * scaleFactor, panel.length * scaleFactor);
            }else{
                rectItem = new QGraphicsRectItem(x, y, panel.length * scaleFactor, panel.width * scaleFactor);
            }
            rectItem->setPen(QPen(Qt::black, 1)); // 1像素黑色边框
            if (panel.isScaned){
                rectItem->setBrush(QBrush(Qt::blue)); //
            }else{
                rectItem->setBrush(QBrush(Qt::green)); //
            }
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
        // view->scale(scaleFactor, scaleFactor);
        // view->setSceneRect(0, 0, this->m_panelsPackage.length, this->m_panelsPackage.width);

        // 创建每个 tab 的页面 widget
        QWidget *tabPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(tabPage);
        layout->setContentsMargins(0, 0, 0, 0); // 设置边距为 0
        layout->addWidget(view); // 将 view 添加到 tabPage 的布局中
        tabPage->setLayout(layout);

        // 设置 QGraphicsView 缩放
        view->setSceneRect(scene->itemsBoundingRect()); // 调整视图的场景矩形以适应所有项的边界
        // view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio); // 缩放视图以适应所有内容

        // 添加新的 tab，页签名字为层号
        QString tabLabel = QString("第 %1 层").arg(i + 1);
        this->ui->tabLayerPreview->addTab(tabPage, tabLabel);
    }
}

// 剔除板件
void frmMain::onBtnRemovePanelClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        int id = button->objectName().toInt();

        // 剔除某个板重新计算
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


// 加入队列
void frmMain::enqueueTask(Package& pack){
    this->m_waitingQueue.enqueue(pack);
}

// 处理队列中的任务
void frmMain::processTasks()
{
    // 队列不为空，且队列头部的任务不为等待扫码状态
    if (!this->m_waitingQueue.isEmpty()){
        if  (!this->m_waitingQueue.head().needsScanConfirmation
             || (this->m_waitingQueue.head().needsScanConfirmation
                 && !this->m_waitingQueue.head().pendingScan)){
            // 出队
            Package pack = this->m_waitingQueue.dequeue();

            // 发送包裹数据
            this->sendFileToHotFolder(pack);

            // 更新表格状态
            this->initForm_PackDataBinding();
        }

    }
}
