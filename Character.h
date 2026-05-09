#ifndef CHARACTER_H
#define CHARACTER_H

#include <QPixmap>
#include <QString>

// ---------- 方向 ----------
enum class Dir { Up = 0, Down = 1, Left = 2, Right = 3 };

// ---------- 走路风格 ----------
enum class WalkStyle { Normal, Thunder };  // 普通 / 雷霆摩擦步

// ---------- 角色基类 ----------
class Character
{
public:
    struct Pos { float x, y; };

    Character();
    virtual ~Character() {}

    // 位置
    void setPos(float x, float y);
    float x() const { return m_pos.x; }
    float y() const { return m_pos.y; }
    Pos pos() const { return m_pos; }

    // 移动
    virtual void move(float vx, float vy, float dt);
    void stop();

    // 方向
    Dir facing() const { return m_facing; }
    void setFacing(Dir d);

    // 动画
    bool isWalking() const { return m_walking; }
    void setWalking(bool w);
    float animPhase() const { return m_animPhase; }

    // 雷霆摩擦步
    WalkStyle walkStyle() const { return m_walkStyle; }
    void setWalkStyle(WalkStyle ws) { m_walkStyle = ws; }

    // 图片访问
    QPixmap& headPix()     { return m_head; }
    QPixmap& bodyPix()     { return m_body; }
    QPixmap& leftArmPix()  { return m_leftArm; }
    QPixmap& rightArmPix() { return m_rightArm; }
    QPixmap& leftLegPix()  { return m_leftLeg; }
    QPixmap& rightLegPix() { return m_rightLeg; }
    QPixmap& fullBodyPix() { return m_fullBody; }
    bool      useFullBody() const { return m_useFullBody; }
    void      setUseFullBody(bool b) { m_useFullBody = b; }

    // 缩放（小孩感谢用）
    float scale() const { return m_scale; }
    void  setScale(float s) { m_scale = s; }

    // 速度
    float speed() const { return m_speed; }
    void  setSpeed(float s) { m_speed = s; }

    // 冰激凌
    bool hasIceCream() const { return m_hasIceCream; }
    void setHasIceCream(bool b) { m_hasIceCream = b; }

    // 舞蹈参数（雷霆摩擦步的脚尖抬起角度）
    float toeLift() const { return m_toeLift; }

protected:
    Pos       m_pos;
    Dir       m_facing;
    bool      m_walking;
    float     m_speed;
    float     m_animPhase;
    WalkStyle m_walkStyle;
    float     m_scale;
    bool      m_hasIceCream;
    float     m_toeLift;       // 脚尖抬起角度
    bool      m_useFullBody;   // 是否使用整张图

    // 身体部件
    QPixmap m_head, m_body;
    QPixmap m_leftArm, m_rightArm;
    QPixmap m_leftLeg, m_rightLeg;

    // 整张图
    QPixmap m_fullBody;
};

#endif
