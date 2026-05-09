#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QSet>
#include <QPixmap>
#include <QElapsedTimer>

class Player;
class NPC;

// ---------- 游戏阶段 ----------
enum class GamePhase {
    Intro,              // 初始：唐嘉琦在店铺内
    WalkingOut,         // 玩家操控走出店铺
    KidFollow,          // 小孩跟在后面
    PlayerStopLook,     // 玩家停下按E观察
    KidBumped,          // 小孩撞到
    PlayerTurnGive,     // 玩家按Z转身递冰激凌
    KidThanks,          // 小孩感谢
    WalkToDance,        // 走到舞动位置
    Dancing,            // 跳舞
    Complete            // 完成
};

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

    void resetGame();


protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    void updateLogic(float dt);
    void processInput(float dt);
    void checkTriggers();
    void drawBackground(QPainter &p);
    void drawPlayer(QPainter &p);
    void drawNPC(QPainter &p);
    void drawUI(QPainter &p);

    // 资源
    QPixmap m_bgImage;
    QPixmap m_iceCreamPic;
    QPixmap m_shopImage;

    // 角色
    Player *m_player;
    NPC    *m_kid;

    // 循环
    QTimer       *m_timer;
    QElapsedTimer m_clock;
    float         m_deltaTime;

    // 输入
    QSet<int> m_keys;

    // 游戏状态
    GamePhase m_phase;
    float     m_phaseTimer;
    QString   m_hintText;

    // 特殊位置
    struct { float x, y; } m_shopExit;     // 店铺出口
    struct { float x, y; } m_danceSpot;    // 舞动位置

    // 递冰激凌方向
    bool m_playerFacingKid;

    //信号
signals:
    void backToLevelSelect();


};

#endif
