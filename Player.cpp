#include "Player.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Player::Player()
    : Character()
    , m_action(PlayerAction::Normal)
    , m_dancePhase(DancePhase::None)
    , m_danceTimer(0)
    , m_danceBeat(0)
    , m_danceBeatTimer(0)
    , m_leftArmExtraRot(0)
    , m_rightArmExtraRot(0)
    , m_bodyExtraRot(0)
    , m_headLookAngle(0)
    , m_bounceOffset(0)
    , m_lookTimer(0)
    , m_lookCount(0)
    , m_giveTimer(0)
{
    m_useFullBody = false;   // 玩家使用拼接模式
    m_walkStyle = WalkStyle::Thunder;  // 默认雷霆姿势
}

Player::~Player() {}

void Player::loadParts(const QString &head, const QString &body,
                       const QString &leftArm, const QString &rightArm,
                       const QString &leftLeg, const QString &rightLeg)
{
    m_head.load(head);
    m_body.load(body);
    m_leftArm.load(leftArm);
    m_rightArm.load(rightArm);
    m_leftLeg.load(leftLeg);
    m_rightLeg.load(rightLeg);
}

// ---------- 主更新 ----------
void Player::update(float dt)
{
    switch (m_action) {
    case PlayerAction::Normal:
    case PlayerAction::GivingIceCream:
        updateNormal(dt);
        break;
    case PlayerAction::LookingAround:
        updateLookAround(dt);
        break;
    case PlayerAction::Dancing:
        updateDancing(dt);
        break;
    }
}

// ---------- 正常行走状态 ----------
void Player::updateNormal(float dt)
{
    if (m_walking) {
        m_animPhase += 10.0f * dt;
        if (m_animPhase > 2 * M_PI) m_animPhase -= 2 * M_PI;

        // 雷霆摩擦步：脚尖抬起
        m_toeLift = fabs(sin(m_animPhase)) * 28.0f;

        // 弹跳
        m_bounceOffset = fabs(sin(m_animPhase * 2)) * 4.0f;
    } else {
        m_toeLift = 0;
        m_bounceOffset *= 0.85f;  // 归位
        m_animPhase *= 0.85f;
    }

    // 递冰激凌动作
    if (m_action == PlayerAction::GivingIceCream) {
        m_giveTimer += dt;
        // 右手向前伸出
        m_rightArmExtraRot = -40.0f * fmin(m_giveTimer / 0.5f, 1.0f);
    } else {
        m_rightArmExtraRot *= 0.9f;
    }

    // 其他额外旋转归位
    m_leftArmExtraRot *= 0.9f;
    m_bodyExtraRot *= 0.9f;
    m_headLookAngle *= 0.9f;
}

// ---------- 观察四周 ----------
void Player::updateLookAround(float dt)
{
    m_lookTimer += dt;

    // 每0.25秒翻转一次头部方向
    float period = 0.3f;
    float t = fmod(m_lookTimer, period) / period;

    if (t < 0.5f) {
        // 向左看
        m_headLookAngle = -40.0f + t * 2 * 80.0f;
    } else {
        // 向右看
        m_headLookAngle = 40.0f - (t - 0.5f) * 2 * 80.0f;
    }

    // 如果超过3秒自动停止
    if (m_lookTimer > 3.0f) {
        m_headLookAngle = 0;
    }
}

// ---------- 舞蹈状态 ----------
void Player::updateDancing(float dt)
{
    m_danceTimer += dt;
    m_danceBeatTimer += dt;

    // 每拍时间（假设120BPM = 0.5秒/拍）
    const float beatDuration = 0.5f;
    const int totalBeats = 12;

    // 更新节拍
    if (m_danceBeatTimer >= beatDuration) {
        m_danceBeatTimer -= beatDuration;
        m_danceBeat++;
        if (m_danceBeat > totalBeats) {
            m_danceBeat = 1;
        }
    }

    float phase = m_danceBeat + m_danceBeatTimer / beatDuration;  // 连续节拍位置

    // 根据节拍确定舞蹈阶段
    if (phase <= 2.0f) {
        // 第一阶段：双肩交叉摆动 + 雷霆摩擦步
        m_dancePhase = DancePhase::Phase1_CrossSwing;
        float t = phase / 2.0f;

        // 雷霆摩擦步（原地左右移动）
        float sideStep = sin(phase * M_PI) * 15.0f;  // 左右15像素
        m_pos.x += sideStep * 0.3f;

        // 手臂交叉摆动
        m_leftArmExtraRot = sin(phase * M_PI * 2) * 30.0f;
        m_rightArmExtraRot = -sin(phase * M_PI * 2) * 30.0f;
        m_bodyExtraRot = sin(phase * M_PI) * 8.0f;
        m_toeLift = fabs(sin(phase * M_PI)) * 25.0f;
        m_bounceOffset = fabs(sin(phase * M_PI * 2)) * 5.0f;
    }
    else if (phase <= 4.0f) {
        // 第二阶段：展示力量姿势，双臂打开
        m_dancePhase = DancePhase::Phase2_PowerPose;
        float t = (phase - 2.0f) / 2.0f;

        m_leftArmExtraRot = -80.0f + t * 10.0f;   // 左臂向外打开
        m_rightArmExtraRot = 80.0f - t * 10.0f;   // 右臂向外打开
        m_bodyExtraRot = sin(t * M_PI) * 5.0f;
        m_headLookAngle = sin(t * M_PI * 2) * 10.0f;  // 头轻微晃动
        m_toeLift = 0;
        m_bounceOffset = sin(t * M_PI * 2) * 3.0f;
    }
    else if (phase <= 8.0f) {
        // 第三阶段：拍四下腿 + 拍四下胸
        m_dancePhase = DancePhase::Phase3_PatBody;
        float localBeat = phase - 4.0f;  // 1-4拍

        if (localBeat <= 4.0f) {
            // 拍腿阶段 (1-4拍)
            float bt = fmod(localBeat, 1.0f);
            if (bt < 0.3f) {
                m_leftArmExtraRot = 60.0f;   // 左手拍左腿
                m_rightArmExtraRot = 60.0f;
            } else {
                m_leftArmExtraRot = 20.0f;
                m_rightArmExtraRot = 20.0f;
            }
        } else {
            // 拍胸阶段 (5-8拍)
            float bt = fmod(localBeat, 1.0f);
            if (bt < 0.3f) {
                m_leftArmExtraRot = -50.0f;  // 手拍胸口
                m_rightArmExtraRot = -50.0f;
            } else {
                m_leftArmExtraRot = -10.0f;
                m_rightArmExtraRot = -10.0f;
            }
        }
        m_bodyExtraRot = sin(localBeat * M_PI / 2) * 5.0f;
        m_bounceOffset = (fmod(localBeat, 1.0f) < 0.3f) ? -5.0f : 0;
    }
    else if (phase <= 12.0f) {
        // 第四阶段：1212转身 + 手指 + 扭屁股
        m_dancePhase = DancePhase::Phase4_TurnAndPoint;
        float localBeat = phase - 8.0f;  // 1-4拍

        if (localBeat <= 2.0f) {
            // 交叉双臂转身
            float tt = localBeat / 2.0f;
            m_leftArmExtraRot = -30.0f * (1 - tt) + 80.0f * tt;
            m_rightArmExtraRot = 30.0f * (1 - tt) - 80.0f * tt;
            m_bodyExtraRot = tt * 180.0f;  // 转身
        } else {
            // 扭屁股 + 手指向上
            float tt = (localBeat - 2.0f) / 2.0f;
            m_leftArmExtraRot = -120.0f;   // 手指向上
            m_rightArmExtraRot = -120.0f;
            m_bodyExtraRot = sin(tt * M_PI * 4) * 15.0f;  // S形摆动
            m_bounceOffset = fabs(sin(tt * M_PI * 2)) * 8.0f;
        }
    }

    // 舞蹈结束后循环
    if (phase > 14.0f) {
        m_danceTimer = 0;
        m_danceBeat = 0;
        m_danceBeatTimer = 0;
    }
}

// ---------- 动作控制 ----------
void Player::startLookAround()
{
    m_action = PlayerAction::LookingAround;
    m_lookTimer = 0;
    m_lookCount = 0;
    m_walking = false;
}

void Player::stopLookAround()
{
    m_action = PlayerAction::Normal;
    m_headLookAngle = 0;
    m_lookTimer = 0;
}

void Player::startGiveIceCream()
{
    m_action = PlayerAction::GivingIceCream;
    m_giveTimer = 0;
    m_hasIceCream = false;
}

void Player::finishGiveIceCream()
{
    m_action = PlayerAction::Normal;
    m_rightArmExtraRot = 0;
    m_giveTimer = 0;
}

void Player::startDancing()
{
    m_action = PlayerAction::Dancing;
    m_danceTimer = 0;
    m_danceBeat = 0;
    m_danceBeatTimer = 0;
    m_walking = false;
    m_walkStyle = WalkStyle::Thunder;
}

void Player::stopDancing()
{
    m_action = PlayerAction::Normal;
    m_dancePhase = DancePhase::None;
    m_leftArmExtraRot = 0;
    m_rightArmExtraRot = 0;
    m_bodyExtraRot = 0;
    m_bounceOffset = 0;
    m_toeLift = 0;
}
