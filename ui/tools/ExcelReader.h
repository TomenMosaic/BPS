#ifndef EXCELREADER_H
#define EXCELREADER_H

#include <QAxObject>
#include <QMessageBox>
#include <QDebug>
#include <QFile>

#include "xlsxdocument.h"

// 读取 excel
class ExcelReader {
public:
    static QList<QMap<int, QVariant>> readExcel(const QString &filePath, const QMap<int, QString> &columnTypes) {
        QList<QMap<int, QVariant>>  dataRows;

        QFile file(filePath);
        if (!file.exists()) {
            QMessageBox::warning(nullptr, "Warning", "File does not exist: " + filePath);
            return dataRows;
        }

        QXlsx::Document xlsx(filePath);
        if (!xlsx.load()) {
            QMessageBox::warning(nullptr, "Warning", "Failed to load Excel file: " + filePath);
            return dataRows;
        }

        int rowCount = xlsx.dimension().lastRow();
        int columnCount = xlsx.dimension().lastColumn();

        for (int row = 2; row <= rowCount; ++row) {
            QMap<int, QVariant> rowData;
            bool isEmptyRow = true;  // 假设行开始时是空的

            for (auto column : columnTypes.keys()) {
                QString columnType = columnTypes.value(column);

                if (column > columnCount) {
                    QMessageBox::warning(nullptr, "Warning",
                                         "Unknown column type for column " + QString::number(column) + ": " + columnType);
                    return dataRows;
                }

                QXlsx::Cell *cell = xlsx.cellAt(row, column);
                if (!cell || cell->value().isNull()) {
                    continue;  // 如果单元格为空或值为null，则跳过此单元格
                }

                QVariant value = cell->value();


                if (columnType == "Int" && value.canConvert<int>()) {
                    rowData[column] = value.toInt();
                    isEmptyRow = false;
                } else if (columnType == "Double" && value.canConvert<double>()) {
                    rowData[column] = value.toDouble();
                    isEmptyRow = false;
                } else if (columnType == "String") {
                    rowData[column] = value.toString();
                    if (!value.toString().isEmpty()) {
                        isEmptyRow = false;
                    }
                }
            }

            // 如果行不为空，则添加到结果中
            if (!isEmptyRow) {
                dataRows.append(rowData);
            }
        }

        return dataRows;
    }
};


#endif // EXCELREADER_H
