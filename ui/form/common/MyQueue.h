#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <QQueue>
#include <QSharedPointer>
#include <functional>

#include <QMutex>
#include <QMutexLocker>

template<typename T>
class MyQueue {
public:

    /**
         * @brief 检查队列是否为空
         *
         * 使用示例:
         * MyQueue<int> queue;
         * bool empty = queue.isEmpty(); // 检查队列是否为空
         *
         * @return bool 如果队列为空返回 true，否则返回 false
         */
    bool isEmpty() const {
        //QMutexLocker locker(&mutex);
        return queue.isEmpty();
    }

    // 向队列添加元素
    void enqueue(const T& item) {
        //QMutexLocker locker(&mutex);
        queue.enqueue(item);
    }

    // 查看队列的第一个元素（不移除）
    QSharedPointer<T> peek() const {
        //QMutexLocker locker(&mutex);
        if (queue.isEmpty()) {
            return QSharedPointer<T>();
        }
        return QSharedPointer<T>(new T(queue.head()));
    }

    /**
        * @brief 根据条件查看队列中的元素（不移除）
        *
        * 使用示例:
        * MyQueue<int> queue;
        * queue.enqueue(5);
        * queue.enqueue(10);
        * queue.enqueue(15);
        * // 查看第一个大于10的元素
        * auto item = queue.peekIf([](const int &x) { return x > 10; });
        * if (item) {
        *     // 处理查看到的元素
        *     int value = *item;
        * }
        *
        * @param predicate 条件函数
        * @return QSharedPointer<T> 指向查看到的元素的智能指针，如果未找到则为 nullptr
        */
    QSharedPointer<T> peekIf(const std::function<bool(const T&)>& predicate = [](const T&) { return true; }) const {
        //QMutexLocker locker(&mutex);
        for (const auto& item : queue) {
            if (predicate(item)) {
                return QSharedPointer<T>(new T(item));
            }
        }
        return QSharedPointer<T>();
    }

    // 从队列取出元素
    T dequeue() {
        //QMutexLocker locker(&mutex);
        if (queue.isEmpty()) {
            throw std::runtime_error("队列为空");
        }
        return queue.dequeue();
    }

    /**
         * @brief 根据条件取出队列中的元素
         *
         * 使用示例:
         * MyQueue<int> queue;
         * queue.enqueue(5);
         * queue.enqueue(10);
         * queue.enqueue(15);
         * auto item = queue.takeIf([](const int &x) { return x > 10; });
         * if (item) {
         *     // 处理取出的元素
         *     int value = *item;
         * }
         *
         * @param predicate 条件函数
         * @return QSharedPointer<T> 指向取出元素的智能指针，如果未找到则为 nullptr
         */
    QSharedPointer<T> dequeueIf(const std::function<bool(const T&)>& predicate) {
        //QMutexLocker locker(&mutex);
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (predicate(*it)) {
                QSharedPointer<T> item(new T(*it));
                queue.erase(it);
                return item;
            }
        }
        return QSharedPointer<T>();
    }

    /**
             * @brief 根据条件修改队列中的元素
             *
             * 使用示例:
             * MyQueue<int> queue;
             * queue.enqueue(5);
             * queue.enqueue(10);
             * // 将第一个大于5的元素增加10
             * bool modified = queue.modifyIf([](const int &x) { return x > 5; },
             *                               [](int &x) { x += 10; });
             *
             * @param predicate 条件函数，用于查找需要修改的元素
             * @param modifier 修改函数，用于修改找到的元素
             * @return bool 是否修改了元素
             */
    bool modifyIf(const std::function<bool(T&)>& predicate, const std::function<void(T&)>& modifier) {
        //QMutexLocker locker(&mutex);
        for (auto& item : queue) {
            if (predicate(item)) {
                modifier(item);
                return true;
            }
        }
        return false;
    }

    uint length() const{
        //QMutexLocker locker(&mutex);
        return queue.length();
    }

private:
    QQueue<T> queue;
    //mutable QMutex mutex;

};

#endif // MYQUEUE_H
