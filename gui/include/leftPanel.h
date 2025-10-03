#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>

class QPushButton;
class QVBoxLayout;

class LeftPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LeftPanel(QWidget *parent = nullptr);
    ~LeftPanel();

signals:
    void pageChanged(int index);

private:
    QVBoxLayout *layout;
    QPushButton *filesBtn;
    QPushButton *processesBtn;
    QPushButton *infoBtn;
};


#endif // LEFTPANEL_H