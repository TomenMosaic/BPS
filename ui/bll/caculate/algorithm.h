#ifndef ALGORITHM_H
#define ALGORITHM_H
#include <QObject>
#include <QTimer>
class Algorithm : public QObject
{
    Q_OBJECT
public:
    explicit Algorithm(QObject *parent = nullptr);
    virtual ~Algorithm();
    virtual double filter(double input) = 0;
    virtual double readData() = 0;
    virtual void clear() = 0;
signals:
    void output(double result);
protected:    
    QTimer m_timer;
};
#endif // ALGORITHM_H
