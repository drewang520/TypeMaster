#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>

/**
 * PracticeWindow — simple free typing practice UI.
 * Displays a random letter and the user types it; tracks WPM and accuracy.
 */
class PracticeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PracticeWindow(QWidget *parent = nullptr);
    ~PracticeWindow() override = default;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void nextLetter();
    void updateStats();

    QLabel *m_promptLabel{nullptr};   // big letter to type
    QLabel *m_statsLabel{nullptr};    // hit/miss/accuracy
    QLabel *m_feedbackLabel{nullptr};

    QChar m_current;
    int   m_hits{0};
    int   m_misses{0};
    int   m_total{0};
};
