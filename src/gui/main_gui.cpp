#include "ova/MainWindow.hpp"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    ova::MainWindow window;
    window.show();
    return application.exec();
}
