#include "gui.h"
#include <QApplication>
#include "MainWindow.h"

int run_gui(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
