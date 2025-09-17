#include "gui/mainWindow.h"
#include "gui/leftPanel.h"
#include "gui/stackPages/files.h"
#include "gui/stackPages/processes.h"
#include "gui/stackPages/info.h"

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

    stack = new QStackedWidget(this);
    stack->addWidget(new Files(this));
    stack->addWidget(new Processes(this));
    stack->addWidget(new Info(this));

    QHBoxLayout *main_layout = new QHBoxLayout(central_widget);
    main_layout->addWidget(left_panel);
    main_layout->addWidget(stack, 1);

    connect(left_panel, &LeftPanel::pageChanged, stack, &QStackedWidget::setCurrentIndex);

    setWindowTitle("JanusLV");
    resize(800, 600);
}

MainWindow::~MainWindow() { }
