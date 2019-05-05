#ifndef REVERSI_H
#define REVERSI_H

#include <iostream>
#include <math.h>
#include <random>
#include <ctime>

#define BLACK -1
#define WHITE 1
#define EMPTY 0
#define BOARDCHECK(a) (((a)>= 1) && ((a) <= 8))
#define MAXTRIAL 1000
using namespace std;

struct Pos_t{
    int x, y; // x is the row, y is the col
    bool operator == (Pos_t & a){return (a.x==x&&a.y==y);}
};
struct Node_t{
    Pos_t nodePos;
    double totalCase, winCase;
    Node_t *parent, *rightSibling, *child;
};
//class Reversi;
class Board
{
public:
    Board();
    void Play(Pos_t p);
    int GetStat(Pos_t p)const{return chessBoard[p.x][p.y];}
    Pos_t GetLastStep(){return lastStep;}
protected:
    int chessBoard[10][10]; // use 10 * 10 to represent a 8 * 8 chess board, checking will be easier
    int nextPiece; // the color of next piece
    //int human;
    Pos_t lastStep; // the position of last step
    // used to implement undo
    Pos_t pastSteps[64];
    int pastColors[64];
    int step; // steps that has passed, numbered 1 if one piece is played
};
class Reversi : public Board
{
public:
    Reversi();
    //Reversi(Reversi & temp);
    void Undo(); // undo the current player's last step
    int GetBlackNum();
    int GetWhiteNum();
    void RestartGame();
    int GetHumanColor(){return human;}
    int GetNextPiece(){return nextPiece;}
    void SetHumanColor(int color){human = color;}
    void Pass(){nextPiece = -nextPiece;}
    int HumanPlay(Pos_t p); // play manually, return 1 if succeed
    void AutoPlay(); // computer select and play using MCT
    int GetAllAvailPos(Pos_t posArray[]); // get the possible next positions
    int UCT(Node_t *arr[], int num); // use UCT to select a node
    int IsGameOver();
    double Score(int color);
    double Simulate(Node_t * p); // return the score for one simulation
    void BackUp(double s, Node_t *p);
    Reversi & operator =(Reversi &temp);
    friend ostream & operator<<(ostream &os, const Reversi b);
private:
    // the Monte Carlo Tree
    Node_t tree[10*MAXTRIAL+1];
    int nodeNum;
    int childIndex[MAXTRIAL];
    int human;


    int IsPosAvail(Pos_t p); // check if a position on the chess can be chosen for next step
    void Clear(Pos_t p){chessBoard[p.x][p.y] = EMPTY;}
};
#endif // REVERSI_H
