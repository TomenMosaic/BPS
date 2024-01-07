#ifndef PANEL_H
#define PANEL_H

#include <QPair>
#include <QString>
#include <QDateTime>
#include <QPoint>

class Panel
{
public:
    int id;
    QString externalId;
    QString no;
    int length;
    int width;
    int height;
    // 板件名称：要有关键词来区分板件，例如“XX门板”的关键词“门”可以确认为门板。每种类别板件可以有多个关键词，若无关键词默认为柜体。
    QString name;
    /*
板件说明：用于区分板件的特殊工艺，例如“把手1”表示为海棠角突出拉手，“格栅”表示为格栅，“整装”表示为整装等，具体含义发送方给出说明文件即可，打包软件可以自行适配。
板件分类举例：(关键词可按拆单软件方式定义，无明确定义)
门板：关键词——门
背板：关键词——背
柜体：关键词——无
格栅：关键词——格/格栅
整装：关键词——整
异型：关键词——异
金属门：关键词——金属门
*/
    QString remark;
    QDateTime createTime;
    int layerNumber; // 板件所在的层
    QPoint position; // 板件在层中的坐标（x, y）
    bool rotated; // 板件是否被旋转
    QString location; // 位置，比如主卧衣柜、客厅鞋柜
    QString sculpt; // 特殊工艺

    Panel(int id, int length, int width, int height, QString name, QString remark)
        : id(id), length(length), width(width), height(height), remark(remark), name(name),
          createTime(QDateTime::currentDateTime()),
          layerNumber(0), position(0, 0), rotated(false) {}

    Panel()
        : length(0), width(0), height(0),
          name(""), remark(""), location(""), sculpt(""),
          createTime(QDateTime::currentDateTime()),
          layerNumber(0), position(0, 0), rotated(false){

    }

    // 拷贝构造函数
    Panel(const Panel& other)
        : id(other.id), externalId(other.externalId), no(other.no),
          length(other.length), width(other.width), height(other.height),
          name(other.name), remark(other.remark), location(other.location), sculpt(other.sculpt),
          createTime(other.createTime),
          layerNumber(other.layerNumber), position(other.position), rotated(other.rotated) {
    }

    Panel& operator=(const Panel& other) {
        if (this != &other) {
            id = other.id;
            externalId = other.externalId;
            no = other.no;
            length = other.length;
            width = other.width;
            height = other.height;
            name = other.name;
            remark = other.remark;
            createTime = other.createTime;
            layerNumber = other.layerNumber;
            position = other.position;
            rotated = other.rotated;
            location = other.location;
            sculpt = other.sculpt;
        }
        return *this;
    }

    // 计算板件面积的函数
    int area() const {
        return length * width;
    }

    // 旋转板件
    void rotate() {
        rotated = !rotated;
    }

    // 重载==运算符 //TODO 是否可以去掉
    bool operator==(const Panel& other) const {
        // 如果有多个属性需要比较，可以扩展比较逻辑
        return this->id == other.id
                && this->no == other.no
                && this->length == other.length
                && this->width == other.width
                && this->height == this->height;
    }

};

#endif // PANEL_H
