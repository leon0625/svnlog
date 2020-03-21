#include "widget.h"

#include <QApplication>
#include "QDebug"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    if(argc > 2)
    {
        qDebug() << argv[1] << " " << argv[2];
        w.rootpath = strdup(argv[1]);
        w.logpath = strdup(argv[2]);
        w.init_logtable(w.logpath);
    }

    w.show();
    return a.exec();
}
