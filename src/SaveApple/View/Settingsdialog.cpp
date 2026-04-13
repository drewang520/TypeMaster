#include "SettingsDialog.h"
#include "SaveApple/Config/GameConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QFrame>
#include <QFont>
#include <QPixmap>
#include <QIcon>
#include <QApplication>


// ── Constructor ──

AppleSettingsDialog::AppleSettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setFixedSize(420, 380);

    // Save originals for Cancel
    const GameConfig& cfg = GameConfig::getInstance();
    m_origSpeed  = cfg.getSpeedLevel();
    m_origFruits = cfg.getMaxFruitsOnScreen();
    m_origTarget = cfg.getLevelTarget();
    m_origSound  = cfg.isSoundEnabled();

    buildUi();
}

// ── UI construction ──
static QSlider* makeSlider(int min, int max, int val, QWidget* parent)
{
    QSlider* s = new QSlider(Qt::Horizontal, parent);
    s->setRange(min, max);
    s->setValue(val);
    s->setFixedHeight(28);
    return s;
}

void AppleSettingsDialog::buildUi()
{
    auto t = [](const char* src) -> QString {
        return QCoreApplication::translate("AppleWindow", src);
    };

    const GameConfig& cfg = GameConfig::getInstance();

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 20);
    root->setSpacing(16);

    // ── Title ──
    QLabel* title = new QLabel(t("Game Settings"), this);
    title->setObjectName(QStringLiteral("settingsTitle"));
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    // ── Separator ──
    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QStringLiteral("color: #333355;"));
    root->addWidget(sep);

    // ── Grid: label | slider | value ──
    QGridLayout* grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(14);
    grid->setColumnStretch(1, 1);

    auto rowLabel = [this](const QString& text) {
        QLabel* l = new QLabel(text, this);
        l->setObjectName(QStringLiteral("settingsKey"));
        return l;
    };
    auto valLabel = [this](const QString& text) {
        QLabel* l = new QLabel(text, this);
        l->setObjectName(QStringLiteral("settingsVal"));
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return l;
    };

    // Row 0 — Speed
    m_speedSlider = makeSlider(1, 10, cfg.getSpeedLevel(), this);
    m_speedVal    = valLabel(QString::number(cfg.getSpeedLevel()));
    grid->addWidget(rowLabel(t("Speed Level")), 0, 0);
    grid->addWidget(m_speedSlider, 0, 1);
    grid->addWidget(m_speedVal,    0, 2);
    connect(m_speedSlider, &QSlider::valueChanged,
            this, &AppleSettingsDialog::onSpeedChanged);

    // Row 1 — Max apples
    m_fruitsSlider = makeSlider(1, 5, cfg.getMaxFruitsOnScreen(), this);
    m_fruitsVal    = valLabel(QString::number(cfg.getMaxFruitsOnScreen()));
    grid->addWidget(rowLabel(t("Max Apples")), 1, 0);
    grid->addWidget(m_fruitsSlider, 1, 1);
    grid->addWidget(m_fruitsVal,    1, 2);
    connect(m_fruitsSlider, &QSlider::valueChanged,
            this, &AppleSettingsDialog::onMaxFruitsChanged);

    // Row 2 — Level target
    m_targetSlider = makeSlider(5, 30, cfg.getLevelTarget(), this);
    m_targetSlider->setSingleStep(5);
    m_targetSlider->setPageStep(5);
    m_targetVal    = valLabel(QString::number(cfg.getLevelTarget()));
    grid->addWidget(rowLabel(t("Level Target")), 2, 0);
    grid->addWidget(m_targetSlider, 2, 1);
    grid->addWidget(m_targetVal,    2, 2);
    connect(m_targetSlider, &QSlider::valueChanged,
            this, &AppleSettingsDialog::onTargetChanged);

    // Row 3 — Sound
    m_soundBtn = new QPushButton(this);
    m_soundBtn->setObjectName(QStringLiteral("soundBtn"));
    m_soundBtn->setFixedHeight(32);
    updateSoundButton();
    grid->addWidget(rowLabel(t("Sound")), 3, 0);
    grid->addWidget(m_soundBtn, 3, 1, 1, 2);
    connect(m_soundBtn, &QPushButton::clicked,
            this, &AppleSettingsDialog::onSoundToggled);

    root->addLayout(grid);
    root->addStretch();

    // ── Bottom buttons ──
    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->setSpacing(16);

    QPushButton* cancelBtn = new QPushButton(t("Cancel"), this);
    cancelBtn->setObjectName(QStringLiteral("cancelBtn"));
    cancelBtn->setFixedHeight(40);
    connect(cancelBtn, &QPushButton::clicked, this, &AppleSettingsDialog::onCancel);

    QPushButton* confirmBtn = new QPushButton(t("Confirm"), this);
    confirmBtn->setObjectName(QStringLiteral("confirmBtn"));
    confirmBtn->setFixedHeight(40);
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);
    root->addLayout(btnRow);
}

// ── Slots ──

void AppleSettingsDialog::onSpeedChanged(int value)
{
    GameConfig::getInstance().setSpeedLevel(value);
    m_speedVal->setText(QString::number(value));
}

void AppleSettingsDialog::onMaxFruitsChanged(int value)
{
    GameConfig::getInstance().setMaxFruits(value);
    m_fruitsVal->setText(QString::number(value));
}

void AppleSettingsDialog::onTargetChanged(int value)
{
    // Snap to nearest multiple of 5
    const int snapped = qRound(value / 5.0) * 5;
    if (snapped != value) {
        m_targetSlider->blockSignals(true);
        m_targetSlider->setValue(snapped);
        m_targetSlider->blockSignals(false);
    }
    GameConfig::getInstance().setLevelTarget(snapped);
    m_targetVal->setText(QString::number(snapped));
}

void AppleSettingsDialog::onSoundToggled()
{
    const bool newVal = !GameConfig::getInstance().isSoundEnabled();
    GameConfig::getInstance().setSoundEnabled(newVal);
    updateSoundButton();
}

void AppleSettingsDialog::onCancel()
{
    // Restore original values
    GameConfig& cfg = GameConfig::getInstance();
    cfg.setSpeedLevel(m_origSpeed);
    cfg.setMaxFruits(m_origFruits);
    cfg.setLevelTarget(m_origTarget);
    cfg.setSoundEnabled(m_origSound);
    reject();
}

void AppleSettingsDialog::updateSoundButton()
{
    auto t = [](const char* src) -> QString {
        return QCoreApplication::translate("AppleWindow", src);
    };
    const bool on = GameConfig::getInstance().isSoundEnabled();
    m_soundBtn->setText(on ? t("ON  ♪") : t("OFF  ✕"));
    m_soundBtn->setStyleSheet(on
        ? QStringLiteral("QPushButton#soundBtn { background:#4CAF50; color:white; "
                         "border:none; border-radius:8px; font-size:13px; font-weight:bold; padding:5px 18px;}"
                         "QPushButton#soundBtn:hover{background:#45a049;}")
        : QStringLiteral("QPushButton#soundBtn { background:#555577; color:#aaaaaa; "
                         "border:none; border-radius:8px; font-size:13px; padding:5px 18px;}"
                         "QPushButton#soundBtn:hover{background:#6666aa;}"));
}