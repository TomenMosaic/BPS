#ifndef CUSTOMERSTATUSBAR_H
#define CUSTOMERSTATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QMap>

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

        // 图标布局
        iconsLayout = new QHBoxLayout();
        layout->addLayout(iconsLayout);
        layout->addStretch(); // 确保图标右对齐

        setLayout(layout);
    }

    // 设置提示文本的公共方法
    void setInfoText(const QString &text) {
        infoLabel->setText(text);
    }

    // 添加图标的公共方法
    void addIcon(const QIcon &icon, const QString &tooltip, std::function<void()> onClick) {
        QAction *action = new QAction(icon, "", this);
        action->setToolTip(tooltip);

        QToolButton *button = new QToolButton(this);
        button->setDefaultAction(action);
        connect(button, &QToolButton::clicked, this, onClick);

        iconsLayout->addWidget(button);
        iconActions.insert(button, action);
    }

    // 修改图标显示状态及提示的方法
    void setIconVisibility(QToolButton *button, bool visible) {
        if (iconActions.contains(button)) {
            button->setVisible(visible);
        }
    }

private:
    QLabel *infoLabel; // 用于显示提示信息的标签
    QHBoxLayout *iconsLayout; // 用于放置图标的布局
    QMap<QToolButton*, QAction*> iconActions; // 存储图标和动作的映射
};

#endif // CUSTOMERSTATUSBAR_H
