#ifndef PACKAGE_H
#define PACKAGE_H

#include "layer.h"

#include <QJsonObject>
#include <QSet>

class Package {
public:
    int id;
    int length;
    int width;
    int height;
    QString no;
    // 客户名称
    QString customerName;
    QList<Layer> layers;

    Package() : length(0), width(0), height(0) {}

    void addLayer(Layer& layer) {
        //TODO 检测是否存在

        layer.layerNumber = layers.length() + 1;
        foreach (Panel* panel, layer.panels){
            if (panel->layerNumber != layer.layerNumber){
                panel->layerNumber = layer.layerNumber;
            }
        }
        layers.append(layer);

        height += layer.height;
    }

    void removeLayer(int layerNumber){
        for (auto it = layers.begin(); it != layers.end(); ++it) {
            if (it->layerNumber == layerNumber) {
                this->height -= it->height;
                layers.erase(it);
                break;
            }
        }
    }

    bool exist(int layerNumber) {
        for (int i = 0; i < layers.size(); ++i) {
            if (layers[i].layerNumber == layerNumber){
                return true;
            }
        }
        return false;
    }

    Layer* single(int layerNumber) {
        for (int i = 0; i < layers.size(); ++i) {
            if (layers[i].layerNumber == layerNumber){
                return &layers[i];
            }
        }
        return nullptr;
    }

    void clearLayers(){
        for (int i = 0; i < layers.size(); ++i){
            foreach (Panel* panel, layers[i].panels){
                panel->layerNumber = 0;
                panel->position = QPoint();
                if (panel->rotated){
                    panel->rotate();
                }
            }

            layers[i].layerNumber = 0;
        }

        layers.clear();

        // 长宽高暂时不清理
    }

    QString getScript(QString expression) const {
        QString script = expression;
        if (! this->customerName.isEmpty()){
            script = script.replace("{CustomerName}", this->customerName);
        }
        script = script.replace("{LayerCount}", QString::number(this->layers.size()));
        script = script.replace("{PackageHeight}", QString::number(this->height));
        script = script.replace("{PackageWidth}", QString::number(this->width));

        // 包含板件名称列表
        QSet<QString> panelNameSet;
        foreach (const Layer layer, layers){
            foreach (Panel* panel, layer.panels){
                panelNameSet.insert(panel->name);
            }
        }
        script = script.replace("{PanelNames}", panelNameSet.toList().join(","));

        return script;
    }


    // 重载==运算符 //TODO 是否可以去掉
    bool operator==(const Package& other) const {
        // 如果有多个属性需要比较，可以扩展比较逻辑
        return this->height == other.height
            && this->length == other.length
            && this->width == other.width;
    }

};

#endif // PACKAGE_H
