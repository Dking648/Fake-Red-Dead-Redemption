#include "MainWindow.h"
#include "MenuWidget.h"
#include "LevelSelectWidget.h"
#include "GameWidget.h"
#include <QUrl>
#include "CowboyRunGame.h"
#include "Level2Widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("荒野大嫖客");
    resize(1280, 720);

    //   页面容器
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    //   页面0：主菜单
    m_menuPage = new MenuWidget(this);
    m_stack->addWidget(m_menuPage);

    //   页面1：关卡选择
    m_levelPage = new LevelSelectWidget(this);
    m_stack->addWidget(m_levelPage);

    //   页面2：第一关游戏
    m_gamePage = new GameWidget(this);
    m_stack->addWidget(m_gamePage);

    //   页面3：牛仔快跑模式
    m_cowboyPage = new CowboyRunGame(this);
    m_stack->addWidget(m_cowboyPage);

    //   页面3：第二关游戏
    m_level2Page = new Level2Widget(this);
    m_stack->addWidget(m_level2Page);

    // 默认显示主菜单
    m_stack->setCurrentIndex(0);

    //   信号连接
    connect(m_cowboyPage, &CowboyRunGame::backToLevelSelect, this, [this]() {
        m_cowboyPage->resetGame();
        m_stack->setCurrentIndex(1);
    });
    connect(m_menuPage, &MenuWidget::cowboyRunClicked, this, [this]() {
        m_cowboyPage->startGame();
        m_stack->setCurrentIndex(3);      // 牛仔快跑页面
        m_cowboyPage->setFocus();
    });
    connect(m_cowboyPage, &CowboyRunGame::requestPauseMainBgm, this, &MainWindow::pauseMainBgm);
    connect(m_cowboyPage, &CowboyRunGame::requestResumeMainBgm, this, &MainWindow::resumeMainBgm);

    connect(m_levelPage, &LevelSelectWidget::level2Clicked, this, [this]() {
        m_level2Page->resetGame();
        m_stack->setCurrentIndex(4);
        m_level2Page->setFocus();
    });
    connect(m_level2Page, &Level2Widget::backToLevelSelect, this, [this]() {
        m_stack->setCurrentIndex(1);
    });

    // 故事模式 → 关卡选择
    connect(m_menuPage, &MenuWidget::storyModeClicked, this, [this]() {
        m_stack->setCurrentIndex(1);
    });

    // 关卡选择 → 第一关
    connect(m_levelPage, &LevelSelectWidget::level1Clicked, this, [this]() {
        m_gamePage->resetGame();
        m_stack->setCurrentIndex(2);
        m_gamePage->setFocus();
    });

    // 游戏里 Esc → 关卡选择
    connect(m_gamePage, &GameWidget::backToLevelSelect, this, [this]() {
        m_stack->setCurrentIndex(1);
    });

    // 关卡选择返回 → 主菜单
    connect(m_levelPage, &LevelSelectWidget::backClicked, this, [this]() {
        m_stack->setCurrentIndex(0);
    });

    //   背景音乐
    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(0.5);

    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmPlayer->setAudioOutput(m_audioOutput);
    m_bgmPlayer->setSource(QUrl("qrc:/music/bgm.mp3"));
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
    m_bgmPlayer->play();

}

MainWindow::~MainWindow() {}

void MainWindow::pauseMainBgm()
{
    if (m_bgmPlayer && m_bgmPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_bgmPlayer->pause();
    }
}

void MainWindow::resumeMainBgm()
{
    if (m_bgmPlayer) {
        m_bgmPlayer->play();
    }
}
