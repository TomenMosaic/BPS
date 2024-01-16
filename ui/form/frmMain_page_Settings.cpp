#include "frmmain.h"
#include "global.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"

#include "common/tableview_controller.h"
#include "ExcelReader.h"

#include <QList>
#include <QScriptEngine>

void frmMain::initForm_SettingDataBinding(){
    // 加载数据绑定到控件，工作模式 下拉框、开启等待扫码 checkbox、加工文件导出路径 linetext
    this->ui->cbWorkMode->setCurrentText("接收包裹数据"); // TODO 根据配置文件的值进行实际的选择
    this->ui->isWaiting4Scan->setCheckState(g_config->getWorkConfig().isWaiting4Scan ? Qt::Checked : Qt::Unchecked); // 是否等待扫码
    this->ui->txtHotFolderPath->setText(g_config->getDeviceConfig().importDir); // 裁纸机上的热文件夹路径

    // 等待扫码的条件
    if (g_config->getWorkConfig().isWaiting4Scan){
        if (this->m_waitingConditions.size() > 0){
            this->ui->txtWaiting4ScanCondition->setText(this->m_waitingConditions[0].Condition);
        }
    }

    // 预值列表
    this->m_tbAddValueConditionsModel = new QStandardItemModel(this);
    this->m_tbAddValueConditionsModel->setHorizontalHeaderLabels({ "ID", "规则名称", "条件", "长度预值", "宽度预值", "高度预值"}); // 列头
    ui->tbAddValueConditions->setModel(this->m_tbAddValueConditionsModel);
    this->m_tbAddValueConditionsModel->setRowCount(0); // ??
    for(int index = 0;index<this->m_thresholdConditions.length();index++)
    {
        auto condition = this->m_thresholdConditions[index]; // 板件
        QList<QStandardItem*>itemList; // 成员
        int columnIndex = 0;

        // id
        QStandardItem *idItem = new QStandardItem(QString::number(condition.ID));
        itemList.insert(columnIndex, idItem);
        columnIndex++;

        // name
        QStandardItem *nameItem = new QStandardItem(condition.Name);
        nameItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, nameItem);
        columnIndex++;

        // condition
        QStandardItem *conditionItem = new QStandardItem(condition.Condition);
        conditionItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, conditionItem);
        columnIndex++;

        // length
        QStandardItem *lengthItem = new QStandardItem(condition.lengthExpression);
        lengthItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, lengthItem);
        columnIndex++;

        // width
        QStandardItem *widthItem = new QStandardItem(condition.widthExpression);
        widthItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, widthItem);
        columnIndex++;

        // height
        QStandardItem *heightItem = new QStandardItem(condition.heightExpression);
        heightItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, heightItem);

        this->m_tbAddValueConditionsModel->appendRow(itemList);
    }
    ui->tbAddValueConditions->setColumnHidden(0, true); // ID列隐藏
    ui->tbAddValueConditions->resizeColumnsToContents(); // 根据内容调整列
    ui->tbAddValueConditions->resizeRowsToContents(); // 这会调整行高以适应内容
    ui->tbAddValueConditions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch

    // 箱型列表
    this->m_tbPackTemplateModel = new QStandardItemModel(this);
    this->m_tbPackTemplateModel->setHorizontalHeaderLabels({ "ID", "规则名称", "条件", "箱型编号"}); // 列头
    ui->tbBoxConditions->setModel(this->m_tbPackTemplateModel);
    this->m_tbPackTemplateModel->setRowCount(0); // ??
    for(int index = 0;index<this->m_packTemplateConditions.length();index++)
    {
        auto condition = this->m_packTemplateConditions[index];
        QList<QStandardItem*>itemList; // 成员
        int columnIndex = 0;

        // id
        QStandardItem *idItem = new QStandardItem(QString::number(condition.ID));
        itemList.insert(columnIndex, idItem);
        columnIndex++;

        // name
        QStandardItem *nameItem = new QStandardItem(condition.Name);
        nameItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, nameItem);
        columnIndex++;

        // condition
        QStandardItem *conditionItem = new QStandardItem(condition.Condition);
        conditionItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, conditionItem);
        columnIndex++;

        // pack template
        QStandardItem *templateItem = new QStandardItem(condition.packTemplate);
        templateItem->setTextAlignment(Qt::AlignCenter);
        itemList.insert(columnIndex, templateItem);

        this->m_tbPackTemplateModel->appendRow(itemList);
    }
    ui->tbBoxConditions->setColumnHidden(0, true); // ID列隐藏
    ui->tbBoxConditions->resizeColumnsToContents(); // 根据内容调整列
    ui->tbBoxConditions->resizeRowsToContents(); // 这会调整行高以适应内容
    ui->tbBoxConditions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 其他列使用 Stretch
}


void frmMain::on_btnSendConfiguration_clicked()
{
    try{
        //1. 保存到配置文件
        DeviceConfig deviceConfig = g_config->getDeviceConfig();
        deviceConfig.importDir = this->ui->txtHotFolderPath->text();
        g_config->setDeviceConfig(deviceConfig);
        WorkConfig workConfig = g_config->getWorkConfig();
        workConfig.isWaiting4Scan = this->ui->isWaiting4Scan->isChecked();
        g_config->setWorkConfig(workConfig);

        //2. 保存到规则表
        //2.1. 扫码等待规则
        if (workConfig.isWaiting4Scan){
            QString condition = this->ui->txtWaiting4ScanCondition->text();
            if (this->m_waitingConditions.size() >= 1){
                this->m_waitingConditions[0].Condition = condition;
            }else{
                ConditionDto dto;
                dto.ID = 0;
                dto.Type = ConditionDto::TypeEnum::waitingCondition;
                dto.Condition = condition;
                dto.action = ConditionDto::ActionEnum::scan;
                this->m_waitingConditions.append(dto);
            }
            this->m_conditionBll->createOrUpdate(m_waitingConditions[0]);
        }

        //2.2. 预值规则
        int rowCount = this->m_tbAddValueConditionsModel->rowCount();
        for (int row = 0; row < rowCount; ++row) {
            int colIndex = 0;
            QModelIndex idIndex  = this->m_tbAddValueConditionsModel->index(row, colIndex);
            int id = this->m_tbAddValueConditionsModel->data(idIndex).toInt();
            colIndex++;

            QModelIndex nameIndex  = this->m_tbAddValueConditionsModel->index(row, colIndex);
            QString name = this->m_tbAddValueConditionsModel->data(nameIndex).toString();
            colIndex++;

            QModelIndex conditionIndex  = this->m_tbAddValueConditionsModel->index(row, colIndex);
            QString condition = this->m_tbAddValueConditionsModel->data(conditionIndex).toString();
            colIndex++;

            QModelIndex lengthIndex  = this->m_tbAddValueConditionsModel->index(row, colIndex);
            QString length = this->m_tbAddValueConditionsModel->data(lengthIndex).toString();
            colIndex++;

            QModelIndex widthIndex  = this->m_tbAddValueConditionsModel->index(row, colIndex);
            QString width = this->m_tbAddValueConditionsModel->data(widthIndex).toString();
            colIndex++;

            QModelIndex heightIndex  = this->m_tbAddValueConditionsModel->index(row, colIndex);
            QString height = this->m_tbAddValueConditionsModel->data(heightIndex).toString();

            if (id <= 0){
                ConditionDto dto;
                dto.ID = id;
                dto.Name = name;
                dto.Type = ConditionDto::TypeEnum::threshold;
                dto.Condition = condition;
                dto.lengthExpression = length;
                dto.widthExpression = width;
                dto.heightExpression = height;
                this->m_thresholdConditions.append(dto);
            }else{
                for (auto& dto : this->m_thresholdConditions) {
                    if (dto.ID == id){
                        dto.Name = name;
                        dto.Condition = condition;
                        dto.lengthExpression = length;
                        dto.widthExpression = width;
                        dto.heightExpression = height;
                        break;
                    }
                }
            }
        }
        for (auto& condition : this->m_thresholdConditions) {
            this->m_conditionBll->createOrUpdate(condition);
        }

        //2.3. 箱型规则
        int packTableRowCount = this->m_tbPackTemplateModel->rowCount();
        for (int row = 0; row < packTableRowCount; ++row) {
            int colIndex = 0;
            QModelIndex idIndex  = this->m_tbPackTemplateModel->index(row, colIndex);
            qDebug() << this->m_tbPackTemplateModel->data(idIndex).toString();
            int id = this->m_tbPackTemplateModel->data(idIndex).toInt();
            colIndex++;

            QModelIndex nameIndex  = this->m_tbPackTemplateModel->index(row, colIndex);
            QString name = this->m_tbPackTemplateModel->data(nameIndex).toString();
            colIndex++;

            QModelIndex conditionIndex  = this->m_tbPackTemplateModel->index(row, colIndex);
            QString condition = this->m_tbPackTemplateModel->data(conditionIndex).toString();
            colIndex++;

            QModelIndex packTemplateIndex  = this->m_tbPackTemplateModel->index(row, colIndex);
            QString packTemplate = this->m_tbPackTemplateModel->data(packTemplateIndex).toString();

            if (id <= 0){
                ConditionDto dto;
                dto.ID = id;
                dto.Name = name;
                dto.Type = ConditionDto::TypeEnum::packTemplateCondition;
                dto.Condition = condition;
                dto.packTemplate = packTemplate;
                this->m_packTemplateConditions.append(dto);
            }else{
                for (auto& dto : this->m_packTemplateConditions) {
                    if (dto.ID == id){
                        dto.Name = name;
                        dto.Condition = condition;
                        dto.packTemplate = packTemplate;
                        break;
                    }
                }
            }
        }
        for (auto& condition : this->m_packTemplateConditions) {
            this->m_conditionBll->createOrUpdate(condition);
        }

        //2.4. 更新配置页的数据
        this->initConfig(); // 重新加载数据
        this->initForm_SettingDataBinding(); // 重新做数据绑定

        QMessageBox::information(this, "", "配置修改成功！");

    }catch (const std::exception &e) {
        // 处理或记录异常
        QMessageBox::warning(this, "错误", QString("Exception caught: %s \n").arg(e.what()));
        qFatal( "Exception caught: %s", e.what());
    }
}

// 勾选了是否等待扫码
void frmMain::on_isWaiting4Scan_stateChanged(int arg1)
{
    this->ui->txtWaiting4ScanCondition->setEnabled(arg1 == Qt::Checked);
}


void frmMain::on_btnInsert_AddValueCondition_clicked(bool checked)
{
    // 在预值列表中增加一行
    int columnCount = this->m_tbAddValueConditionsModel->columnCount(); // 获取模型的列数
    QList<QStandardItem*> itemList;
    for (int i = 0; i < columnCount; ++i) {
        itemList.append(new QStandardItem("")); // 添加一个空字符串作为项的值
    }
    this->m_tbAddValueConditionsModel->appendRow(itemList);
}


void frmMain::on_btnInsert_PackTemplateCondition_clicked(bool checked)
{
    // 在箱型规则列表中增加一行
    int columnCount = this->m_tbPackTemplateModel->columnCount(); // 获取模型的列数
    QList<QStandardItem*> itemList;
    for (int i = 0; i < columnCount; ++i) {
        itemList.append(new QStandardItem("")); // 添加一个空字符串作为项的值
    }
    this->m_tbPackTemplateModel->appendRow(itemList);
}


void frmMain::on_btnRemove_AddValueCondition_clicked(bool checked)
{
    int curRow = this->ui->tbAddValueConditions->currentIndex().row();
    if (curRow == -1) {
        QMessageBox::warning(this, "删除错误", "未选中行，无法删除");
        qWarning() << "未选中行，无法删除";
        return;
    }

    // 删除确认
    QModelIndex idIndex  = this->m_tbAddValueConditionsModel->index(curRow, 0);
    int id = this->m_tbAddValueConditionsModel->data(idIndex).toInt();
    for (auto& condition : this->m_thresholdConditions) {
        if (condition.ID == id){
            condition.RemovedAt = QDateTime::currentDateTime();
            break;
        }
    }
    this->m_tbAddValueConditionsModel->removeRow(curRow);
    qDebug() << QString("删除第%1行").arg(curRow + 1);
}

void frmMain::on_btnRemove_PackTemplateCondition_clicked(bool checked)
{
    int curRow = this->ui->tbBoxConditions->currentIndex().row();
    if (curRow == -1) {
        QMessageBox::warning(this, "删除错误", "未选中行，无法删除");
        qWarning() << "未选中行，无法删除";
        return;
    }

    // 删除确认
    QModelIndex idIndex  = this->m_tbPackTemplateModel->index(curRow, 0);
    int id = this->m_tbPackTemplateModel->data(idIndex).toInt();
    for (auto& condition : this->m_packTemplateConditions) {
        if (condition.ID == id){
            condition.RemovedAt = QDateTime::currentDateTime();
            break;
        }
    }
    this->m_tbPackTemplateModel->removeRow(curRow);
    qDebug() << QString("删除第%1行").arg(curRow + 1);
}

