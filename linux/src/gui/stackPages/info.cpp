#include "gui/stackPages/info.h"

#include <QVBoxLayout>
#include <QLabel>

Info::Info(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    label = new QLabel("Info", this);
    mainLayout->addWidget(label);

    mainLayout->addStretch();
}


Info::~Info() { }