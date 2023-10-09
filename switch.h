#ifndef SWITCH_H
#define SWITCH_H

#include <QtWidgets>
const QColor OKcolor(0xb0d959), notOKcolor(0xff2c26), unappliedColor(255, 165, 0);

class Switch : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)
    Q_PROPERTY(QColor bgColor MEMBER bgColor)
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
    Q_PROPERTY(bool SwitchOnClick READ getSwitchOnClick WRITE setSwitchOnClick)
    bool _switch, _switchOnClick = false, isHorizontal;
    qreal _opacity;
    int _x, _y, _margin;
    QBrush _thumb, _brush;
    QPropertyAnimation *_anim = nullptr;
    QColor bgColor = Qt::transparent;

public:
    Switch(QWidget* parent = nullptr, const QBrush &brush = OKcolor):
        QAbstractButton(parent),
        _switch(false),
        _switchOnClick(false),
        isHorizontal(false),
        _opacity(0.000),
        _margin(3),
        _thumb(notOKcolor),
        _anim(new QPropertyAnimation(this, "offset", this))
    {
        setMinimumHeight(isHorizontal ? 6 : 10);
        setMinimumWidth(isHorizontal ? 10 : 6);
        setCheckable(true);
        setBrush(brush);

        connect(this, &Switch::toggled, this, [=](){
            _switch = isChecked();
            _thumb = _switch ? _brush : notOKcolor;
            setOffset(_margin + (_switch == isHorizontal ? abs(width() - height()) : 0));
        });
    }

    QSize sizeHint() const override { return QSize(height() + 2 * _margin, 2 * (height() + _margin)); }

    QBrush brush() const {return _brush;}

    void setBrush (const QBrush &brsh) {_brush = brsh;}
    void setBgColor(const QColor bgCol) { bgColor = bgCol; }

    int offset() const {return isHorizontal ?_x :_y;}

    void setOffset(int o) {
        (isHorizontal ? _x : _y) = o;
        update();
    }

    bool is_switched() const {return _switch;}

    bool getSwitchOnClick() const {return _switchOnClick;}

    void setSwitchOnClick(const bool &flag) {_switchOnClick = flag;}

    void swipe() {
        _switch = !_switch;
        _thumb = _switch ? _brush : notOKcolor;
        if (_switch) {
            _anim->setStartValue(offset());
            setOffset(isHorizontal ? width() - height() + _margin :  _margin);
            _anim->setEndValue(isHorizontal ? width() - height() + _margin :  _margin);
        } else {
            _anim->setStartValue(isHorizontal ? width() - height() + _margin :  _margin);
            setOffset(isHorizontal ? _margin : height() - width() + _margin);
            _anim->setEndValue(offset());
        }
        _anim->setDuration(200);
        _anim->start();
        this->setChecked(_switch);
        emit switch_changed();
    }

protected:
    void paintEvent(QPaintEvent *e) override {
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(isEnabled() ? (_switch ? brush() : notOKcolor) : Qt::gray);
        p.setOpacity(isEnabled() ? (_switch ? 0.5 : 0.38) : 0.12);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawRoundedRect(QRect(_margin, _margin,
                                width()  - 2 * _margin,
                                height() - 2 * _margin),
                                ((isHorizontal ? height() : width()) - 2 * _margin)/2,
                                ((isHorizontal ? height() : width()) - 2 * _margin)/2);
        p.setOpacity(1.0);
        if (isEnabled()) p.setBrush(_thumb);
        p.drawEllipse(QRectF( isHorizontal ? offset() : _x ,
                              isHorizontal ? _y : offset(),
                             (isHorizontal ? height() : width()) - 2 * _margin,
                             (isHorizontal ? height() : width()) - 2 * _margin));
        e->accept();
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        if (e->button() & Qt::LeftButton) {
            emit clicked(isChecked());
            if (_switchOnClick) swipe();
        }
        QAbstractButton::mouseReleaseEvent(e);
    }

    void resizeEvent(QResizeEvent* e) override {
        isHorizontal = width() > height();
        if(!isHorizontal) {
            _x = _margin;
            setOffset(_switch ? _margin : height() - width() + _margin );
        } else {
            _y = _margin;
            setOffset(_switch ?  width() - height() + _margin : _margin);
        }
        update();
        e->accept();
    };

    void enterEvent(QEvent *e) override {
        setCursor(Qt::PointingHandCursor);
        QAbstractButton::enterEvent(e);
    }

signals:
    void switch_changed();
};

#endif // SWITCH_H
