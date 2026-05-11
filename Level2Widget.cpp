#include "Level2Widget.h"
#include "Player.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Level2Widget::Level2Widget(QWidget *parent)
    : QWidget(parent)
    , m_player(nullptr)
    , m_timer(new QTimer(this))
    , m_deltaTime(0)
    , m_phase(L2Phase::Intro)
    , m_phaseTimer(0)
    , m_hintText("")
    , m_hasDate(false)
    , m_dateEaten(false)
    , m_atCounter(false)
    , m_cartStopped(false)
    , m_manThanked(false)
    , m_flashTimer(0)
    , m_eatTimer(0)
    , m_manAlpha(1.0f)
{
    setFixedSize(1280, 720);
    setFocusPolicy(Qt::StrongFocus);

    // ===== 加载图片 =====
    m_datePic.load(":/images/date.png");        // 红枣
    m_cartPic.load(":/images/cart.png");        // 购物车
    m_manPic.load(":/images/man_full.png");     // 男人整图

    // ===== 创建玩家 =====
    m_player = new Player();
    m_player->loadParts(
        ":/images/tang_head.png",
        ":/images/tang_body.png",
        ":/images/tang_left_arm.png",
        ":/images/tang_right_arm.png",
        ":/images/tang_left_leg.png",
        ":/images/tang_right_leg.png"
        );
    m_player->setPos(120, 420);      // 超市内出生
    m_player->setWalkStyle(WalkStyle::Thunder);

    // ===== 关键位置 =====
    m_dateX     = 300;  m_dateY     = 350;
    m_cart1X    = 180;  m_cart1Y    = 480;   // 唐嘉琦购物车（超市内）
    m_cart2X    = 0;    m_cart2Y    = 0;     // 男人购物车（稍后出现）
    m_cart1Visible = true;
    m_cart2Visible = false;
    m_counterX  = 650;  m_counterY  = 430;
    m_exitX     = 850;  m_exitY     = 450;
    m_manX      = 1050; m_manY      = 420;
    m_danceSpotX = 640; m_danceSpotY = 400;


    m_hintText = "任务：去挑选一颗红枣（走向红枣位置，按 E 捡起）";

    // ===== 游戏循环 =====
    connect(m_timer, &QTimer::timeout, this, &Level2Widget::gameLoop);
    m_timer->start(16);
    m_clock.start();



}

Level2Widget::~Level2Widget() { delete m_player; }

// ==================== 重置 ====================
void Level2Widget::resetGame()
{
    m_phase = L2Phase::Intro;
    m_phaseTimer = 0;
    m_hintText = "任务：去挑选一颗红枣（走向红枣位置，按 E 捡起）";
    m_keys.clear();
    m_hasDate = false;
    m_dateEaten = false;
    m_atCounter = false;
    m_cartStopped = false;
    m_manThanked = false;
    m_flashTimer = 0;
    m_eatTimer = 0;
    m_manAlpha = 1.0f;

    m_player->setPos(120, 420);
    m_player->setWalking(false);
    m_player->setFacing(Dir::Right);
    m_player->stopDancing();

    m_cart1X = 180;  m_cart1Y = 480;
    m_cart2X = 0;    m_cart2Y = 0;
    m_cart1Visible = true;
    m_cart2Visible = false;
}

// ==================== 游戏循环 ====================
void Level2Widget::gameLoop()
{
    m_deltaTime = m_clock.elapsed() / 1000.0f;
    m_clock.restart();
    if (m_deltaTime > 0.1f) m_deltaTime = 0.1f;

    m_phaseTimer += m_deltaTime;

    updateLogic(m_deltaTime);
    processInput(m_deltaTime);
    m_player->update(m_deltaTime);

    update();
}

// 逻辑更新
void Level2Widget::updateLogic(float dt)
{
    float px = m_player->x();
    float py = m_player->y();

    switch (m_phase) {

    // ===== 阶段1：走向红枣 =====
    case L2Phase::Intro: {
        float dx = px - m_dateX;
        float dy = py - m_dateY;
        if (sqrt(dx*dx + dy*dy) < 50) {
            m_hintText = "按 E 捡起红枣放进购物车";
        }
        break;
    }

    // ===== 阶段2：捡起红枣 → 放入购物车 → 立即推车 =====
    case L2Phase::PickDate:
        m_hasDate = true;
        m_phase = L2Phase::PushCart;
        m_phaseTimer = 0;
        m_hintText = "红枣已放进购物车！推到收银台（向右走）";
        m_cart1X = px - 30;
        m_cart1Y = py + 40;
        break;

    // ===== 阶段3：推购物车去收银台 =====
    case L2Phase::PushCart: {
        // 购物车跟着玩家
        m_cart1X = px - 50;
        m_cart1Y = py + 60;

        float dx = px - m_counterX;
        float dy = py - m_counterY;
        if (sqrt(dx*dx + dy*dy) < 50) {
            m_phase = L2Phase::Checkout;
            m_phaseTimer = 0;
            m_atCounter = true;
            // 购物车停在收银台
            m_cart1X = m_counterX - 30;
            m_cart1Y = m_counterY + 30;
            m_hintText = "结账中...请稍候";
        }
        break;
    }

    // ===== 阶段4：结账（2秒） =====
    case L2Phase::Checkout:
        // 购物车留在收银台不动
        m_player->stop();
        if (m_phaseTimer > 2.0f) {
            m_phase = L2Phase::WalkOut;
            m_phaseTimer = 0;
            m_atCounter = false;
            // 红枣从购物车拿出来，拿在手上
            m_hintText = "结账完成！拿着红枣走出超市（向右走）";
        }
        break;

    // ===== 阶段5：走出超市 =====
    case L2Phase::WalkOut:
        // 购物车留在收银台
        m_cart1X = m_counterX - 50;
        m_cart1Y = m_counterY + 50;
        if (px > m_exitX) {
            m_hintText = "按 E 吃掉红枣";
        }
        break;

    // ===== 阶段6：吃红枣 =====
    case L2Phase::EatDate:
        m_eatTimer += dt;
        if (m_eatTimer > 1.5f) {
            m_dateEaten = true;
            m_hasDate = false;
            m_phase = L2Phase::ManAppears;
            m_phaseTimer = 0;
            m_eatTimer = 0;
            // 男人的购物车出现在超市外右侧
            m_cart2X = 1100;
            m_cart2Y = 480;
            m_cart2Visible = true;
            m_manX = 1050;
            m_manY = 420;
            m_hintText = "红枣真甜！...咦？那边有人购物车失控了！";
        }
        break;

    // ===== 阶段7：男人出现 =====
    case L2Phase::ManAppears:
        if (m_phaseTimer > 2.0f) {
            m_phase = L2Phase::CartRunsAway;
            m_phaseTimer = 0;
            m_hintText = "😱 男人的购物车失控了！按 F 闪现去救！";
        }
        break;

    // ===== 阶段8：购物车失控滑走 =====
    case L2Phase::CartRunsAway:
        m_cart2X -= 300.0f * dt;     // 男人的购物车向左滑
        m_manX  -= 120.0f * dt;      // 男人在后面追
        if (m_cart2X < 150) m_cart2X = 150;
        break;

    // ===== 阶段9：闪现到购物车 =====
    case L2Phase::FlashToCart: {
        m_flashTimer += dt;
        if (m_flashTimer >= 0.15f && m_flashTimer < 0.2f) {
            // 瞬移！
            m_player->setPos(m_cart2X + 15, m_cart2Y - 30);
            m_flashX = m_player->x();
            m_flashY = m_player->y();
            m_cartStopped = true;
            m_cart2X = m_player->x() - 30;
            m_cart2Y = m_player->y() + 30;
        }
        if (m_flashTimer > 1.0f) {
            m_phase = L2Phase::ManThanks;
            m_phaseTimer = 0;
            m_flashTimer = 0;
            m_hintText = "男人跑过来感谢唐嘉琦...";
        }
        break;
    }

    // ===== 阶段10：男人感谢 =====
    case L2Phase::ManThanks: {
        float dx = px - m_manX;
        float dy = py - m_manY;
        float dist = sqrt(dx*dx + dy*dy);
        if (dist > 60) {
            m_manX += (dx / dist) * 200.0f * dt;
            m_manY += (dy / dist) * 200.0f * dt;
        }
        if (dist < 60 && m_phaseTimer > 1.5f) {
            m_manThanked = true;
            m_phase = L2Phase::ManLeaves;
            m_phaseTimer = 0;
            m_hintText = "男人带着购物车离开了...走到中央舞动！";
        }
        break;
    }

    // ===== 阶段11：男人离开 =====
    case L2Phase::ManLeaves:
        m_manX  += 200.0f * dt;
        m_cart2X += 200.0f * dt;
        if (m_manX > 1400) {
            m_cart2Visible = false;
            m_phase = L2Phase::WalkToDance;
            m_phaseTimer = 0;
            m_hintText = "走到画面中央开始舞动！";
        }
        break;

    // ===== 阶段12：走向舞动区 =====
    case L2Phase::WalkToDance: {
        float dx = px - m_danceSpotX;
        float dy = py - m_danceSpotY;
        if (sqrt(dx*dx + dy*dy) < 80) {
            m_phase = L2Phase::Dancing;
            m_phaseTimer = 0;
            m_player->stop();
            m_player->startDancing();
            m_hintText = "🎵 雷霆舞动中...";
        }
        break;
    }

    // ===== 阶段13：跳舞 =====
    case L2Phase::Dancing:
        if (m_phaseTimer > 15.0f) {
            m_phase = L2Phase::Complete;
            m_player->stopDancing();
            m_hintText = "🎉 任务完成！唐嘉琦日行一善第二关！";
        }
        break;

    case L2Phase::Complete:
        break;
    }
}

// ==================== 输入处理 ====================
void Level2Widget::processInput(float dt)
{
    if (!m_player) return;
    if (m_phase == L2Phase::Complete) return;
    if (m_phase == L2Phase::Dancing) return;
    if (m_phase == L2Phase::Checkout) return;
    if (m_phase == L2Phase::EatDate) return;
    if (m_phase == L2Phase::FlashToCart) return;

    float vx = 0, vy = 0;
    if (m_keys.contains(Qt::Key_W)) vy -= 1;
    if (m_keys.contains(Qt::Key_S)) vy += 1;
    if (m_keys.contains(Qt::Key_A)) vx -= 1;
    if (m_keys.contains(Qt::Key_D)) vx += 1;

    if (vx != 0 || vy != 0) {
        float len = sqrt(vx*vx + vy*vy);
        vx /= len; vy /= len;
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

// ==================== 键盘事件 ====================
void Level2Widget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit backToLevelSelect();
        return;
    }

    m_keys.insert(event->key());

    int key = event->key();
    float px = m_player->x();
    float py = m_player->y();

    // E 键：捡红枣 / 吃红枣
    if (key == Qt::Key_E) {
        if (m_phase == L2Phase::Intro) {
            float dx = px - m_dateX;
            float dy = py - m_dateY;
            if (sqrt(dx*dx + dy*dy) < 50) {
                m_phase = L2Phase::PickDate;
                m_phaseTimer = 0;
                m_hasDate = true;
                qDebug() << "捡起红枣！";
            }
        }
        else if (m_phase == L2Phase::WalkOut && px > m_exitX) {
            m_phase = L2Phase::EatDate;
            m_phaseTimer = 0;
            m_eatTimer = 0;
            m_player->stop();
            qDebug() << "吃红枣！";
        }
    }

    // F 键：闪现
    if (key == Qt::Key_F && m_phase == L2Phase::CartRunsAway) {
        m_phase = L2Phase::FlashToCart;
        m_phaseTimer = 0;
        m_flashTimer = 0;
        m_flashX = px;
        m_flashY = py;
        qDebug() << "闪现！";
    }

    QWidget::keyPressEvent(event);
}

void Level2Widget::keyReleaseEvent(QKeyEvent *event)
{
    m_keys.remove(event->key());
    QWidget::keyReleaseEvent(event);
}

// ==================== 绘制 ====================
void Level2Widget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    drawBackground(p);
    drawDate(p);
    drawMan(p);
    drawPlayer(p);
    drawCart(p);
    drawFlashEffect(p);
    drawUI(p);

}

// ===== 背景：超市 =====
void Level2Widget::drawBackground(QPainter &p)
{
    // 天空
    p.fillRect(0, 0, width(), 560, QColor(135, 206, 235));
    // 地面
    p.fillRect(0, 560, width(), 160, QColor(160, 180, 160));

    // ===== 超市建筑（左侧） =====
    p.fillRect(0, 0, 800, 560, QColor(240, 240, 230));
    p.setPen(QPen(Qt::darkGray, 4));
    p.drawRect(0, 0, 800, 560);

    // 超市牌子
    p.setPen(Qt::black);
    p.setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    p.drawText(300, 50, "🏪 便民超市");

    // 货架
    for (int sx = 100; sx < 700; sx += 200) {
        p.fillRect(sx, 150, 120, 350, QColor(200, 180, 150));
        p.setPen(QPen(Qt::gray, 2));
        for (int sy = 180; sy < 480; sy += 60) {
            p.drawLine(sx, sy, sx+120, sy);
        }
    }

    // 收银台
    p.fillRect(m_counterX - 40, m_counterY - 20, 80, 60, QColor(180, 160, 130));
    p.setPen(Qt::black);
    p.setFont(QFont("Microsoft YaHei", 10));
    p.drawText(m_counterX - 25, m_counterY + 10, "收银台");

    // 门
    p.fillRect(780, 370, 40, 190, QColor(100, 60, 30));
    p.setPen(Qt::white);
    p.setFont(QFont("Microsoft YaHei", 9));
    p.drawText(782, 470, "出\n口");

    // ===== 舞动区标记 =====
    p.setPen(QPen(QColor(255, 215, 0, 100), 2, Qt::DashLine));
    p.drawEllipse(QPointF(m_danceSpotX, m_danceSpotY), 100, 100);
    p.setPen(QColor(255, 215, 0, 100));
    p.setFont(QFont("Microsoft YaHei", 12));
    p.drawText(m_danceSpotX - 45, m_danceSpotY - 110, "✨ 舞动区");
}

// ===== 红枣 =====
void Level2Widget::drawDate(QPainter &p)
{
    if (m_hasDate || m_dateEaten) return;

    if (!m_datePic.isNull()) {
        p.drawPixmap(m_dateX - 15, m_dateY - 15, 30, 30, m_datePic);
    } else {
        p.setBrush(QColor(180, 30, 30));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(m_dateX, m_dateY), 10, 12);
    }
}

// ===== 购物车 =====
void Level2Widget::drawCart(QPainter &p)
{
    // ===== 第一辆：唐嘉琦的购物车 =====
    if (m_cart1Visible && m_phase <= L2Phase::WalkOut) {
        drawSingleCart(p, m_cart1X, m_cart1Y, m_hasDate && !m_dateEaten && m_phase < L2Phase::WalkOut, false);
    }

    // ===== 第二辆：男人的购物车 =====
    if (m_cart2Visible) {
        drawSingleCart(p, m_cart2X, m_cart2Y, false, true);
    }
}

void Level2Widget::drawSingleCart(QPainter &p, float cx, float cy, bool hasDateInside, bool flip)
{
    p.save();
    p.translate(cx, cy);

    // 左右翻转
    if (flip) {
        p.scale(-1, 1);
    }

    // ===== 🔧 缩放（1.5 = 放大1.5倍，调这个数字）=====
    float cartScale = 1.5f;

    if (!m_cartPic.isNull()) {
        int w = m_cartPic.width() * cartScale;
        int h = m_cartPic.height() * cartScale;
        p.drawPixmap(-w/2, -h/2, w, h, m_cartPic);
    } else {
        // 备用色块
        int w = 70 * cartScale;
        int h = 50 * cartScale;
        p.setBrush(QColor(100, 100, 120));
        p.setPen(QPen(Qt::black, 2));
        p.drawRoundedRect(-w/2, -h/2, w, h, 8, 8);
        p.setBrush(Qt::black);
        p.drawEllipse(QPointF(-22 * cartScale, 27 * cartScale), 6, 6);
        p.drawEllipse(QPointF(22 * cartScale, 27 * cartScale), 6, 6);
    }

    // 红枣
    if (hasDateInside) {
        p.setBrush(QColor(180, 30, 30));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(0, -15 * cartScale), 5, 7);
    }

    p.restore();
}



// ===== 男人 =====
void Level2Widget::drawMan(QPainter &p)
{
    if (m_phase < L2Phase::ManAppears || m_phase == L2Phase::Complete) return;
    if (m_phase == L2Phase::ManLeaves && m_manX > 1350) return;

    p.save();
    p.translate(m_manX, m_manY);

    // 面朝唐嘉琦
    if (m_player->x() < m_manX) p.scale(-1, 1);

    p.setOpacity(m_manAlpha);

    if (!m_manPic.isNull()) {
        p.drawPixmap(-m_manPic.width()/2, -m_manPic.height()/2, m_manPic);
    } else {
        // 备用色块
        p.setBrush(QColor(70, 130, 200));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(-25, -60, 50, 80, 10, 10);
        p.setBrush(QColor(255, 220, 180));
        p.drawEllipse(QPointF(0, -45), 15, 15);
    }

    p.setOpacity(1.0);
    p.restore();
}

// ===== 玩家（复用 drawPlayer 逻辑） =====
void Level2Widget::drawPlayer(QPainter &p)
{
    if (!m_player) return;

    p.save();
    p.translate(m_player->x(), m_player->y());

    float s = m_player->scale();
    p.scale(s, s);

    float bob = m_player->bounceOffset();
    p.translate(0, -bob);

    Dir facing = m_player->facing();
    if (facing == Dir::Left) p.scale(-1, 1);

    float toeLift   = m_player->toeLift();
    float armSwing  = m_player->isWalking() ? sin(m_player->animPhase()) * 22.0f : 0;
    float armExtraL = m_player->leftArmExtraRot();
    float armExtraR = m_player->rightArmExtraRot();
    float bodyRot   = m_player->bodyExtraRot();
    float headLook  = m_player->headLookAngle();

    p.rotate(bodyRot);

    // 身体
    QPixmap &body = m_player->bodyPix();
    if (!body.isNull()) p.drawPixmap(-body.width()/2, -55, body);

    // 头
    QPixmap &head = m_player->headPix();
    if (!head.isNull()) {
        p.save();
        p.translate(0, -95);
        p.rotate(headLook);
        p.drawPixmap(-head.width()/2, -head.height()/2, head);
        p.restore();
    }

    // 左臂
    QPixmap &lArm = m_player->leftArmPix();
    if (!lArm.isNull()) {
        p.save();
        p.translate(-52, -43);
        p.rotate(armSwing + armExtraL);
        p.drawPixmap(-lArm.width()/2, 0, lArm);
        p.restore();
    }

    // 右臂（吃红枣时举到嘴边）
    QPixmap &rArm = m_player->rightArmPix();
    if (!rArm.isNull()) {
        p.save();
        p.translate(52, -43);
        float eatRot = 0;
        if (m_phase == L2Phase::EatDate) {
            eatRot = -60.0f;  // 手举到嘴边
        }
        p.rotate(-armSwing + armExtraR + eatRot);
        p.drawPixmap(-rArm.width()/2, 0, rArm);
        p.restore();
    }

    // 左腿
    QPixmap &lLeg = m_player->leftLegPix();
    if (!lLeg.isNull()) {
        p.save();
        p.translate(-20, 40);
        p.rotate(-armSwing - toeLift);
        p.drawPixmap(-lLeg.width()/2, 0, lLeg);
        p.restore();
    }

    // 右腿
    QPixmap &rLeg = m_player->rightLegPix();
    if (!rLeg.isNull()) {
        p.save();
        p.translate(20, 40);
        p.rotate(armSwing + toeLift);
        p.drawPixmap(-rLeg.width()/2, 0, rLeg);
        p.restore();
    }

    // 手上红枣
    if (m_hasDate && !m_dateEaten
        && (m_phase == L2Phase::WalkOut || m_phase == L2Phase::EatDate)) {

        p.setBrush(QColor(180, 30, 30));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(20, -55), 6, 8);
    }

    p.restore();
}

// ===== 闪现特效 =====
void Level2Widget::drawFlashEffect(QPainter &p)
{
    if (m_phase != L2Phase::FlashToCart) return;

    float t = m_flashTimer;

    if (t < 0.15f) {
        // 闪电特效：从闪现起点到终点画一条线
        float progress = t / 0.15f;
        float fx = m_flashX + (m_player->x() - m_flashX) * progress;
        float fy = m_flashY + (m_player->y() - m_flashY) * progress;

        p.setPen(QPen(QColor(255, 255, 100, (int)(200 * (1-progress))), 3));
        for (int i = 0; i < 5; i++) {
            float rx = (rand() % 40 - 20) * (1 - progress);
            float ry = (rand() % 40 - 20) * (1 - progress);
            p.drawLine(m_flashX + rx, m_flashY + ry, fx + rx, fy + ry);
        }
    } else if (t < 0.3f) {
        // 落点光晕
        float alpha = 150 * (1 - (t - 0.15f) / 0.15f);
        QRadialGradient glow(m_player->x(), m_player->y(), 60);
        glow.setColorAt(0, QColor(255, 255, 200, (int)alpha));
        glow.setColorAt(1, QColor(255, 255, 100, 0));
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(m_player->x(), m_player->y()), 60, 60);
    }
}

// ===== UI =====
void Level2Widget::drawUI(QPainter &p)
{
    // 提示文字背景
    p.fillRect(10, 10, 600, 60, QColor(0, 0, 0, 140));

    p.setPen(Qt::white);
    p.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));

    QStringList lines = m_hintText.split('\n');
    int y = 38;
    for (const QString &line : lines) {
        p.drawText(20, y, line);
        y += 24;
    }

    // 完成画面
    if (m_phase == L2Phase::Complete) {
        p.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 120));
        p.setPen(QColor(255, 215, 0));
        p.setFont(QFont("Microsoft YaHei", 32, QFont::Bold));
        p.drawText(width()/2 - 180, height()/2 - 20, "🎉 第二关完成！");
        p.setFont(QFont("Microsoft YaHei", 18));
        p.drawText(width()/2 - 100, height()/2 + 30, "唐嘉琦 · 日行一善");
        p.setPen(QColor(200, 200, 200));
        p.setFont(QFont("Microsoft YaHei", 13));
        p.drawText(width()/2 - 50, height()/2 + 60, "按 ESC 退出");
    }

    // 控制提示
    p.setPen(QColor(255, 255, 255, 150));
    p.setFont(QFont("Microsoft YaHei", 9));
    p.drawText(10, height() - 15, "WASD=移动 | E=交互 | F=闪现 | ESC=返回");
}
