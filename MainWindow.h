#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMediaPlayer>
#include <QAudioOutput>

class MenuWidget;
class LevelSelectWidget;
class GameWidget;
class CowboyRunGame;
class Level2Widget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QAudioOutput* audioOutput() const { return m_audioOutput; }

public slots:
    void pauseMainBgm();
    void resumeMainBgm();


private:
    QStackedWidget  *m_stack;
    MenuWidget      *m_menuPage;
    LevelSelectWidget *m_levelPage;
    GameWidget      *m_gamePage;
    QMediaPlayer    *m_bgmPlayer;
    QAudioOutput    *m_audioOutput;
    CowboyRunGame   *m_cowboyPage;
    Level2Widget    *m_level2Page;
};

#endif
