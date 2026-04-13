#ifndef APPLEWINDOW_H
#define APPLEWINDOW_H       

#include "IObserver.h"
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

class AppleWindow : public QWidget, public IObserver
{
    Q_OBJECT

public:
    explicit AppleWindow(QWidget* parent = nullptr);
    ~AppleWindow() override;

    void onUpdate() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    // ── Setup helpers ──
    void setupMvc();
    void setupUi();
    void setupGameButtons();
    void setupMusic();
    void setCatchSound();
    void connectSignals();
    void repositionStartBtn();  // 根据当前状态正确定位 startBtn
    void repositionGameButtons();

    // 辅助函数：将3帧的PNG图片条生成一个图片按钮
    QPushButton* makeImageButton(const QString& resourcePath,
                                 const QString& fallbackText,
                                 QWidget* parent);

    // ── MVC components ──
    GameData*       m_gameData      {nullptr};
    GameView*       m_gameView      {nullptr};
    GameController* m_controller    {nullptr};

    // ── HUD 栏控件（布局与原始 AppleWindow 相同） 
    QLabel*      m_scoreLabel {nullptr};
    QLabel*      m_livesLabel {nullptr};
    QLabel*      m_levelLabel {nullptr};
    QPushButton* m_backBtn  {nullptr};
    QPushButton* m_pauseBtn {nullptr};  // // in HUD bar (text button)

    // ── Start/PlayAgain overlay button ──
    QPushButton* m_startBtn {nullptr};

    // ── Four image buttons (bottom-left overlay) ──
    QPushButton* m_imgStartBtn    {nullptr};
    QPushButton* m_imgPauseBtn    {nullptr};
    QPushButton* m_imgSettingsBtn {nullptr};
    QPushButton* m_imgExitBtn     {nullptr};    
    
    // ── Audio ──
    QMediaPlayer* m_musicPlayer {nullptr};
    QSoundEffect* m_catchSound  {nullptr};
};

#endif // APPLEWINDOW_H