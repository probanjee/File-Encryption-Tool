/* FileVault v2.0 — Qt GUI — Application Entry Point */

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("FileVault");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("FileVault");

    MainWindow window;
    window.show();

    return app.exec();
}
