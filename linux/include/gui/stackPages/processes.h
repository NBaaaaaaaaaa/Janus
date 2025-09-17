#ifndef PROCESSES_H
#define PROCESSES_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class Processes : public QWidget
{
    Q_OBJECT
public:
    explicit Processes(QWidget *parent = nullptr);
    ~Processes();

signals:
    void processChanged(int pid);

private slots:
    void browseProcess();
    void checkPid();
    void handleProcessChanged(int pid);

private:
    QLineEdit *lineEdit;
    QPushButton *browseBtn;
};


#endif // PROCESSES_H