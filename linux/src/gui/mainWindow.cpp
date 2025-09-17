#include "gui/mainWindow.h"
#include "gui/leftPanel.h"

#include <QLabel>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    LeftPanel *left_panel = new LeftPanel(this);

    QWidget *page1 = new QWidget(this);
    QVBoxLayout *layout1 = new QVBoxLayout(page1);
    layout1->addWidget(new QLabel("Files", this));

    QWidget *page2 = new QWidget(this);
    QVBoxLayout *layout2 = new QVBoxLayout(page2);
    layout2->addWidget(new QLabel("Processes", this));

    QWidget *page3 = new QWidget(this);
    QVBoxLayout *layout3 = new QVBoxLayout(page3);
    layout3->addWidget(new QLabel("Info", this));

    stack = new QStackedWidget(this);
    stack->addWidget(page1);
    stack->addWidget(page2);
    stack->addWidget(page3);

    QHBoxLayout *main_layout = new QHBoxLayout(central_widget);
    main_layout->addWidget(left_panel);
    main_layout->addWidget(stack, 1);

    connect(left_panel, &LeftPanel::pageChanged, stack, &QStackedWidget::setCurrentIndex);

    setWindowTitle("JanusLV");
    resize(800, 600);
}

MainWindow::~MainWindow() { }
