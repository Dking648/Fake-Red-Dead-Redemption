#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMediaPlayer>
#include <QAudioOutput>

class MenuWidget;
class LevelSelectWidget;
class GameWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QAudioOutput* audioOutput() const { return m_audioOutput; }

private:
    QStackedWidget  *m_stack;
    MenuWidget      *m_menuPage;
    LevelSelectWidget *m_levelPage;
    GameWidget      *m_gamePage;
    QMediaPlayer    *m_bgmPlayer;
    QAudioOutput    *m_audioOutput;
};

#endif
