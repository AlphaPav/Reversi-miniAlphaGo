#include "widget.h"
#include "ui_widget.h"
#include "reversi.h"
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Reversi");
    setFixedSize(480,370);

    blackBtn = new QRadioButton(tr("  Black"), this);
    blackBtn->setGeometry(orin_x+squareSize*8+15, orin_y+50, 80, 20);
    whiteBtn = new QRadioButton(tr("  White"), this);
    whiteBtn->setGeometry(orin_x+squareSize*8+15, orin_y+90, 80, 20);
    colorGroup = new QButtonGroup(this);
    colorGroup->addButton(blackBtn, eBlack);
    colorGroup->addButton(whiteBtn, eWhite);
    colorGroup->setExclusive(true);
    blackBtn->setChecked(true);
    connect(colorGroup, SIGNAL(buttonToggled(int,bool)), this, SLOT(colorBtnsToggled(int,bool)));

    lastTime = totalTime = 0;
    lastTimeLabel = new QLabel(this);
    totalTimeLabel = new QLabel(this);
    lastTimeTipLabel = new QLabel(this);
    totalTimeTipLabel = new QLabel(this);

    lastTimeTipLabel->setGeometry(orin_x+squareSize*8+15, orin_y+130, 260, 20);
    lastTimeLabel->setGeometry(orin_x+squareSize*8+15, orin_y+150, 260, 20);
    totalTimeTipLabel->setGeometry(orin_x+squareSize*8+15, orin_y+180, 260, 20);
    totalTimeLabel->setGeometry(orin_x+squareSize*8+15, orin_y+200, 260, 20);

    undoBtn = new QPushButton("Undo", this);
    undoBtn->setGeometry(orin_x+squareSize*8+15, orin_y+240, 50, 30);
    connect(undoBtn, SIGNAL(clicked()), this, SLOT(undoBtnClicked()));

    pBoard = new Reversi;
}

Widget::~Widget()
{
    delete ui;
    delete pBoard;
}
void Widget::paintEvent(QPaintEvent *)
{
    QFile qssfile(":/new/img/base.qss");//为UI控件设置样式
      qssfile.open(QFile::ReadOnly);
      QString qss;
      qss = qssfile.readAll();
      this->setStyleSheet(qss);

    QPainter painter(this);
    painter.drawPixmap(0,0,480,370,QPixmap(":/new/img/chessboard.png"));
    DrawBoard();
    DrawChess();
    char buf[50];
    sprintf(buf, "Last Step Used: ");
    lastTimeTipLabel->setText(buf);
     sprintf(buf, "%g ms", lastTime);
    lastTimeLabel->setText(buf);
    sprintf(buf, "Total Time Used: ");
    totalTimeTipLabel->setText(buf);
    sprintf(buf, "%g ms", totalTime);
    totalTimeLabel->setText(buf);

}
void Widget::DrawBoard()
{
    int size =8;
    QPainter* paint = new QPainter;
    paint->begin(this);
    paint->setPen(QPen(Qt::darkGray, 2, Qt::SolidLine));
    //draw the horizontal lines
    for(int i = 0; i < size+1; i++)
    {
        // draw the horizontal lines
        QPoint point1(orin_x, orin_y + squareSize*i);
        QPoint point2(orin_x + squareSize*size, orin_y + squareSize*i);
        paint->drawLine(point1, point2);
        // draw the vertical lines
        point1 = {orin_x + squareSize*i, orin_y};
        point2 = {orin_x + squareSize*i, orin_y + squareSize*size};
        paint->drawLine(point1, point2);
    }
    paint->end();
}
void Widget::DrawChess()
{
    if(pBoard == NULL)
        return;
    QPainter * paint = new QPainter;
    paint->begin(this);
    QPixmap * map= new QPixmap();
    for(int i = 1; i <= 8; i++)
    {
        for(int j = 1; j <= 8; j++)
        {
            Pos_t p{i,j};
            int color = pBoard->GetStat(p);
            if(color != WHITE && color != BLACK)
                continue;
            // draw the chess
            if(color == WHITE)
            {
                if( (p.x == pBoard->GetLastStep().x) && (p.y == pBoard->GetLastStep().y ))
                    map= new QPixmap(":/new/img/white_last.png");
//                    paint->setBrush(QBrush(Qt::white,Qt::DiagCrossPattern));

                else
                    map= new QPixmap(":/new/img/white.png");
//                    paint->setBrush(QBrush(Qt::white,Qt::SolidPattern));//毛刷：颜色，实图案
            }
            else if (color == BLACK)
            {
                 if( (p.x == pBoard->GetLastStep().x) && (p.y == pBoard->GetLastStep().y ))
                    map= new QPixmap(":/new/img/black_last.png");
//                    paint->setBrush(QBrush(Qt::black,Qt::Dense2Pattern));
                else
                    map= new QPixmap(":/new/img/black.png");
//                    paint->setBrush(QBrush(Qt::black,Qt::SolidPattern));//毛刷：颜色，实图案
            }
//            paint->drawEllipse((orin_x+(j-1)*squareSize),(orin_y+(i-1)*squareSize),
//                               squareSize, squareSize);
            paint->drawPixmap((orin_x+(j-1)*squareSize),(orin_y+(i-1)*squareSize),
                    squareSize, squareSize,*map);

        }
    }
    paint->end();
}
/* When the left button of mouse is pressed,
 * first check if the location is in the square of the chess board or not,
 * then test if the location can be played.
 * after human plays, get the robot to play at once.
 */
void Widget::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        int x = event->x();
        int y = event->y();
        // if the mouse is in the chess board
        if(x >= orin_x && x <= orin_x + 8*squareSize && y >= orin_y && y <= orin_y + 8*squareSize)
        {
            // compute the col and row
            int col = (x-orin_x+squareSize-1)/squareSize;
            int row = (y-orin_y+squareSize-1)/squareSize;
            // HumanPlay() will try to put the chess at the position,
            // and return 1 if it succeed, return 0 if the position is invalid
            int ret = pBoard->HumanPlay({row, col});
            repaint();
            if(pBoard->IsGameOver())
            {
                PopGameOver();
                return;
            }
            if(ret == 1) // the human has played
            {
                clock_t start = clock();
                pBoard->AutoPlay();
                clock_t end = clock();
                lastTime = end-start;
                totalTime += lastTime;
                repaint();
                if(pBoard->IsGameOver())
                {
                    PopGameOver();
                    return;
                }
                // Now the robot has played, if human has no position to to
                // the robot will play again
                while(pBoard->GetAllAvailPos(NULL) == 0)
                {
                    pBoard->Pass(); // human has no place to go, pass
                    clock_t start = clock();
                    pBoard->AutoPlay();
                    clock_t end = clock();
                    lastTime = end-start;
                    totalTime += lastTime;
                    repaint();
                    if(pBoard->IsGameOver())
                    {
                        PopGameOver();
                        return;
                    }
                }
            }

        }
    }

}
void Widget::colorBtnsToggled(int id, bool state)
{
    if(blackBtn->isChecked())
    {
        pBoard->SetHumanColor(BLACK);

    }
    else if(whiteBtn->isChecked())
    {
        pBoard->SetHumanColor(WHITE);
    }
    // if next step is the robot
    if(pBoard->GetNextPiece() != pBoard->GetHumanColor())
    {
        clock_t start = clock();
        pBoard->AutoPlay();
        clock_t end = clock();
        lastTime = end-start;
        totalTime += lastTime;
        repaint();
    }
}
void Widget::undoBtnClicked()
{
    pBoard->Undo();
    repaint();
}
void Widget::PopGameOver()
{
    QMessageBox msg(this);
    msg.setWindowTitle("Game Over");
    char buf[80];
    char s[2][6] = {"Black", "White"};
    int blackNum = pBoard->GetBlackNum();
    int whiteNum = pBoard->GetWhiteNum();
    int index = blackNum > whiteNum ? 0 : 1;
    if(blackNum == whiteNum)
        sprintf(buf, "black %d VS white %d \n Tie, another game?", blackNum, whiteNum);
    else
        sprintf(buf, "black %d VS white %d \n %s win, another game?", blackNum, whiteNum, s[index]);
    msg.setText(buf);
    msg.setStandardButtons(QMessageBox::Ok);

    if(msg.exec() == QMessageBox::Ok)
    {
        pBoard->RestartGame();
        totalTime = lastTime = 0;
        repaint();
        colorBtnsToggled(0,0);

    }

}
