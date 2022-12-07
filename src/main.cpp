#include <QApplication>
#include <QDebug>
#include "loginwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // create a dialog window
    LoginWindow loginWindow;

    // show the dialog window
    loginWindow.show();

    // run the application
    return app.exec();
}
