#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"

// 舞蹈阶段
enum class DancePhase {
    None,
    Phase1_CrossSwing,      // 双肩交叉摆动
    Phase2_PowerPose,       // 力量姿势
    Phase3_PatBody,         // 拍腿拍胸
    Phase4_TurnAndPoint,    // 1212转身+手指
    Phase5_HipShake,        // 扭屁股+手臂波浪
    Phase6_HandsUp,         // 举手向上
    Finished
};

// 玩家总状态（舞蹈中也可能有子状态，但这里先简化）
enum class PlayerAction {
    Normal,         // 正常行走
    LookingAround,  // 观察四周（头部翻转）
    GivingIceCream, // 递冰激凌
    Dancing         // 跳舞
};

class Player : public Character
{
public:
    Player();
    ~Player();

    // 加载身体部件
    void loadParts(const QString &head, const QString &body,
                   const QString &leftArm, const QString &rightArm,
                   const QString &leftLeg, const QString &rightLeg);

    // 每帧更新
    void update(float dt);

    // 动作控制
    void startLookAround();    // 开始观察四周
    void stopLookAround();
    bool isLookingAround() const { return m_action == PlayerAction::LookingAround; }

    void startGiveIceCream();
    void finishGiveIceCream();
    bool isGivingIceCream() const { return m_action == PlayerAction::GivingIceCream; }

    void startDancing();
    void stopDancing();
    bool isDancing() const { return m_action == PlayerAction::Dancing; }

    // 舞蹈状态访问
    DancePhase dancePhase() const { return m_dancePhase; }
    float danceTimer() const { return m_danceTimer; }
    int   danceBeat() const { return m_danceBeat; }

    // 手臂额外旋转（舞蹈用）
    float leftArmExtraRot() const  { return m_leftArmExtraRot; }
    float rightArmExtraRot() const { return m_rightArmExtraRot; }

    // 身体额外旋转
    float bodyExtraRot() const { return m_bodyExtraRot; }

    // 头部额外参数（观察用）
    float headLookAngle() const { return m_headLookAngle; }

    // 缩放用于弹跳
    float bounceOffset() const { return m_bounceOffset; }

private:
    void updateNormal(float dt);
    void updateLookAround(float dt);
    void updateDancing(float dt);

    PlayerAction m_action;
    DancePhase   m_dancePhase;
    float        m_danceTimer;
    int          m_danceBeat;        // 1-12拍
    float        m_danceBeatTimer;

    // 额外旋转（舞蹈用）
    float m_leftArmExtraRot;
    float m_rightArmExtraRot;
    float m_bodyExtraRot;
    float m_headLookAngle;

    // 弹跳偏移
    float m_bounceOffset;

    // 观察计时
    float m_lookTimer;
    int   m_lookCount;

    // 递冰激凌计时
    float m_giveTimer;
};

#endif
