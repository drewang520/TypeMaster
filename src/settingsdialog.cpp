#include "settingsdialog.h"

#include <QEvent>
#include <QScrollArea>
#include <QApplication>
#include <QTranslator>
#include <QButtonGroup>

// Constructor
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("settingsDialog"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(780, 540);

    setupUi();
    setupStyleSheet();
    retranslateUi();
}

// UI Construction

void SettingsDialog::setupUi()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── 标题栏 ──
    QWidget *titleBar = new QWidget(this);
    titleBar->setObjectName(QStringLiteral("settingsTitleBar"));
    titleBar->setFixedHeight(44);
    titleBar->setAttribute(Qt::WA_StyledBackground, true);

    QHBoxLayout *tbLayout = new QHBoxLayout(titleBar);
    tbLayout->setContentsMargins(16, 0, 8, 0);

    QLabel *titleIcon = new QLabel(QStringLiteral("⚙"), titleBar);
    titleIcon->setObjectName(QStringLiteral("settingsTitleIcon"));

    QLabel *titleText = new QLabel(tr("Settings"), titleBar);
    titleText->setObjectName(QStringLiteral("settingsTitleText"));

    QPushButton *closeBtn = new QPushButton(QStringLiteral("✕"), titleBar);
    closeBtn->setObjectName(QStringLiteral("settingsCloseBtn"));
    closeBtn->setFixedSize(32, 32);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    tbLayout->addWidget(titleIcon);
    tbLayout->addSpacing(8);
    tbLayout->addWidget(titleText);
    tbLayout->addStretch();
    tbLayout->addWidget(closeBtn);

    // ── 主体区域（导航 + 内容）────────────────────────────────────────────
    QWidget *body = new QWidget(this);
    body->setObjectName(QStringLiteral("settingsBody"));
    body->setAttribute(Qt::WA_StyledBackground, true);

    QHBoxLayout *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    // ── 左侧导航栏 ────────────────────────────────────────────────────────
    m_navList = new QListWidget(body);
    m_navList->setObjectName(QStringLiteral("settingsNav"));
    m_navList->setFixedWidth(150);
    m_navList->setFrameShape(QFrame::NoFrame);
    m_navList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 导航项（图标 + 文字）
    struct NavItem { QString icon; QString key; };
    const QList<NavItem> navItems = {
        { QStringLiteral("🌐"), QStringLiteral("language")   },
        { QStringLiteral("🎨"), QStringLiteral("appearance") },
        { QStringLiteral("🔊"), QStringLiteral("sound")      },
        { QStringLiteral("🎮"), QStringLiteral("game")       },
        { QStringLiteral("🌍"), QStringLiteral("network")    },
        { QStringLiteral("ℹ"),  QStringLiteral("about")      },
    };

    for (const auto &item : navItems) {
        QListWidgetItem *li = new QListWidgetItem(
            item.icon + QStringLiteral("  ") + tr(item.key.toUtf8()));
        li->setData(Qt::UserRole, item.key);
        li->setSizeHint(QSize(150, 44));
        m_navList->addItem(li);
    }
    m_navList->setCurrentRow(0);

    // ── 右侧内容区 ────────────────────────────────────────────────────────
    m_stack = new QStackedWidget(body);
    m_stack->setObjectName(QStringLiteral("settingsStack"));

    setupPageLanguage();
    setupPageAppearance();
    setupPageSound();
    setupPageGame();
    setupPageNetwork();
    setupPageAbout();

    bodyLayout->addWidget(m_navList);
    bodyLayout->addWidget(m_stack, 1);

    // ── 底部按钮栏 ────────────────────────────────────────────────────────
    QWidget *footer = new QWidget(this);
    footer->setObjectName(QStringLiteral("settingsFooter"));
    footer->setFixedHeight(52);
    footer->setAttribute(Qt::WA_StyledBackground, true);

    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(16, 0, 16, 0);
    footerLayout->setSpacing(8);

    m_applyBtn  = new QPushButton(tr("Apply"),  footer);
    m_okBtn     = new QPushButton(tr("OK"),     footer);
    m_cancelBtn = new QPushButton(tr("Cancel"), footer);

    m_applyBtn->setObjectName(QStringLiteral("settingsApplyBtn"));
    m_okBtn->setObjectName(QStringLiteral("settingsOkBtn"));
    m_cancelBtn->setObjectName(QStringLiteral("settingsCancelBtn"));

    m_applyBtn->setFixedSize(80, 32);
    m_okBtn->setFixedSize(80, 32);
    m_cancelBtn->setFixedSize(80, 32);

    m_applyBtn->setFocusPolicy(Qt::NoFocus);
    m_okBtn->setFocusPolicy(Qt::NoFocus);
    m_cancelBtn->setFocusPolicy(Qt::NoFocus);

    footerLayout->addStretch();
    footerLayout->addWidget(m_applyBtn);
    footerLayout->addWidget(m_cancelBtn);
    footerLayout->addWidget(m_okBtn);

    connect(m_okBtn,     &QPushButton::clicked, this, [this](){ onApply(); accept(); });
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_applyBtn,  &QPushButton::clicked, this, &SettingsDialog::onApply);

    rootLayout->addWidget(titleBar);
    rootLayout->addWidget(body, 1);
    rootLayout->addWidget(footer);

    connect(m_navList, &QListWidget::currentRowChanged,
            this, &SettingsDialog::onNavItemChanged);
}

// ─────────────────────────────────────────────────────────────────────────────
// 辅助：创建分组 Section
// ─────────────────────────────────────────────────────────────────────────────

QWidget* SettingsDialog::makeSection(const QString &title, QLayout *contentLayout)
{
    QWidget *section = new QWidget;
    section->setAttribute(Qt::WA_StyledBackground, true);
    section->setObjectName(QStringLiteral("settingsSection"));

    QVBoxLayout *vl = new QVBoxLayout(section);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(10);

    QLabel *titleLabel = new QLabel(title, section);
    titleLabel->setObjectName(QStringLiteral("sectionTitle"));

    QFrame *divider = new QFrame(section);
    divider->setFrameShape(QFrame::HLine);
    divider->setObjectName(QStringLiteral("sectionDivider"));

    QWidget *content = new QWidget(section);
    content->setLayout(contentLayout);

    vl->addWidget(titleLabel);
    vl->addWidget(divider);
    vl->addWidget(content);

    return section;
}

// ─────────────────────────────────────────────────────────────────────────────
// Page 1: Language
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupPageLanguage()
{
    QWidget *page = new QWidget;
    page->setObjectName(QStringLiteral("settingsPage"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 20, 24, 20);
    pageLayout->setSpacing(20);

    // ── Section：界面语言 ─────────────────────────────────────────────────
    QVBoxLayout *langLayout = new QVBoxLayout;
    langLayout->setSpacing(8);

    QButtonGroup *langGroup = new QButtonGroup(page);

    // 中文选项
    QWidget *zhRow = new QWidget;
    QHBoxLayout *zhRowLayout = new QHBoxLayout(zhRow);
    zhRowLayout->setContentsMargins(0,0,0,0);
    m_radioZhCN = new QRadioButton(tr("简体中文"), zhRow);
    m_radioZhCN->setObjectName(QStringLiteral("langRadio"));
    m_radioZhCN->setChecked(true);
    QLabel *zhDesc = new QLabel(tr("Simplified Chinese"), zhRow);
    zhDesc->setObjectName(QStringLiteral("langDesc"));
    zhRowLayout->addWidget(m_radioZhCN);
    zhRowLayout->addWidget(zhDesc);
    zhRowLayout->addStretch();

    // 英文选项
    QWidget *enRow = new QWidget;
    QHBoxLayout *enRowLayout = new QHBoxLayout(enRow);
    enRowLayout->setContentsMargins(0,0,0,0);
    m_radioEn = new QRadioButton(tr("English"), enRow);
    m_radioEn->setObjectName(QStringLiteral("langRadio"));
    QLabel *enDesc = new QLabel(tr("English (United States)"), enRow);
    enDesc->setObjectName(QStringLiteral("langDesc"));
    enRowLayout->addWidget(m_radioEn);
    enRowLayout->addWidget(enDesc);
    enRowLayout->addStretch();

    langGroup->addButton(m_radioZhCN);
    langGroup->addButton(m_radioEn);

    langLayout->addWidget(zhRow);
    langLayout->addWidget(enRow);

    pageLayout->addWidget(makeSection(tr("Interface Language"), langLayout));

    // ── Section：语言预览 ─────────────────────────────────────────────────
    QVBoxLayout *previewLayout = new QVBoxLayout;
    previewLayout->setSpacing(6);

    m_langPreviewLabel = new QLabel(this);
    m_langPreviewLabel->setObjectName(QStringLiteral("langPreview"));
    m_langPreviewLabel->setWordWrap(true);
    m_langPreviewLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_langPreviewLabel->setMinimumHeight(60);

    previewLayout->addWidget(m_langPreviewLabel);
    pageLayout->addWidget(makeSection(tr("Preview"), previewLayout));

    // ── Section：重启提示 ─────────────────────────────────────────────────
    QHBoxLayout *noteLayout = new QHBoxLayout;
    QLabel *noteIcon = new QLabel(QStringLiteral("💡"));
    QLabel *noteText = new QLabel(
        tr("Language changes take effect immediately without restart."));
    noteText->setObjectName(QStringLiteral("noteText"));
    noteText->setWordWrap(true);
    noteLayout->addWidget(noteIcon);
    noteLayout->addWidget(noteText, 1);
    pageLayout->addLayout(noteLayout);

    pageLayout->addStretch();

    // 信号
    connect(m_radioZhCN, &QRadioButton::toggled, this, &SettingsDialog::onLanguageSelected);
    connect(m_radioEn,   &QRadioButton::toggled, this, &SettingsDialog::onLanguageSelected);

    // 初始化预览
    m_langPreviewLabel->setText(
        tr("Start Game") + QStringLiteral(" / ") +
        tr("Settings")   + QStringLiteral(" / ") +
        tr("Practice Mode"));

    m_stack->addWidget(page);
}

// ─────────────────────────────────────────────────────────────────────────────
// Page 2: Appearance
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupPageAppearance()
{
    QWidget *page = new QWidget;
    page->setObjectName(QStringLiteral("settingsPage"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 20, 24, 20);
    pageLayout->setSpacing(20);

    // ── 主题选择 ──────────────────────────────────────────────────────────
    QHBoxLayout *themeLayout = new QHBoxLayout;
    themeLayout->setSpacing(12);
    m_themeGroup = new QButtonGroup(page);

    auto makeThemeBtn = [&](const QString &label, const QString &color, int id) {
        QPushButton *btn = new QPushButton(label, page);
        btn->setObjectName(QStringLiteral("themeBtn"));
        btn->setFixedSize(100, 64);
        btn->setCheckable(true);
        btn->setStyleSheet(
            QStringLiteral("QPushButton { background: %1; border-radius: 8px;"
                            " border: 2px solid transparent; color: white; font-weight: bold; }"
                            "QPushButton:checked { border: 2px solid #1565C0; }").arg(color));
        m_themeGroup->addButton(btn, id);
        themeLayout->addWidget(btn);
        return btn;
    };

    makeThemeBtn(tr("Sky Blue"),  QStringLiteral("#5B9BD5"), 0)->setChecked(true);
    makeThemeBtn(tr("Dark"),      QStringLiteral("#2C2C3A"), 1);
    makeThemeBtn(tr("Forest"),    QStringLiteral("#4A7C59"), 2);
    makeThemeBtn(tr("Sunset"),    QStringLiteral("#C0614D"), 3);
    themeLayout->addStretch();

    pageLayout->addWidget(makeSection(tr("Theme"), themeLayout));

    // ── 字体大小 ──────────────────────────────────────────────────────────
    QHBoxLayout *fontLayout = new QHBoxLayout;
    QLabel *fontSmall = new QLabel(tr("A"), page);
    fontSmall->setObjectName(QStringLiteral("fontSmallLabel"));

    m_fontSizeSlider = new QSlider(Qt::Horizontal, page);
    m_fontSizeSlider->setRange(10, 18);
    m_fontSizeSlider->setValue(13);
    m_fontSizeSlider->setTickInterval(2);
    m_fontSizeSlider->setObjectName(QStringLiteral("settingsSlider"));

    QLabel *fontLarge = new QLabel(tr("A"), page);
    fontLarge->setObjectName(QStringLiteral("fontLargeLabel"));

    m_fontSizeValueLabel = new QLabel(QStringLiteral("13px"), page);
    m_fontSizeValueLabel->setObjectName(QStringLiteral("sliderValueLabel"));
    m_fontSizeValueLabel->setFixedWidth(36);
    m_fontSizeValueLabel->setAlignment(Qt::AlignCenter);

    fontLayout->addWidget(fontSmall);
    fontLayout->addWidget(m_fontSizeSlider, 1);
    fontLayout->addWidget(fontLarge);
    fontLayout->addSpacing(8);
    fontLayout->addWidget(m_fontSizeValueLabel);

    connect(m_fontSizeSlider, &QSlider::valueChanged,
            this, &SettingsDialog::onFontSizeChanged);

    pageLayout->addWidget(makeSection(tr("Font Size"), fontLayout));

    // ── 效果开关 ──────────────────────────────────────────────────────────
    QVBoxLayout *fxLayout = new QVBoxLayout;
    m_animCheckBox   = new QCheckBox(tr("Enable window animations"), page);
    m_shadowCheckBox = new QCheckBox(tr("Show card drop shadow"),    page);
    m_animCheckBox->setObjectName(QStringLiteral("settingsCheckBox"));
    m_shadowCheckBox->setObjectName(QStringLiteral("settingsCheckBox"));
    m_animCheckBox->setChecked(true);
    m_shadowCheckBox->setChecked(true);
    fxLayout->addWidget(m_animCheckBox);
    fxLayout->addWidget(m_shadowCheckBox);

    pageLayout->addWidget(makeSection(tr("Visual Effects"), fxLayout));
    pageLayout->addStretch();

    m_stack->addWidget(page);
}

// ─────────────────────────────────────────────────────────────────────────────
// Page 3: Sound
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupPageSound()
{
    QWidget *page = new QWidget;
    page->setObjectName(QStringLiteral("settingsPage"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 20, 24, 20);
    pageLayout->setSpacing(20);

    auto makeVolumeRow = [&](const QString &label, QSlider *&slider,
                              QCheckBox *&muteBox, const QString &icon) {
        QGridLayout *grid = new QGridLayout;
        grid->setSpacing(10);

        QLabel *iconLabel = new QLabel(icon, page);
        iconLabel->setFixedWidth(24);
        QLabel *nameLabel = new QLabel(label, page);
        nameLabel->setObjectName(QStringLiteral("volumeLabel"));

        slider = new QSlider(Qt::Horizontal, page);
        slider->setRange(0, 100);
        slider->setValue(70);
        slider->setObjectName(QStringLiteral("settingsSlider"));

        QLabel *valLabel = new QLabel(QStringLiteral("70"), page);
        valLabel->setObjectName(QStringLiteral("sliderValueLabel"));
        valLabel->setFixedWidth(30);
        valLabel->setAlignment(Qt::AlignCenter);

        muteBox = new QCheckBox(tr("Mute"), page);
        muteBox->setObjectName(QStringLiteral("settingsCheckBox"));

        QSlider *s = slider;
        connect(s, &QSlider::valueChanged, valLabel, [valLabel](int v) {
            valLabel->setText(QString::number(v));
        });
        connect(muteBox, &QCheckBox::toggled, s, [s](bool muted) {
            s->setEnabled(!muted);
        });

        grid->addWidget(iconLabel,  0, 0);
        grid->addWidget(nameLabel,  0, 1);
        grid->addWidget(slider,     0, 2);
        grid->addWidget(valLabel,   0, 3);
        grid->addWidget(muteBox,    0, 4);
        grid->setColumnStretch(2, 1);

        return grid;
    };

    QVBoxLayout *soundLayout = new QVBoxLayout;
    soundLayout->setSpacing(14);
    soundLayout->addLayout(makeVolumeRow(tr("Background Music"),
                                          m_bgmSlider, m_bgmMuteBox,
                                          QStringLiteral("🎵")));
    soundLayout->addLayout(makeVolumeRow(tr("Sound Effects"),
                                          m_sfxSlider, m_sfxMuteBox,
                                          QStringLiteral("🔔")));

    pageLayout->addWidget(makeSection(tr("Volume"), soundLayout));
    pageLayout->addStretch();

    m_stack->addWidget(page);
}

// ─────────────────────────────────────────────────────────────────────────────
// Page 4: Game
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupPageGame()
{
    QWidget *page = new QWidget;
    page->setObjectName(QStringLiteral("settingsPage"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 20, 24, 20);
    pageLayout->setSpacing(20);

    // ── 游戏选项 ──────────────────────────────────────────────────────────
    QVBoxLayout *optLayout = new QVBoxLayout;
    optLayout->setSpacing(10);

    m_showHintBox = new QCheckBox(tr("Show letter hints during game"), page);
    m_capsLockBox = new QCheckBox(tr("Accept both uppercase and lowercase input"), page);
    m_showHintBox->setObjectName(QStringLiteral("settingsCheckBox"));
    m_capsLockBox->setObjectName(QStringLiteral("settingsCheckBox"));
    m_showHintBox->setChecked(true);
    m_capsLockBox->setChecked(true);
    optLayout->addWidget(m_showHintBox);
    optLayout->addWidget(m_capsLockBox);

    pageLayout->addWidget(makeSection(tr("Gameplay"), optLayout));

    // ── 难度设置 ──────────────────────────────────────────────────────────
    QGridLayout *diffLayout = new QGridLayout;
    diffLayout->setSpacing(12);
    diffLayout->setColumnStretch(1, 1);

    QLabel *livesLabel = new QLabel(tr("Starting lives:"), page);
    livesLabel->setObjectName(QStringLiteral("settingsLabel"));
    m_livesSpinBox = new QSpinBox(page);
    m_livesSpinBox->setRange(1, 10);
    m_livesSpinBox->setValue(5);
    m_livesSpinBox->setObjectName(QStringLiteral("settingsSpinBox"));
    m_livesSpinBox->setFixedWidth(80);

    QLabel *diffLabel = new QLabel(tr("Difficulty:"), page);
    diffLabel->setObjectName(QStringLiteral("settingsLabel"));
    m_difficultyBox = new QComboBox(page);
    m_difficultyBox->addItems({tr("Easy"), tr("Normal"), tr("Hard"), tr("Expert")});
    m_difficultyBox->setCurrentIndex(1);
    m_difficultyBox->setObjectName(QStringLiteral("settingsComboBox"));
    m_difficultyBox->setFixedWidth(120);

    diffLayout->addWidget(livesLabel,     0, 0);
    diffLayout->addWidget(m_livesSpinBox, 0, 1, Qt::AlignLeft);
    diffLayout->addWidget(diffLabel,      1, 0);
    diffLayout->addWidget(m_difficultyBox,1, 1, Qt::AlignLeft);

    pageLayout->addWidget(makeSection(tr("Difficulty"), diffLayout));
    pageLayout->addStretch();

    m_stack->addWidget(page);
}

// ─────────────────────────────────────────────────────────────────────────────
// Page 5: Network
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupPageNetwork()
{
    QWidget *page = new QWidget;
    page->setObjectName(QStringLiteral("settingsPage"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 20, 24, 20);
    pageLayout->setSpacing(20);

    // ── 代理设置 ──────────────────────────────────────────────────────────
    QVBoxLayout *proxyLayout = new QVBoxLayout;
    proxyLayout->setSpacing(10);

    m_proxyCheckBox = new QCheckBox(tr("Use HTTP proxy"), page);
    m_proxyCheckBox->setObjectName(QStringLiteral("settingsCheckBox"));

    QWidget *proxyFields = new QWidget(page);
    QGridLayout *proxyGrid = new QGridLayout(proxyFields);
    proxyGrid->setContentsMargins(20, 0, 0, 0);
    proxyGrid->setSpacing(10);
    proxyGrid->setColumnStretch(1, 1);

    QLabel *hostLabel = new QLabel(tr("Host:"), page);
    hostLabel->setObjectName(QStringLiteral("settingsLabel"));
    m_proxyHostEdit = new QLineEdit(page);
    m_proxyHostEdit->setPlaceholderText(QStringLiteral("127.0.0.1"));
    m_proxyHostEdit->setObjectName(QStringLiteral("settingsLineEdit"));
    m_proxyHostEdit->setEnabled(false);

    QLabel *portLabel = new QLabel(tr("Port:"), page);
    portLabel->setObjectName(QStringLiteral("settingsLabel"));
    m_proxyPortSpin = new QSpinBox(page);
    m_proxyPortSpin->setRange(1, 65535);
    m_proxyPortSpin->setValue(7890);
    m_proxyPortSpin->setObjectName(QStringLiteral("settingsSpinBox"));
    m_proxyPortSpin->setFixedWidth(100);
    m_proxyPortSpin->setEnabled(false);

    proxyGrid->addWidget(hostLabel,       0, 0);
    proxyGrid->addWidget(m_proxyHostEdit, 0, 1);
    proxyGrid->addWidget(portLabel,       1, 0);
    proxyGrid->addWidget(m_proxyPortSpin, 1, 1, Qt::AlignLeft);

    proxyLayout->addWidget(m_proxyCheckBox);
    proxyLayout->addWidget(proxyFields);

    connect(m_proxyCheckBox, &QCheckBox::toggled, this, [this](bool on) {
        m_proxyHostEdit->setEnabled(on);
        m_proxyPortSpin->setEnabled(on);
    });

    pageLayout->addWidget(makeSection(tr("Proxy"), proxyLayout));

    // ── 更新设置 ──────────────────────────────────────────────────────────
    QVBoxLayout *updateLayout = new QVBoxLayout;
    m_autoUpdateBox = new QCheckBox(tr("Check for updates automatically"), page);
    m_autoUpdateBox->setObjectName(QStringLiteral("settingsCheckBox"));
    m_autoUpdateBox->setChecked(true);

    QPushButton *checkNowBtn = new QPushButton(tr("Check Now"), page);
    checkNowBtn->setObjectName(QStringLiteral("settingsApplyBtn"));
    checkNowBtn->setFixedWidth(110);
    checkNowBtn->setFocusPolicy(Qt::NoFocus);

    updateLayout->addWidget(m_autoUpdateBox);
    updateLayout->addWidget(checkNowBtn, 0, Qt::AlignLeft);

    pageLayout->addWidget(makeSection(tr("Updates"), updateLayout));
    pageLayout->addStretch();

    m_stack->addWidget(page);
}

// ─────────────────────────────────────────────────────────────────────────────
// Page 6: About
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupPageAbout()
{
    QWidget *page = new QWidget;
    page->setObjectName(QStringLiteral("settingsPage"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 20, 24, 20);
    pageLayout->setSpacing(16);
    pageLayout->setAlignment(Qt::AlignTop);

    QLabel *appIcon = new QLabel(QStringLiteral("⌨"), page);
    appIcon->setAlignment(Qt::AlignCenter);
    appIcon->setStyleSheet(QStringLiteral("font-size: 48px;"));

    QLabel *appName = new QLabel(QStringLiteral("KeyVerse"), page);
    appName->setObjectName(QStringLiteral("aboutAppName"));
    appName->setAlignment(Qt::AlignCenter);

    QLabel *appVersion = new QLabel(tr("Version 1.0.0"), page);
    appVersion->setObjectName(QStringLiteral("aboutVersion"));
    appVersion->setAlignment(Qt::AlignCenter);

    QLabel *appDesc = new QLabel(
        tr("A modern typing game built with Qt 6.\n"
           "Practice your typing speed and have fun!"), page);
    appDesc->setObjectName(QStringLiteral("aboutDesc"));
    appDesc->setAlignment(Qt::AlignCenter);
    appDesc->setWordWrap(true);

    QFrame *div = new QFrame(page);
    div->setFrameShape(QFrame::HLine);
    div->setObjectName(QStringLiteral("sectionDivider"));

    QLabel *qtInfo = new QLabel(
        QStringLiteral("Built with Qt ") + QT_VERSION_STR, page);
    qtInfo->setObjectName(QStringLiteral("aboutVersion"));
    qtInfo->setAlignment(Qt::AlignCenter);

    pageLayout->addStretch();
    pageLayout->addWidget(appIcon);
    pageLayout->addWidget(appName);
    pageLayout->addWidget(appVersion);
    pageLayout->addSpacing(8);
    pageLayout->addWidget(appDesc);
    pageLayout->addWidget(div);
    pageLayout->addWidget(qtInfo);
    pageLayout->addStretch();

    m_stack->addWidget(page);
}

// ─────────────────────────────────────────────────────────────────────────────
// Navigation
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupNavigation(){}

void SettingsDialog::onNavItemChanged(int row)
{
    m_stack->setCurrentIndex(row);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::onLanguageSelected()
{
    if (m_radioZhCN->isChecked()) {
        m_langPreviewLabel->setText(
            QStringLiteral("开始游戏 / 设置 / 练习模式 / 拯救苹果"));
    } else {
        m_langPreviewLabel->setText(
            QStringLiteral("Start Game / Settings / Practice Mode / Save the Apple"));
    }
}

void SettingsDialog::onApply()
{
    if (m_radioZhCN->isChecked())
        emit languageChanged(QStringLiteral("zh_CN"));
    else
        emit languageChanged(QStringLiteral("en"));
}

void SettingsDialog::onFontSizeChanged(int value)
{
    m_fontSizeValueLabel->setText(QString::number(value) + QStringLiteral("px"));
}

// ─────────────────────────────────────────────────────────────────────────────
// i18n
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDialog::changeEvent(event);
}

void SettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    if (m_okBtn)     m_okBtn->setText(tr("OK"));
    if (m_cancelBtn) m_cancelBtn->setText(tr("Cancel"));
    if (m_applyBtn)  m_applyBtn->setText(tr("Apply"));
}

// ─────────────────────────────────────────────────────────────────────────────
// StyleSheet
// ─────────────────────────────────────────────────────────────────────────────

void SettingsDialog::setupStyleSheet()
{
    setStyleSheet(QStringLiteral(R"(
QDialog#settingsDialog {
    background: #F5F7FA;
    border-radius: 10px;
    border: 1px solid #C8D0DC;
}

/* 标题栏 */
QWidget#settingsTitleBar {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #1565C0, stop:0.5 #1E88E5, stop:1 #1565C0);
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
}
QLabel#settingsTitleIcon { color: white; font-size: 16px; background: transparent; }
QLabel#settingsTitleText { color: white; font-size: 15px; font-weight: bold; background: transparent; }
QPushButton#settingsCloseBtn {
    background: transparent; color: white; border: none;
    border-radius: 4px; font-size: 13px;
}
QPushButton#settingsCloseBtn:hover { background: rgba(220,50,50,180); }

/* 导航栏 */
QListWidget#settingsNav {
    background: #EAECF0;
    border: none;
    border-right: 1px solid #D0D4DC;
    padding-top: 8px;
    outline: none;
}
QListWidget#settingsNav::item {
    padding: 10px 16px;
    color: #333344;
    font-size: 13px;
    border-radius: 0px;
}
QListWidget#settingsNav::item:selected {
    background: #FFFFFF;
    color: #1565C0;
    font-weight: bold;
    border-left: 3px solid #1565C0;
}
QListWidget#settingsNav::item:hover:!selected {
    background: #DDE1EA;
}

/* 内容区 */
QStackedWidget#settingsStack { background: #FFFFFF; }
QWidget#settingsPage          { background: #FFFFFF; }
QWidget#settingsBody          { background: #FFFFFF; }

/* Section */
QWidget#settingsSection       { background: transparent; }
QLabel#sectionTitle {
    color: #1565C0;
    font-size: 13px;
    font-weight: bold;
}
QFrame#sectionDivider {
    color: #D8DCE8;
    background: #D8DCE8;
    max-height: 1px;
}

/* 语言 */
QRadioButton#langRadio { font-size: 13px; color: #222233; spacing: 8px; }
QRadioButton#langRadio::indicator { width: 16px; height: 16px; }
QRadioButton#langRadio::indicator:checked {
    background: #1565C0; border-radius: 8px; border: 2px solid #1565C0;
}
QRadioButton#langRadio::indicator:unchecked {
    background: white; border-radius: 8px; border: 2px solid #AABBCC;
}
QLabel#langDesc    { color: #8899AA; font-size: 12px; background: transparent; }
QLabel#langPreview {
    background: #F0F4FA;
    border: 1px solid #D0D8E8;
    border-radius: 6px;
    padding: 10px 12px;
    color: #334455;
    font-size: 13px;
}
QLabel#noteText { color: #7788AA; font-size: 12px; background: transparent; }

/* CheckBox */
QCheckBox#settingsCheckBox { font-size: 13px; color: #222233; spacing: 8px; }
QCheckBox#settingsCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; }
QCheckBox#settingsCheckBox::indicator:unchecked {
    background: white; border: 2px solid #AABBCC;
}
QCheckBox#settingsCheckBox::indicator:checked {
    background: #1565C0; border: 2px solid #1565C0;
}

/* Slider */
QSlider#settingsSlider::groove:horizontal {
    height: 4px; background: #D0D8E8; border-radius: 2px;
}
QSlider#settingsSlider::handle:horizontal {
    width: 14px; height: 14px; margin: -5px 0;
    background: #1E88E5; border-radius: 7px;
}
QSlider#settingsSlider::sub-page:horizontal {
    background: #1E88E5; border-radius: 2px;
}
QLabel#sliderValueLabel { color: #1565C0; font-size: 12px; font-weight: bold; background: transparent; }
QLabel#fontSmallLabel   { color: #8899AA; font-size: 10px; background: transparent; }
QLabel#fontLargeLabel   { color: #334455; font-size: 16px; font-weight: bold; background: transparent; }
QLabel#volumeLabel      { color: #334455; font-size: 13px; background: transparent; min-width: 110px; }

/* ComboBox / SpinBox / LineEdit */
QComboBox#settingsComboBox, QSpinBox#settingsSpinBox, QLineEdit#settingsLineEdit {
    background: white; border: 1px solid #C8D0DC; border-radius: 5px;
    padding: 4px 8px; font-size: 13px; color: #222233;
}
QComboBox#settingsComboBox:focus, QSpinBox#settingsSpinBox:focus, QLineEdit#settingsLineEdit:focus {
    border: 1px solid #1E88E5;
}
QLabel#settingsLabel { font-size: 13px; color: #334455; background: transparent; min-width: 100px; }

/* 底部按钮栏 */
QWidget#settingsFooter {
    background: #F0F2F5;
    border-top: 1px solid #D8DCE8;
    border-bottom-left-radius: 10px;
    border-bottom-right-radius: 10px;
}
QPushButton#settingsOkBtn {
    background: #1565C0; color: white; border: none;
    border-radius: 6px; font-size: 13px; font-weight: bold;
}
QPushButton#settingsOkBtn:hover    { background: #1976D2; }
QPushButton#settingsOkBtn:pressed  { background: #0D47A1; }
QPushButton#settingsCancelBtn, QPushButton#settingsApplyBtn {
    background: white; color: #334455;
    border: 1px solid #C8D0DC; border-radius: 6px; font-size: 13px;
}
QPushButton#settingsCancelBtn:hover, QPushButton#settingsApplyBtn:hover {
    background: #EEF2FA; border-color: #1E88E5; color: #1565C0;
}

/* About 页 */
QLabel#aboutAppName  { font-size: 22px; font-weight: bold; color: #1565C0; background: transparent; }
QLabel#aboutVersion  { font-size: 12px; color: #8899AA; background: transparent; }
QLabel#aboutDesc     { font-size: 13px; color: #334455; background: transparent; }
    )"));
}
