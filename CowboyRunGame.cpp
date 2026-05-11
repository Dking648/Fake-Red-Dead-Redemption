#include "CowboyRunGame.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <cmath>
#include <algorithm>

CowboyRunGame::CowboyRunGame(QWidget *parent)
    : QWidget(parent)
    , m_bpm(130.0f)                      // BPM（根据你的音乐调整）
    , m_beatInterval(60.0f / 130.0f)      // ≈ 0.462s
    , m_songDuration(45.0f)               // 歌曲45秒
    , m_preReadTime(1.2f)                 // 敌人提前1.2秒出现
    , m_judgeLineX(200.0f)                // 判定线位置
    , m_perfectWindow(0.06f)              // Perfect: ±60ms
    , m_goodWindow(0.12f)                 // Good: ±120ms
    , m_enemySpeed(0)
    , m_nextBeatIndex(0)
    , m_gameState(GameState::Over)
    , m_countdownTimer(3.0f)
    , m_gameTime(0)
    , m_score(0)
    , m_combo(0)
    , m_maxCombo(0)
    , m_lives(5)
    , m_perfectCount(0)
    , m_goodCount(0)
    , m_missCount(0)
    , m_lastJudge(JudgeResult::None)
    , m_judgeFlashTimer(0)
    , m_timer(new QTimer(this))
    , m_deltaTime(0)
{
    setFixedSize(1280, 720);
    setFocusPolicy(Qt::StrongFocus);

    // 计算敌人速度
    m_enemySpeed = (1280.0f - m_judgeLineX) / m_preReadTime;

    // 音乐
    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(0.7f);
    m_musicPlayer = new QMediaPlayer(this);
    m_musicPlayer->setAudioOutput(m_audioOutput);
    m_musicPlayer->setSource(QUrl("qrc:/music/cowboy_bgm.mp3"));

    // 加载图片（先用色块代替，有图了再换）
    m_playerImage.load(":/images/tang_body.png");
    m_enemyImage.load(":/images/icecream.png");

    // 生成节拍
    generateBeats();

    // 游戏循环
    connect(m_timer, &QTimer::timeout, this, &CowboyRunGame::gameLoop);

    m_tangHead.load(":/images/tang_head.png");
    m_tangBody.load(":/images/tang_body.png");
    m_tangRightArm.load(":/images/tang_right_arm.png");
    m_tangLeftArm.load(":/images/tang_left_arm.png");
    m_tangRightLeg.load(":/images/tang_right_leg.png");
    m_tangLeftLeg.load(":/images/tang_left_leg.png");


    m_punchTimer = 0;
    m_kickTimer = 0;
    m_punchRot = 0;
    m_kickRot = 0;
}

CowboyRunGame::~CowboyRunGame() {}

// ==================== 生成节拍序列 ====================
void CowboyRunGame::generateBeats()
{
    m_beats.clear();

    QVector<QVector<int>> patterns = {
                                      {0, 1, 0, 1},
                                      {0, 0, 1, 0},
                                      {1, 0, 1, 0},
                                      {0, 1, 1, 0},
                                      {0, 0, 1, 1},
                                      {1, 1, 0, 0},
                                      {0, 1, 0, 1},
                                      {1, 0, 1, 0},
                                      {0, 1, 0, 1},
                                      };

    int totalBeats = m_songDuration / m_beatInterval;
    int patternCount = patterns.size();

    for (int i = 0; i < totalBeats; i++) {
        int pi = (i / 4) % patternCount;
        int bi = i % 4;
        int lane = patterns[pi][bi];

        float time = 2.0f + i * m_beatInterval;
        // 只添加有效轨道
        if (lane >= 0) {
            m_beats.append({time, lane});
        }
    }

    m_nextBeatIndex = 0;
}

// ==================== 主循环 ====================
void CowboyRunGame::gameLoop()
{
    if (m_gameState == GameState::Over) return;

    m_deltaTime = m_clock.elapsed() / 1000.0f;
    m_clock.restart();
    if (m_deltaTime > 0.1f) m_deltaTime = 0.1f;

    // ===== 倒计时阶段 =====
    if (m_gameState == GameState::Preparing) {
        m_countdownTimer -= m_deltaTime;
        if (m_countdownTimer <= 0) {
            m_gameState = GameState::Playing;
            m_gameTime = 0;
            m_musicPlayer->setPosition(0);
            m_musicPlayer->play();
            m_clock.start();
            setFocus();
            activateWindow();
        }

        update();
        return;
    }

    // ===== 游戏进行中 =====
    m_gameTime += m_deltaTime;
    m_judgeFlashTimer -= m_deltaTime;

    // 挥手动画
    if (m_punchTimer > 0) {
        m_punchTimer -= m_deltaTime;
        float t = 1.0f - m_punchTimer / 0.25f;
        if (t < 0.5f) {
            m_punchRot = -120.0f * (t / 0.5f);
        } else {
            m_punchRot = -120.0f * (1.0f - (t - 0.5f) / 0.5f);
        }
        if (m_punchTimer <= 0) {
            m_punchRot = 0;
        }
    }

    // 踢腿动画
    if (m_kickTimer > 0) {
        m_kickTimer -= m_deltaTime;
        float t = 1.0f - m_kickTimer / 0.25f;
        if (t < 0.5f) {
            m_kickRot = 70.0f * (t / 0.5f);
        } else {
            m_kickRot = 70.0f * (1.0f - (t - 0.5f) / 0.5f);
        }
        if (m_kickTimer <= 0) {
            m_kickRot = 0;
        }
    }

    spawnEnemies(m_gameTime);
    updateEnemies(m_deltaTime);

    if (m_lives <= 0 || m_gameTime > m_songDuration + 2.0f) {
        m_gameState = GameState::Over;
        m_musicPlayer->stop();
    }

    update();
}


// ==================== 生成敌人 ====================
void CowboyRunGame::spawnEnemies(float currentTime)
{
    while (m_nextBeatIndex < m_beats.size()) {
        const BeatNote &note = m_beats[m_nextBeatIndex];

        if (note.time - currentTime > m_preReadTime) {
            break;   // 还没到预读时间
        }

        // 生成敌人
        Enemy e;
        e.beatTime = note.time;
        e.lane = note.lane;
        e.x = 1280.0f;
        e.y = (note.lane == 0) ? 250.0f : 450.0f;
        e.active = true;
        e.judged = false;
        m_enemies.append(e);
        m_nextBeatIndex++;
    }
}


// ==================== 更新敌人位置 ====================
void CowboyRunGame::updateEnemies(float dt)
{
    for (int i = m_enemies.size() - 1; i >= 0; i--) {
        Enemy &e = m_enemies[i];
        if (!e.active) continue;

        // 根据节拍时间计算位置
        float timeToBeat = e.beatTime - m_gameTime;
        e.x = m_judgeLineX + timeToBeat * m_enemySpeed;

        // 飞过判定线太远 → Miss
        if (e.x < m_judgeLineX - 80.0f && !e.judged) {
            e.judged = true;
            e.active = false;
            m_combo = 0;
            m_lives--;
            m_missCount++;
            m_lastJudge = JudgeResult::Miss;
            m_judgeFlashTimer = 0.4f;
            m_comboText = "";
        }

        // 超出屏幕左边 → 移除
        if (e.x < -100.0f) {
            m_enemies.remove(i);
        }
    }
}

// ==================== 判定 ====================
void CowboyRunGame::judgeHit(int lane)
{
    float bestDiff = 999.0f;
    int   bestIdx = -1;



    for (int i = 0; i < m_enemies.size(); i++) {
        Enemy &e = m_enemies[i];
        if (!e.active || e.judged) continue;

        float diff = fabs(e.beatTime - m_gameTime);


        if (e.lane != lane) continue;

        if (diff < m_goodWindow && diff < bestDiff) {
            bestDiff = diff;
            bestIdx = i;
        }
    }

    if (bestIdx >= 0) {
        Enemy &e = m_enemies[bestIdx];
        e.judged = true;
        e.active = false;



        if (bestDiff <= m_perfectWindow) {
            m_lastJudge = JudgeResult::Perfect;
            m_score += 100 + m_combo * 5;
            m_combo++;
            m_perfectCount++;
            m_judgeFlashTimer = 0.3f;
            m_comboText = "Perfect!";
            m_comboTextTimer = 0.6f;
        } else {
            m_lastJudge = JudgeResult::Good;
            m_score += 50 + m_combo * 2;
            m_combo++;
            m_goodCount++;
            m_judgeFlashTimer = 0.3f;
            m_comboText = "Good";
            m_comboTextTimer = 0.4f;
        }

        if (m_combo > m_maxCombo) m_maxCombo = m_combo;

        // 触发动划
        if (lane == 0) {
            m_punchTimer = 0.25f;
            m_punchRot = 0;

        }
        if (lane == 1) {
            m_kickTimer = 0.25f;
            m_kickRot = 0;

        }

    } else {

        m_combo = 0;
        m_lastJudge = JudgeResult::Miss;
        m_judgeFlashTimer = 0.2f;
    }
}


// ==================== 键盘 ====================
void CowboyRunGame::keyPressEvent(QKeyEvent *event)
{

    if (event->key() == Qt::Key_Space && m_gameState == GameState::Over) {
        startGame();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        resetGame();                    // 会停音乐、设 Over、发信号恢复主BGM
        emit backToLevelSelect();
        return;
    }


    if (m_gameState != GameState::Playing) return;

    int lane = -1;
    if (event->key() == Qt::Key_W || event->key() == Qt::Key_Up)
        lane = 0;  // 上轨道
    else if (event->key() == Qt::Key_S || event->key() == Qt::Key_Down)
        lane = 1;  // 下轨道

    if (lane >= 0) {
        judgeHit(lane);
    }
}

// ==================== 开始/重置 ====================
void CowboyRunGame::startGame()
{

    m_gameState = GameState::Preparing;
    m_countdownTimer = 3.0f;
    m_gameTime = 0;
    m_score = 0;
    m_combo = 0;
    m_maxCombo = 0;
    m_lives = 5;
    m_perfectCount = 0;
    m_goodCount = 0;
    m_missCount = 0;
    m_lastJudge = JudgeResult::None;
    m_judgeFlashTimer = 0;
    m_nextBeatIndex = 0;
    m_enemies.clear();
    m_comboText = "";
    m_punchTimer = 0;
    m_kickTimer = 0;
    m_punchRot = 0;
    m_kickRot = 0;

    generateBeats();

    emit requestPauseMainBgm();

    m_clock.start();
    m_timer->start(16);
    setFocus();
    update();
    qDebug() << "startGame called!";

}


void CowboyRunGame::resetGame()
{
    m_gameState = GameState::Over;
    m_timer->stop();
    m_musicPlayer->stop();
    m_enemies.clear();

    emit requestResumeMainBgm();

    update();
}


// ==================== 绘制 ====================
void CowboyRunGame::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_gameState == GameState::Preparing) {
        // 倒计时画面
        p.fillRect(rect(), QColor(30, 20, 10));
        drawBackground(p);
        drawPlayer(p);
        drawJudgeLine(p);

        p.setPen(QColor(255, 215, 0));
        p.setFont(QFont("Arial", 80, QFont::Bold));
        int count = (int)ceil(m_countdownTimer);
        p.drawText(width() / 2 - 40, height() / 2, QString::number(count));
        return;
    }

    if (m_gameState == GameState::Over && m_gameTime == 0) {
        // 游戏还没开始过的初始画面
        p.fillRect(rect(), QColor(30, 20, 10));
        // ... 保持原来未开始的画面代码
    }


    drawBackground(p);
    drawEnemies(p);
    drawPlayer(p);
    drawJudgeLine(p);
    drawHitEffect(p);
    drawUI(p);

    if (m_gameState == GameState::Over) {
        p.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 150));
        p.setPen(QColor(255, 215, 0));
        p.setFont(QFont("Microsoft YaHei", 32, QFont::Bold));
        p.drawText(width()/2 - 100, height()/2 - 60, "游戏结束");
        p.setFont(QFont("Microsoft YaHei", 18));
        p.setPen(Qt::white);
        p.drawText(width()/2 - 80, height()/2, QString("得分: %1").arg(m_score));
        p.drawText(width()/2 - 80, height()/2 + 30, QString("最大连击: %1").arg(m_maxCombo));
        p.drawText(width()/2 - 80, height()/2 + 60, QString("Perfect: %1  Good: %2  Miss: %3")
                                                            .arg(m_perfectCount).arg(m_goodCount).arg(m_missCount));
        p.setFont(QFont("Microsoft YaHei", 13));
        p.setPen(QColor(200, 200, 200));
        p.drawText(width()/2 - 50, height()/2 + 100, "按 ESC 退出");
    }
}

void CowboyRunGame::drawBackground(QPainter &p)
{
    // 西部风格背景
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0, QColor(255, 140, 0));     // 橙色黄昏
    sky.setColorAt(0.5, QColor(200, 80, 0));
    sky.setColorAt(1, QColor(80, 40, 20));      // 深棕色地面
    p.fillRect(rect(), sky);

    // 地面
    p.fillRect(0, 560, width(), 160, QColor(60, 30, 10));

    // 轨道线
    p.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DashLine));
    p.drawLine(0, 250, width(), 250);  // 上轨道
    p.drawLine(0, 450, width(), 450);  // 下轨道

    // 轨道标签
    p.setPen(QColor(255, 255, 255, 60));
    p.setFont(QFont("Arial", 20));
    p.drawText(10, 245, "W");
    p.drawText(10, 445, "S");
}

void CowboyRunGame::drawJudgeLine(QPainter &p)
{
    // 判定线
    int lx = (int)m_judgeLineX;

    // 辉光效果
    QLinearGradient glow(lx - 20, 0, lx + 20, 0);
    glow.setColorAt(0, QColor(255, 215, 0, 0));
    glow.setColorAt(0.5, QColor(255, 215, 0, 100));
    glow.setColorAt(1, QColor(255, 215, 0, 0));
    p.fillRect(lx - 20, 0, 40, height(), glow);

    // 主线
    p.setPen(QPen(QColor(255, 215, 0, 200), 3));
    p.drawLine(lx, 0, lx, height());

    // 判定圈（上下轨道处）
    p.setPen(QPen(QColor(255, 215, 0, 180), 2));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(lx, 250), 30, 30);
    p.drawEllipse(QPointF(lx, 450), 30, 30);
}

void CowboyRunGame::drawEnemies(QPainter &p)
{
    for (const Enemy &e : m_enemies) {
        if (!e.active) continue;

        // 飞向判定线的敌人
        p.save();
        p.translate(e.x, e.y);

        // 外圈
        float scale = 0.8f + (m_judgeLineX / e.x) * 0.4f;  // 越近越大
        p.scale(scale, scale);

        // 敌人图标（冰激凌或其他）
        if (!m_enemyImage.isNull()) {
            p.drawPixmap(-20, -20, 40, 40, m_enemyImage);
        } else {
            // 备用：彩色方块
            QColor color = (e.lane == 0) ? QColor(255, 100, 100) : QColor(100, 180, 255);
            p.setBrush(color);
            p.setPen(QPen(Qt::white, 2));
            p.drawRoundedRect(-20, -20, 40, 40, 8, 8);
        }
        p.restore();
    }
}

void CowboyRunGame::drawHitEffect(QPainter &p)
{
    if (m_judgeFlashTimer <= 0) return;

    int lx = (int)m_judgeLineX;

    QColor flashColor;
    switch (m_lastJudge) {
    case JudgeResult::Perfect:
        flashColor = QColor(255, 215, 0, (int)(m_judgeFlashTimer * 500));
        break;
    case JudgeResult::Good:
        flashColor = QColor(100, 255, 100, (int)(m_judgeFlashTimer * 400));
        break;
    case JudgeResult::Miss:
        flashColor = QColor(255, 50, 50, (int)(m_judgeFlashTimer * 400));
        break;
    default:
        break;
    }

    if (flashColor.alpha() > 0) {
        p.setPen(Qt::NoPen);
        p.setBrush(flashColor);
        p.drawEllipse(QPointF(lx, 350), 80, 120);
    }
}

void CowboyRunGame::drawUI(QPainter &p)
{
    // 分数
    p.setPen(QColor(255, 215, 0));
    p.setFont(QFont("Arial", 24, QFont::Bold));
    p.drawText(20, 50, QString("分数: %1").arg(m_score));

    // Combo
    if (m_combo >= 5) {
        p.setPen(QColor(255, 100, 50));
        p.setFont(QFont("Arial", 18, QFont::Bold));
        p.drawText(20, 80, QString("Combo: %1").arg(m_combo));
    }

    // 生命
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 14));
    QString livesStr = "❤️ ";
    for (int i = 0; i < m_lives; i++) livesStr += "❤️";
    p.drawText(width() - 200, 40, livesStr);

    // 进度条
    float progress = m_gameTime / m_songDuration;
    p.fillRect(0, height() - 10, (int)(width() * progress), 10, QColor(255, 215, 0, 150));

    // 判定文字
    if (m_comboTextTimer > 0) {
        p.setPen(QColor(255, 215, 0));
        p.setFont(QFont("Arial", 28, QFont::Bold));
        p.drawText(width() / 2 - 60, 120, m_comboText);
        m_comboTextTimer -= m_deltaTime;
    }
}

void CowboyRunGame::drawPlayer(QPainter &p)
{
    int cx = (int)m_judgeLineX - 60;    // 角色左右位置
    int cy = 350;                        // 角色地面高度


    float scale = 0.75f;

    p.save();
    p.translate(cx, cy);
    p.scale(scale, scale);               // 统一缩放

    // ===== 左臂（自然垂放） =====
    if (!m_tangLeftArm.isNull()) {
        p.save();
        p.translate(-58, -50);
        p.rotate(-10);
        p.drawPixmap(-m_tangLeftArm.width()/2, 0, m_tangLeftArm);
        p.restore();
    }

    // ===== 身体 =====
    if (!m_tangBody.isNull()) {
        p.drawPixmap(-m_tangBody.width()/2, -65, m_tangBody);
    }

    // ===== 左腿 =====
    if (!m_tangLeftLeg.isNull()) {
        p.save();
        p.translate(-15, 26);
        p.drawPixmap(-m_tangLeftLeg.width()/2, 0, m_tangLeftLeg);
        p.restore();
    }

    // ===== 右腿（踢腿动画） =====
    if (!m_tangRightLeg.isNull()) {
        p.save();
        p.translate(10, 26);
        p.rotate(-m_kickRot);
        p.drawPixmap(-m_tangRightLeg.width()/2, 0, m_tangRightLeg);
        p.restore();
    }

    // ===== 头 =====
    if (!m_tangHead.isNull()) {
        p.drawPixmap(-m_tangHead.width()/2, -145, m_tangHead);
    }

    // ===== 右臂（挥手动画） =====
    if (!m_tangRightArm.isNull()) {
        p.save();
        p.translate(53, -55);
        p.rotate(m_punchRot);
        p.drawPixmap(-m_tangRightArm.width()/2, 0, m_tangRightArm);
        p.restore();
    }

    p.restore();
}


