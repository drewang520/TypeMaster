#include "Fruit.h"

// ── Constructor ──

Fruit::Fruit(QChar letter, float x, float y, float speed, FruitType type)
    : m_letter(letter)
    , m_x(x)
    , m_y(y)
    , m_speed(speed)
    , m_type(type)
{
}

// ── 动作 / 动画 ──

void Fruit::update()
{
    if (m_state == State::Falling)
        m_y += m_speed;
}

void Fruit::tickAnim()
{
    if (m_animFrame > 0)
        --m_animFrame;
}

// ── 状态转换 ──

void Fruit::catchFruit()
{
    m_state      = State::Caught;
    m_animFrame  = 20;  // 显示捕获图，持续 20帧
}

void Fruit::breakFruit()
{
    m_state      = State::Broken;
    m_animFrame  = 25;  // 显示损坏图，持续 25 个帧
}

// ── 访问器 ──

QRect Fruit::getBounds() const
{
    return QRect(static_cast<int>(m_x) - kRadius,
                 static_cast<int>(m_y) - kRadius,
                 kRadius * 2,
                 kRadius * 2);
}