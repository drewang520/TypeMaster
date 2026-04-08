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
    // setWindowTitle(tr("TypeMaster — Typing Game")); // 设置窗口标题，使用 tr() 以支持国际化
}

void MainWindow::setupUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName(QStringLiteral("centralWidget"));
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // ── Topbar ──
    setupTopBar(m_centralWidget);
    m_mainLayout->addWidget(m_topBar);

    // ── Header ──
    QWidget *headerWidget = new QWidget(m_centralWidget);
    headerWidget->setObjectName(QStringLiteral("headerWidget"));
    headerWidget->setFixedHeight(120);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(8);

    m_titleLabel = new QLabel(tr("Welcome to Kingsoft TypeMaster"), headerWidget);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel(tr("Choose your game — improve your typing speed!"), headerWidget);
    m_subtitleLabel->setObjectName(QStringLiteral("subtitleLabel"));
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_subtitleLabel);

    // ── Scroll area ──
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
    rowWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_cardsLayout = new QHBoxLayout(rowWidget);
    m_cardsLayout->setContentsMargins(40, 20, 40, 20);
    m_cardsLayout->setSpacing(40);
    m_cardsLayout->setAlignment(Qt::AlignCenter | Qt::AlignVCenter); //居中对齐
    // No alignment — use stretch weights for equal distribution + centering

    // ── Game cards ────────────────────────────────────────────────────────
    m_appleCard = new GameCard(
        tr("Save the Apple"),
        tr("Type the letters shown on falling apples before they hit the ground.\n"
           "Catch as many as you can — each miss costs a life!"),
        QStringLiteral(":/images/Apple/save.png"),
        QStringLiteral("apple"),
        rowWidget
    );

    m_spaceCard = new GameCard(
        tr("Space War"),
        tr("Enemy ships are invading! Type the letters on each enemy to blast them.\n"
           "Survive as long as possible to beat the high score."),
        QStringLiteral(":/images/Space/space.png"),
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

    m_cardsLayout->addWidget(m_appleCard);
    m_cardsLayout->addWidget(m_spaceCard);
    m_cardsLayout->addWidget(m_practiceCard);

    outerV->addWidget(rowWidget);
    outerV->addStretch(1);   // vertical centering — bottom padding

    scrollArea->setWidget(m_cardsContainer);
    
    // ── Footer ──
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

void MainWindow::setupTopBar(QWidget *parent)
{
    m_topBar = new QWidget(parent);
    m_topBar->setObjectName(QStringLiteral("m_topBar"));
    m_topBar->setFixedHeight(45);

    QHBoxLayout *m_topBarLayout = new QHBoxLayout(m_topBar);
    m_topBarLayout->setContentsMargins(12, 0, 8, 0);
    m_mainLayout->setSpacing(2);
    
    // 左侧：Logo占位
    QLabel *logoLabel = new QLabel(m_topBar);
    logoLabel->setObjectName(QStringLiteral("logoLabel"));
    logoLabel->setFixedSize(44, 44);
    QPixmap logoPixmap(":/images/Common/logo_48.png");
    logoLabel->setPixmap(
        logoPixmap.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setWindowIcon(QIcon(QStringLiteral(":/images/Common/logo.png"))); // 设置应用程序图标
    
    // 中间：品牌信息（垂直布局，需要一个中间容器）
    QWidget *centerWiget = new QWidget(m_topBar);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWiget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(2);
    centerLayout->setAlignment(Qt::AlignCenter);


    QLabel *brandLabel = new QLabel(tr("Kingsoft Typing Master"), centerWiget);
    QLabel *sloganLabel = new QLabel(tr("Tech Geeks Changing the World"), centerWiget);
    brandLabel->setObjectName(QStringLiteral("brandLabel"));
    sloganLabel->setObjectName(QStringLiteral("sloganLabel"));
    brandLabel->setAlignment(Qt::AlignCenter);
    sloganLabel->setAlignment(Qt::AlignCenter);

    centerLayout->addWidget(brandLabel);
    centerLayout->addWidget(sloganLabel);
    
    QLabel *yearLabel = new QLabel(tr("2026"), m_topBar);
    yearLabel->setObjectName(QStringLiteral("yearLabel"));
    yearLabel->setAlignment(Qt::AlignCenter); 

    // QLabel *appLabel = new QLabel(m_topBar);
    // appLabel->setObjectName(QStringLiteral("appLabel"));
    // appLabel->setFixedSize(20, 20);
    // QPixmap appPixmap(":/images/Common/apple.png");
    // appLabel->setPixmap(
    //     appPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // QLabel *airplaneLabel = new QLabel(m_topBar);
    // airplaneLabel->setObjectName(QStringLiteral("airplaneLabel"));
    // airplaneLabel->setFixedSize(35, 35);
    // QPixmap airplanePixmap(":/images/Common/airplane.png");
    // airplaneLabel->setPixmap(
    //     airplanePixmap.scaled(35, 35, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QPushButton *minBtn   = new QPushButton(tr("⚊"), m_topBar);
    m_maxBtn   = new QPushButton(tr("☐"), m_topBar);
    QPushButton *closeBtn = new QPushButton(tr("✕"), m_topBar);

    minBtn->setObjectName(QStringLiteral("minBtn"));
    m_maxBtn->setObjectName(QStringLiteral("maxBtn"));
    closeBtn->setObjectName(QStringLiteral("closeBtn"));

    minBtn->setFocusPolicy(Qt::NoFocus);
    m_maxBtn->setFocusPolicy(Qt::NoFocus);
    closeBtn->setFocusPolicy(Qt::NoFocus);

    minBtn->setFixedSize(38, 38);
    m_maxBtn->setFixedSize(38, 38);
    closeBtn->setFixedSize(38, 38);

    connect(minBtn,   &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(m_maxBtn,   &QPushButton::clicked, this, &MainWindow::animateToggleMaximize);
    connect(closeBtn, &QPushButton::clicked, this, &QMainWindow::close);

    m_topBarLayout->addWidget(logoLabel);
    m_topBarLayout->addSpacing(8); // logo和中间内容之间的间距
    m_topBarLayout->addWidget(centerWiget); 
    m_topBarLayout->addWidget(yearLabel);
    m_topBarLayout->addStretch(1); // 左侧伸缩，推挤中心和右侧内容
    // m_topBarLayout->addWidget(appLabel);
    // m_topBarLayout->addSpacing(8); // logo和中间内容之间的间距
    // m_topBarLayout->addWidget(airplaneLabel);
    // m_topBarLayout->addStretch(1); // 左侧伸缩，推挤中心和右侧内容
    m_topBarLayout->addWidget(minBtn);
    m_topBarLayout->addWidget(m_maxBtn);
    m_topBarLayout->addWidget(closeBtn);
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

void MainWindow::mousePressEvent(QMouseEvent *event) 
{
    if (event->button() == Qt::LeftButton
        && event->position().y() < 60) {  // 只在 m_topBar 区域内才能拖
        m_dragging = true;
        m_dragPos  = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) 
{
    if (m_dragging && (event->buttons() & Qt::LeftButton))
        move(event->globalPosition().toPoint() - m_dragPos);
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) 
{
    m_dragging = false;
    QMainWindow::mouseReleaseEvent(event);
}


// ── 通用：把窗口 geometry 动画到目标区域 ─────────────────────────
void MainWindow::animateTo(const QRect &target, int durationMs)
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry", this);
    anim->setDuration(durationMs);
    anim->setStartValue(geometry());
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::OutCubic);  // 先快后慢，有弹性感
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ── 最大化 / 还原切换 ─────────────────────────────────────────────
void MainWindow::animateToggleMaximize()
{
    if (!m_isMaximized) {
        // ── 放大 ──────────────────────────────────────────────────
        m_normalGeometry = geometry();  // 记住当前位置，还原时用

        // availableGeometry = 屏幕可用区域（排除任务栏），不会盖住任务栏
        QRect screen = QApplication::screenAt(geometry().center())
                           ->availableGeometry();

        animateTo(screen, 220);

        m_isMaximized = true;
        m_maxBtn->setText(QStringLiteral("❐"));  // 切换为"还原"图标

    } else {
        // ── 还原 ──────────────────────────────────────────────────
        animateTo(m_normalGeometry, 200);

        m_isMaximized = false;
        m_maxBtn->setText(QStringLiteral("☐"));  // 切换回"最大化"图标
    }
}

// ── 最小化动画（保持原来的，加上几何收缩）────────────────────────
void MainWindow::animateMinimize()
{
    if (m_minimizeAnim->state() == QAbstractAnimation::Running) return;

    // 窗口向下收缩
    QPropertyAnimation *geomAnim = new QPropertyAnimation(this, "geometry", this);
    geomAnim->setDuration(180);
    geomAnim->setStartValue(geometry());
    QRect endRect = geometry();
    endRect.setTop(endRect.bottom() - 2);
    endRect.setLeft(endRect.center().x() - 100);
    endRect.setWidth(200);
    geomAnim->setEndValue(endRect);
    geomAnim->setEasingCurve(QEasingCurve::InCubic);
    geomAnim->start(QAbstractAnimation::DeleteWhenStopped);

    m_minimizeAnim->start();  // 淡出透明度
}