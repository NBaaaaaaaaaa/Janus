#include "gui/stackPages/files.h"
#include "gui/stackPages/hexFilter.h"
#include "core/inject.h"


#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QTextEdit>

Files::Files(QWidget *parent) : QWidget(parent)
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

    fmEEntryRadio = new QRadioButton("Изменение адреса точки входа", jumpMethodsGroup);
    fmInitRadio = new QRadioButton("Изменение адреса функции в init", jumpMethodsGroup);
    fmInitArrayRadio = new QRadioButton("Изменение адреса функции в init_array", jumpMethodsGroup);
    fmFiniRadio = new QRadioButton("Изменение адреса функции в fini", jumpMethodsGroup);
    fmFiniArrayRadio = new QRadioButton("Изменение адреса функции в fini_array", jumpMethodsGroup);
    fmPltRadio = new QRadioButton("Модифицирует элемент первой импортированной функции в plt", jumpMethodsGroup);
    fmEEntryRadio->setChecked(true);

    jumpMethodsLayout->addWidget(fmEEntryRadio);
    jumpMethodsLayout->addWidget(fmInitRadio);
    jumpMethodsLayout->addWidget(fmInitArrayRadio);
    jumpMethodsLayout->addWidget(fmFiniRadio);
    jumpMethodsLayout->addWidget(fmFiniArrayRadio);
    jumpMethodsLayout->addWidget(fmPltRadio);

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
    mainLayout->addWidget(new QLabel("Files", this));
    mainLayout->addWidget(findWidget);
    mainLayout->addWidget(jumpMethodsGroup);
    mainLayout->addWidget(payloadWidget);
    mainLayout->addWidget(injectBtn);


    mainLayout->addStretch();

    connect(browseBtn, &QPushButton::clicked, this, &Files::browseFile);
    connect(loadPayloadBtn, &QPushButton::clicked, this, &Files::loadPayload);
    connect(injectBtn, &QPushButton::clicked, this, &Files::injectPayload);
}

Files::~Files() { }

void Files::browseFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите файл", "", "Все файлы (*.*)");
    if (!path.isEmpty())
    {
        pathEdit->setText(path);
    }
}

void Files::loadPayload()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите файл", "", "Все файлы (*.*)");
    if (!path.isEmpty())
    {
        // отобразить в payloadEdit
        payloadEdit->setText(path);
    }
}

void Files::injectPayload()
{
    enum MethodsJumpsFile mjf;
    struct Payload payload;
    QByteArray pathData;
    QByteArray payloadData;
    QString path = pathEdit->text().trimmed();
    QString pld = payloadEdit->toPlainText().trimmed();

    if (path.isEmpty() || (!path.isEmpty() && !QFile::exists(path)))
    {
        QMessageBox::warning(this, "Ошибка", "Файл не найден!");
        return;
    }

    if (pld.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет полезной нагрузки!");
        return;
    }

    if (fmEEntryRadio->isChecked()) {
        mjf = FM_E_ENTRY;
    } else if (fmInitRadio->isChecked()) {
        mjf = FM_INIT;
    } else if (fmInitArrayRadio->isChecked()) {
        mjf = FM_INIT_ARRAY;
    } else if (fmFiniRadio->isChecked()) {
        mjf = FM_FINI;
    } else if (fmFiniArrayRadio->isChecked()) {
        mjf = FM_FINI_ARRAY;
    } else if (fmPltRadio->isChecked()) {
        mjf = FM_PLT;
    }

    payloadData = pld.toUtf8();
    payload.size = payloadData.size();
    payload.addr = malloc(payload.size);

    if (!payload.addr) {
        QMessageBox::warning(this, "Ошибка", "Не удалось выделить память");
        return;
    }

    memcpy(payload.addr, payloadData.constData(), payload.size);

    pathData = path.toUtf8();
    injectPayloadManager(TT_FILE, pathData.data(), &payload, mjf);

    free(payload.addr);
}

