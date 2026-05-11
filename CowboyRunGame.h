#ifndef COWBOYRUNGAME_H
#define COWBOYRUNGAME_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVector>

// ---------- 节拍数据 ----------
struct BeatNote {
    float time;     // 节拍时间点（秒）
    int   lane;     // 0=上轨道, 1=下轨道
};

// ---------- 活跃敌人 ----------
struct Enemy {
    float beatTime;  // 节拍时间
    int   lane;      // 轨道
    float x, y;      // 屏幕位置
    bool  active;
    bool  judged;    // 是否已被判定
};

// ---------- 判定结果 ----------
enum class JudgeResult { None, Perfect, Good, Miss };

class CowboyRunGame : public QWidget
{
    Q_OBJECT

public:
    explicit CowboyRunGame(QWidget *parent = nullptr);
    ~CowboyRunGame();

    void startGame();
    void resetGame();

signals:
    void backToLevelSelect();
    void requestPauseMainBgm();
    void requestResumeMainBgm();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    void generateBeats();              // 生成节拍序列
    void updateEnemies(float dt);      // 更新敌人位置
    void spawnEnemies(float currentTime); // 生成新敌人
    void judgeHit(int lane);           // 判定按键
    void drawBackground(QPainter &p);
    void drawJudgeLine(QPainter &p);
    void drawEnemies(QPainter &p);
    void drawUI(QPainter &p);
    void drawHitEffect(QPainter &p);
    void drawPlayer(QPainter &p);


    // 游戏参数
    float m_bpm;                // BPM
    float m_beatInterval;       // 每拍间隔（秒）
    float m_songDuration;       // 歌曲时长
    float m_preReadTime;        // 预读时间（敌人提前出现）

    // 判定参数
    float m_judgeLineX;         // 判定线 X 坐标
    float m_perfectWindow;      // Perfect 窗口（秒）
    float m_goodWindow;         // Good 窗口（秒）
    float m_enemySpeed;         // 敌人移动速度

    // 节拍数据
    QVector<BeatNote> m_beats;
    int m_nextBeatIndex;

    // 活跃敌人
    QVector<Enemy> m_enemies;

    // 游戏状态
    enum class GameState { Preparing, Playing, Over };
    GameState m_gameState;
    float     m_countdownTimer;
    float   m_gameTime;
    int     m_score;
    int     m_combo;
    int     m_maxCombo;
    int     m_lives;
    int     m_perfectCount;
    int     m_goodCount;
    int     m_missCount;

    // 判定特效
    JudgeResult m_lastJudge;
    float       m_judgeFlashTimer;

    // 计时
    QTimer       *m_timer;
    QElapsedTimer m_clock;
    float         m_deltaTime;

    // 音乐
    QMediaPlayer *m_musicPlayer;
    QAudioOutput *m_audioOutput;

    // 图片
    QPixmap m_playerImage;
    QPixmap m_enemyImage;

    // Combo 大字
    QString m_comboText;
    float   m_comboTextTimer;

    // ===== 唐嘉琦模型 =====
    QPixmap m_tangHead;
    QPixmap m_tangBody;
    QPixmap m_tangRightArm;
    QPixmap m_tangLeftArm;
    QPixmap m_tangRightLeg;
    QPixmap m_tangLeftLeg;

    // ===== 动画状态 =====
    float m_punchTimer;    // 挥手动画计时
    float m_kickTimer;     // 踢腿动画计时
    float m_punchRot;      // 手臂当前旋转角度
    float m_kickRot;       // 腿部当前旋转角度

};

#endif
