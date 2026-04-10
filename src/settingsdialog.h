#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H    

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override = default;

signals:
    void languageChanged(const QString &locale);

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void setupNavigation();
    void setupPageLanguage();
    void setupPageAppearance();
    void setupPageSound();
    void setupPageGame();
    void setupPageNetwork();
    void setupPageAbout();
    void setupStyleSheet();
    void retranslateUi();

    // ── 帮助函数：创建统一风格的分组块 ──────────────────────────
    QWidget* makeSection(const QString &title, QLayout *contentLayout);
    QFrame*  makeDivider();

    // ── 导航 ──────────────────────────────────────────────────────
    QListWidget    *m_navList{nullptr};
    QStackedWidget *m_stack{nullptr};

    // ── 底部按钮 ──────────────────────────────────────────────────
    QPushButton *m_okBtn{nullptr};
    QPushButton *m_cancelBtn{nullptr};
    QPushButton *m_applyBtn{nullptr};

    // ── 语言页控件 ────────────────────────────────────────────────
    QRadioButton *m_radioZhCN{nullptr};
    QRadioButton *m_radioEn{nullptr};
    QLabel       *m_langPreviewLabel{nullptr};

    // ── 外观页控件 ────────────────────────────────────────────────
    QButtonGroup *m_themeGroup{nullptr};
    QSlider      *m_fontSizeSlider{nullptr};
    QLabel       *m_fontSizeValueLabel{nullptr};
    QCheckBox    *m_animCheckBox{nullptr};
    QCheckBox    *m_shadowCheckBox{nullptr};

    // ── 音效页控件 ────────────────────────────────────────────────
    QSlider   *m_bgmSlider{nullptr};
    QSlider   *m_sfxSlider{nullptr};
    QCheckBox *m_bgmMuteBox{nullptr};
    QCheckBox *m_sfxMuteBox{nullptr};

    // ── 游戏页控件 ────────────────────────────────────────────────
    QCheckBox *m_showHintBox{nullptr};
    QCheckBox *m_capsLockBox{nullptr};
    QSpinBox  *m_livesSpinBox{nullptr};
    QComboBox *m_difficultyBox{nullptr};

    // ── 网络页控件 ────────────────────────────────────────────────
    QCheckBox  *m_proxyCheckBox{nullptr};
    QLineEdit  *m_proxyHostEdit{nullptr};
    QSpinBox   *m_proxyPortSpin{nullptr};
    QCheckBox  *m_autoUpdateBox{nullptr};

private slots:
    void onNavItemChanged(int row);
    void onLanguageSelected();
    void onApply();
    void onFontSizeChanged(int value);
};

#endif // SETTINGSDIALOG_H