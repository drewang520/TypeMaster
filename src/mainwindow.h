#ifndef MAINWINDOW_H
#define MAINWINDOW_H    

#include <QMainWindow>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
 
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

private:
    void setupUi();
    void setupStyleSheet();
    void connectSignals();

    QWidget      *m_centralWidget{nullptr};
    QVBoxLayout  *m_mainLayout{nullptr};
    QLabel       *m_titleLabel{nullptr};
    QLabel       *m_subtitleLabel{nullptr};
    QWidget      *m_cardsContainer{nullptr};
    QHBoxLayout  *m_cardsLayout{nullptr};

    GameCard *m_appleCard{nullptr};
    GameCard *m_spaceCard{nullptr};
    GameCard *m_practiceCard{nullptr};

private slots:
    void onAppleGameRequested();
    void onSpaceGameRequested();
    void onPracticeGameRequested();
};

#endif // MAINWINDOW_H