#ifndef FULLSCREENMASK_H
#define FULLSCREENMASK_H

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QDesktopWidget>
#include <QTextToSpeech>

class FullScreenMask : public QWidget {
    Q_OBJECT

private:
    static FullScreenMask *m_instance;
    QLabel *m_label;
    QTimer *m_timer;
    QTextToSpeech *m_speech;

    //
    FullScreenMask() {
        // 设置全屏和其他属性
        setGeometry(QApplication::desktop()->screenGeometry());
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setStyleSheet("background-color: rgba(144, 238, 144, 0.7); border: 2px solid green;");

        // 文本
        m_label = new QLabel(this);
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setStyleSheet("QLabel { color : red; font: 40pt; }");
        m_label->setGeometry(this->rect());

        // 定时器
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &FullScreenMask::close);

        // 设置 QTextToSpeech
        m_speech = new QTextToSpeech(this);
        m_speech->setLocale(QLocale::Chinese); // 设置为中文
        m_speech->setRate(-0.1);//设置语速-1.0到1.0
        m_speech->setPitch(1.0);//设置音高-1.0到1.0
        m_speech->setVolume(1.0);//设置音量0.0-1.0
    }

public:
    static FullScreenMask *getInstance() {
        if (!m_instance) {
            m_instance = new FullScreenMask();
        }
        return m_instance;
    }

    void showMessage(const QString &message, int timeout = 3000, bool isSpeak = false, QString speakMsg = "") {
        // 关闭一次的弹窗
        if (m_instance && m_instance->isVisible()) {
            m_instance->close();
        }

        // 如果正在播放语音，停止
        if (m_speech->state() == QTextToSpeech::Speaking) {
            m_speech->stop();
        }

        // 显示文本
        m_label->setText(message);

        // 是否延迟关闭
        if (timeout > 0) {
            m_timer->start(timeout);
        } else {
            m_timer->stop();
        }

        // 全屏
        showFullScreen();

        // 播放语音
        if (isSpeak) {
            if (speakMsg.isEmpty()){
                m_speech->say(message);
            }else{
                m_speech->say(speakMsg);
            }
        }
    }

    ~FullScreenMask() {
        m_instance = nullptr;
    }
};

#endif // FULLSCREENMASK_H
