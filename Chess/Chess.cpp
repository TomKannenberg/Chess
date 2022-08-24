//chess by Tom Kannenberg started 9.8.2022

#include <iostream>
#include <random>
#include <windows.h>
#include <winuser.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <conio.h>
#include <algorithm>
class Figure;
struct VectorHasher {
    int operator()(const std::vector<int> &V) const {
        int hash = V.size();
        for(auto &i : V) {
            hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};
std::unordered_map<std::vector<int>, Figure*, VectorHasher> figures;
std::unordered_map<std::vector<int>, bool, VectorHasher> figuresb;
std::vector<std::vector<int>> vbuffer = {};
Figure* lf; //last figure
int fc = 0; // figure count / debugging purpose
int tc = 0; // turn count
bool cp = true; // current player (true = white)
bool pc; //players color
int cx,cy; // cursor position
int ws = 10; // window size
int yo = 9; // y offset
int xo = 0; // x offset
bool tp = false; // two player mode
bool ep = false; // en passant
std::vector<int> wkp = {5,1}; // white king position
std::vector<int> bkp = {5,8}; // black king position
std::vector<int> w = {4,2}; std::vector<int> b = {5,7}; // white and black starting position
std::vector<int> cxy = {cx,cy}; // cursor position vector

void ConsoleSetup();
void SetupBoard();
void ShowConsoleCursor(bool showFlag);
void SetCursorPosition(int x, int y);
void WriteInColor(unsigned short color, std::string outputString, int line = 0,int character = 0);
void Turn(bool matt = false ,bool &white = cp);
void CursorScroll(bool up, int &x,int max = 8);
void PlayerTurn();
void tryDraw(std::vector<int> oxy);
void drawPossible(std::vector<std::vector<int>>);
void ComputerTurn();
bool vContains(std::vector<int> f);
int FastRand(int min, int max);

class Figure {
    protected:
        bool white; //color
        int x,y; //position
        std::vector<std::vector<int>> ad; //attack directions
        std::vector<int> xy; //x,y position
    public:
        Figure(int,int,bool, std::string ff, std::vector<std::vector<int>> attack) {
            f = ff;
            ad = attack;
        }
        virtual ~Figure() {
            figures.erase(xy);
            fc--;
        }
        const void UpdateFigure() {
            WriteInColor(white ? 14 : 8, f,((y-yo)*-1), (x*2+xo));
            figuresb[xy] = true;
        }
        const void UpdateFigure(int c) {
            WriteInColor(white ? c * 16 + 14 : c * 16 +8, f,((y-yo)*-1), (x*2+xo));
        }
        const void UpdateFigure(std::vector<int> oxy,int c = false) {
            WriteInColor(white ? c * 16 + 14 : c * 16 +8, f,((y-yo)*-1), (x*2+xo));
        }
        virtual void Move(std::vector<int> axy) {
            WriteInColor(14, " ",((y-yo)*-1), (x*2+xo));
            figuresb[xy] = false;
            figures.erase(xy);
            figuresb[axy] = true;
            x = axy[0];
            y = axy[1];
            xy = {x,y};
            figures[xy] = this;
            for (std::vector<int> f : vbuffer) {
                if (figuresb[f]) {
                    figures[f]->UpdateFigure(f,0);
                } else {
                    WriteInColor(15 ," ",((f[1]-yo)*-1), (f[0]*2+xo));
                }
            }
            vbuffer = {};
            WriteInColor(16 * 3 +(white ? 14 : 8), f,((axy[1]-yo)*-1), (axy[0]*2+xo));
            if (vContains(white ? bkp : wkp)) {
                
            }
        }
        virtual std::vector<std::vector<int>> PossibleMoves() {
            std::vector<std::vector<int>> moves;
            return moves;
        }
        bool canAttack (std::vector<int> a) {
            for (std::vector<int> f : ad) {
                if (f[0] == a[0] && f[1] == a[1]) {
                    return true;
                }
            }
            return false;
        }
        std::string f; //ascii chess figure
        const bool isWhite() {
            return white;
        }
        bool isKingSafe() {
            bool kingsafe = true;
            std::vector<int> k = white ? wkp : bkp; //king position
            std::vector<int> direction = {
                white ? k[0] - x == 0 ? 0 : k[0] - x > 0 ? -1 : 1 : k[0] - x == 0 ? 0 : k[0] - x > 0 ? -1 : 1,
                white ? k[1] - y == 0 ? 0 : k[1] - y > 0 ? -1 : 1 : k[1] - y == 0 ? 0 : k[1] - y > 0 ? -1 : 1}; // 3 states -1 , 0 , 1
            bool d[2] = {true,true}; //direction Straigt or Diagonal ?
            bool ink = true; //is not king
            for (int i = 1; i < 9; i++) {
                Figure* fi;
                if (k[0] + i * direction[0] > 0 && k[0] + i * direction[0] < 9 && k[1] + i * direction[1] > 0 && k[1] + i * direction[1] < 9) {
                    if (figuresb[{x - i * direction[0], y - i * direction[1]}] && ink) {
                        fi = figures[{x - i * direction[0], y - i * direction[1]}];
                        std::vector<int> l = {x - i * direction[0], y - i * direction[1]};
                        if (fi->isWhite() == white && l != k) {
                            break;
                        } else if (l[0] == k[0] && l[1] == k[1]) {
                            ink = false;
                        }
                    }
                    if (figuresb[{x + i * direction[0], y + i * direction[1]}]) {
                        fi = figures[{x + i * direction[0], y + i * direction[1]}];
                        if (fi->isWhite() != white) {
                            if (fi->canAttack(direction)) {
                                kingsafe = false;   
                            }
                        } else {
                            break;
                        }
                    }
                }
            }
            return kingsafe;
        }
};

class Rook : protected Figure {
public:
    Rook(int xx, int yy, bool side) : Figure(xx,yy,side, "♖", {{0,1},{0,-1},{1,0},{-1,0}}) {
        x = xx; y = yy; xy = {x,y}; white = side;
        figures[xy] = this;
        UpdateFigure();
        fc++;
    }

    std::vector<std::vector<int>> PossibleMoves() override {
        std::vector<std::vector<int>> moves;
        std::vector<std::vector<int>> pmoves;
        moves.push_back(xy);
        bool ks = isKingSafe();
        std::vector<int> k = white ? wkp : bkp; //king position
        std::vector<int> d = {
            white ? k[0] - x == 0 ? 0 : k[0] - x > 0 ? -1 : 1 : k[0] - x == 0 ? 0 : k[0] - x > 0 ? -1 : 1,
            white ? k[1] - y == 0 ? 0 : k[1] - y > 0 ? -1 : 1 : k[1] - y == 0 ? 0 : k[1] - y > 0 ? -1 : 1}; // 3 states -1 , 0 , 1
        bool b[4] = {true,true,true,true};
        for (int i = 1; i < 9; i++) {
            if (y+i < 9 && b[0] && (ks || d[0] == 0 &&  d[1] == 1)) {
                if (figuresb[{x,y+i}]) {
                    if (figures[{x,y+i}]->isWhite() != white) {
                        moves.push_back({x,y+i});
                        b[0] = false;
                    } else {
                        b[0] = false;
                    }
                } else {
                    moves.push_back({x,y+i});
                }
            }
            WriteInColor(5, std::to_string(ks) + " " + std::to_string(d[0]) + " " + std::to_string(d[1]),0);
            if (y-i > 0 && b[1] && (ks || d[0] == 0 &&  d[1] == -1)) {
                if (figuresb[{x,y-i}]) {
                    if (figures[{x,y-i}]->isWhite() != white) {
                        moves.push_back({x,y-i});
                        b[1] = false;
                    } else {
                        b[1] = false;
                    }
                } else {
                    moves.push_back({x,y-i});
                }
            }
            if (x+i < 9 && b[2] && (ks || d[0] == 1 &&  d[1] == 0)) {
                if (figuresb[{x+i,y}] && b[2]) {
                    if (figures[{x+i,y}]->isWhite() != white) {
                        moves.push_back({x+i,y});
                        b[2] = false;
                    } else {
                        b[2] = false;
                    }
                } else {
                    moves.push_back({x+i,y});
                }
            }
            if (x-i > 0 && b[3] && (ks || d[0] == -1 &&  d[1] == 0)) {
                if (figuresb[{x-i,y}]) {
                    if (figures[{x-i,y}]->isWhite() != white) {
                        moves.push_back({x-i,y});
                        b[3] = false;
                    } else {
                        b[3] = false;
                    }
                } else {
                    moves.push_back({x-i,y});
                }
            }
        }
        return moves;
    }
};

class Knight : protected Figure {
public:
    Knight(int xx, int yy, bool side) : Figure(xx,yy,side,"♘",{}) {
        x = xx; y = yy; xy = {x,y}; white = side;
        figures[xy] = this;
        UpdateFigure();
        fc++;
    }
    std::vector<std::vector<int>> PossibleMoves() {
        std::vector<std::vector<int>> moves;
        std::vector<std::vector<int>> pmoves;
        moves.push_back(xy);
        if (isKingSafe()) {
            if (figuresb[{x+1,y+2}]) {
                if (figures[{x+1,y+2}]->isWhite() != white) {
                    moves.push_back({x+1,y+2});
                }
            } else if (x+1 < 9 && y+2 < 9) {
                moves.push_back({x+1,y+2});
            }
            if (figuresb[{x+1,y-2}]) {
                if(figures[{x+1,y-2}]->isWhite() != white) {
                    moves.push_back({x+1,y-2});
                }
            } else if (x+1 < 9 && y-2 > 0) {
                moves.push_back({x+1,y-2});
            }
            if (figuresb[{x-1,y+2}]) {
                if (figures[{x-1,y+2}]->isWhite() != white) {
                    moves.push_back({x-1,y+2});
                }
            } else if (x-1 > 0 && y+2 < 9) {
                moves.push_back({x-1,y+2});
            }
            if (figuresb[{x-1,y-2}]) {
                if (figures[{x-1,y-2}]->isWhite() != white) {
                    moves.push_back({x-1,y-2});
                }
            } else if (x-1 > 0 && y-2 > 0) {
                moves.push_back({x-1,y-2});
            }
            if (figuresb[{x+2,y+1}]) {
                if ( figures[{x+2,y+1}]->isWhite() != white) {
                    moves.push_back({x+2,y+1});
                }
            } else if (x+2 < 9 && y+1 < 9) {
                moves.push_back({x+2,y+1});
            }
            if (figuresb[{x+2,y-1}]) {
                if (figures[{x+2,y-1}]->isWhite() != white) {
                    moves.push_back({x+2,y-1});
                }
            } else if (x+2 < 9 && y-1 > 0) {
                moves.push_back({x+2,y-1});
            }
            if (figuresb[{x-2,y+1}]) {
                if (figures[{x-2,y+1}]->isWhite() != white) {
                    moves.push_back({x-2,y+1});
                }
            } else if (x-2 > 0 && y+1 < 9) {
                moves.push_back({x-2,y+1});
            }
            if (figuresb[{x-2,y-1}]) {
                if (figures[{x-2,y-1}]->isWhite() != white) {
                    moves.push_back({x-2,y-1});
                }
            } else if (x-2 > 0 && y-1 > 0) {
                moves.push_back({x-2,y-1});
            }
        }
        return moves;
    }
};

class Bishop : protected Figure {
public:
    Bishop(int xx, int yy, bool side) : Figure(xx,yy,side,"♗",{{1,1},{1,-1},{-1,1},{-1,-1}}) {
        x = xx; y = yy; xy = {x,y}; white = side;
        figures[xy] = this;
        UpdateFigure();
        fc++;
    }

    std::vector<std::vector<int>> PossibleMoves() override {
        std::vector<std::vector<int>> moves;
        std::vector<std::vector<int>> pmoves;
        moves.push_back(xy);
        if (isKingSafe()) {
            bool b[4] = {true,true,true,true};
            for (int i = 1; i < 9; i++) {
                if (x+i < 9 && y+i < 9 && b[0]) {
                    if (figuresb[{x+i,y+i}]) {
                        if (figures[{x+i,y+i}]->isWhite() != white) {
                            moves.push_back({x+i,y+i});
                            b[0] = false;
                        } else {
                            b[0] = false;
                        }
                    } else {
                        moves.push_back({x+i,y+i});
                    }
                }
                if (x-i > 0 && y+i < 9 && b[1]) {
                    if (figuresb[{x-i,y+i}]) {
                        if (figures[{x-i,y+i}]->isWhite() != white) {
                            moves.push_back({x-i,y+i});
                            b[1] = false;
                        } else {
                            b[1] = false;
                        }
                    } else {
                        moves.push_back({x-i,y+i});
                    }
                }
                if (x+i < 9 && y-i > 0 && b[2]) {
                    if (figuresb[{x+i,y-i}]) {
                        if (figures[{x+i,y-i}]->isWhite() != white) {
                            moves.push_back({x+i,y-i});
                            b[2] = false;
                        } else {
                            b[2] = false;
                        }
                    } else {
                        moves.push_back({x+i,y-i});
                    }
                }
                if (x-i > 0 && y-i > 0 && b[3]) {
                    if (figuresb[{x-i,y-i}]) {
                        if (figures[{x-i,y-i}]->isWhite() != white) {
                            moves.push_back({x-i,y-i});
                            b[3] = false;
                        } else {
                            b[3] = false;
                        }
                    } else {
                        moves.push_back({x-i,y-i});
                    }
                }
                if (b[0] == false && b[1] == false && b[2] == false && b[3] == false) {
                    break;
                }
            }
        }
        return moves;
    }
};

class Queen : protected Figure {
public:
    Queen(int xx, int yy, bool side) : Figure(xx,yy,side,"♕",{{1,1},{1,-1},{-1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}}) {
        x = xx; y = yy; xy = {x,y}; white = side;
        figures[xy] = this;
        UpdateFigure();
        fc++;
    }
   std::vector<std::vector<int>> PossibleMoves() override {
        
        
        std::vector<std::vector<int>> moves;
        std::vector<std::vector<int>> pmoves;
        moves.push_back(xy);
        if (isKingSafe()) {
            bool b[8] = {true,true,true,true,true,true,true,true};
            for (int i = 1; i < 9; i++) {
                if (x + i < 9 && b[0]) {
                    if (figuresb[{x+i,y}]) {
                        if (figures[{x+i,y}]->isWhite() != white) {
                            moves.push_back({x+i,y});
                            b[0] = false;
                        } else {
                            b[0] = false;
                        }
                    } else {
                        moves.push_back({x+i,y});
                    }
                }
                if (x - i > 0 && b[1]) {
                    if (figuresb[{x-i,y}]) {
                        if (figures[{x-i,y}]->isWhite() != white) {
                            moves.push_back({x-i,y});
                            b[1] = false;
                        } else {
                            b[1] = false;
                        }
                    } else {
                        moves.push_back({x-i,y});
                    }
                }
                if (y + i < 9 && b[2]) {
                    if (figuresb[{x,y+i}]) {
                        if (figures[{x,y+i}]->isWhite() != white) {
                            moves.push_back({x,y+i});
                            b[2] = false;
                        } else {
                            b[2] = false;
                        }
                    } else {
                        moves.push_back({x,y+i});
                    }
                }
                if (y - i > 0 && b[3]) {
                    if (figuresb[{x,y-i}]) {
                        if (figures[{x,y-i}]->isWhite() != white) {
                            moves.push_back({x,y-i});
                            b[3] = false;
                        } else {
                            b[3] = false;
                        }
                    } else {
                        moves.push_back({x,y-i});
                    }
                }
                if (x + i < 9 && y + i < 9 && b[4]) {
                    if (figuresb[{x+i,y+i}]) {
                        if (figures[{x+i,y+i}]->isWhite() != white) {
                            moves.push_back({x+i,y+i});
                            b[4] = false;
                        } else {
                            b[4] = false;
                        }
                    } else {
                        moves.push_back({x+i,y+i});
                    }
                }
                if (x + i < 9 && y - i > 0 && b[5]) {
                    if (figuresb[{x+i,y-i}]) {
                        if (figures[{x+i,y-i}]->isWhite() != white) {
                            moves.push_back({x+i,y-i});
                            b[5] = false;
                        } else {
                            b[5] = false;
                        }
                    } else {
                        moves.push_back({x+i,y-i});
                    }
                }
                if (x - i > 0 && y + i < 9 && b[6]) {
                    if (figuresb[{x-i,y+i}]) {
                        if (figures[{x-i,y+i}]->isWhite() != white) {
                            moves.push_back({x-i,y+i});
                            b[6] = false;
                        } else {
                            b[6] = false;
                        }
                    } else {
                        moves.push_back({x-i,y+i});
                    }
                }
                if (x - i > 0 && y - i > 0 && b[7]) {
                    if (figuresb[{x-i,y-i}]) {
                        if (figures[{x-i,y-i}]->isWhite() != white) {
                            moves.push_back({x-i,y-i});
                            b[7] = false;
                        } else {
                            b[7] = false;
                        }
                    } else {
                        moves.push_back({x-i,y-i});
                    }
                }
                if (b[0] == false && b[1] == false && b[2] == false && b[3] == false && b[4] == false && b[5] == false && b[6] == false && b[7] == false) {
                    break;
                }
            }
        }
        return moves;
    }
};

class King : protected Figure {
    bool castling = true;
    bool check = false;
    
    public:
    King(int xx, int yy, bool side) : Figure(xx,yy,side,"♔",{}) {
        x = xx; y = yy; xy = {x,y}; white = side;
        figures[xy] = this;
        UpdateFigure();
        fc++;
    }
   std::vector<std::vector<int>> PossibleMoves() override {
       std::vector<std::vector<int>> moves;
        std::vector<std::vector<int>> pmoves;
        moves.push_back(xy);
        if (x + 1 < 9) {
            if (figuresb[{x+1,y}]) {
                if (figures[{x+1,y}]->isWhite() != white) {
                    moves.push_back({x+1,y});
                }
            } else {
                moves.push_back({x+1,y});
            }
        }
        if (x - 1 > 0) {
            if (figuresb[{x-1,y}]) {
                if (figures[{x-1,y}]->isWhite() != white) {
                    moves.push_back({x-1,y});
                }
            } else {
                moves.push_back({x-1,y});
            }
        }
        if (y + 1 < 9) {
            if (figuresb[{x,y+1}]) {
                if (figures[{x,y+1}]->isWhite() != white) {
                    moves.push_back({x,y+1});
                }
            } else {
                moves.push_back({x,y+1});
            }
        }
        if (y - 1 > 0) {
            if (figuresb[{x,y-1}]) {
                if (figures[{x,y-1}]->isWhite() != white) {
                    moves.push_back({x,y-1});
                }
            } else {
                moves.push_back({x,y-1});
            }
        }
        if (x + 1 < 9 && y + 1 < 9) {
            if (figuresb[{x+1,y+1}]) {
                if (figures[{x+1,y+1}]->isWhite() != white) {
                    moves.push_back({x+1,y+1});
                }
            } else {
                moves.push_back({x+1,y+1});
            }
        }
        if (x + 1 < 9 && y - 1 > 0) {
            if (figuresb[{x+1,y-1}]) {
                if (figures[{x+1,y-1}]->isWhite() != white) {
                    moves.push_back({x+1,y-1});
                }
            } else {
                moves.push_back({x+1,y-1});
            }
        }
        if (x - 1 > 0 && y + 1 < 9) {
            if (figuresb[{x-1,y+1}]) {
                if (figures[{x-1,y+1}]->isWhite() != white) {
                    moves.push_back({x-1,y+1});
                }
            } else {
                moves.push_back({x-1,y+1});
            }
        }
        if (x - 1 > 0 && y - 1 > 0) {
            if (figuresb[{x-1,y-1}]) {
                if (figures[{x-1,y-1}]->isWhite() != white) {
                    moves.push_back({x-1,y-1});
                }
            } else {
                moves.push_back({x-1,y-1});
            }
        }
        return moves;
    }
};

class Pawn : protected Figure {
    public:
        bool specialmove = true;
        Pawn(int xx, int yy, bool side) : Figure(xx,yy,side,"♙",{}) {
            x = xx; y = yy; xy = {x,y}; white = side;
            figures[xy] = this;
            UpdateFigure();
            fc++;
        }
        void Move(std::vector<int> axy) override {
            if (axy[1] == 8 || axy[1] == 1) {
                int ascension = 1;
                int c;
                bool ex = true;
                vbuffer.erase(vbuffer.begin());
                if (figuresb[{x+1,y+(white ? 1 : -1)}]) {
                    WriteInColor(
                        figures[{x+1,y+(white ? 1 : -1)}]->isWhite() ? 14 : 8,
                        figures[{x+1,y+(white ? 1 : -1)}]->f,((y+(white ? 1 : -1)-yo)*-1),
                        ((x+1)*2+xo));
                }
                if (figuresb[{x-1,y+(white ? 1 : -1)}]) {
                    WriteInColor(
                        figures[{x-1,y+(white ? 1 : -1)}]->isWhite() ? 14 : 8,
                        figures[{x-1,y+(white ? 1 : -1)}]->f,((y+(white ? 1 : -1)-yo)*-1),
                        ((x-1)*2+xo));
                }
                WriteInColor(14, " ",((y-yo)*-1), (x*2+xo));
                WriteInColor(16 * 3 + (white ? 14 : 8), "♕",((axy[1]-yo)*-1), (axy[0]*2+xo));
                while (ex) {
                    c = _getch();
                    if (c == 75 || c == 'A' || c == 'a') {
                        CursorScroll(false,ascension,4);
                    } else if (c == 77 || c == 'D' || c == 'd') {
                        CursorScroll(true,ascension,4);
                    } else if (c == 32) {
                        ex = false;
                    }
                    if (ascension == 1) {
                        WriteInColor(16 * 3 +(white ? 14 : 8), "♕",((axy[1]-yo)*-1), (axy[0]*2+xo));
                    } else if (ascension == 2) {
                        WriteInColor(16 * 3 +(white ? 14 : 8), "♖",((axy[1]-yo)*-1), (axy[0]*2+xo));
                    } else if (ascension == 3) {
                        WriteInColor(16 * 3 +(white ? 14 : 8), "♘",((axy[1]-yo)*-1), (axy[0]*2+xo));
                    } else if (ascension == 4) {
                        WriteInColor(16 * 3 +(white ? 14 : 8), "♗",((axy[1]-yo)*-1), (axy[0]*2+xo));
                    }
                }
                if (ascension == 1) {
                    new Queen(axy[0],axy[1],white);
                } else if (ascension == 2) {
                    new Rook(axy[0],axy[1],white);
                } else if (ascension == 3) {
                    new Knight(axy[0],axy[1],white);
                } else if (ascension == 4) {
                    new Bishop(axy[0],axy[1],white);
                }
                figures.erase(xy);
                lf = figures[axy];
                WriteInColor(0, " ",((axy[1]-yo)*-1), (axy[0]*2+xo));
                figuresb[xy] = false;
                delete this;
            } else {
                WriteInColor(14, " ",((y-yo)*-1), (x*2+xo));
                figuresb[xy] = false;
                figures.erase(xy);
                figuresb[axy] = true;
                if (axy[1] == y+2 || axy[1] == y-2) {
                    ep = true;
                }
                x = axy[0];
                y = axy[1];
                xy = {x,y};
                figures[xy] = this;
                specialmove = false;
                for (std::vector<int> f : vbuffer) {
                    if (figuresb[f]) {
                        figures[f]->UpdateFigure(f,0);
                    } else {
                        WriteInColor(15 ," ",((f[1]-yo)*-1), (f[0]*2+xo));
                    }
                }
                vbuffer = {};
                
                WriteInColor(16 * 3 + (white ? 14 : 8), f,((axy[1]-yo)*-1), (axy[0]*2+xo));
            }
        }

        std::vector<std::vector<int>> PossibleMoves() override {
            std::vector<std::vector<int>> moves;
            std::vector<std::vector<int>> pmoves;
            bool ks = isKingSafe();
            std::vector<int> k = white ? wkp : bkp; //king position
            std::vector<int> d = {
                white ? k[0] - x == 0 ? 0 : k[0] - x > 0 ? -1 : 1 : k[0] - x == 0 ? 0 : k[0] - x > 0 ? -1 : 1,
                white ? k[1] - y == 0 ? 0 : k[1] - y > 0 ? -1 : 1 : k[1] - y == 0 ? 0 : k[1] - y > 0 ? -1 : 1}; // 3 states -1 , 0 , 1
            moves.push_back(xy);
            if (isKingSafe()) {
                int a = this->isWhite() ? 1 : -1;
                int b = this->isWhite() ? 2 : -2;
                if (!figuresb[{x,y+a}] && y+a > 0 && y+a < 9 && (ks || (k[0] == 0 && k[1] == a))) {
                    moves.push_back({x,y+a});
                }
                if (!figuresb[{x,y+b}] && y+b > 0 && y+b < 9 && specialmove && (ks || (k[0] == 0 && k[1] == a))) {
                    if (!figuresb[{x,y+a}] && y+a > 0 && y+a < 9) {
                        moves.push_back({x,y+b});   
                    }
                }
                if (figuresb[{x+1,y+a}] && y+a > 0 && y+a < 9 && (ks || (k[0] == 1 && k[1] == a))) {
                    if (figures[{x+1,y+a}]->isWhite() != white) {
                        moves.push_back({x+1,y+a});
                    }
                }
                if (figuresb[{{x-1,y+a}}] && y+a > 0 && y+a < 9 && (ks || (k[0] == -1 && k[1] == a))) {
                    if (figures[{x-1,y+a}]->isWhite() != white) {
                        moves.push_back({x-1,y+a});
                    }
                }
            }
            return moves;
                
        }
};

int main() {
    ConsoleSetup();
    
    for (int a = 1; a < 9; a++) {
        for (int b = 1; b < 9; b++) {
            figuresb[{a,b}] = false;
        }
    }
    
    ShowConsoleCursor(false);
    SetupBoard();
    SetCursorPosition(0,0);
    pc = FastRand(0,1);
    pc = false;
    cxy = pc ? w : b;
    Turn(false);
}

HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
HWND consolewindow = GetConsoleWindow();
RECT r;
DPI_AWARENESS_CONTEXT dpiContext;

void ConsoleSetup() {
    SetWindowLong(consolewindow, GWL_STYLE, GetWindowLong(consolewindow, GWL_STYLE)&~WS_SIZEBOX&~WS_SYSMENU);
    SetConsoleOutputCP(CP_UTF8);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = ws*8.8;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"MYRIAD PRO");
    dpiContext = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    SetThreadDpiAwarenessContext(dpiContext);
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    MoveWindow(consolewindow,r.left, r.top, ws* 100, ws*100, TRUE);
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), {csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1});
}

void SetupBoard() {
    bool temp = false;
    for (int i = 0; i < 2; i++) {
        temp = !temp;
        
        int y = temp ? 2 : 7;
        
        for (int j = 1; j < 9; j++) {
            new Pawn(j,y,temp);
        }
        //256 Black && 240 White
        y += temp ? -1 : 1;

        new Rook(1,y,temp);
        new Rook(8,y,temp);
        
        new Knight(2,y,temp);
        new Knight(7,y,temp);

        new Bishop(3,y,temp);
        new Bishop(6,y,temp);
        
        new Queen(4,y,temp);
        new King(5,y,temp);
        
    }
}

void WriteInColor(unsigned short color, std::string outputString, int line,int character) {
    SetCursorPosition(character,line);
    SetConsoleTextAttribute(hcon, color);
    std::cout << outputString;
    SetConsoleTextAttribute(hcon, 0);
}

void SetCursorPosition(int x, int y) {
    std::cout.flush();
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hcon, coord);
}

void ShowConsoleCursor(bool showFlag) {

    CONSOLE_CURSOR_INFO     cursorInfo;
    cursorInfo.dwSize = 1;
    GetConsoleCursorInfo(hcon, &cursorInfo);
    cursorInfo.bVisible = showFlag;
    SetConsoleCursorInfo(hcon, &cursorInfo);
}

void Turn(bool matt, bool &white) {
    tc++;
    PlayerTurn();
    ShowConsoleCursor(false);
    if (pc) { if (white) {
            PlayerTurn();   
        } else {
            ComputerTurn();   
        }
    } else { if (white) {
            ComputerTurn();
        } else {
            PlayerTurn();  
        }
    }
    
}

void PlayerTurn() {
    if (ep) {
        ep = false;
    }
    int c;
    int ox = cx;
    int oy = cy;
    cx = cxy[0];
    cy = cxy[1];
    std::string temp;
   std::vector<int> oxy = {ox,oy};
    if (figuresb[cxy]) {
        figures[cxy]->UpdateFigure(oxy,3);
        WriteInColor(0," ",13);
    } else {
        WriteInColor(94 ," ",((cy-yo)*-1), (cx*2+xo));
    }
    while (true) {
        SetCursorPosition(0,0);
        ShowConsoleCursor(false);
        c = _getch();
        if (c == 72 | c == 'W' | c == 'w') {
            oxy = {cx,cy};
            CursorScroll(true,cy);
            cxy = {cx,cy};
            tryDraw(oxy);
        } else if (c == 80 || c == 'S' || c == 's') {
            oxy = {cx,cy};
            CursorScroll(false,cy);
            cxy = {cx,cy};
            tryDraw(oxy);
        } else if (c == 75 || c == 'A' || c == 'a') {
            oxy = {cx,cy};
            CursorScroll(false,cx);
            cxy = {cx,cy};
            tryDraw(oxy);
        } else if (c == 77 || c == 'D' || c == 'd') {
            oxy = {cx,cy};
            CursorScroll(true,cx);
            cxy = {cx,cy};
            tryDraw(oxy);
        } else if (c == 32) {
            if (figuresb[cxy]) {
                drawPossible(figures[cxy]->PossibleMoves());
                lf = figures[cxy];
            } else if (vContains(cxy)) {
                bool t = lf->f == "♙" ? false : true;
                lf->Move(cxy);
                if (!t || (t && lf->f != "♙")) {
                          vbuffer = {};
                          lf->UpdateFigure(oxy,3);
                }
                for (std::vector<int> f : vbuffer) {
                    if (figuresb[f]) {
                        figures[f]->UpdateFigure(f,0);
                    } else {
                        WriteInColor(15 ," ",((f[1]-yo)*-1), (f[0]*2+xo));
                    }
                }
            }
        }

        else { WriteInColor(0,std::to_string(c),13); }
        Sleep(1);
    }
}

void tryDraw(std::vector<int> oxy) {
    if (figuresb[cxy]) {
        figures[cxy]->UpdateFigure(oxy,3);
    } else {
        WriteInColor(16*11+14 ," ",((cy-yo)*-1), (cx*2+xo));
    }
    if (vContains({oxy})) {
        if (oxy == vbuffer[0]) {
            figures[oxy]->UpdateFigure(oxy,6);
        } else if (figuresb[oxy]) {
            figures[oxy]->UpdateFigure(oxy,4);
        } else {
            WriteInColor(16 * 12 + 14 ," ",((oxy[1]-yo)*-1), (oxy[0]*2+xo));
        }
    } else if (figuresb[oxy]) {
        figures[oxy]->UpdateFigure(oxy,0);
    } else {
        WriteInColor(14," ",((oxy[1]-yo)*-1), (oxy[0]*2+xo));
    }
}

void drawPossible(std::vector<std::vector<int>> v) {
    for (std::vector<int> f : vbuffer) {
        if (figuresb[f]) {
            figures[f]->UpdateFigure(f,0);
        } else {
            WriteInColor(15 ," ",((f[1]-yo)*-1), (f[0]*2+xo));
        }
    }
    if (vbuffer.size() > 0) {figures[vbuffer[0]]->UpdateFigure(0);}
    vbuffer = v;
   std::vector<int> cv = {cx,cy};
    for (std::vector<int> f : v) {
        if (figuresb[f] && cv != f) {
            figures[f]->UpdateFigure(f,4);
        } else {
            WriteInColor(16 * 12 + 14 ," ",((f[1]-yo)*-1), (f[0]*2+xo));
        }
    }
    if (vbuffer.size() > 0) {figures[vbuffer[0]]->UpdateFigure(3);}
}

void ComputerTurn() {
    
}

bool vContains(std::vector<int> f) {
    for (std::vector<int> v : vbuffer) {
        if (v[0] == f[0] && v[1] == f[1]) {
            return true;
        }
    }
    return false;
}


void CursorScroll(bool up, int &n, int max) {
    n = up ? n < max ? n+1 : 1 : n > 1 ? n-1 : max;
}

std::random_device rd;
std::mt19937 mt(rd());

int FastRand(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(mt);
}