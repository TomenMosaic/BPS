#ifndef COMMON_H
#define COMMON_H

#include <QPoint>
#include <QHash>

class SpatialHashmap {
private:
    QHash<QPoint, bool> occupiedPoints;

public:
    void occupy(const QPoint& point) {
        occupiedPoints[point] = true;
    }

    void free(const QPoint& point) {
        occupiedPoints.remove(point);
    }

    bool isOccupied(const QPoint& point) {
        return occupiedPoints.contains(point) && occupiedPoints[point];
    }

};
inline uint qHash(const QPoint& p) {
    return uint(p.x()) * 31u + uint(p.y());
}


#endif // COMMON_H
