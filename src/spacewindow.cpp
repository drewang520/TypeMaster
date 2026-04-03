#include "spacewindow.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QLinearGradient>
#include <QPixmap>
#include <QRandomGenerator>

SpaceWindow::SpaceWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Space War"));
    setMinimumSize(700, 500);
    resize(800, 600);

    // Load background
    QPixmap bg(QStringLiteral(":/images/Space/SPACE_MAINMENU_BG.png"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setObjectName(QStringLiteral("placeholderIcon"));
    iconLabel->setText(QStringLiteral("🚀"));
    QFont bigFont(QStringLiteral("Arial"), 72);
    iconLabel->setFont(bigFont);

    QLabel *titleLabel = new QLabel(tr("Space War"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName(QStringLiteral("placeholderTitle"));

    QLabel *msgLabel = new QLabel(
        tr("Coming soon!\nGame logic will be implemented in the next sprint."), this);
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->setObjectName(QStringLiteral("placeholderMsg"));

    QPushButton *backBtn = new QPushButton(tr("◀ Back to Menu"), this);
    backBtn->setObjectName(QStringLiteral("bigButton"));
    backBtn->setFixedSize(200, 50);
    connect(backBtn, &QPushButton::clicked, this, &QWidget::close);

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(msgLabel);
    layout->addSpacing(20);
    layout->addWidget(backBtn, 0, Qt::AlignHCenter);
}

void SpaceWindow::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0.0, QColor(0x0D0D2B));
    grad.setColorAt(0.5, QColor(0x1A1A3E));
    grad.setColorAt(1.0, QColor(0x000010));
    p.fillRect(rect(), grad);

    // Star field decoration
    p.setPen(QColor(255, 255, 255, 180));
    QRandomGenerator rng(42); // deterministic seed for stable star positions
    for (int i = 0; i < 120; ++i) {
        const int sx = rng.bounded(width());
        const int sy = rng.bounded(height());
        const int sr = 1 + (i % 3 == 0 ? 1 : 0);
        p.drawEllipse(sx, sy, sr, sr);
    }

    QWidget::paintEvent(event);
}
