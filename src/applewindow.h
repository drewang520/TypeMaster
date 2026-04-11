#ifndef APPLEWINDOW_H
#define APPLEWINDOW_H       

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>

class GameData;
class GameView;
class GameController;

/**
 * AppleWindow — 轻量级外壳窗口（第4周 MVC 重构）。
 *
 * 职责（且仅限于此）：
 *   1. 创建并连接 MVC 三元组：GameData / GameView / GameController
 *   2. 管理窗口界面元素：标题、返回按钮、暂停/开始按钮
 *   3. 管理背景音乐并处理音频事件
 *   4. 转发窗口调整大小事件，确保 GameView 始终填满内容区域
 *
 * 所有游戏逻辑均位于 GameController 中。
 * 所有渲染逻辑均位于 GameView 中。
 * 所有状态数据均位于 GameData 中。
 */

class AppleWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AppleWindow(QWidget* parent = nullptr);
    ~AppleWindow() override = default;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    // ── Setup helpers ──
    void setupMvc();
    void setupUi();
    void setupMusic();
    void setCatchSound();
    void connectSignals();

    /** 根据当前状态刷新“暂停/开始”按钮的文本和可见性。 */
    void updateButtons();

    // ── MVC components ──
    GameData*       m_gameData      {nullptr};
    GameView*       m_gameView      {nullptr};
    GameController* m_controller    {nullptr};

    // ── Window chrome ──
    QPushButton* m_backBtn  {nullptr};
    QPushButton* m_pauseBtn {nullptr};
    QPushButton* m_startBtn {nullptr};

    // ── Audio ──
    QMediaPlayer* m_musicPlayer {nullptr};
    QSoundEffect* m_catchSound  {nullptr};
};

#endif // APPLEWINDOW_H