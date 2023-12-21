#include "algorithm.h"

#include <QList>
#include <QDebug>
#include <QQueue>


Algorithm::Algorithm(int maxLengthExceed, int maxWidthExceed) :
    m_maxLengthExceed(maxLengthExceed), m_maxWidthExceed(maxWidthExceed)
{

}

/**
 * 对板件进行排序的函数
 * @param panels 板件列表
 */
void Algorithm::sortPanels(QList<Panel>& panels) {
    std::sort(panels.begin(), panels.end(), [](const Panel& a, const Panel& b) {
        // 首先比较面积
        if (a.area() != b.area()) return a.area() > b.area();
        // 如果面积相同 & 长度不同，就比较长
        if (std::max(a.length, a.width) > std::max(b.length, b.width)){
            return true;
        }
        // 如果长度也相同，最后比较厚度
        return a.height > b.height;
    });
}

/**
 * 检查指定位置是否可以放置板件，避免与已放置的板件重叠
 * @param layer 当前层的所有已放置板件
 * @param panel 要放置的板件
 * @param posX 板件的横坐标位置
 * @param posY 板件的纵坐标位置
 * @return 如果位置可用，返回 true；否则返回 false
 */
/*bool Algorithm::canPlacePanel(const QList<Panel*> layerPanels, const Panel& panel,
                              int posX, int posY, int maxPackageLength, int maxPackageWidth) {
    int panelWidth = panel.rotated ? panel.length : panel.width;
    int panelLength = panel.rotated ? panel.width : panel.length;

    // 检查板件是否超出包裹的最大长宽
    if (posX + panelLength > maxPackageLength || posY + panelWidth > maxPackageWidth) {
        return false; // 超出最大长宽
    }

    for (const Panel* placedPanel : layerPanels) {
        int placedPanelLength = placedPanel->rotated ? placedPanel->width :  placedPanel->length;
        int placedPanelWidth =  placedPanel->rotated ? placedPanel->length :  placedPanel->width;
        int placedPanelX =  placedPanel->position.x();
        int placedPanelY =  placedPanel->position.y();

        // 1. 检查板件是否重叠
        if (posX < placedPanelX + placedPanelLength
                && posX + panelLength > placedPanelX
                && posY < placedPanelY + placedPanelWidth
                && posY + panelWidth > placedPanelY) {
            return false; // 1.1 发现重叠
        }
    }
    return true; // 1.2 未发现重叠，位置可用
}*/

bool Algorithm::canPlacePanel(const QList<Panel*> layerPanels, const Panel& panel,
                              int posX, int posY, int maxPackageLength, int maxPackageWidth, bool isFix = false) {
    // rect 对象的width相当于 length，height相当于width
    int panelLength = panel.rotated ? panel.width : panel.length;
    int panelWidth = panel.rotated ? panel.length : panel.width;

    // 检查板件是否超出包裹的最大长宽
    if (!isFix){
        if (posX + panelLength > maxPackageLength || posY + panelWidth > maxPackageWidth) {
            return false; // 超出最大长宽
        }
    }

    // 检查是否和已经存在的板件重叠
    for (const Panel* placedPanel : layerPanels) {
        int placedPanelLength = placedPanel->rotated ? placedPanel->width : placedPanel->length;
        int placedPanelWidth = placedPanel->rotated ? placedPanel->length : placedPanel->width;
        int placedPanelX = placedPanel->position.x();
        int placedPanelY = placedPanel->position.y();

        if (posX < placedPanelX + placedPanelLength // x值在已存在的板件中
                && posX + panelLength > placedPanelX // 已存在板件的x在 “检测板件” 中
                && posY < placedPanelY + placedPanelWidth
                && posY + panelWidth > placedPanelY) {
            return false; // 发现重叠
        }
    }
    return true; // 未发现重叠，位置可用
}

// 检查板件下方是否有足够的支撑
bool Algorithm::isStableForPlacement(SpatialHashmap spaceMap, Panel& panel, int posX, int posY) {
    int supportCount = 0;  // 支撑点的数量
    for (int x = posX; x < posX + panel.length; ++x) {
        // 检查板件下方的点是否被占用，从而提供支撑
        if (spaceMap.isOccupied(QPoint(x, posY + panel.width))) {
            ++supportCount;
        }
    }
    // 如果支撑点的数量小于板件长度的一半，视为不稳定
    return supportCount >= panel.length / 2;
}

/**
 * 在当前层中为板件查找合适的位置
 * @param layer 当前层的所有已放置板件
 * @param panel 要放置的板件
 * @param maxWidth 层的最大宽度
 * @param maxHeight 层的最大高度
 * @return 如果找到合适位置，返回 true；否则返回 false
 */
bool Algorithm::findPositionForPanel(Layer& layer, Panel& panel, int layerLength, int layerWidth) {
    // 当前层面积 + 板件面积 > 最大面积，就跳过
    if (layer.getUsedArea() + panel.area() > layerLength * layerWidth){
        return false;
    }

    // 尝试放置板件，优先填满横向空间
    auto tryPlacePanel = [&](bool rotate) {
        if (rotate){
            // 长宽比为1的时候，旋转没有意义
            if (panel.width == panel.length){
                return false;
            }
            panel.rotate();
        }

        int panelWidth = panel.rotated ? panel.length : panel.width;
        int panelLength = panel.rotated ? panel.width : panel.length;

        // 板件的长宽不能超出层的最大长宽
        if (panelLength > layerLength || panelWidth > layerWidth){
            return false;
        }

        for (int posY = 0; posY <= layerWidth - panelWidth; ++posY) {
            for (int posX = 0; posX <= layerLength - panelLength; ++posX) {
                //TODO 如果超出区域，就退出循环
                if (canPlacePanel(layer.panels, panel, posX, posY, layerLength, layerWidth)
                        //&& isStableForPlacement(layer.getSpaceMap(), panel, posX, posY)
                        ) {
                    panel.position = QPoint(posX, posY);
                    return true;
                }
            }
        }
        if (rotate) panel.rotate(); // 如果未放置，旋转回原始方向
        return false;
    };

    // 从左到右，从上到下尝试放置板件，如果放不下，尝试旋转后放置
    return tryPlacePanel(false) || tryPlacePanel(true);
}

bool Algorithm::forcePlacePanelInPackageLayers(Package& package, Panel& originPanel) {

    int minimalExceed = INT_MAX;
    QPoint bestPosition(-1, -1);
    Layer* bestLayer = nullptr;
    Panel testPanel = originPanel;
    bool rotationNeeded = false; // 是否真的需要旋转

    // 如果已经旋转，就要调转成正常的，再进行计算
    if (testPanel.rotated){
        testPanel.rotate();
    }

    auto tryPlacePanel = [&](bool rotate) {
        if (rotate){
            // 长宽比为1的时候，旋转没有意义
            if (testPanel.width == testPanel.length) return false;

            testPanel.rotate(); // 旋转
        }

        // 根据是否旋转确定实际的长宽
        int panelWidth = rotate ? testPanel.length : testPanel.width;
        int panelLength = rotate ? testPanel.width : testPanel.length;

        // 遍历已存在的板件，检测是否重叠，并更新到一个最优的位置上
        for (Layer& layer : package.layers) {
            for (int posX = 0; posX <= package.length; ++posX) {
                for (int posY = 0; posY <= package.width; ++posY) {
                    int exceedLength = posX + panelLength - package.length;
                    int exceedWidth = posY + panelWidth - package.width;

                    // 检查是否超出最大允许超出范围
                    if (exceedWidth > this->m_maxWidthExceed || exceedLength > this->m_maxLengthExceed){
                        //isContinueUp = true;
                        break;
                    }

                    // 是否重叠
                    int skipX = 0, skipY = 0;
                    for (const Panel* placedPanel : qAsConst(layer.panels)) {
                        int placedPanelLength = placedPanel->rotated ? placedPanel->width : placedPanel->length;
                        int placedPanelWidth = placedPanel->rotated ? placedPanel->length : placedPanel->width;
                        int placedPanelX = placedPanel->position.x();
                        int placedPanelY = placedPanel->position.y();

                        if (posX < placedPanelX + placedPanelLength // x值在已存在的板件中
                                && posX + panelLength > placedPanelX // 已存在板件的x在 “检测板件” 中
                                && posY < placedPanelY + placedPanelWidth
                                && posY + panelWidth > placedPanelY) {
                            //int tmpSkipX = placedPanelX + placedPanelLength - posX;
                            int tmpSkipY = placedPanelY + placedPanelWidth - posY;
                            /*if (tmpSkipX > 0 && tmpSkipX > skipX){
                                skipX = tmpSkipX;
                            }*/
                            if (tmpSkipY > 0 && tmpSkipY > skipY){
                                skipY = tmpSkipY;
                            }
                        }
                    }
                    if (skipX > 0 || skipY > 0){
                        if (skipX > 0){
                            posX += skipX;
                        }
                        if (skipY > 0){
                            posY += skipY;
                        }
                        continue;
                    }

                    // 寻找长度超出的 最小层
                    if (exceedLength < minimalExceed) {
                        minimalExceed = exceedLength;
                        bestPosition = QPoint(posX, posY);
                        bestLayer = &layer;
                        rotationNeeded = rotate;
                    }

                }

            }
        }

        // 没有按照预期旋转
        // 设置为旋转 & 不需要旋转
        if (rotate && !rotationNeeded){
            testPanel.rotate(); // 转回去
        }
    };

    // 尝试放置板件，无论是正常放置还是旋转后放置
    tryPlacePanel(false); // 正常放置
    tryPlacePanel(true);  // 旋转后放置

    if (bestLayer != nullptr) {
        if (rotationNeeded && !originPanel.rotated) originPanel.rotate();  // 如果需要旋转，则旋转板件
        originPanel.position = bestPosition;  // 更新板件位置
        bestLayer->addPanel(originPanel);  // 将板件添加到最佳层

        // 板件长/宽
        int panelWidth = originPanel.rotated ? originPanel.length : originPanel.width;
        int panelLength = originPanel.rotated ? originPanel.width : originPanel.length;

        // 更新包裹尺寸
        package.length = std::max(package.length, originPanel.position.x() + panelLength);
        package.width = std::max(package.width, originPanel.position.y() + panelWidth);
        return true;
    }

    return false;
}

// 重新排列层的函数
void Algorithm::rearrangeLayers(Package& package) {
    if (package.layers.length() <= 2){
        return;
    }

    QVector<Layer*> sandwichLayers;  // 存储尺寸与包裹尺寸相同的层
    QVector<Layer*> otherLayers;     // 存储其他的层

    // 1. 将层分为夹心层和其他层
    // 1.1. 最大面积
    int maxArea = package.layers[0].getUsedArea();
    // 1.2. 设定阈值，例如最大面积的95% //TODO 可以作为参数
    int threshold = maxArea * 0.95;
    // 1.3.
    for (Layer& layer : package.layers) {
        // 当前层只有一块板，且面积大于设定的阈值
        bool isSandwichLayer = layer.panels.length() == 1 && layer.getUsedArea() >= threshold;
        if (isSandwichLayer) {
            sandwichLayers.append(&layer);
        } else {
            otherLayers.append(&layer);
        }
    }
    // 如果夹心层为空，其他层为空，或者夹心层只有一层就没有必要重新排序了
    if (sandwichLayers.isEmpty() || otherLayers.isEmpty() || sandwichLayers.length() == 1){
        return;
    }

    // 2. 对夹心层进行排序
    std::sort(sandwichLayers.begin(), sandwichLayers.end(), [](const Layer* a, const Layer* b) {
        return a->height > b->height;  // 高度
    });

    // 3. 对其他层进行排序
    std::sort(otherLayers.begin(), otherLayers.end(), [](const Layer* a, const Layer* b) {
        return a->getUsedArea() > b->getUsedArea();  // 面积
    });

    // 4. 夹心饼干效果
    // 4.2. 交替放置其他层和夹心层
    package.clearLayers();
    while (!sandwichLayers.isEmpty() || !otherLayers.isEmpty()) {
        if (!sandwichLayers.isEmpty()) {
            auto val = sandwichLayers.takeFirst();
            package.addLayer(*val);
        }
        if (!otherLayers.isEmpty()) {
            auto val = otherLayers.takeFirst();
            package.addLayer(*val);
        }
    }
}

void Algorithm::updatePackageSize(QList<Panel>& panels, Package& package){
    //1. 找出最大长宽，考虑旋转的情况
    for (Panel& panel : panels) {
        // 无论矩形是否旋转，比较长度和宽度，找出最大的值
        int rotatedMaxLength = std::max(panel.width, panel.length);
        int rotatedMaxWidth = std::min(panel.width, panel.length);

        package.width = std::max(package.width, rotatedMaxWidth);
        package.length = std::max(package.length, rotatedMaxLength);
    }

    //2. 1 块大板+1 窄条时，如果窄条堆叠在大板上面会导致很多填充物，所以需要和大板并排放
    if (panels.size() == 2){
        auto panelMax = panels[0];
        auto panelNarrowStrip = panels[1];
        if (panelNarrowStrip.width <= this->m_maxLengthExceed){ // 窄条
            if ((panelMax.area() - panelNarrowStrip.area()) > 50*100*10){ // 假设填充物的尺寸是 50*100，多余10个就是过多
                package.width += panelNarrowStrip.width;
            }
        }
    }
}

Package Algorithm::createLayers(QList<Panel>& panels) {
    Package package;

    //0. valid
    if (panels.isEmpty()){
        return package;
    }

    //1. 对所有板件进行排序
    this->sortPanels(panels);

    //2. 找出最大长宽，考虑旋转的情况
    this->updatePackageSize(panels, package);

    //3. 遍历所有板件并尝试放置它们
    for (int i = 0; i < panels.size(); i++) {
        Panel& panel = panels[i];
        bool placed = false;

        //3.1. 逐一查找层
        for (Layer& layer : package.layers) {
            // 尝试在当前层放置板件 （包括旋转尝试）
            if (this->findPositionForPanel(layer, panel, package.length, package.width)) {
                layer.addPanel(panel);
                placed = true;
                break; // 可以放置后就退出层遍历
            }
        }

        //3.2. 如果无法放置，则创建新层
        if (!placed) {
            Layer newLayer = Layer();
            // 首先在新层中查找是否有位置可以放，因为可能会需要旋转
            if (this->findPositionForPanel(newLayer, panel, package.length, package.width)) {
                newLayer.addPanel(panel);
            } else{
                QString msg = QString("panel(%1,%2,%3,%4) algorithm error!").
                        arg(panel.id).
                        arg(panel.name).
                        arg(panel.length).
                        arg(panel.width);
                qWarning() << msg;
            }
            package.addLayer(newLayer);
        }
    }

    //4. 查找最后一层，看是否可以抽出板件强制放到其他层
    if (package.layers.size() > 1){
        Layer lastLayer = package.layers[package.layers.size() - 1];
        if (lastLayer.panels.size() == 1){
            Panel* panel = lastLayer.panels[0];

            // 层中空余的空间太多，即填充物需要放很多
            if ( (package.length * package.width - lastLayer.getUsedArea()) / 50*100  >= 3){
                package.removeLayer(lastLayer.layerNumber); // 移除某层
                if (!this->forcePlacePanelInPackageLayers(package, *panel)) { //
                    package.addLayer(lastLayer); // 加回去
                }
            }

        }
    }

    //4. 夹心饼干效果
    //instance.rearrangeLayers(package);

    //5. 更新数据
    //5.1. 对板件列表排序
    std::sort(panels.begin(), panels.end(), [](const Panel& a, const Panel& b) {
        if (a.layerNumber != b.layerNumber)
            return a.layerNumber < b.layerNumber;
        if (a.position.x() != b.position.x())
            return a.position.x() < b.position.x();
        return a.position.y() < b.position.y();
    });

    return package;
}
