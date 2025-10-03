#include "stackPages/hexFilter.h"

#include <QEvent>
#include <QKeyEvent>

HexFilter::HexFilter(QObject *parent) : QObject(parent)
{
}

HexFilter::~HexFilter()
{
}

bool HexFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QChar ch = keyEvent->text().isEmpty() ? QChar() : keyEvent->text()[0];
        int key = keyEvent->key();

        // Разрешаем цифры 0-9, буквы A-F/a-f, пробел
        if (ch.isDigit() || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') || ch == ' ')
            return false; 

        // Разрешаем управляющие клавиши
        if (key == Qt::Key_Backspace || key == Qt::Key_Delete || key == Qt::Key_Enter || key == Qt::Key_Return || key == Qt::Key_Left || key == Qt::Key_Right)
            return false;

        return true;
    }

    return QObject::eventFilter(obj, event);
}