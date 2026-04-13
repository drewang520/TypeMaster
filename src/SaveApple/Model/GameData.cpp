#include "GameData.h"
#include <algorithm>

// ── Constructor ──
GameData::GameData() = default;

// ── Score ──
void GameData::addScore(int value)
{
    m_score += value;
    notifyObservers();
}

// ── Lives ──
void GameData::loseLife()
{
    if (m_lives > 0)
        --m_lives;
    notifyObservers();
}

void GameData::addLife()
{
    if (m_lives < m_maxLives)
        ++m_lives;
    notifyObservers();
}

// ── Level ──
void GameData::setLevel(int level)
{
    m_level = level;
    notifyObservers();
}

// ── 每层捕获计数器 ──
void GameData::incrementLevelCaught()
{
    ++m_levelCaught;
    // 此处不调用 notify — 控制器会在完整更新后调用 notifyObservers
}
 
void GameData::resetLevelCaught()
{
    m_levelCaught = 0;
}


// ── Statistics ──
void GameData::incrementCaught()
{
    ++m_caughtCount;
    // No observer notification — Controller calls addScore() separately
}

void GameData::incrementMissed()
{
    ++m_missedCount;
}

// ── Game state ──
void GameData::setState(GameState state)
{
    m_state = state;
    notifyObservers();
}

// ── Full reset ──
void GameData::reset()
{
    m_score       = 0;
    m_lives       = m_maxLives;
    m_level       = 1;
    m_caughtCount = 0;
    m_missedCount = 0;
    m_levelCaught  = 0;
    m_state       = GameState::Ready;
    m_fruits.clear();
    m_basketApples.clear();
    m_activeLetters.clear();
    notifyObservers();
}

// ── Observer pattern ──
void GameData::attach(IObserver* observer)
{
    m_observers.push_back(observer);
}

void GameData::detach(IObserver* observer)
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), observer),
        m_observers.end());
}

void GameData::notifyObservers()
{
    for (IObserver* obs : m_observers)
        obs->onUpdate();
}

