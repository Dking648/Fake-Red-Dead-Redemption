#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPixmap>

class MainWindow;

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(MainWindow *mainWindow, QWidget *parent = nullptr);

signals:
    void storyModeClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    MainWindow *m_mainWindow;
    QPixmap     m_bgImage;
    QPushButton *m_btnStory;
    QPushButton *m_btnDuel;
    QPushButton *m_btnSettings;
};

#endif
