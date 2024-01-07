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
    // 订单号
    QString orderNo;

    QList<Layer> layers;
    // 是否需要扫码确认
    bool needsScanConfirmation = false;
    // 等待
    bool pendingScan = false;

    Package() : length(0), width(0), height(0) {}


    // 拷贝构造函数
    Package(const Package& other)
        : id(other.id), length(other.length), width(other.width),
          height(other.height), no(other.no), customerName(other.customerName),
          orderNo(other.orderNo),
          layers(other.layers), // 假定Layer也实现了深拷贝
          needsScanConfirmation(other.needsScanConfirmation),
          pendingScan(other.pendingScan) {

    }

    // 拷贝赋值运算符
    Package& operator=(const Package& other) {
        if (this != &other) {
            id = other.id;
            length = other.length;
            width = other.width;
            height = other.height;
            no = other.no;
            customerName = other.customerName;
            orderNo = other.orderNo;
            layers = other.layers; // 假定Layer也实现了深拷贝
            needsScanConfirmation = other.needsScanConfirmation;
            pendingScan = other.pendingScan;
        }
        return *this;
    }


    void addLayer(Layer& layer) {
        //TODO 检测是否存在

        layer.layerNumber = layers.length() + 1;
        for (Panel& panel: layer.panels){
            if (panel.layerNumber != layer.layerNumber){
                panel.layerNumber = layer.layerNumber;
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
            for (Panel& panel: layers[i].panels){
                panel.layerNumber = 0;
                panel.position = QPoint();
                if (panel.rotated){
                    panel.rotate();
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
        if (! this->orderNo.isEmpty()){
            script = script.replace("{OrderNo}", this->orderNo);
        }
        script = script.replace("{LayerCount}", QString::number(this->layers.size())); // 层数
        script = script.replace("{PackageHeight}", QString::number(this->height)); // 包裹高度
        script = script.replace("{PackageWidth}", QString::number(this->width)); // 包裹宽度
        script = script.replace("{PackageLength}", QString::number(this->length)); // 包裹长度

        // 包含板件名称、说明、位置、特殊工艺列表
        QSet<QString> panelNameSet; // name
        QSet<QString> panelRemarkSet; // remark
        QSet<QString> panelLocationSet; // 位置
        QSet<QString> panelSculptSet; // 工艺
        for (const Layer& layer: qAsConst(layers)){
            for (const Panel& panel: qAsConst(layer.panels)){
                panelNameSet.insert(panel.name);
                panelRemarkSet.insert(panel.remark);
                panelLocationSet.insert(panel.location);
                panelSculptSet.insert(panel.sculpt);
            }
        }
        script = script.replace("{PanelNames}", panelNameSet.toList().join(","));
        script = script.replace("{PanelRemarks}", panelRemarkSet.toList().join(","));
        script = script.replace("{PanelLocations}", panelLocationSet.toList().join(","));
        script = script.replace("{PanelSculpts}", panelSculptSet.toList().join(","));

        return script;
    }

    QString getKey() const{
        QSet<QString> panelLocationSet; // 位置
        for (const Layer& layer: qAsConst(layers)){
            for (const Panel& panel: qAsConst(layer.panels)){
                panelLocationSet.insert(panel.location);
            }
        }

        QString result ;
        if (customerName.isEmpty() && panelLocationSet.isEmpty()){
            result = no;
        }else{
            result = QString("%1_%2").arg(customerName, panelLocationSet.toList().join(","));
        }
        return result;
    }

    // 重载==运算符 //TODO 是否可以去掉
    bool operator==(const Package& other) const {
        // 如果有多个属性需要比较，可以扩展比较逻辑
        return this->height == other.height
            && this->length == other.length
            && this->width == other.width
            && this->no == other.no;
    }

};

#endif // PACKAGE_H
