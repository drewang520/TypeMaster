#include "practicewindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QFont>
#include <QRandomGenerator>
#include <QPainter>
#include <QLinearGradient>

PracticeWindow::PracticeWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Practice Mode"));
    setMinimumSize(600, 480);
    resize(700, 500);
    setFocusPolicy(Qt::StrongFocus);

    setObjectName(QStringLiteral("practiceWindow"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(16);
    layout->setContentsMargins(40, 30, 40, 30);

    // Title
    QLabel *title = new QLabel(tr("Practice Mode"), this);
    title->setAlignment(Qt::AlignCenter);
    title->setObjectName(QStringLiteral("placeholderTitle"));

    QLabel *hint = new QLabel(tr("Type the letter shown below — no lives, no pressure!"), this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setObjectName(QStringLiteral("placeholderMsg"));

    // Big prompt letter
    m_promptLabel = new QLabel(this);
    m_promptLabel->setAlignment(Qt::AlignCenter);
    m_promptLabel->setObjectName(QStringLiteral("practicePrompt"));
    m_promptLabel->setMinimumHeight(140);

    QFont bigFont(QStringLiteral("Arial"), 96, QFont::Bold);
    m_promptLabel->setFont(bigFont);

    // Feedback
    m_feedbackLabel = new QLabel(QStringLiteral(" "), this);
    m_feedbackLabel->setAlignment(Qt::AlignCenter);
    m_feedbackLabel->setObjectName(QStringLiteral("practiceFeedback"));
    m_feedbackLabel->setMinimumHeight(36);

    // Stats
    m_statsLabel = new QLabel(tr("Hits: 0   Misses: 0   Accuracy: —"), this);
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_statsLabel->setObjectName(QStringLiteral("hudLabel"));

    // Reset + Back
    QHBoxLayout *btnRow = new QHBoxLayout;
    QPushButton *resetBtn = new QPushButton(tr("Reset"), this);
    resetBtn->setObjectName(QStringLiteral("hudButton"));
    resetBtn->setFixedWidth(100);
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_hits = m_misses = m_total = 0;
        updateStats();
        nextLetter();
    });

    QPushButton *backBtn = new QPushButton(tr("◀ Back"), this);
    backBtn->setObjectName(QStringLiteral("hudButton"));
    backBtn->setFixedWidth(100);
    connect(backBtn, &QPushButton::clicked, this, &QWidget::close);

    btnRow->addStretch();
    btnRow->addWidget(resetBtn);
    btnRow->addWidget(backBtn);
    btnRow->addStretch();

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addSpacing(10);
    layout->addWidget(m_promptLabel);
    layout->addWidget(m_feedbackLabel);
    layout->addWidget(m_statsLabel);
    layout->addLayout(btnRow);

    nextLetter();
}

void PracticeWindow::nextLetter()
{
    m_current = QChar('A' + QRandomGenerator::global()->bounded(26));
    m_promptLabel->setText(m_current);
    m_feedbackLabel->setStyleSheet(QString());
    m_feedbackLabel->setText(QStringLiteral(" "));
}

void PracticeWindow::keyPressEvent(QKeyEvent *event)
{
    const QString text = event->text().toUpper();
    if (text.isEmpty()) { QWidget::keyPressEvent(event); return; }

    ++m_total;
    if (text[0] == m_current) {
        ++m_hits;
        m_feedbackLabel->setStyleSheet(QStringLiteral("color: #4CAF50; font-size: 22px; font-weight: bold;"));
        m_feedbackLabel->setText(tr("✓ Correct!"));
    } else {
        ++m_misses;
        m_feedbackLabel->setStyleSheet(QStringLiteral("color: #F44336; font-size: 22px; font-weight: bold;"));
        m_feedbackLabel->setText(tr("✗ Try again — you typed: %1").arg(text[0]));
    }

    updateStats();
    if (text[0] == m_current) nextLetter();
}

void PracticeWindow::updateStats()
{
    const QString acc = (m_total > 0)
        ? QString::number(100 * m_hits / m_total) + QStringLiteral("%")
        : QStringLiteral("—");
    m_statsLabel->setText(tr("Hits: %1   Misses: %2   Accuracy: %3")
                              .arg(m_hits).arg(m_misses).arg(acc));
}
