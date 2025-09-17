#include "gui/mainWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    label = new QLabel("Hola", this);
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);

    setWindowTitle("JanusLV");
    resize(400, 200);
}

MainWindow::~MainWindow() { }
