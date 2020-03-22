#include "widget.h"

#include <QApplication>
#include "QDebug"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    a.setWindowIcon(QIcon("logo.ico"));
    w.show();
    return a.exec();
}
