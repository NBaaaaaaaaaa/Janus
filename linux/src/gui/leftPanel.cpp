#include "gui/leftPanel.h"
#include <QPushButton>
#include <QVBoxLayout>

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
{
    filesBtn = new QPushButton("Files", this);
    processesBtn = new QPushButton("Processes", this);
    infoBtn = new QPushButton("Indo", this);

    layout = new QVBoxLayout(this);
    layout->addWidget(filesBtn);
    layout->addWidget(processesBtn);
    layout->addWidget(infoBtn);

    layout->addStretch();

    connect(filesBtn, &QPushButton::clicked, [this]() {emit pageChanged(0); });
    connect(processesBtn, &QPushButton::clicked, [this]() {emit pageChanged(1); });
    connect(infoBtn, &QPushButton::clicked, [this]() {emit pageChanged(2); });
}

LeftPanel::~LeftPanel() { }