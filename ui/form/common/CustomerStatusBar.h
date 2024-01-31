#ifndef CUSTOMERSTATUSBAR_H
#define CUSTOMERSTATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QMap>
#include <QTimer>

class CustomStatusBar : public QWidget {
    Q_OBJECT

public:
    // 构造函数
    CustomStatusBar(QWidget *parent = nullptr) : QWidget(parent) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 0, 10, 0);

        // 初始化信息标签
        infoLabel = new QLabel(this);
        layout->addWidget(infoLabel);

        // 加一个间隔
        layout->addStretch(1);  // 参数1表示弹性空间的比例

        // 图标布局
        iconsLayout = new QHBoxLayout();
        layout->addLayout(iconsLayout);
        layout->addStretch(); // 确保图标右对齐

        setLayout(layout);
    }

    // 左下角 提示文本 的设置
    void setInfoText(const QString &text) {
        QTimer::singleShot(10, this, [this, text](){
            infoLabel->setText(text);
        });
    }

    // 添加图标的公共方法
    void addIcon(const QIcon &icon, const QString &label, const QString &tooltip, std::function<void()> onClick) {
        if (label.isEmpty()) {
            return;  // 如果 label 为空，则不添加按钮
        }

        QAction *action = new QAction(this);
        if (!icon.isNull()) {
            // 如果 icon 不为空，设置为只显示图标
            action->setIcon(icon);
        } else {
            // 如果 icon 为空，设置为只显示 label
            action->setText(label);
        }

        // 如果提供了 tooltip，则设置 tooltip
        if (!tooltip.isEmpty()) {
            action->setToolTip(tooltip);
        }

        QToolButton *button = new QToolButton(this);
        button->setDefaultAction(action);
        connect(button, &QToolButton::clicked, this, onClick);

        // 如果 icon 不为空，设置图标大小
        if (!icon.isNull()) {
            button->setIconSize(QSize(24, 24));
        }

        iconsLayout->addWidget(button);
        buttonMap.insert(label, button);
    }

    void setIconVisibility(const QString &label, bool visible) {
        if (buttonMap.contains(label)) {
            buttonMap[label]->setVisible(visible);
        }
    }

    void setTooltip(const QString &label, const QString &tooltip, bool isError = false) {
        if (buttonMap.contains(label)) {
            QAction *action = buttonMap[label]->defaultAction();
            if (action) {
                action->setToolTip(tooltip);

                // 获取关联的按钮
                QToolButton *button = qobject_cast<QToolButton *>(action->parentWidget());
                if (button) {
                    // 如果 isError 为 true，设置按钮文本的颜色为红色
                    if (isError) {
                        button->setStyleSheet("color: red;");
                    } else {
                        // 重置回默认样式
                        button->setStyleSheet("");
                    }
                }
            }
        }
    }

private:
    QLabel *infoLabel;
    QHBoxLayout *iconsLayout;
    QMap<QString, QToolButton*> buttonMap; // 映射标签到按钮
};

#endif // CUSTOMERSTATUSBAR_H
