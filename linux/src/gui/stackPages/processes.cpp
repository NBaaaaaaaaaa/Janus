#include "gui/stackPages/processes.h"
#include "gui/stackPages/hexFilter.h"
#include "core/inject.h"

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
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QTextEdit>

Processes::Processes(QWidget *parent) : QWidget(parent)
{   
    pathEdit = new QLineEdit(this);
    browseBtn = new QPushButton("Выбрать...", this);

    // Создание виджета ввода файла
    QWidget *findWidget = new QWidget(this);
    QHBoxLayout *findLayout = new QHBoxLayout(findWidget);
    findLayout->addWidget(pathEdit);
    findLayout->addWidget(browseBtn);

    // Создание виджета выбора метода перехода на полезную нагрузку
    jumpMethodsGroup = new QGroupBox("Метод перехода на полезную нагрузку", this);
    QVBoxLayout *jumpMethodsLayout = new QVBoxLayout(jumpMethodsGroup);

    pmIpRadio = new QRadioButton("Изменение значение регистра адреса текущей инструкции", jumpMethodsGroup);
    pmCurInstRadio = new QRadioButton("Изменение текущей инструкции", jumpMethodsGroup);
    pmGotRadio = new QRadioButton("Изменение адреса ипортируемой функции", jumpMethodsGroup);
    pmFiniRadio = new QRadioButton("Изменение адреса функции в fini", jumpMethodsGroup);
    pmFiniArrayRadio = new QRadioButton("Изменение адреса функции в fini_array", jumpMethodsGroup);
    pmIpRadio->setChecked(true);

    jumpMethodsLayout->addWidget(pmIpRadio);
    jumpMethodsLayout->addWidget(pmCurInstRadio);
    jumpMethodsLayout->addWidget(pmGotRadio);
    jumpMethodsLayout->addWidget(pmFiniRadio);
    jumpMethodsLayout->addWidget(pmFiniArrayRadio);

    // Создание виджета ввода полезной нагрузки
    QWidget *payloadWidget = new QWidget(this);
    QVBoxLayout *payloadLayout = new QVBoxLayout(payloadWidget);

    QWidget *payloadHeadWidget = new QWidget(this);
    QHBoxLayout *payloadHeadLayout = new QHBoxLayout(payloadHeadWidget);

    loadPayloadBtn = new QPushButton("Загрузить из файла", this);

    payloadHeadLayout->addWidget(new QLabel("Hex payload:", this));
    payloadHeadLayout->addWidget(loadPayloadBtn);

    payloadEdit = new QTextEdit(this);
    payloadEdit->setPlaceholderText("Введите hex данные (например: 48 65 6C 6C 6F 20 57 6F 72 6C 64)\nИли: 48656C6C6F20576F726C64");
    payloadEdit->setAcceptRichText(false);
    payloadEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    payloadEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    payloadEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    payloadEdit->setWordWrapMode(QTextOption::WrapAnywhere); 
    payloadEdit->setFont(QFont("Courier New", 10));
    payloadEdit->setMinimumHeight(150);

    HexFilter *filter = new HexFilter(this);
    payloadEdit->installEventFilter(filter);

    payloadLayout->addWidget(payloadHeadWidget);
    payloadLayout->addWidget(payloadEdit);

    // Кнопка выполнения инъекции
    injectBtn = new QPushButton("Выполнить инъекцию", this);

    // Добавление всего в основной layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel("Processes", this));
    mainLayout->addWidget(findWidget);
    mainLayout->addWidget(jumpMethodsGroup);
    mainLayout->addWidget(payloadWidget);
    mainLayout->addWidget(injectBtn);


    mainLayout->addStretch();

    connect(browseBtn, &QPushButton::clicked, this, &Processes::browseProcess);
    connect(loadPayloadBtn, &QPushButton::clicked, this, &Processes::loadPayload);
    connect(injectBtn, &QPushButton::clicked, this, &Processes::injectPayload);
}

Processes::~Processes() { }

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
            pathEdit->setText(QString::number(selectedPid));
        }
    }
}

void Processes::loadPayload()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите файл", "", "Все файлы (*)");
    if (path.isEmpty())
    {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QString hexContent = fileData.toHex(' ').toUpper();
    
    payloadEdit->setPlainText(hexContent);
}

void Processes::injectPayload()
{
    enum MethodsJumpsProc mjp;
    struct Payload payload;
    QByteArray pathData;
    QByteArray payloadData;
    QString path = pathEdit->text().trimmed();
    QString pld = payloadEdit->toPlainText().trimmed();
    enum InjectStatus injectStatus;

    bool ok;
    int id = path.toInt(&ok);

    if (!ok || !QDir(QString("/proc/%1").arg(id)).exists()) {
        QMessageBox::warning(this, "Ошибка", "Процесс с таким PID не найден!");
        return;
    }

    if (pld.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет полезной нагрузки!");
        return;
    }

    if (pmIpRadio->isChecked()) {
        mjp = PM_IP;
    } else if (pmCurInstRadio->isChecked()) {
        mjp = PM_CUR_INST;
    } else if (pmGotRadio->isChecked()) {
        mjp = PM_GOT;
    } else if (pmFiniRadio->isChecked()) {
        mjp = PM_FINI;
    } else if (pmFiniArrayRadio->isChecked()) {
        mjp = PM_FINI_ARRAY;
    }

    payloadData = QByteArray::fromHex(pld.toUtf8());
    payload.size = payloadData.size();
    payload.addr = malloc(payload.size);

    if (!payload.addr) {
        QMessageBox::warning(this, "Ошибка", "Не удалось выделить память");
        return;
    }

    memcpy(payload.addr, payloadData.constData(), payload.size);

    pathData = QString("/proc/%1").arg(id).toUtf8();
    injectPayloadManager(TT_PROC, pathData.data(), &payload, mjp);

    // надо делать связь строк с кодом 
    qDebug() << "stat = " << injectStatus;

    free(payload.addr);
}
