#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "common.h"
#include "panel.h"
#include "layer.h"
#include "package.h"

#include <QObject>

class Algorithm
{
public:
    Algorithm(int maxLengthExceed, int maxWidthExceed, int maxWidth4Strip);


    /**
     * 创建板件层级，包括处理“夹心饼干”效果
     * @param panels 所有板件
     * @return 按层级组织的板件列表
     */
    PackageAO createLayers(QList<Panel>& panels);

    // 分包
    QList<PackageAO> createPackages(QList<Panel>& panels);

private:

    void rearrangeLayers(PackageAO& package);
    int calculateContinuousSpaceAt(const QVector<QVector<bool>>& occupied, int startX, int startY,
                                              int maxWidth, int maxHeight);
    int estimateContinuousSpace(const QList<Panel>& layer, const Panel& panel,
                                           int posX, int posY, int maxWidth, int maxHeight);
    bool findPositionForPanel(Layer& layer, Panel& panel, int maxWidth, int maxHeight);
    bool forcePlacePanelInPackageLayers(PackageAO& package, Panel& panel);
    bool isStableForPlacement(SpatialHashmap spaceMap, Panel& panel, int posX, int posY);
    //bool canPlacePanel(const QList<Panel*> layerPanels, const Panel& panel, int posX, int posY, int maxPackageLength, int maxPackageWidth);
    bool canPlacePanel(const QList<Panel> layerPanels, const Panel& panel,
                       int posX, int posY, int maxPackageLength, int maxPackageWidth, bool isFix);
    void sortPanels(QList<Panel>& panels);
    // bool panelComparator(const Panel& a, const Panel& b);

    void updatePackageSize(QList<Panel>& panels, PackageAO& package);

private:
    // 最大可超出的尺寸
    int m_maxLengthExceed;
    int m_maxWidthExceed;
    int m_maxWidth4Strip;
};

#endif // ALGORITHM_H
