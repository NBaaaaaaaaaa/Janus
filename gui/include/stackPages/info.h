#ifndef INFO_H
#define INFO_H

#include <QWidget>

class QLabel;

class Info : public QWidget
{
    Q_OBJECT

public:
    explicit Info(QWidget *parent = nullptr);
    ~Info();
    
private:
    QLabel *label;
};

#endif // INFO_H