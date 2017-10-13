#include "gamewidget.h"
#include "puzzle.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(game);

    QApplication a(argc, argv);
    GameWidget game(Puzzle::MinSize);
	game.show();

    return a.exec();
}
