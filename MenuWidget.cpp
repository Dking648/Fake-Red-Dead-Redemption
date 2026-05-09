#include "MenuWidget.h"
#include "MainWindow.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QSlider>
#include <QMessageBox>
#include <QApplication>

MenuWidget::MenuWidget(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
{
    setFixedSize(1280, 720);
    m_bgImage.load(":/images/menu_bg.jpg");

    QString btnStyle =
        "QPushButton {"
        "  background-color: rgba(60, 40, 20, 200);"
        "  color: gold;"
        "  font-size: 22px;"
        "  font-weight: bold;"
        "  border: 2px solid gold;"
        "  border-radius: 8px;"
        "  padding: 12px 0px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(100, 70, 30, 220);"
        "  border: 3px solid yellow;"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(40, 20, 10, 220);"
        "}";

    // 故事模式按钮
    m_btnStory = new QPushButton("📖 故事模式", this);
    m_btnStory->setStyleSheet(btnStyle);
    m_btnStory->setFixedSize(280, 55);
    m_btnStory->move(width() / 2 - 140, 380);
    connect(m_btnStory, &QPushButton::clicked, this, &MenuWidget::storyModeClicked);

    // 牛仔快跑按钮
    m_btnDuel = new QPushButton("🏃 牛仔快跑", this);
    m_btnDuel->setStyleSheet(btnStyle);
    m_btnDuel->setFixedSize(280, 55);
    m_btnDuel->move(width() / 2 - 140, 455);
    connect(m_btnDuel, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "牛仔快跑模式即将开放，敬请期待！");
    });

    // 设置按钮
    m_btnSettings = new QPushButton("⚙️ 设置", this);
    m_btnSettings->setStyleSheet(btnStyle);
    m_btnSettings->setFixedSize(280, 55);
    m_btnSettings->move(width() / 2 - 140, 530);
    connect(m_btnSettings, &QPushButton::clicked, this, [this]() {
        QDialog *dlg = new QDialog(this);
        dlg->setWindowTitle("设置");
        dlg->setFixedSize(350, 200);
        dlg->setStyleSheet("background-color: #2a1a0a;");

        QVBoxLayout *layout = new QVBoxLayout(dlg);

        QLabel *volLabel = new QLabel("🔊 音量", dlg);
        volLabel->setStyleSheet("color: gold; font-size: 16px;");
        layout->addWidget(volLabel);

        QSlider *volSlider = new QSlider(Qt::Horizontal, dlg);
        volSlider->setRange(0, 100);
        volSlider->setValue(50);
        layout->addWidget(volSlider);

        connect(volSlider, &QSlider::valueChanged, this, [this](int v) {
            m_mainWindow->audioOutput()->setVolume(v / 100.0);
        });

        QPushButton *quitBtn = new QPushButton("退出游戏", dlg);
        quitBtn->setStyleSheet(
            "QPushButton { background-color: #8b0000; color: white; font-size: 16px;"
            "border: 2px solid #ff4444; border-radius: 6px; padding: 8px; }"
            "QPushButton:hover { background-color: #aa0000; }"
            );
        layout->addWidget(quitBtn);

        connect(quitBtn, &QPushButton::clicked, this, []() {
            QApplication::quit();
        });

        dlg->exec();
    });
}

void MenuWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    // 背景图
    if (!m_bgImage.isNull()) {
        p.drawPixmap(0, 0, width(), height(), m_bgImage);
    } else {
        p.fillRect(rect(), QColor(30, 15, 5));
    }

    // 半透明遮罩
    p.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 100));

    // 标题
    QFont titleFont("Microsoft YaHei", 52, QFont::Bold);
    p.setFont(titleFont);

    // 阴影
    p.setPen(QColor(0, 0, 0, 150));
    p.drawText(width() / 2 - 242, 162, "荒野大嫖客");
    // 主体
    p.setPen(QColor(255, 215, 0));
    p.drawText(width() / 2 - 240, 160, "荒野大嫖客");

    // 副标题
    p.setFont(QFont("Microsoft YaHei", 14));
    p.setPen(QColor(200, 180, 140));
    p.drawText(width() / 2 - 80, 200, "— 日行一善 · 雷霆舞动 —");

    // 版本号
    p.setFont(QFont("Arial", 10));
    p.setPen(QColor(150, 150, 150));
    p.drawText(width() / 2 - 30, height() - 20, "v1.0  Beta");
}
