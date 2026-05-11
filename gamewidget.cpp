#include "GameWidget.h"
#include "Player.h"
#include "NPC.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <cmath>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , m_player(nullptr)
    , m_kid(nullptr)
    , m_timer(new QTimer(this))
    , m_deltaTime(0)
    , m_phase(GamePhase::Intro)
    , m_phaseTimer(0)
    , m_hintText("")
    , m_playerFacingKid(false)
{
    setFocusPolicy(Qt::StrongFocus);

    // 加载背景
    m_shopImage.load(":/images/shop_bg.png");
    m_iceCreamPic.load(":/images/icecream.png");

    // 创建玩家唐嘉琦
    m_player = new Player();
    m_player->loadParts(
        ":/images/tang_head.png",
        ":/images/tang_body.png",
        ":/images/tang_left_arm.png",
        ":/images/tang_right_arm.png",
        ":/images/tang_left_leg.png",
        ":/images/tang_right_leg.png"
        );
    m_player->setPos(130, 260);
    m_player->setHasIceCream(true);
    m_player->setWalkStyle(WalkStyle::Thunder);

    // 创建小孩NPC
    m_kid = new NPC();
    m_kid->loadImage(":/images/kid_full.png");
    m_kid->setPos(350, 280);  // 初始在店铺外远处
    m_kid->setHasIceCream(true);

    // 关键位置
    m_shopExit  = {260, 280};   // 店铺出口坐标
    m_danceSpot = {640, 400};   // 舞动位置

    // 启动
    connect(m_timer, &QTimer::timeout, this, &GameWidget::gameLoop);
    m_timer->start(16);
    m_clock.start();

    m_hintText = "任务：用 WASD 控制唐嘉琦从蜜雪冰城走出，手上拿着冰激凌";
}

GameWidget::~GameWidget()
{
    delete m_player;
    delete m_kid;
}

// ==================== 游戏循环 ====================
void GameWidget::gameLoop()
{
    m_deltaTime = m_clock.elapsed() / 1000.0f;
    m_clock.restart();
    if (m_deltaTime > 0.1f) m_deltaTime = 0.1f;

    m_phaseTimer += m_deltaTime;

    updateLogic(m_deltaTime);
    processInput(m_deltaTime);
    checkTriggers();

    m_player->update(m_deltaTime);
    m_kid->update(m_deltaTime);

    update();
}

// ==================== 逻辑更新 ====================
void GameWidget::updateLogic(float dt)
{
    float px = m_player->x();
    float py = m_player->y();

    switch (m_phase) {

    // ===== 阶段1：初始，等玩家走出店铺 =====
    case GamePhase::Intro:
        if (px > m_shopExit.x) {
            m_phase = GamePhase::WalkingOut;
            m_hintText = "走出店铺！小孩在附近...";
        }
        break;

    // ===== 阶段2：玩家已走出店铺 =====
    case GamePhase::WalkingOut:
        if (px > m_shopExit.x + 40) {
            m_phase = GamePhase::KidFollow;
            m_kid->follow(m_player, 70.0f);   // 小孩跟在后面
            m_hintText = "按 E 停下观察四周";
        }
        break;

    // ===== 阶段3：小孩跟随中，等玩家按E =====
    case GamePhase::KidFollow:
        break;

    // ===== 阶段4：玩家按E观察四周，小孩继续靠近 =====
    case GamePhase::PlayerStopLook: {
        // 让小孩靠近到背后（缩小跟随距离到30）
        m_kid->follow(m_player, 30.0f);

        float dx = m_player->x() - m_kid->x();
        float dy = m_player->y() - m_kid->y();
        float dist = sqrt(dx * dx + dy * dy);

        // 距离小于50就触发碰撞
        if (dist < 50.0f && m_phaseTimer > 0.8f) {
            m_phase = GamePhase::KidBumped;
            m_phaseTimer = 0;
            m_kid->onBumped();
            m_player->stopLookAround();
            m_player->stop();
            m_hintText = "😱 小孩撞到了唐嘉琦！冰激凌掉地上了...\n按 Z 转身递出自己的冰激凌";
        }
        break;
    }

    // ===== 阶段5：碰撞后，等玩家按Z =====
    case GamePhase::KidBumped:
        break;

    // ===== 阶段6：玩家递冰激凌 =====
    case GamePhase::PlayerTurnGive:
        if (m_phaseTimer > 1.0f) {
            m_phase = GamePhase::KidThanks;
            m_phaseTimer = 0;
            m_kid->onThank();
            m_player->finishGiveIceCream();
            m_hintText = "小孩收到冰激凌，开心地笑了！😊";
        }
        break;

    // ===== 阶段7：小孩感谢 =====
    case GamePhase::KidThanks:
        if (m_kid->isThankDone()) {
            m_phase = GamePhase::WalkToDance;
            m_phaseTimer = 0;
            // 小孩迅速跑出画面
            m_kid->stopFollowing();
            // 给小孩一个目标：画面右边外
            m_hintText = "走到画面中心开始mvp结算画面！";
        }
        break;

    // ===== 阶段8：等玩家走到中央 =====
    case GamePhase::WalkToDance: {
        // 小孩自动向右跑出画面
        m_kid->setPos(m_kid->x() + 300.0f * dt, m_kid->y());
        m_kid->setWalking(true);
        m_kid->setFacing(Dir::Right);

        // 玩家走到画面中央（x 在 500~750，y 在 300~500）
        if (px > 480 && px < 750 && py > 280 && py < 500) {
            m_phase = GamePhase::Dancing;
            m_phaseTimer = 0;
            m_player->stop();
            m_player->startDancing();
            m_hintText = "🎵 雷霆舞动中...";
        }
        break;
    }

    // ===== 阶段9：跳舞中 =====
    case GamePhase::Dancing:
        if (m_phaseTimer > 15.0f) {
            m_phase = GamePhase::Complete;
            m_player->stopDancing();
            m_hintText = "🎉 任务完成！唐嘉琦日行一善！";
        }
        break;

    // ===== 完成 =====
    case GamePhase::Complete:
        break;
    }
}



// ==================== 输入处理 ====================
void GameWidget::processInput(float dt)
{
    if (!m_player) return;
    if (m_phase == GamePhase::Complete) return;
    if (m_phase == GamePhase::Dancing) return;
    if (m_phase == GamePhase::PlayerStopLook) return;
    if (m_phase == GamePhase::PlayerTurnGive) return;

    // ✅ WalkToDance 阶段可以移动
    // ✅ KidThanks 阶段也可以移动（提前走向中央）

    float vx = 0, vy = 0;
    if (m_keys.contains(Qt::Key_W)) vy -= 1;
    if (m_keys.contains(Qt::Key_S)) vy += 1;
    if (m_keys.contains(Qt::Key_A)) vx -= 1;
    if (m_keys.contains(Qt::Key_D)) vx += 1;

    if (vx != 0 || vy != 0) {
        float len = sqrt(vx * vx + vy * vy);
        vx /= len;
        vy /= len;

        m_player->move(vx, vy, dt);
        m_player->setWalking(true);

        if (fabs(vx) > fabs(vy))
            m_player->setFacing(vx > 0 ? Dir::Right : Dir::Left);
        else
            m_player->setFacing(vy > 0 ? Dir::Down : Dir::Up);
    } else {
        m_player->setWalking(false);
    }
}


// ==================== 触发检测 ====================
void GameWidget::checkTriggers()
{
    // 此函数目前空置，逻辑已在 updateLogic 中处理
}

// ==================== 键盘事件 ====================
void GameWidget::keyPressEvent(QKeyEvent *event)
{
    // Esc 返回关卡选择
    if (event->key() == Qt::Key_Escape) {
        emit backToLevelSelect();
        return;
    }

    m_keys.insert(event->key());

    int key = event->key();

    // E 键：观察四周
    if (key == Qt::Key_E && m_phase == GamePhase::KidFollow && !m_player->isLookingAround()) {
        m_phase = GamePhase::PlayerStopLook;
        m_phaseTimer = 0;
        m_player->stop();
        m_player->startLookAround();
        m_hintText = "唐嘉琦正在观察四周...";
    }

    // Z 键：转身递冰激凌
    if (key == Qt::Key_Z && m_phase == GamePhase::KidBumped) {
        m_phase = GamePhase::PlayerTurnGive;
        m_phaseTimer = 0;

        // 转身面对小孩
        if (m_kid->x() > m_player->x())
            m_player->setFacing(Dir::Right);
        else
            m_player->setFacing(Dir::Left);

        m_player->startGiveIceCream();
        m_hintText = "唐嘉琦把自己的冰激凌递给了小孩...";
    }

    QWidget::keyPressEvent(event);
}

void GameWidget::keyReleaseEvent(QKeyEvent *event)
{
    m_keys.remove(event->key());
    QWidget::keyReleaseEvent(event);
}

// ==================== 绘制 ====================
void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    drawBackground(p);
    drawPlayer(p);
    drawNPC(p);
    drawUI(p);
}

void GameWidget::drawBackground(QPainter &p)
{
    // 天空
    p.fillRect(0, 0, width(), 560, QColor(135, 206, 235));

    // 地面
    p.fillRect(0, 560, width(), 160, QColor(140, 200, 110));

    // ===== 蜜雪冰城店铺图片（左上角） =====
    if (!m_shopImage.isNull()) {
        int shopW = 280;
        int shopH = 350;
        int shopX = 0;
        int shopY = 0;
        p.drawPixmap(shopX, shopY, shopW, shopH, m_shopImage);
    } else {
        // 没有图片时画色块
        p.fillRect(0, 0, 280, 350, QColor(220, 200, 170));
        p.setPen(Qt::black);
        p.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
        p.drawText(80, 40, "🍦 蜜雪冰城");
        p.fillRect(250, 240, 30, 110, QColor(100, 60, 30));
        p.setPen(Qt::white);
        p.setFont(QFont("Microsoft YaHei", 9));
        p.drawText(252, 280, "出\n口");
    }

    // ===== 舞动区标记（画面中央） =====
    p.setPen(QPen(QColor(255, 215, 0, 120), 3, Qt::DashLine));
    p.drawEllipse(QPointF(m_danceSpot.x, m_danceSpot.y), 120, 120);
    p.setPen(QColor(255, 215, 0));
    p.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    p.drawText(m_danceSpot.x - 60, m_danceSpot.y - 130, "✨ 舞动区 ✨");
}


// ---------- 绘制玩家（拼接模式） ----------
void GameWidget::drawPlayer(QPainter &p)
{
    if (!m_player) return;

    // ========== 🔧 比例调整区（改这些数字就行）==========
    float headScale    = 1.0f;    // 头缩放比例
    float bodyScale    = 1.0f;    // 身体缩放比例
    float armScale     = 1.0f;    // 手臂缩放比例
    float legScale     = 1.0f;    // 腿缩放比例

    float headY        = -105.0f;  // 头 Y 位置（负数=往上）
    float bodyY        = -65.0f;  // 身体 Y 位置
    float armX         = 52.0f;   // 手臂 X 偏移（肩膀宽度）
    float armY         = -48.0f;  // 手臂 Y 位置
    float legX         = 18.0f;   // 腿 X 偏移
    float legY         = 36.0f;   // 腿 Y 位置
    // ====================================================

    p.save();
    p.translate(m_player->x(), m_player->y());

    float s = m_player->scale();
    p.scale(s, s);

    float bob = m_player->bounceOffset();
    p.translate(0, -bob);

    Dir facing = m_player->facing();
    if (facing == Dir::Left) {
        p.scale(-1, 1);
    }

    float toeLift    = m_player->toeLift();
    float armSwing   = m_player->isWalking() ? sin(m_player->animPhase()) * 22.0f : 0;
    float armExtraL  = m_player->leftArmExtraRot();
    float armExtraR  = m_player->rightArmExtraRot();
    float bodyRot    = m_player->bodyExtraRot();
    float headLook   = m_player->headLookAngle();

    p.rotate(bodyRot);

    // --- 身体 ---
    QPixmap &body = m_player->bodyPix();
    if (!body.isNull()) {
        float bw = body.width() * bodyScale;
        float bh = body.height() * bodyScale;
        p.drawPixmap(-bw / 2, bodyY, bw, bh, body);
    }

    // --- 头部 ---
    QPixmap &head = m_player->headPix();
    if (!head.isNull()) {
        p.save();
        p.translate(0, headY);
        p.rotate(headLook);
        float hw = head.width() * headScale;
        float hh = head.height() * headScale;
        p.drawPixmap(-hw / 2, -hh / 2, hw, hh, head);
        p.restore();
    }

    // --- 左臂 ---
    QPixmap &lArm = m_player->leftArmPix();
    if (!lArm.isNull()) {
        p.save();
        p.translate(-armX, armY);
        p.rotate(armSwing + armExtraL);
        float aw = lArm.width() * armScale;
        float ah = lArm.height() * armScale;
        p.drawPixmap(-aw / 2, 0, aw, ah, lArm);
        p.restore();
    }

    // --- 右臂 ---
    QPixmap &rArm = m_player->rightArmPix();
    if (!rArm.isNull()) {
        p.save();
        p.translate(armX, armY);
        p.rotate(-armSwing + armExtraR);
        float aw = rArm.width() * armScale;
        float ah = rArm.height() * armScale;
        p.drawPixmap(-aw / 2, 0, aw, ah, rArm);
        p.restore();
    }

    // --- 左腿 ---
    QPixmap &lLeg = m_player->leftLegPix();
    if (!lLeg.isNull()) {
        p.save();
        p.translate(-legX, legY);
        p.rotate(-armSwing - toeLift);
        float lw = lLeg.width() * legScale;
        float lh = lLeg.height() * legScale;
        p.drawPixmap(-lw / 2, 0, lw, lh, lLeg);
        p.restore();
    }

    // --- 右腿 ---
    QPixmap &rLeg = m_player->rightLegPix();
    if (!rLeg.isNull()) {
        p.save();
        p.translate(legX, legY);
        p.rotate(armSwing + toeLift);
        float lw = rLeg.width() * legScale;
        float lh = rLeg.height() * legScale;
        p.drawPixmap(-lw / 2, 0, lw, lh, rLeg);
        p.restore();
    }

    // --- 冰激凌 ---
    if (m_player->hasIceCream()) {
        QPixmap ice = m_iceCreamPic.scaled(22, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.drawPixmap(armX - 5, armY + 5, ice);
    }

    p.restore();
}


// ---------- 绘制NPC（整图模式） ----------
void GameWidget::drawNPC(QPainter &p)
{
    if (!m_kid) return;

    p.save();
    p.translate(m_kid->x(), m_kid->y());

    // 感谢缩放
    float s = m_kid->scale();
    p.scale(s, s);

    // 左右翻转
    if (m_kid->facing() == Dir::Left)
        p.scale(-1, 1);

    QPixmap &pic = m_kid->fullBodyPix();
    if (!pic.isNull()) {
        int w = pic.width();
        int h = pic.height();

        // 走路弹跳
        float bob = 0;
        if (m_kid->isWalking())
            bob = sin(m_kid->animPhase() * 2) * 3.0f;

        p.drawPixmap(-w/2, -h/2 + bob, pic);
    }

    // 冰激凌
    if (m_kid->hasIceCream()) {
        QPixmap ice = m_iceCreamPic.scaled(18, 26, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.drawPixmap(10, -20, ice);
    }

    // 掉落的冰激凌
    if (m_phase == GamePhase::KidBumped ||
        m_phase == GamePhase::PlayerTurnGive ||
        m_phase == GamePhase::KidThanks) {
        QPixmap ice = m_iceCreamPic.scaled(16, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.setOpacity(0.7);
        p.drawPixmap(-5, 40, ice);
        p.setOpacity(1.0);
    }

    p.restore();
}

// ---------- UI ----------
void GameWidget::drawUI(QPainter &p)
{
    // 提示文字背景
    p.fillRect(10, 10, 600, 70, QColor(0, 0, 0, 140));

    p.setPen(Qt::white);
    p.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));

    QStringList lines = m_hintText.split('\n');
    int y = 38;
    for (const QString &line : lines) {
        p.drawText(20, y, line);
        y += 24;
    }

    // 完成画面
    if (m_phase == GamePhase::Complete) {
        p.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 100));
        p.setPen(QColor(255, 215, 0));
        p.setFont(QFont("Microsoft YaHei", 32, QFont::Bold));
        p.drawText(width()/2 - 180, height()/2 - 20, "🎉 任务完成！");
        p.setFont(QFont("Microsoft YaHei", 18));
        p.drawText(width()/2 - 130, height()/2 + 30, "唐嘉琦 · 日行一善");
        p.setPen(QColor(200, 200, 200));
        p.setFont(QFont("Microsoft YaHei", 13));
        p.drawText(width()/2 - 50, height()/2 + 60, "按 ESC 退出");
    }

    // 控制提示
    p.setPen(QColor(255, 255, 255, 180));
    p.setFont(QFont("Microsoft YaHei", 10));
    p.drawText(10, height() - 20, "WASD=移动 | E=观察 | Z=转身递冰激凌");
}

void GameWidget::resetGame()
{
    m_phase = GamePhase::Intro;
    m_phaseTimer = 0;
    m_hintText = "任务：用 WASD 控制唐嘉琦从蜜雪冰城走出，手上拿着冰激凌";
    m_keys.clear();

    m_player->setPos(130, 260);
    m_player->setHasIceCream(true);
    m_player->setWalking(false);
    m_player->setFacing(Dir::Right);
    m_player->stopLookAround();
    m_player->finishGiveIceCream();
    m_player->stopDancing();

    m_kid->setPos(350, 280);
    m_kid->setHasIceCream(true);
    m_kid->stopFollowing();
    m_kid->setScale(1.0f);

    m_kid->reset();
}
