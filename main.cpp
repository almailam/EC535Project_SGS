// BU EC535 Project - Smart Gardening System (SGS)
// Abdulaziz AlMailam & Samuel Gulinello

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
