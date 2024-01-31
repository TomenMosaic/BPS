#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QMovie>

class LoadingWidget : public QWidget {
public:
    // 构造函数
    LoadingWidget(QWidget *parent = nullptr, bool showProgressBar = true)
        : QWidget(parent), progressBarVisible(showProgressBar) {
        if (parent) {
            setGeometry(parent->geometry());
        }

        // 设置为无边框的对话框风格，并使背景透明
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        setStyleSheet("background-color: rgba(144, 238, 144, 0.5);");

        // 设置为应用程序模态，阻止与其他窗口的交互
        setWindowModality(Qt::ApplicationModal);

        // 创建布局
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter); // 居中对齐
        setLayout(layout);

        if (showProgressBar){
            // 进度条
            progressBar = new QProgressBar(this);
            progressBar->setRange(0, 100); // 设置进度条的范围
            layout->addWidget(progressBar);
            progressBar->setVisible(showProgressBar); // 根据参数决定是否显示进度条
        }else{
            progressBar = nullptr;

            // 加载动画
            QLabel *animationLabel = new QLabel(this);
            QMovie *movie = new QMovie(":/image/loading.gif");
            animationLabel->setMovie(movie);
            movie->start();
            layout->addWidget(animationLabel);
        }

        // 信息标签
        infoLabel = new QLabel("正在处理...", this);
        layout->addWidget(infoLabel);
    }

    // 设置进度条的进度
    void setProgress(int value) {
        if (progressBarVisible && progressBar) {
            progressBar->setValue(value);
        }
    }

    // 设置信息标签的文本
    void setInfo(const QString &info) {
        infoLabel->setText(info);
    }

    void showEvent(QShowEvent *event) override {
        QWidget::showEvent(event);
        raise();
    }


private:
    QLabel *infoLabel;
    QProgressBar *progressBar;
    bool progressBarVisible; // 控制进度条是否可见
};

#endif // LOADINGWIDGET_H
