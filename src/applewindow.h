#ifndef APPLEWINDOW_H
#define APPLEWINDOW_H       

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioOutput>
#include <QRandomGenerator>

// ── Data types ────────────────────────────────────────────────────────────────

struct AppleItem {
    QChar   letter;      // letter shown on apple
    qreal   x{0};        // centre x (pixels)
    qreal   y{0};        // centre y (pixels)
    qreal   speed{0};    // fall speed (pixels / tick)

    enum class State { Falling, Caught, Broken } state{State::Falling};
    int animFrame{0};    // countdown frames for Caught/Broken animation

    bool isActive() const { return state == State::Falling; }
};

// ── Game states ───────────────────────────────────────────────────────────────

enum class GameState { Ready, Running, Paused, GameOver };

// ── Apple game window ─────────────────────────────────────────────────────────

class AppleWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AppleWindow(QWidget *parent = nullptr);
    ~AppleWindow() override = default;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // ── UI setup ─────────────────────────────────────────────────────────
    void setupUi();
    void updateHud();

    // —— Background music ───────────────────────────────────────────────────
    void setupMusic();

    // ── Game logic ────────────────────────────────────────────────────────
    void startGame();
    void pauseGame();
    void resumeGame();
    void endGame();
    void resetGame();

    void spawnApple();
    void updateApples();
    bool tryTypeLetter(QChar ch);

    // ── Rendering helpers ─────────────────────────────────────────────────
    void drawBackground(QPainter &p);
    void drawApples(QPainter &p);
    void drawBasket(QPainter &p);
    void drawHud(QPainter &p);
    void drawOverlay(QPainter &p);


    // Background music player
    QMediaPlayer *m_musicPlayer{nullptr};

    // ── HUD widgets (overlay on top of game area) ─────────────────────────
    QLabel      *m_scoreLabel{nullptr};
    QLabel      *m_livesLabel{nullptr};
    QLabel      *m_levelLabel{nullptr};
    QPushButton *m_pauseBtn{nullptr};
    QPushButton *m_startBtn{nullptr};
    QPushButton *m_backBtn{nullptr};

    // ── Resources ─────────────────────────────────────────────────────────
    QPixmap m_bgPixmap;
    QPixmap m_appleNormal;
    QPixmap m_appleBad;
    QPixmap m_appleBasket;
    QPixmap m_appleSmall;

    // ── Game loop ─────────────────────────────────────────────────────────
    QTimer *m_gameTimer{nullptr};
    int     m_tickCount{0};
    int     m_spawnInterval{60};   // ticks between apple spawns
    int     m_nextSpawn{0};

    // ── Game data ─────────────────────────────────────────────────────────
    GameState         m_state{GameState::Ready};
    QVector<AppleItem> m_apples;
    int               m_score{0};
    int               m_lives{5};
    int               m_level{1};
    int               m_caughtCount{0};  // total caught this game
    int               m_missedCount{0};  // total missed this game
    QVector<QChar>    m_basketApples;    // small apples in basket display

    static constexpr int  kAppleRadius   = 32;
    static constexpr int  kMaxLives      = 5;
    static constexpr int  kBasketY       = 20;   // distance from bottom
    static constexpr int  kTickMs        = 30;   // timer interval

private slots:
    void onTick();
};


#endif // APPLEWINDOW_H