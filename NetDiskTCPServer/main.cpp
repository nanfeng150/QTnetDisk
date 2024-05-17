#include "tcpserver.h"
#include <QApplication>
#include "dboperate.h"
#include <QDebug>
#include "rsakey.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TcpServer w;
    if(1 == w.getWidgetType())
        w.show();

    return a.exec();
}
