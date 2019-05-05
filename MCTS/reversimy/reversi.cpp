#include "reversi.h"
using namespace std;
static double C = sqrt(2);  // the tunable value of UCB
Board::Board():nextPiece(BLACK),step(0)
{
    for(int i = 0; i < 10; i++)
        for(int j = 0; j < 10; j++)
            chessBoard[i][j] = EMPTY;
    chessBoard[4][5] = chessBoard[5][4] = BLACK;
    chessBoard[4][4] = chessBoard[5][5] = WHITE;
    lastStep = {0,0};
}

Reversi::Reversi()
{
    nodeNum = 0;
    human = BLACK;
}
void Reversi::Undo()
{
    int canUndo = 0;
    int i = step-1;
    for( ; i >= 0; i--)
    {
        if(pastColors[i] == nextPiece)
        {
            canUndo = 1;
            break;
        }
    }
    if(canUndo == 0)
        return;
    //
    RestartGame();
    int tmp = i;
    for(int i = 0; i < tmp; i++)
    {
        if(i >= 1 && (pastColors[i-1] == pastColors[i]))
        {
            Pass();
        }
        Play(pastSteps[i]);
    }
}
int Reversi::GetBlackNum()
{
    int num = 0;
    for(int i = 1; i <=8; i++)
        for(int j = 1; j <= 8; j++)
        {
            if(chessBoard[i][j] == BLACK)
                num++;
        }
    return num;
}
int Reversi::GetWhiteNum()
{
    int num = 0;
    for(int i = 1; i <=8; i++)
        for(int j = 1; j <= 8; j++)
        {
            if(chessBoard[i][j] == WHITE)
                num++;
        }
    return num;
}
void Reversi::RestartGame()
{
    for(int i = 0; i < 10; i++)
        for(int j = 0; j < 10; j++)
            chessBoard[i][j] = EMPTY;
    chessBoard[4][5] = chessBoard[5][4] = BLACK;
    chessBoard[4][4] = chessBoard[5][5] = WHITE;
    lastStep = {0,0};
    nodeNum = 0;
    nextPiece = BLACK;
    step = 0;
}
void Reversi::BackUp(double s, Node_t *p)
{
    while(p->parent != NULL)
    {
        p = p->parent;
        s = 1 - s;
        p->winCase += s;
        p->totalCase += 1;
    }
}
double Reversi::Score(int color)
{
    int c1, c2;
    c1 = c2 = 0;
    for(int i = 1; i <= 8; i++)
    {
        for(int j = 1; j <= 8; j++)
        {
            if(chessBoard[i][j] == color)
                c1++;
            else if(chessBoard[i][j] == -color)
                c2++;
        }
    }
    if(c1 > c2)
        return 1;
    else if(c1 < c2)
        return 0;
    else
        return 0.5;
}
double Reversi::Simulate(Node_t *p)
{
    int color = nextPiece;
    int state = 0;
    int availPosNum;
    Pos_t availPos[60];
    Play(p->nodePos);
    while(state != 2)
    {
        availPosNum = GetAllAvailPos(availPos);
        if(availPosNum > 0)
        {
            state = 0;
            //default_random_engine e(time(0));
            //uniform_int_distribution<int> u(0, availPosNum-1);
            Play(availPos[rand()%availPosNum]);
        }
        else
        {
            state++;
            nextPiece = -nextPiece;
            continue;
        }
    }
    return Score(color);
}

Reversi& Reversi::operator=(Reversi &temp)
{
    for(int i = 0; i < 10; i++)
    {
        for(int j = 0; j < 10; j++)
            chessBoard[i][j] = temp.chessBoard[i][j];
    }
    nextPiece = temp.nextPiece;
    step = temp.step;
    return *this;
}
// now with the assumption that the position p is legal
void Board::Play(Pos_t p)
{
    Pos_t temp;
    chessBoard[p.x][p.y] = nextPiece;
    lastStep = p;
    pastSteps[step] = p;
    pastColors[step] = nextPiece;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            temp.x = p.x - 1 + i;
            temp.y = p.y - 1 + j;
            if(GetStat(temp) == -nextPiece)
            {
                int deltaX = temp.x - p.x;
                int deltaY = temp.y - p.y;
                while(BOARDCHECK(temp.x+deltaX) && BOARDCHECK(temp.y+deltaY))
                {
                    temp.x += deltaX;
                    temp.y += deltaY;
                    if(GetStat(temp) == nextPiece)
                    {
                        temp.x -= deltaX;
                        temp.y -= deltaY;
                        while((temp.x != p.x) || (temp.y != p.y))
                        {
                            chessBoard[temp.x][temp.y] = - chessBoard[temp.x][temp.y];
                            temp.x -= deltaX;
                            temp.y -= deltaY;
                        }
                        break;
                    }
                    else if(GetStat(temp) == EMPTY)  // bug1  before
                        break;

                }
            }
        }
    }
    nextPiece = -nextPiece;
    step++;
}
int Reversi::UCT(Node_t* arr[], int num)
{
    double * score = new double[num];
    double total = 0;
    for(int i = 0; i < num; i++)
    {
        total += arr[i]->totalCase;
    }
    double tmp = log(total)*2;
    int maxIndex = 0;
    for(int i = 0; i < num; i++)
    {
        score[i] = arr[i]->winCase / arr[i]->totalCase + C * sqrt(tmp/arr[i]->totalCase);
        if(score[maxIndex] < score[i])
            maxIndex = i;
    }
    delete []score;
    return maxIndex;
}
int Reversi::IsGameOver()
{
    int ret = 0;
    if(GetAllAvailPos(NULL) == 0)
    {
        nextPiece = -nextPiece;
        if(GetAllAvailPos(NULL) == 0)
            ret = 1;
        nextPiece = -nextPiece;
    }
    return ret;
}
void Reversi::AutoPlay()
{
    const int eachNodeNum = 5; // play 5 times on each node
    Reversi tmpBd; // just used to record the board
    int availPosNum;
    Pos_t availPos[60];
    Node_t *pNode = tree; // & tree[0]
    availPosNum = GetAllAvailPos(availPos);
    if(availPosNum == 0) // no step to go, pass
    {
        nextPiece = -nextPiece;
        return;
    }
    tree[0].parent = tree[0].rightSibling = NULL;
    tree[0].totalCase = tree[0].winCase = 0;
    nodeNum = 1;
    for(int i = 0; i < availPosNum; i++)
    {
        if(i < availPosNum - 1)
            tree[nodeNum+i].rightSibling = &tree[nodeNum+i+1];
        else
            tree[nodeNum+i].rightSibling = NULL;
        tree[nodeNum+i].child = NULL;
        tree[nodeNum+i].parent = &tree[0];
        tree[nodeNum+i].winCase = tree[nodeNum+i].totalCase = 0;
        tree[nodeNum+i].nodePos = availPos[i];
    }
    // set the child
    tree[0].child = &tree[nodeNum];

    nodeNum += availPosNum;

    for(int i = 0; i < MAXTRIAL; i++) // update the Monte Carlo Tree for MAXTRIAL times
    {
        tmpBd = *this;
        pNode = &tree[0];
        while(pNode->child != NULL)
        {
            int num = 1;
            Node_t *tmpNode = pNode->child;
            while(tmpNode->totalCase != 0 && tmpNode->rightSibling != NULL)
            {
                tmpNode = tmpNode->rightSibling;
                num++;
            }
            if(tmpNode->totalCase == 0) // we have got the next node to simulate
            {
                pNode = tmpNode;
                break;
            }
            else // the nodes of this layer have all been simulated, we need to find in next layer
            {
                Node_t* *childNodes = new Node_t*[num];
                int i = 1;
                pNode = pNode->child;
                childNodes[0] = pNode;
                while(pNode->rightSibling != NULL)
                {
                    pNode = pNode->rightSibling;
                    childNodes[i++] = pNode;
                }
                // use UCB with tree to select a node
                pNode = childNodes[UCT(childNodes, num)];
                tmpBd.Play(pNode->nodePos);
                delete[]childNodes;
            }
        }
        if(pNode->totalCase > 0) // add new nodes and randomly select one of them
        {
            availPosNum = tmpBd.GetAllAvailPos(availPos);
            if(availPosNum == 0)
            {
                double s = pNode->winCase / pNode->totalCase;
                pNode->winCase += s*eachNodeNum;
                pNode->totalCase += 1*eachNodeNum;
                // backup and continue for next loop
                BackUp(s, pNode);
                continue;
            }
            else // create new nodes
            {
                //shuffle the nodes for random
                //default_random_engine e(time(0));
                //uniform_int_distribution<int> u(0, availPosNum-1);
                for(int i = 0; i < availPosNum; i++)
                {
                    int r = rand()%availPosNum;
                    Pos_t temp = availPos[i];
                    availPos[i] = availPos[r];
                    availPos[r] = temp;
                }
                for(int i = 0; i < availPosNum; i++)
                {
                    if(i < availPosNum-1)
                        tree[nodeNum+i].rightSibling = &tree[nodeNum+i+1];
                    else
                        tree[nodeNum+i].rightSibling = NULL;
                    tree[nodeNum+i].child = NULL;
                    tree[nodeNum+i].parent = pNode;
                    tree[nodeNum+i].winCase = tree[nodeNum+i].totalCase = 0;
                    tree[nodeNum+i].nodePos = availPos[i];
                }
                pNode->child = &tree[nodeNum];
                nodeNum += availPosNum;
                pNode = pNode->child;
            }
        }
        // we can simulate multiple times on pNode
        for(int i = 0; i < 5; i++)
        {
            Reversi r;
            r = tmpBd;
        // simulate on pNode
        double s = r.Simulate(pNode);
        pNode->totalCase += 1;
        pNode->winCase += s;
        // backup
        BackUp(s, pNode);
        }
    }
    Node_t *select;
    select = pNode = tree[0].child;
    //cout << pNode->winCase << " / "<< pNode->totalCase <<"  "<< pNode->nodePos.x<<","<<pNode->nodePos.y <<endl;
    while(pNode->rightSibling != NULL)
    {
        pNode = pNode->rightSibling;
        // changes here, choose the node that is visited most instead of the highest rate
        //if(pNode->winCase/pNode->totalCase > select->winCase/select->totalCase)
        if(pNode->totalCase > select->totalCase)
            select = pNode;
        //cout << pNode->winCase << " / "<< pNode->totalCase <<"  "<< pNode->nodePos.x<<","<<pNode->nodePos.y <<endl;
    }
    Play(select->nodePos);
    nodeNum = 1;
    //printf("The AI selected :(%d, %d)\n",select->nodePos.x,select->nodePos.y);
}
int Reversi::HumanPlay(Pos_t p)
{
    /*
    if(p.x == 0) // pass
    {
        nextPiece = -nextPiece;
        return;
    }*/
    if(IsPosAvail(p) == 1)
        Play(p);
    else
    {
        return 0;
        /*
        printf("the position is not legal, please input again:\n");
        //cin.sync();
        cin >> p.x;
        cin >> p.y;
        HumanPlay(p);*/
    }
    return 1;
}
int Reversi::IsPosAvail(Pos_t p)
{
    Pos_t temp = p;
    if(p.x < 1 || p.x > 8 || p.y < 1 || p.y > 8) // the position is out of the board
        return -1;
    if(GetStat(p) != EMPTY) // there is already piece in the position
        return 0;

    for(int i = 0; i < 3; i++) // check the surrounding
    {
        for(int j = 0; j < 3; j++)
        {
            temp.x = p.x - 1 + i;
            temp.y = p.y - 1 + j;
            if(GetStat(temp) == EMPTY || GetStat(temp) == nextPiece)
                continue;
            else
            {
                int deltaX = temp.x - p.x;
                int deltaY = temp.y - p.y;
                while(BOARDCHECK(temp.x+deltaX) && BOARDCHECK(temp.y+deltaY))
                {
                    temp.x += deltaX;
                    temp.y += deltaY;
                    if(GetStat(temp) == nextPiece)
                        return 1;
                    else if(GetStat(temp) == EMPTY)
                        break;
                }

            }
        }
    }
    return 0;
}
int Reversi::GetAllAvailPos(Pos_t posArray[])
{
    Pos_t temp;
    int num = 0;
    for(int i = 1; i <= 8; i++)
    {
        for(int j = 1; j <= 8; j++)
        {
            temp.x = i;
            temp.y = j;
            if(IsPosAvail(temp) == 1)
            {
                if(posArray != NULL)
                    posArray[num] = temp;
                num++;
            }
        }
    }
    return num;
}

ostream & operator<<(ostream &os, const Reversi b)
{
    Pos_t p;
    os << "Curent state:\n";
    for(p.y = 8; p.y >= 1; p.y--)
    {
        for(p.x = 1; p.x <= 8; p.x ++)
        {
            if(b.GetStat(p) == EMPTY)
                os << '*';
            else if(b.GetStat(p) == WHITE)
                os << '#';
            else
                os << '+';
        }
        os<< endl;
    }
    return os;
}
