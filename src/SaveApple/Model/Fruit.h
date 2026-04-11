#ifndef FRUIT_H
#define FRUIT_H

#include <QChar>
#include <QRect>

enum class FruitType { Apple };

/**
 * 水果 — 一个下落的水果实体（模型层）。
 *
 * 包含位置、速度、字母和动画状态。
 * 控制器在每个时间步长调用 update()；视图通过
 * 获取器读取数据进行渲染，而不修改状态。
 */
class Fruit
{
public:
    enum class State { Falling, Caught, Broken };

    Fruit() = default;
    Fruit(QChar letter, float x, float y, float speed,
          FruitType type = FruitType::Apple);

    // ── 移动 / 动画 ──
    /** 根据速度调整垂直位置（每帧游戏循环调用一次）。 */
    void update();

    /** 减少动画倒计时帧数。 */
    void tickAnim();

    // ── 状态转换 ──
    /** 将水果标记为已捕获；启动捕获闪烁动画。 */
    void catchFruit();

    /** 将水果标记为已破（被击中）；开始播放破裂动画。 */
    void breakFruit();

    // ── 访问器 ──
    QChar     getLetter()     const { return m_letter;     }
    float     getX()          const { return m_x;          }
    float     getY()          const { return m_y;          }
    float     getSpeed()      const { return m_speed;      }
    State     getState()      const { return m_state;      }
    int       getAnimFrame()  const { return m_animFrame;  }
    FruitType getType()       const { return m_type;       }

    /** 与坐标轴对齐的边界矩形（以 x, y 为中心）。 */
    QRect getBounds() const;

    /** 捕获此水果时获得的分数。 */
    int getScoreValue() const { return m_scoreValue; }

    bool isActive()    const { return m_state == State::Falling; }

   /** 当动画完全结束且应移除该实体时，此属性为 true。 */
    bool isExpired()   const
    {
        return m_state != State::Falling && m_animFrame == 0;
    }

private:
    QChar     m_letter     {};
    float     m_x          {0.f};
    float     m_y          {0.f};
    float     m_speed      {0.f};
    FruitType m_type       {FruitType::Apple};
    State     m_state      {State::Falling};
    int       m_animFrame  {0};
    int       m_scoreValue {10};

    static constexpr int kRadius = 32;
};

#endif // FRUIT_H