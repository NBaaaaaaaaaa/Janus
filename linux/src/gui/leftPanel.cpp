#include "gui/leftPanel.h"
#include <QPushButton>
#include <QVBoxLayout>

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
{
    files_btn = new QPushButton("Files", this);
    processes_btn = new QPushButton("Processes", this);
    info_btn = new QPushButton("Indo", this);

    layout = new QVBoxLayout(this);
    layout->addWidget(files_btn);
    layout->addWidget(processes_btn);
    layout->addWidget(info_btn);

    layout->addStretch();

    connect(files_btn, &QPushButton::clicked, [this]() {emit pageChanged(0); });
    connect(processes_btn, &QPushButton::clicked, [this]() {emit pageChanged(1); });
    connect(info_btn, &QPushButton::clicked, [this]() {emit pageChanged(2); });
}

LeftPanel::~LeftPanel() { }