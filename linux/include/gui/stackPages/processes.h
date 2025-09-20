#ifndef PROCESSES_H
#define PROCESSES_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QGroupBox;
class QRadioButton;
class QTextEdit;
class QScrollArea;

class Processes : public QWidget
{
    Q_OBJECT
public:
    explicit Processes(QWidget *parent = nullptr);
    ~Processes();

private slots:
    void browseProcess();
    void loadPayload();
    void injectPayload();

private:
    QLineEdit *pathEdit;
    QPushButton *browseBtn;

    QGroupBox *jumpMethodsGroup;
    QRadioButton *pmIpRadio;
    QRadioButton *pmCurInstRadio;
    QRadioButton *pmGotRadio;
    QRadioButton *pmFiniRadio;
    QRadioButton *pmFiniArrayRadio;

    QTextEdit *payloadEdit;

    QPushButton *loadPayloadBtn;
    QPushButton *injectBtn;
};


#endif // PROCESSES_H