#pragma once

#include <QWidget>

/**
 * SpaceWindow — placeholder UI for the Space War game.
 * Game logic to be implemented in a future iteration.
 */
class SpaceWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SpaceWindow(QWidget *parent = nullptr);
    ~SpaceWindow() override = default;

protected:
    void paintEvent(QPaintEvent *event) override;
};
