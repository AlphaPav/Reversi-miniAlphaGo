#include "reversi.h"
#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    //w.pBoard->Play({3,4});
    QFont font;
    font.setFamily("Segoe UI"); // Tahoma 宋体
    qApp->setFont(font);

    return a.exec();
}
