#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent> // for the events of mouse
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QMessageBox>
#include "reversi.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    Reversi *pBoard;
    ~Widget();
    //
private slots:
    void colorBtnsToggled(int, bool);
    void undoBtnClicked();
private:
    Ui::Widget *ui;
    const int orin_x = 20;
    const int orin_y = 20;
    const int squareSize = 40;
    typedef enum{
        eBlack,
        eWhite
    }playerType_t;
    // the buttons: black && white
    QButtonGroup *colorGroup;
    QRadioButton *blackBtn;
    QRadioButton *whiteBtn;
    // the undo button
    QPushButton *undoBtn;
    // two labels to show elapsed time
    QLabel *lastTimeTipLabel;
    QLabel *totalTimeTipLabel;

    QLabel *lastTimeLabel;
    QLabel *totalTimeLabel;
    double lastTime;
    double totalTime;
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void DrawBoard();
    void DrawChess();
    // pop a message box when game over
    void PopGameOver();
};

#endif // WIDGET_H
