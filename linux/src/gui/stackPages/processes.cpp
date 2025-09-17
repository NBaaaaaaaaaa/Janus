#include "gui/stackPages/processes.h"

#include <QDir>
#include <QStringList>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>

Processes::Processes(QWidget *parent) : QWidget(parent)
{   
    lineEdit = new QLineEdit(this);
    browseBtn = new QPushButton("Выбрать...", this);

    QWidget *findPanel = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(findPanel);
    layout->addWidget(lineEdit);
    layout->addWidget(browseBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel("Processes", this));
    mainLayout->addWidget(findPanel);
    
    mainLayout->addStretch();

    connect(browseBtn, &QPushButton::clicked, this, &Processes::browseProcess);
    connect(lineEdit, &QLineEdit::returnPressed, this, &Processes::checkPid);
    connect(this, &Processes::processChanged, this, &Processes::handleProcessChanged);

}

Processes::~Processes() { }

void Processes::checkPid()
{
    bool ok;
    int id = lineEdit->text().trimmed().toInt(&ok);
    if (!ok || !QDir(QString("/proc/%1").arg(id)).exists()) {
        QMessageBox::warning(this, "Ошибка", "Процесс с таким PID не найден!");
        return;
    }
    emit processChanged(id);
}

void Processes::browseProcess()
{
    QList<QPair<int, QString>> processes;
    QDir procDir("/proc");
    for (const QString &entry : procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        bool isNumber;
        int pid = entry.toInt(&isNumber);
        if (isNumber) {
            QFile file(QString("/proc/%1/comm").arg(pid));
            QString name;
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
                name = file.readLine().trimmed();
            processes.append(qMakePair(pid, name));
        }
    }

    std::sort(processes.begin(), processes.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b){
        return a.first < b.first;
    });

    QDialog dlg(this);
    dlg.setWindowTitle("Выберите процесс");
    QVBoxLayout *dlgLayout = new QVBoxLayout(&dlg);

    QListWidget *listWidget = new QListWidget(&dlg);
    for (const auto &p : processes) {
        listWidget->addItem(QString("%1: %2").arg(p.first).arg(p.second));
    }
    dlgLayout->addWidget(listWidget);

    QPushButton *okBtn = new QPushButton("OK", &dlg);
    dlgLayout->addWidget(okBtn);

    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(listWidget, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);

    if (dlg.exec() == QDialog::Accepted) {
        QString selected = listWidget->currentItem() ? listWidget->currentItem()->text() : "";
        if (!selected.isEmpty()) {
            int selectedPid = selected.section(':', 0, 0).toInt();
            lineEdit->setText(QString::number(selectedPid));
            emit processChanged(selectedPid);
        }
    }
}

void Processes::handleProcessChanged(int pid)
{
    qDebug() << "Процесс выбран:" << pid;
}
