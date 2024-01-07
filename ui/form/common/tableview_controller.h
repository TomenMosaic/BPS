#ifndef TABLEVIEW_CONTROLLER_H // 防止头文件重复包含
#define TABLEVIEW_CONTROLLER_H

#include <QMenu>
#include <QAction>
#include <QTableView>
#include <functional>
#include <QList>

/**
 * @brief MyController 类用于在 QTableView 中创建自定义的上下文菜单。
 *
 * MyController 允许用户为 QTableView 指定一个自定义的菜单构建器。
 * 当用户在表格的某一行上点击右键时，会根据构建器生成相应的菜单。
 */
class QTableViewController : public QObject {
    Q_OBJECT

public:
    // 菜单构建器类型，它是一个函数，返回 QAction 列表
    using MenuBuilder = std::function<QList<QAction*>(QTableView*, const QModelIndex&)>;

    /**
     * @brief 构造一个 MyController 对象。
     *
     * @param view 指向 QTableView 的指针。
     * @param builder 菜单构建器函数。
     * @param parent 父对象，默认为 nullptr。
     */
    QTableViewController(QTableView *view, MenuBuilder builder, QObject *parent = nullptr)
        : QObject(parent), m_tableView(view), m_buildMenu(std::move(builder)) {
        m_tableView->viewport()->installEventFilter(this);
    }

signals:
    /**
     * @brief 当菜单中的某个动作被触发时发出的信号。
     *
     * @param action 被触发的 QAction。
     * @param index 触发动作的模型索引。
     *
     * @example
     * connect(controller, &MyController::menuActionTriggered,
     *         this, &MainWindow::onMenuActionTriggered);
     */
    void menuActionTriggered(QAction *action, const QModelIndex &index);

protected:
    // 重写 QObject 的 eventFilter 方法
    bool eventFilter(QObject *object, QEvent *event) override {
        // 检查事件是否是右键菜单请求
        if (object == m_tableView->viewport() && event->type() == QEvent::ContextMenu) {
            auto *contextMenuEvent = static_cast<QContextMenuEvent*>(event);
            // 获取鼠标右键点击的模型索引
            QModelIndex index = m_tableView->indexAt(contextMenuEvent->pos());

            // 如果点击的位置是有效的（即在某一行上）
            if (index.isValid()) {
                QMenu menu;
                // 使用提供的函数构建菜单
                QList<QAction*> actions = m_buildMenu(m_tableView, index);
                // 将动作添加到菜单中
                for (QAction *action : actions) {
                    menu.addAction(action);
                }

                // 显示菜单并等待用户选择
                QAction *selectedAction = menu.exec(contextMenuEvent->globalPos());
                // 如果选择了某个动作
                if (selectedAction) {
                    // 发出菜单动作触发的信号
                    emit menuActionTriggered(selectedAction, index);
                }

                return true;
            }
        }
        // 对于其他事件，调用基类的 eventFilter 方法
        return QObject::eventFilter(object, event);
    }

private:
    QTableView *m_tableView; // 指向关联的 QTableView
    MenuBuilder m_buildMenu; // 用于构建菜单的函数
};

#endif // TABLEVIEW_CONTROLLER_H
