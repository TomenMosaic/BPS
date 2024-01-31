#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/FullScreenMask.h"
#include "common/LoadingWidget.h"
#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>

FullScreenMask* FullScreenMask::m_instance = nullptr;

// 导入板件数据
void frmMain::on_btnImport_clicked()
{
    // 弹窗选择文件
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Excel Files (*.xlsx *.xls)"));
    if (fileName.isEmpty()) {
        return;
    }

    // 提示文本
    //LoadingWidget loadingWidget(nullptr);
    //loadingWidget.show();
    QString message = "板件数据导入开始...";
    //loadingWidget.setInfo(message);
    this->m_customStatusBar->setInfoText(message);

    // 列序号 map
    auto mappings = g_config->getPrePackConfig().getImportMappings();
    QMap<int, QString> columnTypes;
    QMap<int, ColumnMapping> columnTypes2;
    for (const auto& mapping : qAsConst(mappings) ) {
        columnTypes[mapping.columnNumber] = mapping.dataType;
        columnTypes2[mapping.columnNumber] = mapping;
    }

    // 解析导入的文件
    message = "文件解析中...";
    //loadingWidget.setInfo(message);
    this->m_customStatusBar->setInfoText(message);
    auto varsList = ExcelReader::readExcel(fileName, columnTypes);
    QList<Panel> importPanels;
    for (const auto& vars: qAsConst(varsList) ){
        Panel panel = Panel();
        for (auto it = vars.constBegin(); it != vars.constEnd(); ++it) {
            auto propertyName = columnTypes2[it.key()].propertyName;
            auto var = it.value();
            panel.setProperty(propertyName, var);
        }
        importPanels.append(panel);
    }

    // 绑定到导入板件列表中
    this->m_importPanels = importPanels;
    this->page_yfb_panelDataBinding(); // 刷新数据

    // 导出按钮 disable
    this->ui->btnExport->setEnabled(false);

    // 切换到 “导入数据” tab 页
    this->ui->tabYFB->setCurrentIndex(1);
    QFileInfo fileInfo(fileName);
    this->m_customStatusBar->setInfoText(fileInfo.fileName() + " 数据导入成功！");
}

// 绑定板件列表数据到 QTableView
void frmMain::page_yfb_panelDataBinding(){   
    // table ------
    this->ui->tbImportPanels->verticalHeader()->setVisible(false); // 显示表头

    this->m_tbImportPanelsModel = new QStandardItemModel(this);
    this->m_tbImportPanelsModel->setHorizontalHeaderLabels({"ID", "UPI", "名称", "尺寸（单位：mm）", "说明", "柜体"}); // 列头
    this->ui->tbImportPanels->setModel(this->m_tbImportPanelsModel);

    this->m_tbImportPanelsModel->setRowCount(0); // ??
    QMap<int, bool> shouldHideColumn;
    for (int i = 0; i < this->m_tbImportPanelsModel->columnCount(); i++){
        shouldHideColumn.insert(i, true);
    }
    for(int index = 0;index<this->m_importPanels.length();index++)
    {
        Panel panel = this->m_importPanels[index]; // 板件
        QList<QStandardItem*>itemList; // 成员
        int columnIndex = 0;

        // ID
        QStandardItem *idItem = new QStandardItem(QString::number(panel.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, idItem);
        columnIndex++;

        // UPI
        if (!panel.no.isEmpty()){
            shouldHideColumn[columnIndex] = false;
        }
        QStandardItem *noItem = new QStandardItem(panel.no);
        noItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, noItem);
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

        this->m_tbImportPanelsModel->appendRow(itemList);
    }

    for (auto i = shouldHideColumn.begin(); i != shouldHideColumn.end(); ++i) {
        int column = i.key();
        bool hide = i.value();
        if (hide){
            this->ui->tbImportPanels->setColumnHidden(column, true); // 列隐藏
        }
    }
    this->ui->tbImportPanels->setColumnHidden(0, true); // ID列隐藏
    this->ui->tbImportPanels->resizeColumnsToContents(); // 根据内容调整列
    this->ui->tbImportPanels->resizeRowsToContents(); // 这会调整行高以适应内容
    this->ui->tbImportPanels->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch
}

void frmMain::page_yfb_tvAlgorithmPanles_DataBinding(QList<Panel> panels){
    // table ------
    this->ui->tvAlgorithmPanels->verticalHeader()->setVisible(false); // 显示表头

    QStandardItemModel *panelModel = new QStandardItemModel(this);
    panelModel->setHorizontalHeaderLabels({"ID", "UPI", "名称", "尺寸（单位：mm）", "说明", "柜体", "层", "是否旋转"}); // 列头
    this->ui->tvAlgorithmPanels->setModel(panelModel);

    panelModel->setRowCount(0); // ??
    QMap<int, bool> shouldHideColumn;
    for (int i = 0; i < panelModel->columnCount(); i++){
        shouldHideColumn.insert(i, true);
    }
    for(int index = 0;index<panels.length();index++)
    {
        Panel panel = panels[index]; // 板件
        QList<QStandardItem*>itemList; // 成员
        int columnIndex = 0;

        // ID
        QStandardItem *idItem = new QStandardItem(QString::number(panel.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, idItem);
        columnIndex++;

        // UPI
        if (!panel.no.isEmpty()){
            shouldHideColumn[columnIndex] = false;
        }
        QStandardItem *noItem = new QStandardItem(panel.no);
        noItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, noItem);
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
    }

    for (auto i = shouldHideColumn.begin(); i != shouldHideColumn.end(); ++i) {
        int column = i.key();
        bool hide = i.value();
        if (hide){
            this->ui->tvAlgorithmPanels->setColumnHidden(column, true); // 列隐藏
        }
    }
    this->ui->tvAlgorithmPanels->setColumnHidden(0, true); // ID列隐藏
    this->ui->tvAlgorithmPanels->resizeColumnsToContents(); // 根据内容调整列
    this->ui->tvAlgorithmPanels->resizeRowsToContents(); // 这会调整行高以适应内容
    this->ui->tvAlgorithmPanels->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch

}

void frmMain::page_yfb_tvAlgorithmPackages_DataBinding(){
    this->ui->tvAlgorithmPackages->verticalHeader()->setVisible(false); // 显示表头

    this->m_algorithmPackagesModel = new QStandardItemModel(this);
    this->m_algorithmPackagesModel->setHorizontalHeaderLabels({ "ID", "包号", "尺寸", "板件数量"}); // 列头
    this->ui->tvAlgorithmPackages->setModel(m_algorithmPackagesModel);

    this->m_algorithmPackagesModel->setRowCount(0); // ??

    for(int index = 0;index<this->m_algorithmPackages.length();index++)
    {
        auto package = this->m_algorithmPackages[index]; // row
        QList<QStandardItem*>itemList; // 成员
        int colIndex = 0;

        // ID
        QStandardItem *idItem = new QStandardItem(QString::number(package.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, idItem);
        colIndex++;

        // no
        QStandardItem *noItem = new QStandardItem(package.no);
        noItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, noItem);
        colIndex++;

        // size
        QString formattedDimensions = QString("%1 x %2 x %3")
                .arg(QString::number(package.length),
                     QString::number(package.width),
                     QString::number(package.height));
        QStandardItem *dimensionItem = new QStandardItem(formattedDimensions);
        dimensionItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, dimensionItem);
        colIndex++;

        // total
        QStandardItem *totalItem = new QStandardItem(QString::number(package.getPanelTotal()));
        totalItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(colIndex, totalItem);
        colIndex++;

        this->m_algorithmPackagesModel->appendRow(itemList);
    }

    this->ui->tvAlgorithmPackages->setColumnHidden(0, true); // ID列隐藏
    this->ui->tvAlgorithmPackages->resizeColumnsToContents(); // 根据内容调整列
    this->ui->tvAlgorithmPackages->resizeRowsToContents(); // 这会调整行高以适应内容
    this->ui->tvAlgorithmPackages->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch
    this->ui->tvAlgorithmPackages->horizontalHeader()->setSectionResizeMode( this->m_algorithmPackagesModel->columnCount() - 1,
                                                                            QHeaderView::ResizeToContents); // 按钮列使用 ResizeToContents

    // pack table
    this->ui->tvAlgorithmPackages->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this->ui->tvAlgorithmPackages->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &previous) {
        int rowIndex = current.row();
        if (rowIndex < 0 || current.row() == previous.row()){
            return ;
        }

        //1. 包裹id
        QModelIndex modelIndex = this->m_algorithmPackagesModel->index(rowIndex, 0);
        int packId = this->m_algorithmPackagesModel->data(modelIndex).toInt();

        //2. 显示 当前选中包裹对应的板件列表
        PackageAO* currentPackage = nullptr;
        if (packId <= 0){
            currentPackage = &m_algorithmPackages[rowIndex];
            page_yfb_tvAlgorithmPanles_DataBinding(this->m_algorithmPackages[rowIndex].getPanels());
        }else{
            for (auto& package : this->m_algorithmPackages) {
                if (package.id == packId){
                    currentPackage = &package;
                   page_yfb_tvAlgorithmPanles_DataBinding(package.getPanels());
                   break;
                }
            }
        }

        //3. 显示预览图
        if (currentPackage != nullptr){
            this->ui->tabAlgorithmPreview->clear(); // 清空 QTabWidget 中的所有 tabs
            QSize tabSize = this->ui->tabAlgorithmPreview->size(); // 获取 tab 的容器的大小

            //3.1. 计算缩放因子
            qreal scaleX = tabSize.width() / qreal(currentPackage->length);
            qreal scaleY = tabSize.height() / qreal(currentPackage->width);
            qreal scaleFactor = qMin(scaleX, scaleY) * 0.94;

            //3.2. 遍历层，在tab中创建新的tab，tab的内容是预览图
            for (int i = 0; i < currentPackage->layers.size(); ++i) {
                const Layer &layer = currentPackage->layers[i];
                QGraphicsView *view = getLayerView(layer,currentPackage->length, currentPackage->width, scaleFactor );

                // 创建每个 tab 的页面 widget
                QWidget *tabPage = new QWidget();
                QVBoxLayout *layout = new QVBoxLayout(tabPage);
                layout->setContentsMargins(0, 0, 0, 0); // 设置边距为 0
                layout->addWidget(view); // 将 view 添加到 tabPage 的布局中
                tabPage->setLayout(layout);

                // 设置 QGraphicsView 缩放
               // view->setSceneRect(scene->itemsBoundingRect()); // 调整视图的场景矩形以适应所有项的边界
                // view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio); // 缩放视图以适应所有内容

                // 添加新的 tab，页签名字为层号
                QString tabLabel = QString("第 %1 层").arg(i + 1);
                this->ui->tabAlgorithmPreview->addTab(tabPage, tabLabel);
            }
        }

    });
}

void frmMain::on_btnAlgorithm_clicked()
{
    // 根据一个标识先分包
    QMap<QString, QList<Panel>> groupPanels;
    for (const Panel &panel : qAsConst(this->m_importPanels)) {
        groupPanels[panel.sculpt].append(panel);
    }

    // 创建包裹数据
    this->m_algorithmPackages.clear();
    int index = 1;
    for (auto i = groupPanels.begin(); i != groupPanels.end(); ++i, ++index) {
        auto panels = i.value();
        auto package = this->m_algorithm->createLayers(panels);
        package.orderNo = panels[0].orderNo; // 订单号
        package.customerName = panels[0].customerName; // 客户名称
        package.flowNo = index; // 流水号
        package.no = package.createNo(index); // 包裹号
        this->m_algorithmPackages.append(package);
    }

    // 绑定到包裹 table    
    this->page_yfb_tvAlgorithmPackages_DataBinding();

    // 切换到 “计算结果” tab 页
    this->ui->tabYFB->setCurrentIndex(2);

    // 设置当前选中的包裹
    if (this->m_algorithmPackagesModel->rowCount() > 0){
        QModelIndex modelIndex = this->m_algorithmPackagesModel->index(0, 1);
        //QItemSelectionModel *selectionModel = this->ui->tvPackList->selectionModel();
        //selectionModel->select(modelIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        this->ui->tvAlgorithmPackages->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->ui->tvAlgorithmPackages->setCurrentIndex(modelIndex);
    }

    // 提示
    this->m_customStatusBar->setInfoText("计算结束！");
}

void frmMain::on_pushButton_clicked()
{
    // 保存到数据库
    QList<PackageDto> groups;
    for (auto& obj : this->m_algorithmPackages){
        auto dto = PackageDto(obj, PackageDto::StatusEnum::Status_Step1_Calculated);
        groups.append(dto);
    }
    this->m_packBll->insertByPackStructs(groups);

    // 导出按钮enable
    this->ui->btnExport->setEnabled(true);
}

void frmMain::on_btnExport_clicked()
{
    // 导出 pdf
}



