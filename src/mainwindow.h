#ifndef MAINWINDOW_H
#define MAINWINDOW_H    

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPropertyAnimation>   
#include <QGraphicsOpacityEffect> 
#include <QMouseEvent>

class GameCard;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override; // 事件过滤器，用于捕获顶栏按钮的点击事件

private:
    void setupUi();
    void setupTopBar(QWidget *parent);
    void setupStyleSheet();
    void connectSignals();

    void animateToggleMaximize(); // 切换最大化/恢复时的动画
    void animateTo(const QRect &targetGeometry, int duration = 220); // 220ms 是一个常见的动画时长，既能让动画流畅又不会感觉拖沓
    
    QWidget      *m_topBar{nullptr};
    static constexpr int      m_topBarHeight{45}; // 顶栏高度，便于在鼠标事件中判断是否在顶栏区域内
    QPoint m_dragPos; // 追踪鼠标拖动时的偏移位置
    bool   m_dragging{false}; // 追踪是否正在拖动窗口
    
    QWidget      *m_centralWidget{nullptr};
    QVBoxLayout  *m_mainLayout{nullptr};
    QLabel       *m_titleLabel{nullptr};
    QLabel       *m_subtitleLabel{nullptr};
    QWidget      *m_cardsContainer{nullptr};
    QHBoxLayout  *m_cardsLayout{nullptr};

    QPushButton *m_maxBtn{nullptr}; // 直接访问最大化按钮，便于更新其状态
    bool m_isMaximized{false}; // 追踪当前窗口状态，便于切换最大化/恢复
    QRect m_normalGeometry; // 存储窗口未最大化时的几何信息，以便恢复使用

    // 拖拽方向枚举
    enum ResizeDir {
        None        = 0,
        Left        = 1,
        Right       = 2,
        Top         = 4,
        Bottom      = 8,
        TopLeft     = Top    | Left,
        TopRight    = Top    | Right,
        BottomLeft  = Bottom | Left,
        BottomRight = Bottom | Right,
    };

    ResizeDir hitTest(const QPoint &pos) const;  // 检测鼠标在哪个边/角
    void      updateCursor(ResizeDir dir);        // 更新光标形状

    static constexpr int kResizeMargin = 6;      // 边缘检测宽度（像素）

    // 拖拽缩放状态
    ResizeDir m_resizeDir{None};
    QPoint    m_resizeStartGlobal;   // 按下时的全局坐标
    QRect     m_resizeStartGeometry; // 按下时的窗口几何

    GameCard *m_appleCard{nullptr};
    GameCard *m_spaceCard{nullptr};
    GameCard *m_practiceCard{nullptr};

private slots:
    void onAppleGameRequested();
    void onSpaceGameRequested();
    void onPracticeGameRequested();
};

#endif // MAINWINDOW_H