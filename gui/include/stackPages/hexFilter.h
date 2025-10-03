#ifndef HEXFILTER_H
#define HEXFILTER_H

#include <QWidget>

class HexFilter : public QObject {
    Q_OBJECT
public:
    explicit HexFilter(QObject *parent = nullptr);
    ~HexFilter();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};


#endif // HEXFILTER_H