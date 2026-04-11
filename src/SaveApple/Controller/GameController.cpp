#include "GameController.h"

#include <QRandomGenerator>
#include <algorithm>

#include "../Model/GameData.h"
#include "../Model/Fruit.h"
#include "../View/GameView.h"
#include "../Config/GameConfig.h"

// ── Constructor ──

GameController::GameController(GameData* model, GameView* view, QObject* parent)
    : QObject(parent)
    , m_model(model)
    , m_view(view)
{
    const GameConfig& cfg = GameConfig::getInstance();
    m_spawnInterval = cfg.getInitialSpawnInterval();

    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(cfg.getTickMs());
    connect(m_gameTimer, &QTimer::timeout, this, &GameController::onTick);
}

// ── Public control slots ──

void GameController::startGame()
{
    m_model->setState(GameState::Running);
    m_tickCount     = 0;
    m_nextSpawn     = 0;
    m_spawnInterval = GameConfig::getInstance().getInitialSpawnInterval();
    m_gameTimer->start();
    emit gameStateChanged();
    m_view->setFocus();
}

void GameController::pauseGame()
{
    m_model->setState(GameState::Paused);
    m_gameTimer->stop();
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
    m_model->reset();
    m_spawnInterval = GameConfig::getInstance().getInitialSpawnInterval();
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
    fruits[bestIdx].catchFruit();

    // Update model
    m_model->addScore(GameConfig::getInstance().getBaseScorePerCatch()
                      * m_model->getLevel());
    m_model->incrementCaught();

    QVector<QChar>& basket = m_model->getBasketApples();
    if (basket.size() < GameConfig::getInstance().getMaxBasketDisplay())
        basket.append(ch);

    emit fruitCaught();
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
        m_nextSpawn = m_tickCount + m_spawnInterval;
    }

    updateLevel();
    m_model->notifyObservers();
}

void GameController::updateFruits()
{
    const GameConfig& cfg  = GameConfig::getInstance();
    const int groundY = m_view->height()
                        - cfg.getBasketOffsetY()
                        - cfg.getAppleRadius();

    QVector<Fruit>& fruits = m_model->getFruits();

    for (Fruit& f : fruits) {
        if (f.isActive()) {
            f.update();

            if (static_cast<int>(f.getY()) >= groundY) {
                f.breakFruit();
                m_model->incrementMissed();
                m_model->loseLife(); // notifyObservers() inside loseLife()

                if (m_model->getLives() <= 0) {
                    m_model->setState(GameState::GameOver);
                    m_gameTimer->stop();
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
    if (m_model->getFruits().size() >= cfg.getMaxFruitsOnScreen())
        return;

    const QChar letter = QChar('A' + QRandomGenerator::global()->bounded(26));
    const int margin   = cfg.getAppleRadius() + 10;
    const float x      = static_cast<float>(
        QRandomGenerator::global()->bounded(margin, m_view->width() - margin));
    const float y      = static_cast<float>(cfg.getHudHeight() + cfg.getAppleRadius());
    const float speed  = cfg.getBaseSpeed()
                       + m_model->getLevel() * cfg.getSpeedPerLevel()
                       + static_cast<float>(
                           QRandomGenerator::global()->generateDouble()
                           * cfg.getSpeedRandRange());

    m_model->getFruits().append(Fruit(letter, x, y, speed));
}

void GameController::updateLevel()
{
    const GameConfig& cfg     = GameConfig::getInstance();
    const int         newLvl  = 1 + m_model->getCaughtCount() / cfg.getCatchesPerLevel();

    if (newLvl != m_model->getLevel()) {
        m_model->setLevel(newLvl); // notifies observers
        m_spawnInterval = qMax(cfg.getMinSpawnInterval(),
                               cfg.getInitialSpawnInterval()
                               - (newLvl - 1) * cfg.getSpawnDecrement());
    }
}