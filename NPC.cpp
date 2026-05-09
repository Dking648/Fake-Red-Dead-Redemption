#include "NPC.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

NPC::NPC()
    : Character()
    , m_state(NPCState::Idle)
    , m_target(nullptr)
    , m_followDist(100.0f)
    , m_stateTimer(0)
    , m_thankDone(false)
    , m_iceCreamDropped(false)
{
    m_useFullBody = true;
    m_speed = 140.0f;
    m_hasIceCream = true;
}

NPC::~NPC() {}

void NPC::loadImage(const QString &path)
{
    m_fullBody.load(path);
}

void NPC::follow(Character *target, float followDistance)
{
    m_target = target;
    m_followDist = followDistance;
    m_state = NPCState::Following;
}

void NPC::stopFollowing()
{
    m_state = NPCState::Idle;
    m_target = nullptr;
    m_walking = false;
}

void NPC::onBumped()
{
    m_state = NPCState::Bumped;
    m_stateTimer = 0;
    m_iceCreamDropped = true;
    m_hasIceCream = false;
    m_walking = false;
}

void NPC::onThank()
{
    m_state = NPCState::Thanking;
    m_stateTimer = 0;
    m_thankDone = false;
}

void NPC::update(float dt)
{
    switch (m_state) {

    case NPCState::Following: {
        if (m_target) {
            float dx = m_target->x() - m_pos.x;
            float dy = m_target->y() - m_pos.y;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist > m_followDist) {
                float vx = dx / dist;
                float vy = dy / dist;
                move(vx, vy, dt);
                m_walking = true;

                if (fabs(vx) > fabs(vy))
                    m_facing = (vx > 0) ? Dir::Right : Dir::Left;
            } else {
                m_walking = false;
            }
        }
        break;
    }

    case NPCState::Bumped:
        m_stateTimer += dt;
        m_walking = false;
        break;

    case NPCState::Thanking: {
        m_stateTimer += dt;
        float t = m_stateTimer;
        m_scale = 1.0f + sin(t * 8.0f) * 0.15f;
        if (t > 2.0f) {
            m_scale = 1.0f;
            m_thankDone = true;
            m_state = NPCState::Idle;
        }
        break;
    }

    default:
        break;
    }
}

void NPC::reset()
{
    m_state = NPCState::Idle;
    m_stateTimer = 0;
    m_thankDone = false;
    m_iceCreamDropped = false;
    m_hasIceCream = true;
    m_scale = 1.0f;
    m_walking = false;
    m_target = nullptr;
}

