#include "applewindow.h"
#include "SaveApple/Model/GameData.h"
#include "SaveApple/View/GameView.h"
#include "SaveApple/Controller/GameController.h"
#include "SaveApple/Config/GameConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>

// ── Constructor ──

AppleWindow::AppleWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Save the Apple"));
    setMinimumSize(700, 550);
    resize(800, 620);

    setupMvc();
    setupUi();
    setupMusic();
    setCatchSound();
    connectSignals();
}

// ── MVC wiring ──
void AppleWindow::setupMvc()
{
    m_gameData   = new GameData();
    m_gameView   = new GameView(m_gameData, this);
    m_controller = new GameController(m_gameData, m_gameView, this);

    // 将视图注册为观察者，以便在模型每次发生变化时重新绘制
    m_gameData->attach(m_gameView);
}

// ── Window chrome ──
void AppleWindow::setupUi()
{
    const int hudH = GameConfig::getInstance().getHudHeight();

    // ── HUD 栏（固定高度，位于 GameView 上方） ──
    QWidget* hud = new QWidget(this);
    hud->setObjectName(QStringLiteral("appleHud"));
    hud->setFixedHeight(hudH);
    hud->move(0, 0);

    QHBoxLayout* hudLayout = new QHBoxLayout(hud);
    hudLayout->setContentsMargins(12, 0, 12, 0);
    hudLayout->setSpacing(20);

    m_backBtn = new QPushButton(tr("◀ Back"), hud);
    m_backBtn->setObjectName(QStringLiteral("hudButton"));
    m_backBtn->setFixedWidth(90);
    connect(m_backBtn, &QPushButton::clicked, this, &QWidget::close);

    m_pauseBtn = new QPushButton(tr("Pause"), hud);
    m_pauseBtn->setObjectName(QStringLiteral("hudButton"));
    m_pauseBtn->setFixedWidth(80);
    m_pauseBtn->setVisible(false);

    hudLayout->addWidget(m_backBtn);
    hudLayout->addStretch();
    hudLayout->addWidget(m_pauseBtn);

    // ── GameView 填充 HUD 下方的区域 ──
    // We manually position it; resizeEvent keeps it in sync.
    m_gameView->setGeometry(0, hudH, width(), height() - hudH);
    m_gameView->setFocusPolicy(Qt::StrongFocus);

    // ── 开始 / 重玩按钮（位于 GameView 正上方） ──
    m_startBtn = new QPushButton(tr("Start Game"), this);
    m_startBtn->setObjectName(QStringLiteral("bigButton"));
    m_startBtn->setFixedSize(200, 55);

    // 初始位置
    resizeEvent(nullptr);
}

// ── 尺寸调整的传播 ──
void AppleWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    const int hudH = GameConfig::getInstance().getHudHeight();

    // HUD 占据全宽
    if (auto* hud = findChild<QWidget*>(QStringLiteral("appleHud")))
        hud->resize(width(), hudH);

    // GameView 负责填充其余部分
    if (m_gameView)
        m_gameView->setGeometry(0, hudH, width(), height() - hudH);

    // 将“开始”按钮置于窗口下半部分中央
    if (m_startBtn)
        m_startBtn->move((width() - m_startBtn->width()) / 2,
                         height() / 2 + 40);
}

// ── 信号/插槽连接 ──
void AppleWindow::connectSignals()
{
    // 暂停按钮
    connect(m_pauseBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isRunning())
            m_controller->pauseGame();
        else if (m_gameData->isPaused())
            m_controller->resumeGame();
    });

    // 开始 / 重播按钮
    connect(m_startBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isGameOver())
            m_controller->restartGame();
        m_controller->startGame();
        updateButtons();
    });

    // 状态发生任何变化后，刷新 Chrome
    connect(m_controller, &GameController::gameStateChanged,
            this, &AppleWindow::updateButtons);

    // 接住水果时播放接球音效
    connect(m_controller, &GameController::fruitCaught, this, [this]() {
        if (m_catchSound)
            m_catchSound->play();
    });
}

// ── 刷新按钮 ──
void AppleWindow::updateButtons()
{
    const bool gameActive = m_gameData->isRunning() || m_gameData->isPaused();

    m_pauseBtn->setVisible(gameActive);
    m_pauseBtn->setText(m_gameData->isPaused() ? tr("Resume") : tr("Pause"));

    m_startBtn->setVisible(!gameActive);
    m_startBtn->setText(m_gameData->isGameOver() ? tr("Play Again") : tr("Start Game"));
}

// ── 背景音乐 ──
void AppleWindow::setupMusic()
{
    m_musicPlayer = new QMediaPlayer(this);

    QAudioOutput* audioOutput = new QAudioOutput(this);
    m_musicPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.2f);

    m_musicPlayer->setSource(
        QUrl(QStringLiteral("qrc:/sounds/Apple/APPLE_BG.wav")));
    m_musicPlayer->setLoops(QMediaPlayer::Infinite);
    m_musicPlayer->play();
}

// ── 捕捉音效 ──
void AppleWindow::setCatchSound()
{
    m_catchSound = new QSoundEffect(this);
    m_catchSound->setSource(
        QUrl(QStringLiteral("qrc:/sounds/Apple/APPLE_IN.wav")));
    m_catchSound->setVolume(0.7f);
    m_catchSound->setLoopCount(0);
}
