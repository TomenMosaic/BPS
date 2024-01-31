#ifndef PACKAGE_H
#define PACKAGE_H

#include "layer.h"

#include <QJsonObject>
#include <QSet>



class PackageAO {
public:
    enum PackageAOBlockEventEnum{
        ScanRC = 1,
        CG = 2
    };

public:
    int id = 0;
    int length = 0;
    int width = 0;
    int height = 0;

    // 包裹条码
    QString no;
    // 客户名称
    QString customerName;
    // 订单号
    QString orderNo;
    // 流水号（一个订单分组里面的第几包）
    int flowNo = -1;

    // 已扫码的板件数据
    int scanPanelCount = 0;
    // 包裹中板件的总数量
    //int panelTotal;

    // 层列表
    QList<Layer> layers;

    //TODO 放在这里不合适，需要单独的对象
    //QSet<PackageAOBlockEventEnum> blockEvents;

    // 等待
    //bool waiting = false;

    // 来源IP
    //QString originIP;

    PackageAO() : id(0), length(0), width(0), height(0) {}

    // 拷贝构造函数
    PackageAO(const PackageAO& other)
        : id(other.id), length(other.length), width(other.width),height(other.height),
          no(other.no), customerName(other.customerName),orderNo(other.orderNo),
          flowNo(other.flowNo),
          scanPanelCount(other.scanPanelCount),
          layers(other.layers) // 假定Layer也实现了深拷贝
          {

    }

    // 拷贝赋值运算符
    PackageAO& operator=(const PackageAO& other) {
        if (this != &other) {
            id = other.id;
            length = other.length;
            width = other.width;
            height = other.height;
            no = other.no;
            customerName = other.customerName;
            orderNo = other.orderNo;
            layers = other.layers; // 假定Layer也实现了深拷贝            
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

    QList<Panel> getPanels() const{
        QList<Panel> panels;
        for (auto& layer : this->layers){
            panels.append(layer.panels);
        }
        return panels;
    }

    int getPanelTotal() const{
        int result;
        for (auto& layer : this->layers){
            result += layer.panels.size();
        }
        return result;
    }

    QString createNo(int flowNo) const{
        QString format = "P{OrderNo}{FlowNo}";
        //QDateTime now = QDateTime::currentDateTime();

        QString result = format.replace("{OrderNo}", orderNo)
                               .replace("{FlowNo}", QString("%1").arg(flowNo, 3, 10, QChar('0'))); // 假设flowNo是5位数，不足前面补0

        return result;
    }

    // 重载==运算符 //TODO 是否可以去掉
    bool operator==(const PackageAO& other) const {
        // 如果有多个属性需要比较，可以扩展比较逻辑
        return this->height == other.height
            && this->length == other.length
            && this->width == other.width
            && this->no == other.no;
    }

};

#endif // PACKAGE_H
