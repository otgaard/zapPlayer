#include "zapPlayer.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    zapPlayer w;
    w.show();

    return a.exec();
}
