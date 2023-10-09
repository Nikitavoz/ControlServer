#ifndef ACTUALLABEL_H
#define ACTUALLABEL_H
#include <QtWidgets>

class ActualLabel: public QLabel {
    Q_OBJECT
using QLabel::QLabel;
public:
    void mouseDoubleClickEvent(QMouseEvent *event) {
        if(event->button() & Qt::LeftButton) emit doubleclicked(text());
        event->accept();
    }
signals:
    void doubleclicked(QString);
};

#endif // ACTUALLABEL_H
