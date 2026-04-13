#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

/**
 * SettingsDialog — 模态设置面板。
 *
 * 控件：
 *   速度等级  (1-10)   — 苹果下落的速度
 *   最大苹果数   (1-5)    — 屏幕上同时出现的苹果数量
 *   关卡目标 (5-30)   — 完成关卡所需的接住次数
 *   声音        开/关   — 全局音频开关
 *
 * 打开时从 GameConfig 读取初始值；
 * 滑块移动时立即将值写回（实时预览）。
 * 点击“取消”将恢复原始值。
 */

class AppleSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppleSettingsDialog(QWidget* parent = nullptr);
    ~AppleSettingsDialog() override = default;

private slots:
    void onSpeedChanged(int value);
    void onMaxFruitsChanged(int value);
    void onTargetChanged(int value);
    void onSoundToggled();
    void onCancel();

private:
    void buildUi();
    QPushButton* makeImageButton(const QString& path, const QString& fallbackText,
                                 int w, int h, QWidget* parent);
    void updateSoundButton();

    // Sliders
    QSlider* m_speedSlider  {nullptr};
    QSlider* m_fruitsSlider {nullptr};
    QSlider* m_targetSlider {nullptr};

    // Value labels next to each slider
    QLabel* m_speedVal  {nullptr};
    QLabel* m_fruitsVal {nullptr};
    QLabel* m_targetVal {nullptr};

    // Sound toggle button
    QPushButton* m_soundBtn {nullptr};

    // Saved originals for Cancel
    int  m_origSpeed  {5};
    int  m_origFruits {5};
    int  m_origTarget {10};
    bool m_origSound  {true};
};

#endif // SETTINGS_DIALOG_H