#include "gui/stackPages/files.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>

Files::Files(QWidget *parent) : QWidget(parent)
{
    lineEdit = new QLineEdit(this);
    browseBtn = new QPushButton("Выбрать...", this);

    QWidget *findPanel = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(findPanel);
    layout->addWidget(lineEdit);
    layout->addWidget(browseBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel("Files", this));
    mainLayout->addWidget(findPanel);

    mainLayout->addStretch();

    connect(browseBtn, &QPushButton::clicked, this, &Files::browseFile);
    connect(lineEdit, &QLineEdit::returnPressed, this, &Files::checkFileExists);
    connect(this, &Files::fileChanged, this, &Files::handleFileChanged);
}

Files::~Files() { }

void Files::browseFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите файл", "", "Все файлы (*.*)");
    if (!path.isEmpty())
    {
        lineEdit->setText(path);
        emit fileChanged(path);
    }
}

void Files::checkFileExists()
{
    QString path = lineEdit->text().trimmed();
    if (!path.isEmpty() && !QFile::exists(path))
    {
        QMessageBox::warning(this, "Ошибка", "Файл не найден!");
        return;
    }
    emit fileChanged(path);
}

void Files::handleFileChanged(const QString &path)
{
    qDebug() << "Файл выбран:" << path;
}