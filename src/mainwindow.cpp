#include "mainwindow.h"
#include "gamecard.h"
#include "applewindow.h"
#include "spacewindow.h"
#include "practicewindow.h"

#include <QScrollArea>
#include <QFile>
#include <QApplication>
#include <QResizeEvent>
#include <QFontDatabase>

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

    m_titleLabel = new QLabel(tr("Welcome to the KeyVerse"), headerWidget);
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

    // ── Game cards ──
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

    m_cardsLayout->addWidget(m_appleCard);
    m_cardsLayout->addWidget(m_spaceCard);
    m_cardsLayout->addWidget(m_practiceCard);

    outerV->addWidget(rowWidget);
    outerV->addStretch(1);   // vertical centering — bottom padding

    scrollArea->setWidget(m_cardsContainer);
    
    // ── Footer ──
    QWidget *footerWidget = new QWidget(m_centralWidget);
    footerWidget->setObjectName(QStringLiteral("footerWidget"));
    footerWidget->setFixedHeight(30);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerWidget);
    QLabel *footerLabel = new QLabel(
        tr("KeyVerse v1.0").arg(QT_VERSION_STR), footerWidget);
    footerLabel->setObjectName(QStringLiteral("footerLabel"));
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLayout->addWidget(footerLabel);

    m_mainLayout->addWidget(headerWidget);
    m_mainLayout->addWidget(scrollArea, 1);
    m_mainLayout->addWidget(footerWidget);

    setMouseTracking(true); // 启用鼠标跟踪，以便在没有按下鼠标按钮时也能接收 mouseMoveEvent 事件，用于实现悬停调整窗口大小的功能
    m_centralWidget->setMouseTracking(true); // 同样启用 centralWidget 的鼠标跟踪，确保在窗口边缘也能正确检测鼠标位置
    for (QWidget *child : findChildren<QWidget*>()) {
        child->setMouseTracking(true); // 递归启用子控件的鼠标跟踪，确保在任何区域都能检测到鼠标位置
        child->installEventFilter(this); // 安装事件过滤器，以便在子控件上也能检测鼠标事件，辅助实现窗口调整大小的功能
    }
}

void MainWindow::setupTopBar(QWidget *parent)
{
    m_topBar = new QWidget(parent);
    m_topBar->setObjectName(QStringLiteral("m_topBar"));
    m_topBar->setFixedHeight(m_topBarHeight);

    QHBoxLayout *m_topBarLayout = new QHBoxLayout(m_topBar);
    m_topBarLayout->setContentsMargins(12, 0, 8, 0);
    m_mainLayout->setSpacing(2);
    
    // 左侧：Logo占位
    QLabel *logoLabel = new QLabel(m_topBar);
    logoLabel->setObjectName(QStringLiteral("logoLabel"));
    logoLabel->setFixedSize(42, 42);
    QPixmap logoPixmap(":/icons/typemaster_logo.png");
    logoLabel->setPixmap(
        logoPixmap.scaled(42, 42, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setWindowIcon(QIcon(QStringLiteral(":/icons/typemaster_logo.png"))); // 设置应用程序图标
        
    // 中间：品牌信息（垂直布局，需要一个中间容器）
    QWidget *centerWiget = new QWidget(m_topBar);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWiget);
    centerLayout->setContentsMargins(0, 1, 0, 1);
    centerLayout->setSpacing(1);
    centerLayout->setAlignment(Qt::AlignCenter);

    // 从资源文件加载（例如将字体文件放到 .qrc 中）
    int fontId = QFontDatabase::addApplicationFont(":/fonts/zkceyyt.ttf");
    if (fontId != -1) 
        qDebug() << "Font loaded successfully with ID:" << fontId;

    QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont brandFont(family);

    QLabel *brandLabel = new QLabel(tr("KeyVerse"), centerWiget);
    QLabel *sloganLabel = new QLabel(tr("Tech Geeks Changing the World"), centerWiget);
    brandLabel->setObjectName(QStringLiteral("brandLabel"));
    brandLabel->setFont(brandFont);
    sloganLabel->setObjectName(QStringLiteral("sloganLabel"));
    brandLabel->setAlignment(Qt::AlignCenter);
    sloganLabel->setAlignment(Qt::AlignCenter);
    
    centerLayout->addWidget(brandLabel);
    centerLayout->addWidget(sloganLabel);
    
    QLabel *yearLabel = new QLabel(tr("2026"), m_topBar);
    yearLabel->setObjectName(QStringLiteral("yearLabel"));
    yearLabel->setAlignment(Qt::AlignCenter); 
    
    QPushButton *minBtn   = new QPushButton(m_topBar);
    minBtn->setObjectName(QStringLiteral("minBtn"));
    minBtn->setIcon(QIcon(QStringLiteral(":/icons/minimize.png")));
    minBtn->setIconSize(QSize(14, 14));

    m_maxBtn   = new QPushButton(m_topBar);
    m_maxBtn->setObjectName(QStringLiteral("maxBtn"));
    m_maxBtn->setIcon(QIcon(QStringLiteral(":/icons/maximize.png")));
    m_maxBtn->setIconSize(QSize(14, 14));

    QPushButton *closeBtn = new QPushButton(m_topBar);
    closeBtn->setObjectName(QStringLiteral("closeBtn"));
    closeBtn->setIcon(QIcon(QStringLiteral(":/icons/close.png")));
    closeBtn->setIconSize(QSize(14, 14));

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

// void MainWindow::mousePressEvent(QMouseEvent *event) 
// {
//     if (event->button() == Qt::LeftButton
//         && event->position().y() < m_topBarHeight) {  // 只在 m_topBar 区域内才能拖
//         m_dragging = true;
//         m_dragPos  = event->globalPosition().toPoint() - frameGeometry().topLeft();
//     }
//     QMainWindow::mousePressEvent(event);
// }

// void MainWindow::mouseMoveEvent(QMouseEvent *event) 
// {
//     if (m_dragging && (event->buttons() & Qt::LeftButton))
//         move(event->globalPosition().toPoint() - m_dragPos);
//     QMainWindow::mouseMoveEvent(event);
// }

// void MainWindow::mouseReleaseEvent(QMouseEvent *event) 
// {
//     m_dragging = false;
//     QMainWindow::mouseReleaseEvent(event);
// }


// ── 通用：把窗口 geometry 动画到目标区域 ──
void MainWindow::animateTo(const QRect &target, int durationMs)
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry", this);
    anim->setDuration(durationMs);
    anim->setStartValue(geometry());
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::OutCubic);  // 先快后慢，有弹性感
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ── 最大化 / 还原切换 ──
void MainWindow::animateToggleMaximize()
{
    if (!m_isMaximized) {
        // ── 放大 ──
        m_normalGeometry = geometry();  // 记住当前位置，还原时用

        // availableGeometry = 屏幕可用区域（排除任务栏），不会盖住任务栏
        QRect screen = QApplication::screenAt(geometry().center())
                           ->availableGeometry();

        animateTo(screen, 220);

        m_isMaximized = true;
        m_maxBtn->setIcon(QIcon(QStringLiteral(":/icons/restore.png")));

    } else {
        // ── 还原 ──
        animateTo(m_normalGeometry, 200);

        m_isMaximized = false;
        m_maxBtn->setIcon(QIcon(QStringLiteral(":/icons/maximize.png")));  // 切换回"最大化"图标
    }
}

// hitTest：判断鼠标在哪个区域（边缘/角落/内部），用于实现鼠标悬停时改变光标形状，以及在边缘拖动调整窗口大小
MainWindow::ResizeDir MainWindow::hitTest(const QPoint &pos) const
{
    const int x = pos.x();
    const int y = pos.y();
    const int w = width();
    const int h = height();
    const int m = kResizeMargin;

    int dir = None;
    if (x <= m)       dir |= Left;
    if (x >= w - m)   dir |= Right;
    if (y <= m)       dir |= Top;
    if (y >= h - m)   dir |= Bottom;

    return static_cast<ResizeDir>(dir);
}

// updateCursor：根据方向设置光标 形状，提供用户界面反馈，让用户知道可以拖动调整窗口大小
void MainWindow::updateCursor(ResizeDir dir)
{
    switch (dir) {
    case Left:
    case Right:
        setCursor(Qt::SizeHorCursor);   break;
    case Top:
    case Bottom:
        setCursor(Qt::SizeVerCursor);   break;
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor); break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;
    default:
        setCursor(Qt::ArrowCursor);     break;
    }
}

// 改造三个鼠标事件，兼容已有的拖动逻辑，同时在边缘区域实现调整窗口大小的功能
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_isMaximized) {
        // 最大化时只允许拖动 topBar（会触发还原）
        if (event->button() == Qt::LeftButton
            && event->position().y() < 45) {
            m_dragging = true;
            m_dragPos  = event->globalPosition().toPoint() - frameGeometry().topLeft();
        }
        return;
    }    

    if (event->button() != Qt::LeftButton) return;

    const QPoint localPos = event->position().toPoint();
    m_resizeDir = hitTest(localPos);

    if (m_resizeDir != None) {
        // 边缘：开始缩放
        m_resizeStartGlobal   = event->globalPosition().toPoint();
        m_resizeStartGeometry = geometry();
    } else if (localPos.y() < 45) {
        // topBar 区域：开始拖动窗口
        m_dragging = true;
        m_dragPos  = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint localPos  = event->position().toPoint();
    const QPoint globalPos = event->globalPosition().toPoint();

    if (!(event->buttons() & Qt::LeftButton)) {
        // 没按下按钮时只更新光标
        updateCursor(hitTest(localPos));
        return;
    }

    if (m_resizeDir != None) {
        // ── 缩放逻辑 ───────────────────────────────────────────────
        const QPoint delta = globalPos - m_resizeStartGlobal;
        QRect r = m_resizeStartGeometry;

        if (m_resizeDir & Left) {
            r.setLeft(r.left() + delta.x());
        }
        if (m_resizeDir & Right) {
            r.setRight(r.right() + delta.x());
        }
        if (m_resizeDir & Top) {
            r.setTop(r.top() + delta.y());
        }
        if (m_resizeDir & Bottom) {
            r.setBottom(r.bottom() + delta.y());
        }

        // 不小于最小尺寸
        if (r.width()  >= minimumWidth() &&
            r.height() >= minimumHeight()) {
            setGeometry(r);
        }

    } else if (m_dragging) {
        // ── 拖动逻辑（原有）───────────────────────────────────────
        move(globalPos - m_dragPos);
    }

    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging  = false;
    m_resizeDir = None;
    setCursor(Qt::ArrowCursor);
    QMainWindow::mouseReleaseEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    switch (event->type()) {

    case QEvent::MouseMove: {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        // 把子控件坐标转换成 MainWindow 的本地坐标
        QPoint localPos = mapFromGlobal(me->globalPosition().toPoint());

        if (!(me->buttons() & Qt::LeftButton)) {
            // 没按键时只更新光标
            updateCursor(hitTest(localPos));
        }
        break;
    }

    case QEvent::Leave:
        // 鼠标离开窗口时恢复默认光标
        if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
            setCursor(Qt::ArrowCursor);
        }
        break;

    default:
        break;
    }

    return QMainWindow::eventFilter(watched, event);
}

