#include "GameController.h"
#include "SaveApple/Model/GameData.h"
#include "SaveApple/Model/Fruit.h"
#include "SaveApple/View/GameView.h"
#include "SaveApple/Config/GameConfig.h"

#include <QRandomGenerator>
#include <QSet>
#include <algorithm>

// ── Constructor ──

GameController::GameController(GameData* model, GameView* view, QObject* parent)
    : QObject(parent)
    , m_model(model)
    , m_view(view)
{
    const GameConfig& cfg = GameConfig::getInstance();

    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(cfg.getTickMs());
    connect(m_gameTimer, &QTimer::timeout, this, &GameController::onTick);

    // 用于“关卡完成”自动跳过延迟的一次性定时器
    m_levelTimer = new QTimer(this);
    m_levelTimer->setSingleShot(true);
    connect(m_levelTimer, &QTimer::timeout, this, &GameController::onLevelCompleteEnd);
}

// ── Public control slots ──

void GameController::startGame()
{
    m_tickCount     = 0;
    m_nextSpawn     = 0;
    m_model->setState(GameState::Running);
    m_gameTimer->start();
    emit gameStateChanged();
    m_view->setFocus();
}

void GameController::pauseGame()
{
    m_model->setState(GameState::Paused);
    m_gameTimer->stop();
    m_levelTimer->stop();
    emit gameStateChanged();
}

void GameController::resumeGame()
{
    m_model->setState(GameState::Running);
    m_gameTimer->start();
    emit gameStateChanged();
    m_view->setFocus();
}

void GameController::restartGame()
{
    m_gameTimer->stop();
    m_levelTimer->stop();
    m_model->reset();
    m_tickCount     = 0;
    m_nextSpawn     = 0;
    emit gameStateChanged();
}

// ── Key handling ──

void GameController::handleKeyPress(QChar ch)
{
    if (!m_model->isRunning())
        return;

    QVector<Fruit>& fruits = m_model->getFruits();

    // Find the lowest (most urgent) matching falling fruit
    int   bestIdx = -1;
    float bestY   = -1.f;

    for (int i = 0; i < fruits.size(); ++i) {
        const Fruit& f = fruits[i];
        if (f.isActive() && f.getLetter() == ch && f.getY() > bestY) {
            bestY   = f.getY();
            bestIdx = i;
        }
    }

    if (bestIdx < 0)
        return;

    // Mark as caught
    const QChar letter = fruits[bestIdx].getLetter();
    fruits[bestIdx].catchFruit();
    m_model->removeActiveLetter(letter);

    const GameConfig& cfg = GameConfig::getInstance();
    m_model->addScore(cfg.getBaseScorePerCatch() * m_model->getLevel());
    m_model->incrementCaught();
    m_model->incrementLevelCaught();
 
    if (m_model->getBasketApples().size() < cfg.getMaxBasketDisplay())
        m_model->getBasketApples().append(letter);
 
    emit fruitCaught();
 
    // Check if this catch triggered level completion
    checkLevelComplete();
 
    m_model->notifyObservers();
}

// ── Core game tick ──

void GameController::onTick()
{
    ++m_tickCount;
    updateFruits();

    // Spawn
    if (m_tickCount >= m_nextSpawn) {
        spawnFruit();
        m_nextSpawn = m_tickCount + GameConfig::getInstance().getSpawnInterval();
    }

    m_model->notifyObservers();
}

// ── Level-complete auto-advance ──
void GameController::onLevelCompleteEnd()
{
    // Clear remaining fruits from the completed level
    for (Fruit& f : m_model->getFruits())
        if (f.isActive())
            m_model->removeActiveLetter(f.getLetter());
    m_model->getFruits().clear();
 
    // Advance level
    m_model->setLevel(m_model->getLevel() + 1);
    m_model->resetLevelCaught();
 
    m_model->setState(GameState::Running);
    m_gameTimer->start();
    emit gameStateChanged();
    m_view->setFocus();
}

// ── Internal helpers ──
void GameController::updateFruits()
{
    // Requirement: apple is missed when it reaches 70% of the GameView height
    const int groundY = static_cast<int>(m_view->height()
                                         * GameConfig::getInstance().getGroundRatio());

    QVector<Fruit>& fruits = m_model->getFruits();

    for (Fruit& f : fruits) {
        if (f.isActive()) {
            f.update();

            if (static_cast<int>(f.getY()) >= groundY) {
                m_model->removeActiveLetter(f.getLetter());
                f.breakFruit();
                m_model->incrementMissed();
                m_model->loseLife(); // notifyObservers() inside loseLife()

                if (m_model->getLives() <= 0) {
                    m_gameTimer->stop();
                    m_levelTimer->stop();
                    m_model->setState(GameState::GameOver);
                    emit gameStateChanged();
                    return;
                }
            }
        } else {
            f.tickAnim();
        }
    }

    // Remove fully expired animations
    fruits.erase(
        std::remove_if(fruits.begin(), fruits.end(),
                       [](const Fruit& f) { return f.isExpired(); }),
        fruits.end());
}

void GameController::spawnFruit()
{
    const GameConfig& cfg = GameConfig::getInstance();

    // Don't spawn while in level-complete animation
    if (!m_model->isRunning())
        return;

    // Respect max-on-screen setting
    if (m_model->getFruits().size() >= cfg.getMaxFruitsOnScreen())
        return;

    // ── No-duplicate letter constraint ──
    const QSet<QChar>& active = m_model->getActiveLetters();
    if (active.size() >= 26)
        return; // every letter is already on screen
    QChar letter;
    int attempts = 0;
    do {
        letter = QChar('A' + QRandomGenerator::global()->bounded(26));
        ++attempts;
    } while (active.contains(letter) && attempts < 50);
 
    if (active.contains(letter))
        return; // failed to find a free letter
    
    // ── Position and speed ──
    const int margin = cfg.getAppleRadius() + 10;
    const float x = static_cast<float>(
        QRandomGenerator::global()->bounded(margin, m_view->width() - margin));
    const float y = static_cast<float>(cfg.getAppleRadius());
 
    const float speed = cfg.computeBaseSpeed(m_model->getLevel())
                      + static_cast<float>(
                            QRandomGenerator::global()->generateDouble()
                            * cfg.getSpeedRandRange());
 
    m_model->addActiveLetter(letter);
    m_model->getFruits().append(Fruit(letter, x, y, speed));
}

void GameController::checkLevelComplete()
{
    const GameConfig& cfg = GameConfig::getInstance();
 
    if (m_model->getLevelCaught() < cfg.getLevelTarget())
        return;
 
    // Level target reached!
    m_gameTimer->stop();
    m_model->setState(GameState::LevelComplete);
    emit gameStateChanged();
 
    // Auto-advance after the overlay display delay
    const int delayMs = cfg.getLevelCompleteDelay() * cfg.getTickMs();
    m_levelTimer->start(delayMs);
}