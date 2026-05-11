#ifndef LEVEL2WIDGET_H
#define LEVEL2WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QSet>
#include <QPixmap>
#include <QElapsedTimer>

class Player;

// ===== 关卡阶段 =====
enum class L2Phase {
    Intro,          // 初始：在超市内，走向红枣
    PickDate,       // 捡起红枣
    PushCart,       // 推购物车到收银台
    Checkout,       // 结账中
    WalkOut,        // 走出超市
    EatDate,        // 吃红枣
    ManAppears,     // 男人出现，往购物车放东西
    CartRunsAway,   // 购物车失控
    FlashToCart,    // 唐嘉琦闪现停购物车
    ManThanks,      // 男人感谢
    ManLeaves,      // 男人离开
    WalkToDance,    // 走到舞动区
    Dancing,        // 跳舞
    Complete        // 完成
};

class Level2Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Level2Widget(QWidget *parent = nullptr);
    ~Level2Widget();
    void resetGame();

signals:
    void backToLevelSelect();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    void updateLogic(float dt);
    void processInput(float dt);
    void drawBackground(QPainter &p);
    void drawPlayer(QPainter &p);
    void drawMan(QPainter &p);
    void drawCart(QPainter &p);
    void drawDate(QPainter &p);
    void drawUI(QPainter &p);
    void drawFlashEffect(QPainter &p);
    void drawSingleCart(QPainter &p, float cx, float cy, bool hasDateInside, bool flip);

    // 资源
    QPixmap m_bgImage;
    QPixmap m_datePic;          // 红枣图片
    QPixmap m_cartPic;          // 购物车图片
    QPixmap m_manPic;           // 男人整图

    // 角色
    Player *m_player;

    // NPC 和物体位置
    float m_manX, m_manY;
    // 两辆购物车
    float m_cart1X, m_cart1Y;   // 唐嘉琦的购物车
    float m_cart2X, m_cart2Y;   // 男人的购物车
    bool  m_cart1Visible;       // cart1 是否可见
    bool  m_cart2Visible;       // cart2 是否可见

    float m_dateX, m_dateY;
    float m_counterX, m_counterY;   // 收银台位置
    float m_exitX, m_exitY;         // 超市出口
    float m_danceSpotX, m_danceSpotY;

    // 游戏循环
    QTimer       *m_timer;
    QElapsedTimer m_clock;
    float         m_deltaTime;

    // 输入
    QSet<int> m_keys;

    // 状态
    L2Phase m_phase;
    float   m_phaseTimer;
    QString m_hintText;

    // 标记
    bool m_hasDate;
    bool m_dateEaten;
    bool m_atCounter;
    bool m_cartStopped;
    bool m_manThanked;

    // 特别动画
    float m_flashTimer;      // 闪现特效
    float m_eatTimer;        // 吃红枣计时
    float m_manAlpha;        // 男人透明度
    float m_flashX, m_flashY;// 闪现起点
};

#endif
