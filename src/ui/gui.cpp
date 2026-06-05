#include "gui.h"
#include <QApplication>
#include "MainWindow.h"
#include <QIcon>

int run_gui(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/suboptimal.png"));

    MainWindow w;
    w.show();
    return app.exec();
}
