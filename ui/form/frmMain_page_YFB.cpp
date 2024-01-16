#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/tableview_controller.h"
#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>

// 导入板件数据
void frmMain::on_btnImport_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Excel Files (*.xlsx *.xls)"));
    if (fileName.isEmpty()) {
        return;
    }

    // 列序号 map
    auto mappings = g_config->getPrePackConfig().getImportMappings();
    QMap<int, QString> columnTypes;
    QMap<int, ColumnMapping> columnTypes2;
    for (const auto& mapping : qAsConst(mappings) ) {
        columnTypes[mapping.columnNumber] = mapping.dataType;
        columnTypes2[mapping.columnNumber] = mapping;
    }

    // 解析导入的文件
    auto varsList = ExcelReader::readExcel(fileName, columnTypes);
    QList<Panel> importPanels;
    for (const auto& vars: qAsConst(varsList) ){
        Panel panel;
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
    this->m_algorithmPackagesModel->setHorizontalHeaderLabels({ "ID", "包号", "尺寸", "数量"}); // 列头
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
        QStandardItem *totalItem = new QStandardItem(package.getPanelTotal());
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

        // 包裹id
        QModelIndex modelIndex = this->m_algorithmPackagesModel->index(rowIndex, 0);
        int packId = this->m_algorithmPackagesModel->data(modelIndex).toInt();

        // 当前选中包裹
        if (packId <= 0){
            page_yfb_tvAlgorithmPanles_DataBinding(this->m_algorithmPackages[rowIndex].getPanels());
        }else{
            for (auto& package : this->m_algorithmPackages) {
                if (package.id == packId){
                   page_yfb_tvAlgorithmPanles_DataBinding(package.getPanels());
                   break;
                }
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
}

void frmMain::on_pushButton_clicked()
{
    // 保存到数据库
    for (const auto& package : qAsConst(this->m_algorithmPackages)){
        this->m_packBll->insertByPackStruct(package);
    }

    // 导出按钮enable
    this->ui->btnExport->setEnabled(true);

    //TODO 在状态栏显示计算结果保存成功
}

void frmMain::on_btnExport_clicked()
{
    // 导出 pdf
}



