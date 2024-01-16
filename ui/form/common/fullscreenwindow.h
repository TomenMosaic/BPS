#ifndef FULLSCREENWINDOW_H
#define FULLSCREENWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QDesktopWidget>
#include <QTextToSpeech>

class FullScreenWindow : public QWidget {
    static FullScreenWindow *currentInstance;

public:
    FullScreenWindow(const QString &textToShow) {
        // 关闭当前运行的实例
        if (currentInstance) {
            currentInstance->close();
        }
        currentInstance = this;

        // 设置全屏
        this->setGeometry(QApplication::desktop()->screenGeometry());
        this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        this->setAttribute(Qt::WA_TranslucentBackground); // 设置背景透明

        // 添加标签显示文本
        QLabel *label = new QLabel(textToShow, this);
        label->setStyleSheet("QLabel { color : white; font: 20pt; }");
        label->setAlignment(Qt::AlignCenter);
        label->setGeometry(QApplication::desktop()->screenGeometry());

        // 设置定时器，10秒后关闭遮罩层
        QTimer::singleShot(10000, this, &QWidget::close);
    }

    ~FullScreenWindow() {
        if (currentInstance == this) {
            currentInstance = nullptr;
        }
    }

    void speak(const QString &text) {
        QTextToSpeech *speech = new QTextToSpeech(this);
        speech->say(text);
    }
};

#endif // FULLSCREENWINDOW_H
