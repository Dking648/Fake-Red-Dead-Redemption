#ifndef LEVELSELECTWIDGET_H
#define LEVELSELECTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPixmap>

class LevelSelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LevelSelectWidget(QWidget *parent = nullptr);

signals:
    void level1Clicked();
    void backClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap     m_bgImage;
    QPushButton *m_btnLevel1;
    QPushButton *m_btnLevel2;
    QPushButton *m_btnBack;
};

#endif
