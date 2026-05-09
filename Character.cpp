#include "Character.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Character::Character()
    : m_pos{0, 0}
    , m_facing(Dir::Right)
    , m_walking(false)
    , m_speed(200.0f)
    , m_animPhase(0)
    , m_walkStyle(WalkStyle::Normal)
    , m_scale(1.0f)
    , m_hasIceCream(false)
    , m_toeLift(0)
    , m_useFullBody(false)
{
}

void Character::setPos(float x, float y)
{
    m_pos.x = x;
    m_pos.y = y;
}

void Character::setFacing(Dir d)
{
    m_facing = d;
}

void Character::setWalking(bool w)
{
    m_walking = w;
    if (!w) {
        m_animPhase = 0;
        m_toeLift = 0;
    }
}

void Character::stop()
{
    m_walking = false;
    m_animPhase = 0;
    m_toeLift = 0;
}

void Character::move(float vx, float vy, float dt)
{
    m_pos.x += vx * m_speed * dt;
    m_pos.y += vy * m_speed * dt;

    // 动画相位
    m_animPhase += 10.0f * dt;
    if (m_animPhase > 2 * M_PI)
        m_animPhase -= 2 * M_PI;

    // 雷霆摩擦步：脚尖抬起
    if (m_walkStyle == WalkStyle::Thunder) {
        m_toeLift = fabs(sin(m_animPhase)) * 25.0f;  // 脚尖抬起0~25度
    } else {
        m_toeLift = 0;
    }
}
