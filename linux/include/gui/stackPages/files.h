#ifndef FILES_H
#define FILES_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QGroupBox;
class QRadioButton;
class QTextEdit;
class QScrollArea;

class Files : public QWidget
{
    Q_OBJECT

public:
    explicit Files(QWidget *parent = nullptr);
    ~Files();

private slots:
    void browseFile();
    void checkFileExists();
    void loadPayload();
    void injectPayload();

private:
    QLineEdit *pathEdit;
    QPushButton *browseBtn;

    QGroupBox *jumpMethodsGroup;
    QRadioButton *fmEEntryRadio;
    QRadioButton *fmInitRadio;
    QRadioButton *fmInitArrayRadio;
    QRadioButton *fmFiniRadio;
    QRadioButton *fmFiniArrayRadio;
    QRadioButton *fmPltRadio;

    QTextEdit *payloadEdit;

    QPushButton *loadPayloadBtn;
    QPushButton *injectBtn;
};

#endif // FILES_H