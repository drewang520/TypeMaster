#ifndef GAMECARD_H
#define GAMECARD_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * GameCard — a clickable card widget shown on the main launcher screen.
 * Displays a preview image, game title, short description, and a Play button.
 */
class GameCard : public QFrame
{
    Q_OBJECT

public:
    explicit GameCard(const QString &title,
                      const QString &description,
                      const QString &imagePath,
                      const QString &gameId,
                      QWidget *parent = nullptr);
    ~GameCard() override = default;

    QString gameId() const { return m_gameId; }

signals:
    void playRequested(const QString &gameId);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupUi(const QString &title,
                 const QString &description,
                 const QString &imagePath);

    QString      m_gameId;
    QLabel      *m_previewLabel{nullptr};
    QLabel      *m_titleLabel{nullptr};
    QLabel      *m_descLabel{nullptr};
    QPushButton *m_playButton{nullptr};
};

#endif // GAMECARD_H   