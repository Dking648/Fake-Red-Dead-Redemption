#ifndef NPC_H
#define NPC_H

#include "Character.h"

enum class NPCState {
    Idle,
    Following,
    Bumped,
    Thanking
};

class NPC : public Character
{
public:
    NPC();
    ~NPC();

    void loadImage(const QString &path);

    // 跟随目标
    void follow(Character *target, float followDistance);
    void stopFollowing();

    // 碰撞
    void onBumped();       // 被撞，冰激凌掉落
    void onThank();        // 感谢（放大缩小）

    // 更新
    void update(float dt);

    bool bumped() const { return m_state == NPCState::Bumped; }
    bool thanking() const { return m_state == NPCState::Thanking; }
    bool isThankDone() const { return m_thankDone; }

    void reset();


private:
    NPCState    m_state;
    Character  *m_target;
    float       m_followDist;
    float       m_stateTimer;
    bool        m_thankDone;
    bool        m_iceCreamDropped;
};

#endif
