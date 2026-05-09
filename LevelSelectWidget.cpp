#include "LevelSelectWidget.h"
#include <QPainter>
#include <QMessageBox>

LevelSelectWidget::LevelSelectWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(1280, 720);
    m_bgImage.load(":/images/menu_bg.jpg");

    QString btnStyle =
        "QPushButton {"
        "  background-color: rgba(60, 40, 20, 200);"
        "  color: gold;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "  border: 2px solid gold;"
        "  border-radius: 8px;"
        "  padding: 20px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(100, 70, 30, 220);"
        "}";

    // 第一关
    m_btnLevel1 = new QPushButton("🍦 第一关\n日行一善 · 冰激凌", this);
    m_btnLevel1->setStyleSheet(btnStyle);
    m_btnLevel1->setFixedSize(300, 100);
    m_btnLevel1->move(width() / 2 - 340, 300);
    connect(m_btnLevel1, &QPushButton::clicked, this, &LevelSelectWidget::level1Clicked);

    // 第二关
    m_btnLevel2 = new QPushButton("🔒 第二关\n？？？", this);
    m_btnLevel2->setStyleSheet(btnStyle);
    m_btnLevel2->setFixedSize(300, 100);
    m_btnLevel2->move(width() / 2 + 40, 300);
    connect(m_btnLevel2, &QPushButton::clicked, this, []() {
        QMessageBox::information(nullptr, "提示", "第二关正在开发中，敬请期待！");
    });

    // 返回按钮
    m_btnBack = new QPushButton("← 返回主菜单", this);
    m_btnBack->setStyleSheet(
        "QPushButton { background: transparent; color: gold; font-size: 16px;"
        "border: 1px solid gold; border-radius: 6px; padding: 8px 20px; }"
        "QPushButton:hover { background: rgba(100,70,30,180); }"
        );
    m_btnBack->setFixedSize(160, 40);
    m_btnBack->move(20, 20);
    connect(m_btnBack, &QPushButton::clicked, this, &LevelSelectWidget::backClicked);
}

void LevelSelectWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (!m_bgImage.isNull()) {
        p.drawPixmap(0, 0, width(), height(), m_bgImage);
    } else {
        p.fillRect(rect(), QColor(30, 15, 5));
    }

    p.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 100));

    p.setPen(QColor(255, 215, 0));
    p.setFont(QFont("Microsoft YaHei", 36, QFont::Bold));
    p.drawText(width() / 2 - 120, 150, "选择关卡");

    p.setFont(QFont("Microsoft YaHei", 14));
    p.setPen(QColor(200, 180, 140));
    p.drawText(width() / 2 - 60, 200, "故事模式");
}
