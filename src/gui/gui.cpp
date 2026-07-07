#include "gui.h"
#include <QApplication>
#include "MainWindow.h"
#include <QIcon>

int run_gui(int argc, char *argv[]) {
    QGuiApplication::setDesktopFileName("com.sams200.suboptimal");
    QApplication app(argc, argv);
    app.setApplicationName("suboptimal");
    app.setDesktopFileName("suboptimal");
    app.setWindowIcon(QIcon(":/assets/suboptimal.png"));

    MainWindow w;
    w.show();
    return app.exec();
}
