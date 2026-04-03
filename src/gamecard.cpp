#include "gamecard.h"

#include <QPixmap>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QEnterEvent>
#include <QSoundEffect>
#include <QUrl>
#include <QStyle>

static QSoundEffect *s_sfxHover = nullptr;
static QSoundEffect *s_sfxClick = nullptr;

static void ensureSounds(QObject *parent)
{
    if (!s_sfxHover) {
        s_sfxHover = new QSoundEffect(parent);
        s_sfxHover->setSource(QUrl(QStringLiteral("qrc:/sounds/Common/ANIBTN_ENTER")));
        s_sfxHover->setVolume(0.35f);
    }
    if (!s_sfxClick) {
        s_sfxClick = new QSoundEffect(parent);
        s_sfxClick->setSource(QUrl(QStringLiteral("qrc:/sounds/Common/BTN_CLICK")));
        s_sfxClick->setVolume(0.6f);
    }
}

GameCard::GameCard(const QString &title,
                   const QString &description,
                   const QString &imagePath,
                   const QString &gameId,
                   QWidget *parent)
    : QFrame(parent)
    , m_gameId(gameId)
{
    ensureSounds(this);
    setupUi(title, description, imagePath);

    setObjectName(QStringLiteral("gameCard"));
    setCursor(Qt::PointingHandCursor);
    setFrameShape(QFrame::StyledPanel);

    // ── Responsive sizing ─────────────────────────────────────────────────
    // Minimum so cards never become unreadably small
    setMinimumSize(200, 300);
    // Maximum so cards don't stretch absurdly wide on ultra-wide monitors
    setMaximumSize(420, 540);
    // Allow the layout to stretch the card to fill available space equally
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(shadow);
}

void GameCard::setupUi(const QString &title,
                        const QString &description,
                        const QString &imagePath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 16);
    layout->setSpacing(0);

    // ── Preview image (proportional height ~44% of card) ─────────────────
    m_previewLabel = new QLabel(this);
    m_previewLabel->setObjectName(QStringLiteral("cardPreview"));
    m_previewLabel->setMinimumHeight(120);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    // Expand horizontally, fixed vertically via ratio below
    m_previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPixmap px(imagePath);
    if (!px.isNull()) {
        // Defer actual scaled drawing to resizeEvent; store original
        m_previewLabel->setProperty("srcPath", imagePath);
        m_previewLabel->setPixmap(
            px.scaled(QSize(280, 160),
                      Qt::KeepAspectRatioByExpanding,
                      Qt::SmoothTransformation));
        m_previewLabel->setScaledContents(false);
    } else {
        // Gradient fallback
        QPixmap fallback(280, 160);
        fallback.fill(Qt::transparent);
        QPainter p(&fallback);
        p.setRenderHint(QPainter::Antialiasing);
        QLinearGradient grad(0, 0, 0, 160);
        if (m_gameId == QStringLiteral("apple")) {
            grad.setColorAt(0, QColor(0x4CAF50));
            grad.setColorAt(1, QColor(0x1B5E20));
        } else if (m_gameId == QStringLiteral("space")) {
            grad.setColorAt(0, QColor(0x1A237E));
            grad.setColorAt(1, QColor(0x000051));
        } else {
            grad.setColorAt(0, QColor(0x7B1FA2));
            grad.setColorAt(1, QColor(0x1A0033));
        }
        p.fillRect(fallback.rect(), grad);
        p.setPen(QColor(255, 255, 255, 160));
        p.setFont(QFont(QStringLiteral("Arial"), 40));
        const QString icon =
            (m_gameId == QStringLiteral("apple")) ? QStringLiteral("🍎")
          : (m_gameId == QStringLiteral("space"))  ? QStringLiteral("🚀")
                                                   : QStringLiteral("⌨");
        p.drawText(fallback.rect(), Qt::AlignCenter, icon);
        m_previewLabel->setPixmap(fallback);
    }

    // ── Text area ─────────────────────────────────────────────────────────
    QWidget *textArea = new QWidget(this);
    textArea->setObjectName(QStringLiteral("cardTextArea"));
    QVBoxLayout *textLayout = new QVBoxLayout(textArea);
    textLayout->setContentsMargins(16, 12, 16, 0);
    textLayout->setSpacing(8);

    m_titleLabel = new QLabel(title, textArea);
    m_titleLabel->setObjectName(QStringLiteral("cardTitle"));
    m_titleLabel->setWordWrap(true);

    m_descLabel = new QLabel(description, textArea);
    m_descLabel->setObjectName(QStringLiteral("cardDesc"));
    m_descLabel->setWordWrap(true);
    m_descLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_playButton = new QPushButton(tr("Play"), textArea);
    m_playButton->setObjectName(QStringLiteral("playButton"));
    m_playButton->setCursor(Qt::PointingHandCursor);
    m_playButton->setFixedHeight(40);

    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_descLabel, 1);
    textLayout->addWidget(m_playButton);

    // Preview takes ~45% of height, text area takes rest
    layout->addWidget(m_previewLabel, 45);
    layout->addWidget(textArea, 55);

    connect(m_playButton, &QPushButton::clicked, this, [this]() {
        if (s_sfxClick) s_sfxClick->play();
        emit playRequested(m_gameId);
    });
}

void GameCard::enterEvent(QEnterEvent *event)
{
    QFrame::enterEvent(event);
    if (s_sfxHover) s_sfxHover->play();
    setProperty("hovered", true);
    style()->polish(this);
}

void GameCard::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);
    setProperty("hovered", false);
    style()->polish(this);
}
