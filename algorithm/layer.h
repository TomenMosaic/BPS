#ifndef LAYER_H
#define LAYER_H

#include "panel.h"
#include "common.h"

class Layer {
public:
    int layerNumber;
    QList<Panel> panels;
    int height;

    Layer() : layerNumber(0), height(0), private_usedArea(0) {

    }

    // 拷贝构造函数
    Layer(const Layer& other) :
        layerNumber(other.layerNumber), height(other.height), private_usedArea(other.private_usedArea) {
        for (Panel panel : other.panels) {
            Panel newPanel(panel);
            panels.append(newPanel); // 假设Panel有合适的拷贝构造函数
        }
    }

    // 拷贝赋值运算符
    Layer& operator=(const Layer& other) {
        if (this != &other) {
            layerNumber = other.layerNumber;
            height = other.height;

            panels.clear();

            for (Panel panel : other.panels) {
                Panel newPanel(panel);
                panels.append(newPanel); // 假设Panel有合适的拷贝构造函数
            }
        }
        return *this;
    }

    // 添加板件到层
    void addPanel(Panel& panel) {
        // 添加到列表
        panel.layerNumber = layerNumber;
        panels.append(panel);

        // 更新总高度
        if (panel.height > height) {
            height = panel.height;
        }

        // 更新使用面积
        private_usedArea = 0;
        for (const auto& panel : this->panels){
            private_usedArea += panel.area();
        }

        // 标记不可重叠的区域
        for (int x = panel.position.x() + 1; x < panel.position.x() + panel.length - 1; ++x) {
            for (int y = panel.position.y() + 1; y < panel.position.y() + panel.width - 1; ++y) {
                private_usedSpaceMap.occupy(QPoint(x, y));
            }
        }

    }

    // 计算板件面积的函数
    int getUsedArea() const {
        return private_usedArea;
    }


    SpatialHashmap getSpaceMap(){
        return this->private_usedSpaceMap;
    }

    bool isExist(const QPoint point){
        return this->private_usedSpaceMap.isOccupied(point);
    }

    bool isPointOccupied(QPoint point, int panelLength, int panelWidth) {
        for (int posX = 0; posX <= point.x() + panelLength; ++posX) {
            for (int posY = 0; posY <= point.y() + panelWidth; ++posY) {
                if (this->private_usedSpaceMap.isOccupied(point)){
                    return true;
                }
            }
        }

        return false;
    }

    SpatialHashmap getUnusedArea(int layerLength, int layerWidth){
        SpatialHashmap unusedSpaceMap;
        for (int posX = 0; posX <= layerLength; ++posX) {
            for (int posY = 0; posY <= layerWidth; ++posY) {
                QPoint point(posX, posY);
                if (!this->private_usedSpaceMap.isOccupied(point)){
                     unusedSpaceMap.occupy(point);
                }
            }
        }
        return unusedSpaceMap;
    }

    // 重载==运算符 //TODO 是否可以去掉
    bool operator==(const Layer& other) const {
        // 如果有多个属性需要比较，可以扩展比较逻辑
        return this->height == other.height
            && this->layerNumber == other.layerNumber;
    }

private:
    // 使用的面积
    int private_usedArea;

    SpatialHashmap private_usedSpaceMap; //TODO 变成一个二维数组是否更快？？？

};

#endif // LAYER_H
