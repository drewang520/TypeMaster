#ifndef MAINWINDOW_H
#define MAINWINDOW_H    

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPropertyAnimation>   // ← 新增
#include <QGraphicsOpacityEffect> // ← 新增

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


private:
    void setupUi();
    void setupTopBar(QWidget *parent);
    void setupStyleSheet();
    void connectSignals();

    void animateMinimize();
    void animateToggleMaximize(); // 切换最大化/恢复时的动画
    void animateTo(const QRect &targetGeometry, int duration = 220); // 220ms 是一个常见的动画时长，既能让动画流畅又不会感觉拖沓
    
    QWidget      *m_topBar{nullptr};
    QWidget      *m_centralWidget{nullptr};
    QMenuBar      *m_menuBar{nullptr};
    QVBoxLayout  *m_mainLayout{nullptr};
    QLabel       *m_titleLabel{nullptr};
    QLabel       *m_subtitleLabel{nullptr};
    QWidget      *m_cardsContainer{nullptr};
    QHBoxLayout  *m_cardsLayout{nullptr};

    QPoint m_dragPos;
    bool   m_dragging{false};

    QPropertyAnimation    *m_minimizeAnim{nullptr};  // ← 新增
    bool m_isMaximized{false}; // 追踪当前窗口状态，便于切换最大化/恢复
    QRect m_normalGeometry; // 存储窗口未最大化时的几何信息，以便恢复使用
    QPushButton *m_maxBtn{nullptr}; // 直接访问最大化按钮，便于更新其状态

    GameCard *m_appleCard{nullptr};
    GameCard *m_spaceCard{nullptr};
    GameCard *m_practiceCard{nullptr};

private slots:
    void onAppleGameRequested();
    void onSpaceGameRequested();
    void onPracticeGameRequested();
};

#endif // MAINWINDOW_H