#ifndef FILES_H
#define FILES_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class Files : public QWidget
{
    Q_OBJECT

public:
    explicit Files(QWidget *parent = nullptr);
    ~Files();

signals:
    void fileChanged(const QString &path);

private slots:
    void browseFile();
    void checkFileExists();
    void handleFileChanged(const QString &path);

private:
    QLineEdit *lineEdit;
    QPushButton *browseBtn;
};

#endif // FILES_H