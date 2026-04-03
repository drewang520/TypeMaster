#include "mainwindow.h"
#include "gamecard.h"
#include "applewindow.h"
#include "spacewindow.h"
#include "practicewindow.h"

#include <QScrollArea>
#include <QFile>
#include <QApplication>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();  // 创建所有控件和布局
    setupStyleSheet(); // 加载并应用样式表
    connectSignals();   // 连接信号和槽

    setMinimumSize(800, 560); // 设置窗口的最小尺寸，确保界面元素不会过度拥挤   
    resize(1100, 720);     // 设置窗口的初始尺寸，提供一个适合大多数屏幕的默认大小
    setWindowTitle(tr("TypeMaster — Typing Game")); // 设置窗口标题，使用 tr() 以支持国际化
}

void MainWindow::setupUi()
{
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName(QStringLiteral("centralWidget"));
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // ── Header ────────────────────────────────────────────────────────────
    QWidget *headerWidget = new QWidget(m_centralWidget);
    headerWidget->setObjectName(QStringLiteral("headerWidget"));
    headerWidget->setFixedHeight(120);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(8);

    m_titleLabel = new QLabel(tr("TypeMaster"), headerWidget);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel(tr("Choose your game — improve your typing speed!"), headerWidget);
    m_subtitleLabel->setObjectName(QStringLiteral("subtitleLabel"));
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_subtitleLabel);

    // ── Scroll area ───────────────────────────────────────────────────────
    QScrollArea *scrollArea = new QScrollArea(m_centralWidget);
    scrollArea->setObjectName(QStringLiteral("scrollArea"));
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // ── Cards container: outer VBox centres the row vertically ───────────
    m_cardsContainer = new QWidget();
    m_cardsContainer->setObjectName(QStringLiteral("cardsContainer"));

    QVBoxLayout *outerV = new QVBoxLayout(m_cardsContainer);
    outerV->setContentsMargins(0, 0, 0, 0);
    outerV->setSpacing(0);
    outerV->addStretch(1);   // vertical centering — top padding

    // Inner row widget
    QWidget *rowWidget = new QWidget(m_cardsContainer);
    rowWidget->setObjectName(QStringLiteral("cardsRow"));

    m_cardsLayout = new QHBoxLayout(rowWidget);
    m_cardsLayout->setContentsMargins(40, 20, 40, 20);
    m_cardsLayout->setSpacing(30);
    // No alignment — use stretch weights for equal distribution + centering

    // ── Game cards ────────────────────────────────────────────────────────
    m_appleCard = new GameCard(
        tr("Save the Apple"),
        tr("Type the letters shown on falling apples before they hit the ground.\n"
           "Catch as many as you can — each miss costs a life!"),
        QStringLiteral(":/images/Apple/APPLE_NORMAL.png"),
        QStringLiteral("apple"),
        rowWidget
    );

    m_spaceCard = new GameCard(
        tr("Space War"),
        tr("Enemy ships are invading! Type the letters on each enemy to blast them.\n"
           "Survive as long as possible to beat the high score."),
        QStringLiteral(":/images/Space/SPACE_BACKGROUND.png"),
        QStringLiteral("space"),
        rowWidget
    );

    m_practiceCard = new GameCard(
        tr("Practice Mode"),
        tr("Free typing practice — no lives, no pressure.\n"
           "Perfect for warming up your fingers."),
        QStringLiteral(":/images/Common/PUBLIC_START.png"),
        QStringLiteral("practice"),
        rowWidget
    );

    // Equal stretch weight (1) for each card → fills space evenly
    // Side stretches (1) → cards are centred when window is wide
    m_cardsLayout->addStretch(1);
    m_cardsLayout->addWidget(m_appleCard,    3);
    m_cardsLayout->addWidget(m_spaceCard,    3);
    m_cardsLayout->addWidget(m_practiceCard, 3);
    m_cardsLayout->addStretch(1);

    outerV->addWidget(rowWidget);
    outerV->addStretch(1);   // vertical centering — bottom padding

    scrollArea->setWidget(m_cardsContainer);
    
    // ── Footer ────────────────────────────────────────────────────────────
    QWidget *footerWidget = new QWidget(m_centralWidget);
    footerWidget->setObjectName(QStringLiteral("footerWidget"));
    footerWidget->setFixedHeight(40);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerWidget);
    QLabel *footerLabel = new QLabel(
        tr("TypeMaster v1.0  ·  Qt %1").arg(QT_VERSION_STR), footerWidget);
    footerLabel->setObjectName(QStringLiteral("footerLabel"));
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLayout->addWidget(footerLabel);

    m_mainLayout->addWidget(headerWidget);
    m_mainLayout->addWidget(scrollArea, 1);
    m_mainLayout->addWidget(footerWidget);
}

void MainWindow::setupStyleSheet()
{
    QFile qssFile(QStringLiteral(":/styles/main.qss"));
    if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(qssFile.readAll()));
        qssFile.close();
    }
}

void MainWindow::connectSignals()
{
    connect(m_appleCard,    &GameCard::playRequested, this, &MainWindow::onAppleGameRequested);
    connect(m_spaceCard,    &GameCard::playRequested, this, &MainWindow::onSpaceGameRequested);
    connect(m_practiceCard, &GameCard::playRequested, this, &MainWindow::onPracticeGameRequested);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
}

void MainWindow::onAppleGameRequested()
{
    AppleWindow *w = new AppleWindow(this);
    w->setWindowFlags(Qt::Window);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void MainWindow::onSpaceGameRequested()
{
    SpaceWindow *w = new SpaceWindow(this);
    w->setWindowFlags(Qt::Window);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void MainWindow::onPracticeGameRequested()
{
    PracticeWindow *w = new PracticeWindow(this);
    w->setWindowFlags(Qt::Window);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}
